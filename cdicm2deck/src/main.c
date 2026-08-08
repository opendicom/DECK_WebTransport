// project: cdicm2deck
// target: ...
// file: main.c
// created by jacquesfauquex on 20260715.
#include <stdio.h>
#include <stdarg.h>

#include "uapi.h"

//defined global
int exitValue=exitZeroError;
u64   DICMidx;//CDICM pointer

void E(int code, const char *format, ...) {
   if (code==0) return;
   fprintf(stderr, "%lu [%d] ",DICMidx, exitValue);
   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
   fputc('\n', stderr);
   if (code!=0) exit(code);
}


struct timespec start,append,commit,finish;
FILE *inFile;

char *CKEY;//contextual keys buffer
char *BUFF;//used for CS preread and other temp one read buffer. malloc in prerequisite or open
/*
 *size defined in uCreate
 *(max use 16 bytes x 10 encapsulation levels) min size 0xFF
 *is also used as kv buffer. 0x3000 bytes can buffer LT 10280 bytes before converting it to utf-8
 **/

//recursive
int dicmDataset(
   u32 kloc,         // current offset
   struct Ercle *attr,// read attr up to before value
   u16 keycs,        // key charset
   u64 beforebyte,   // read up to byte
   u32 beforetag     // read up to attr. On return, attr is read and found in CKEY
)
{
   while (
       (exitValue==exitZeroError)
    && (DICMidx < beforebyte)
    && (attr->e < beforetag) //comparison in little endian
   )
   {
      switch (attr->r) {
         //num
         case FD: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvFD,attr->l);CKEYread(attr);} break;
         case FL: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvFL,attr->l);CKEYread(attr);} break;
         case SL: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvSL,attr->l);CKEYread(attr);} break;
         case SS: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvSS,attr->l);CKEYread(attr);} break;
         case UL: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvUL,attr->l);CKEYread(attr);} break;
         case US: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvUS,attr->l);CKEYread(attr);} break;
         case AT: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvAT,attr->l);CKEYread(attr);} break;
         case UI: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvUI,attr->l);CKEYread(attr);} break;
         //ascii
         case AS:
         case DT:
         case DA:
         case TM: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvTP,attr->l);CKEYread(attr);} break;
         //code
         case CS: {
            attr->c=REPERTOIRE_GL;
            BUFFread(attr->l);
            if (attr->e == 0x00080005){
               u16 repidxs=repertoireidx(BUFF,attr->l);
               if (repidxs==0x09)
               {
                  E(exitBadRepertoire, "bad repertoire %.*s",attr->l,BUFF);
                  exitValue=exitBadRepertoire;
               }
               else
               {
                  keycs=(keycs & 0x8000) | repidxs;
                  attr->c=repidxs;
               }
            }
            DICMidx+=attr->l;
            CKEYread(attr);
         } break;
         case AE:
         case DS:
         case IS: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kvTA,attr->l);CKEYread(attr);} break;
         //repertoire
         case LO:
         case LT:
         case SH:
         case ST: { attr->c=keycs;         vrAppend(kloc,kvTA,attr->l);CKEYread(attr);} break;
         case PN: { attr->c=keycs;         vrAppend(kloc,kvPN,attr->l);CKEYread(attr);} break;
         //large length numbers
         case OF:
         case OD:
         case OB:
         case OW:
         case OL:
         case OV:
         case SV:
         case UV: { attr->c=REPERTOIRE_GL; vrAppend(kloc,kv01,attr->l);CKEYread(attr);} break;
         //large length repertoire
         case UC:
         case UT: { attr->c=keycs;         vrAppend(kloc,kvTL,attr->l);CKEYread(attr);} break;
         case UR: { attr->c=ISO_IR192;     vrAppend(kloc,kvTU,attr->l);CKEYread(attr);} break;//RFC3986
