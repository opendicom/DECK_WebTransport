// project: cdicm2deck
// target: ...
// file: main.c
// created by jacquesfauquex on 20260715.
#include <stdio.h>
#include <stdarg.h>

#include "uapi.h"

struct timespec starttime,inputtime, parsetime,finishtime;
FILE *inFile;

char *DICM;//CDICM in memory
u64   DICMsize;
u64   DICMidx;
char *CKEY;//contextual keys
u32  CKEYidx=0;
//recursive
int dicmDataset(
   struct Ercle *attr,// read attr up to before value
   u16 keycs,        // key charset
   u64 beforebyte,   // read up to byte
   u32 beforetag     // read up to attr. On return, attr is read and found in CKEY
)
{
   while ((DICMidx < beforebyte) && (attr->e < beforetag))
   {
      switch (attr->r) {
         //num
         case FD: { attr->c=REPERTOIRE_GL; val(kvFD,attr);key(attr);} break;
         case FL: { attr->c=REPERTOIRE_GL; val(kvFL,attr);key(attr);} break;
         case SL: { attr->c=REPERTOIRE_GL; val(kvSL,attr);key(attr);} break;
         case SS: { attr->c=REPERTOIRE_GL; val(kvSS,attr);key(attr);} break;
         case UL: { attr->c=REPERTOIRE_GL; val(kvUL,attr);key(attr);} break;
         case US: { attr->c=REPERTOIRE_GL; val(kvUS,attr);key(attr);} break;
         case AT: { attr->c=REPERTOIRE_GL; val(kvAT,attr);key(attr);} break;
         //ascii
         case UI: { attr->c=REPERTOIRE_GL; val(kvUI,attr);key(attr);} break;
         case AS:
         case DT:
         case DA:
         case TM: { attr->c=REPERTOIRE_GL; val(kvTP,attr);key(attr);} break;
         //code
         case CS: {
            attr->c=REPERTOIRE_GL;
            val(kvCS,attr);
            if (attr->e == 0x00080005){
               u16 repidxs=repertoireidx(DICM+DICMidx-attr->l,attr->l);
               if (repidxs==0x09)
               {
                  fprintf(stderr,"main CS [%lu] bad repertoire (%d)\n", DICMidx, exitBadRepertoire);
                  exit(exitBadRepertoire);
               }
               else keycs=(keycs & 0x8000) | repidxs;
            }
            key(attr);
         } break;
         case AE:
         case DS:
         case IS: { attr->c=REPERTOIRE_GL; val(kvTA,attr);key(attr);} break;
         //repertoire
         case LO:
         case LT:
         case SH:
         case ST: { attr->c=keycs;         val(kvTA,attr);key(attr);} break;
         case PN: { attr->c=keycs;         val(kvPN,attr);key(attr);} break;
         //large length numbers
         case OF:
         case OD:
         case OB:
         case OW:
         case OL:
         case OV:
         case SV:
         case UV: { attr->c=REPERTOIRE_GL; val(kv01,attr);key(attr);} break;
         //large length repertoire
         case UC:
         case UT: { attr->c=keycs;         val(kvTL,attr);key(attr);} break;
         case UR: { attr->c=ISO_IR192;     val(kvTU,attr);key(attr);} break;//RFC3986
#pragma mark SQ
         case SQ://sequence
         {
            u64 beforebyteSQ;
            if (attr->l==0xFFFFFFFF) beforebyteSQ=beforebyte;//SQ undefined, byte limit will be of the dataset
            else {
               beforebyteSQ=DICMidx + attr->l;
               if (beforebyteSQ > beforebyte) {
                  fprintf(stderr,"main SQ [%lu] truncated (%d)\n", DICMidx, exitErrorSQtruncated);
                  exit(exitErrorSQtruncated);
               }
            }
            u32 *itemnumber=(u32 *)(CKEY+CKEYidx+4);//pointer to last four bytes of SQ

            //SQ length = 0x00000000
            if (attr->l==0) {
               *itemnumber=0x0;
               val(kvSA, attr);
               *itemnumber=0xffffffff;
               val(kvSZ, attr);
               key(attr);
               continue;
            }
            val(kvSA, attr);

#pragma mark itemattr
            //SQ is part of the context and should not be overwritten
            CKEYidx+=8;
            struct Ercle * itemattr=(struct Ercle*) (CKEY+CKEYidx);
            //read first IT tag (or SQ end tag)
            key(itemattr);
            if (itemattr->e==0xfffee0dd) {
               //SQ end tag
               CKEYidx-=8;
               *itemnumber=0xFFFFFFFF;
               val(kvSZ, 0);
               key(attr);//read attr post SQ
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
//                     E(exitErrorITtruncated,"IT truncated %lu",DICMidx);
                     return exitErrorITtruncated;
                  }
               }

               val(kvIA, 0);
               key(itemattr);
               dicmDataset(itemattr,keycs,(u32)beforebyteIT,0xfffee00d);
               //write IZ
               if (itemattr->e==0xfffee00d)
               {  //end item tag present
                  itemattr->E=0xFFFFFFFF;
                  itemattr->r=IZ;
                  itemattr->l=0;
                  val(kvIZ, 0);
                  key(itemattr);
               }
               else val(kvIZ, 0);
               *itemnumber=u32swap(u32swap(*itemnumber)+1);
            }//end while item


#pragma mark back to SQ level
            CKEYidx-=8;
            val(kvSZ, attr);
            if (itemattr->e==0xfffee0dd)
            {  //end sq tag present
               //read new attr
               key(attr);
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
         case UN: { attr->l=REPERTOIRE_GL; val(kvUN,attr);key(attr);} break;
         default: {exit(-10);} break;
      }//end switch
   }//end while (*index < beforebyte)
   if (attr->E == 0xFCFFFCFF)
   {
      if (attr->l==0) DICMidx-=12;
      else
      {
         //real trailing padding
         attr->c=REPERTOIRE_GL;
         val(kv01,attr);
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
   clock_gettime(CLOCK_MONOTONIC, &starttime);
   if ((argc >2) && (strcmp(argv[2],"1.2.840.10008.1.2.1")!=0)) exit(exitNotExplicitLittleEndian);

   struct stat st;
   stat(argv[1], &st);
   DICMsize= st.st_size;
   if (DICMsize < 140) exit(exitNoDataset);

   input(argc, argv);
   CKEY=malloc(96);
   struct Ercle * baseattr=(struct Ercle*) CKEY;

   DICMidx=0x9E;//0x9E 0002,0002
   key(baseattr);
   if (baseattr->e!=0x00020002) exit(exitNotDICM);
   clock_gettime(CLOCK_MONOTONIC, &inputtime);
   dicmDataset(baseattr,0,DICMsize,0xfffcfffc);
   clock_gettime(CLOCK_MONOTONIC, &parsetime);
   trail(argc, argv);
   clock_gettime(CLOCK_MONOTONIC, &finishtime);

   fprintf(stderr,"total 0.%09ld\n", finishtime.tv_nsec - starttime.tv_nsec);
   fprintf(stderr,"input 0.%09ld\n", inputtime.tv_nsec - starttime.tv_nsec);
   fprintf(stderr,"parse 0.%09ld\n", parsetime.tv_nsec - inputtime.tv_nsec);
   fprintf(stderr,"trail 0.%09ld\n", finishtime.tv_nsec - parsetime.tv_nsec);

 exit(exitZeroError);
}