#pragma mark SQ
         case SQ://sequence
         {
            u64 beforebyteSQ;
            if (attr->l==0xFFFFFFFF) beforebyteSQ=beforebyte;//SQ undefined, byte limit will be of the dataset
            else {
               beforebyteSQ=DICMidx + attr->l;
               if (beforebyteSQ > beforebyte) {
                  E(exitErrorSQtruncated,"%s","SQ truncated %lu");
                  exitValue=exitErrorSQtruncated;
                  continue;
               }
            }
            u32 *itemnumber=(u32 *)(CKEY+kloc+4);//pointer to last four bytes of SQ

            //SQ length = 0x00000000
            if (attr->l==0) {
               *itemnumber=0x0;
               vrAppend(kloc,kvSA, attr->l);
               *itemnumber=0xffffffff;
               vrAppend(kloc,kvSZ, 0);
               CKEYread(attr);
               continue;
            }
            vrAppend(kloc,kvSA, 0);

#pragma mark itemattr
            //SQ is part of the context and should not be overwritten
            kloc+=8;
            struct Ercle * itemattr=(struct Ercle*) (CKEY+kloc);
            //read first IT tag (or SQ end tag)
            CKEYread(itemattr);
            if (itemattr->e==0xfffee0dd) {
               //SQ end tag
               kloc-=8;
               *itemnumber=0xFFFFFFFF;
               vrAppend(kloc,kvSZ, 0);
               CKEYread(attr);//read attr post SQ
               continue;
            }

            u64 beforebyteIT;
            *itemnumber=u32swap(1);
            //for each item
            while ((DICMidx < beforebyteSQ) && (itemattr->e==0xfffee000)) {

               //define beforebyeIT
               if ((itemattr->r==0xffff)&&(itemattr->c==0xffff)) beforebyteIT=beforebyteSQ;
               else {
                  beforebyteIT=DICMidx + itemattr->r + (itemattr->c << 16);
                  if (beforebyteIT > beforebyteSQ) {
                     E(exitErrorITtruncated,"IT truncated %lu",DICMidx);
                     return exitErrorITtruncated;
                  }
               }

               vrAppend(kloc,kvIA, 0);
               CKEYread(itemattr);
               dicmDataset(kloc,itemattr,keycs,(u32)beforebyteIT,0xfffee00d);
               //write IZ
               if (itemattr->e==0xfffee00d)
               {  //end item tag present
                  itemattr->E=0xFFFFFFFF;
                  itemattr->r=IZ;
                  itemattr->l=0;
                  vrAppend(kloc,kvIZ, 0);
                  CKEYread(itemattr);
               }
               else vrAppend(kloc,kvIZ, 0);
               *itemnumber=u32swap(u32swap(*itemnumber)+1);
            }//end while item


#pragma mark back to SQ level
            kloc-=8;
            vrAppend(kloc,kvSZ, 0);
            if (itemattr->e==0xfffee0dd)
            {  //end sq tag present
               //read new attr
               CKEYread(attr);
            }
            else {
               //should not read new attr after end of switch
               //should transfer itemattr to attr instead
               attr->E=itemattr->E;
               attr->r=itemattr->r;
               attr->c=itemattr->c;
               attr->l=itemattr->l;
               attr->e=itemattr->e;
            }

         } break;


            //unknown
            // https://dicom.nema.org/medical/dicom/current/output/html/part05.html#sect_6.2.2
            //5. The Value Length Field of VR UN may contain Undefined Length (FFFFFFFFH), in which case the contents can be assumed to be encoded with Implicit VR. See Section 7.5.1 to determine how to parse Data Elements with an Undefined Length.
         case UN: { attr->l=REPERTOIRE_GL; vrAppend(kloc,kvUN,attr->l);CKEYread(attr);} break;

         case 0xFFFF:return true;//end of buffer

         default:
         {
            if (attr->E==0 && attr->r==0 && attr->l==0) return true;
            E(-99,"error unknown vr at index %lu %08x %c%c %d",DICMidx, attr->E,attr->r % 0x100,attr->r / 0x100,attr->l);
            return false;
         }
      //---------
      }//end switch
   }//end while (*index < beforebyte)
   if (attr->E == 0xFCFFFCFF)
   {
      if (attr->l==0) DICMidx-=12;
      else
      {
         //real trailing padding
         attr->c=REPERTOIRE_GL;
         vrAppend(kloc,kv01,attr->l);
      }
   }
   return exitZeroError;
}

int main(int argc,  char *argv[]) {
   /*  (args defined in the calling script or directly in the xcr option of dcmtk storescp
    *  The first four are required in the same order
    *  all the args are passed to uCreate
   0 command name defined by target
   1 #p/#f dir path / dicm file name
   2 ...
   */
   clock_gettime(CLOCK_MONOTONIC, &start);
   if ((argc >2) && (strcmp(argv[2],"1.2.840.10008.1.2.1")!=0)) exit(exitNotExplicitLittleEndian);

//CDICM size
   struct stat st;
   stat(argv[1], &st);
   u64 beforebyte= st.st_size;
   if (beforebyte < 140) exit(exitNoDataset);

//uPrerequisite
   uPrerequisite(argc, argv);

   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL)
   {
      if (ferror(stdin)) {
         E(-98,"%d: %s\n", errno, strerror(errno));
         exit errno;
      }
      exit(exitErrorFropenCDICM);
   }

//uCreate
   uCreate(argc, argv);

//read first tag after group 2 length
//   if (fseek(inFile, DICMidx, SEEK_SET)!=0) exit(exitNotDICM);//0x9E 0002,0002
   CKEY=malloc(96);
   struct Ercle * baseattr=(struct Ercle*) CKEY;
   CKEYread(baseattr);
   if (baseattr->e!=0x00020002) exit(exitNotDICM);

//uAppend (repeated call within dicmDataset) and uCommit
   clock_gettime(CLOCK_MONOTONIC, &append);
   dicmDataset(0,baseattr,0,beforebyte,0xfffcfffc);
   clock_gettime(CLOCK_MONOTONIC, &commit);
   uClose(argc, argv);
   fclose(inFile);
   clock_gettime(CLOCK_MONOTONIC, &finish);
   //fprintf(stderr,"%s\n","=================");
   fprintf(stderr,"total 0.%09lld\n", finish.tv_nsec - start.tv_nsec);
   //fprintf(stderr,"%s\n","-----------------");
   fprintf(stderr,"pre   0.%09lld\n", append.tv_nsec - start.tv_nsec);
   fprintf(stderr,"parse 0.%09lld\n", commit.tv_nsec - append.tv_nsec);
   fprintf(stderr,"post  0.%09lld\n", finish.tv_nsec - commit.tv_nsec);
 exit(exitValue);
}
