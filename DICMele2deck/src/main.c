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
char *CKEY;//contextual keys [0]=key chain size
u32   CKEYidx=1;

// for pixel processing
u8 isImage=false;
//needed as input to grok
u16  spp;//sample per plane = components
u16  photocode;//photometric interpretation
u16  rows;
u16  cols;
u16  alloc;
u16  bits;
u16  high;
u16  sign;//pixrep 0028013 0=unsigned 1=signed
u16  comp;//planar 0 = RGB del pixel; 1 = componentes RGB (samples)
int  fram;//number of frames


//for SOP identification
char eDA[4];
u32 eDAlength;
char eUI[48];
u32 eUIlength;
char sUI[48];
u32 sUIlength;
char cUI[48];
u32 cUIlength;
char iUI[48];
u32 iUIlength;
char pUI[48];//pyramid
u32 pUIlength;


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
         case US: {
            switch (attr->e) {
               case 0x280011: cols=DICM[DICMidx]+(DICM[DICMidx+1]<<8); break;
               case 0x280010: rows=DICM[DICMidx]+(DICM[DICMidx+1]<<8); break;
               case 0x280002: spp= DICM[DICMidx]+(DICM[DICMidx+1]<<8); break;
               case 0x280101: bits=DICM[DICMidx]+(DICM[DICMidx+1]<<8); break;//15=16, 13=14, 11=12
               case 0x280103: sign=DICM[DICMidx]+(DICM[DICMidx+1]<<8); break;//?"s":"u"
               case 0x280106: comp=DICM[DICMidx]+(DICM[DICMidx+1]<<8); break;
            }
            attr->c=REPERTOIRE_GL; val(kvUS,attr);key(attr);
         } break;
         case AT: { attr->c=REPERTOIRE_GL; val(kvAT,attr);key(attr);} break;
         //ascii
         case UI: {
            attr->c=REPERTOIRE_GL;
            switch (attr->e) {
               case 0x00080018: {
                  iUIlength=ui2b64( DICM+DICMidx, attr->l, iUI );
                  val(kvUi,attr);key(attr);
               } break; //instance uid
               case 0x00080019: {
                  pUIlength=ui2b64( DICM+DICMidx, attr->l, pUI );
                  val(kvUs,attr);key(attr);
               } break; //PyramidUID
               case 0x0020000D: {
                  eUIlength=ui2b64( DICM+DICMidx, attr->l, eUI );
                  val(kvUe,attr);key(attr);
               } break; //study uid
               case 0x0020000E: {
                  sUIlength=ui2b64( DICM+DICMidx, attr->l, sUI );
                  val(kvUs,attr);key(attr);
               } break; //series uid
               default:         {val(kvUI,attr);key(attr);} break;
            }
         } break;
         case DA: {
            attr->c=REPERTOIRE_GL;
            if (attr->l && (attr->e==0x00080020)) {
               eDAlength=ui2b64( DICM+DICMidx+2, 6, eDA );//with no milenium nor century
            }
            val(kvTP,attr);
            key(attr);
         } break;
         case AS:
         case DT:
         case TM: { attr->c=REPERTOIRE_GL; val(kvTP,attr);key(attr);} break;
         //code
         case CS: {
            attr->c=REPERTOIRE_GL;
            switch (attr->e) {
               case 0x00080005:{
                  val(kvCs,attr);
                  u16 repidxs=repertoireidx(DICM+DICMidx-attr->l,attr->l);
                  if (repidxs==0x09)
                  {
                     fprintf(stderr,"main CS [%lu] bad repertoire (%d)\n", DICMidx, exitBadRepertoire);
                     exit(exitBadRepertoire);
                  }
                  else keycs=(keycs & 0x8000) | repidxs;
                  key(attr);
               }; break;
                  /*
               case 0x00080008:{//CS image type itype
               }; break;
               case 0x00280004:{//photocode (photometric interpretation)
               }; break;
               */
               default: { val(kvCS,attr); key(attr);} break;
            }
         } break;
         case AE:
         case DS: { attr->c=REPERTOIRE_GL; val(kvTA,attr);key(attr);} break;
         case IS: {
            if (attr->e ==0x00280008) fram=atoi(DICM+DICMidx);//IS numberOfFrames
            attr->c=REPERTOIRE_GL;
            val(kvTA,attr);
            key(attr);
         } break;
         //repertoire
         case LO:
         case LT:
         case SH:
         case ST: { attr->c=keycs;         val(kvTS,attr);key(attr);} break;
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
            //SQ length = 0x00000000
            if (attr->l==0) {
               val(kvSa, attr);
               val(kvSZ, attr);
               key(attr);
               continue;
            }
            u64 beforebyteSQ;
            if (attr->l==0xFFFFFFFF) {
               val(kvSA, attr);
               beforebyteSQ=beforebyte;//SQ undefined, byte limit will be of the dataset
            }
            else {
               val(kvSa, attr);
               beforebyteSQ=DICMidx + attr->l;
               if (beforebyteSQ > beforebyte) {
                  fprintf(stderr,"main SQ [%lu] truncated (%d)\n", DICMidx, exitErrorSQtruncated);
                  exit(exitErrorSQtruncated);
               }
            }
            u32 *itemnumber=(u32 *)(CKEY+CKEYidx+4);//pointer to last four bytes of SQ



#pragma mark itemattr
            //SQ is part of the context and should not be overwritten
            CKEY[0]+=8;
            CKEYidx+=8;

            struct Ercle * itemattr=(struct Ercle*) (CKEY+CKEYidx);
            //read first IT tag (or SQ end tag)
            key(itemattr);
            if (itemattr->e==0xfffee0dd) {
               //SQ end tag
               CKEY[0]-=8;
               CKEYidx-=8;
               *itemnumber=0xFFFFFFFF;
               val(kvSZ, attr);
               key(attr);//read attr post SQ
               continue;
            }

            u64 beforebyteIT;
            *itemnumber=u32swap(1);
            //for each item
            while ((DICMidx < beforebyteSQ) && (itemattr->e==0xfffee000)) {

               //define beforebyeIT
               if ((itemattr->r==0xffff)&&(itemattr->c==0xffff)) {
                  beforebyteIT=beforebyteSQ;
                  val(kvIA, attr);
               }
               else {
                  beforebyteIT=DICMidx + itemattr->r + (itemattr->c << 16);
                  if (beforebyteIT > beforebyteSQ) {
//                     E(exitErrorITtruncated,"IT truncated %lu",DICMidx);
                     return exitErrorITtruncated;
                  }
                  val(kvIa, attr);
               }

               key(itemattr);
               dicmDataset(itemattr,keycs,(u32)beforebyteIT,0xfffee00d);

               //write IZ
               if (itemattr->e==0xfffee00d)
               {  //end item tag present
                  itemattr->E=0xFFFFFFFF;
                  itemattr->r=IZ;
                  itemattr->l=0;
                  val(kvIZ, attr);
                  key(itemattr);
               }
               else val(kvIz, attr);

               *itemnumber=u32swap(u32swap(*itemnumber)+1);
            }//end while item


#pragma mark back to SQ level
            CKEY[0]-=8;
            CKEYidx-=8;
            if (itemattr->e==0xfffee0dd)
            {  //end sq tag present
               //read new attr
               val(kvSZ, itemattr);
               key(attr);
            }
            else {
            val(kvSz, itemattr);
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
   return exitZeroError;
}

int main(int argc,  char *argv[]) {
   /*  args
   0 command name defined by target
   1 #p/#f dir path / dicm file name
   2 ...
   */
   clock_gettime(CLOCK_MONOTONIC, &starttime);
   if (argc<2) exit(exitNoArg1FilePath);
   struct stat st;
   stat(argv[1], &st);
   DICMsize= st.st_size;
   if (DICMsize < 140) exit(exitNoDataset);

   uinput(argc, argv);
   isImage=isItImage(*(u64*)(DICM+190),32,16);//zero means no image

   //DICM explicit little endian?
   u16 *cVL=(u16*)(DICM+0xA4);//class value offset 0xA6
   u16 *iVL=(u16*)(DICM+0xA4+*cVL+0x8);//value offset 0xAE+*cVL
   u16 *sVL=(u16*)(DICM+0xA4+*cVL+0x8+*iVL+0x8);//value offset 0xB6+*cVL+*iVL
   if (strncmp(DICM+0xB6+*cVL+*iVL,"1.2.840.10008.1.2.1",*sVL)!=0) exit(exitNotExplicitLittleEndian);

   CKEY=malloc(97);
   CKEY[0]=(u8)8;
   struct Ercle * baseattr=(struct Ercle*)(CKEY+1);

   DICMidx=0x9E;//0x9E 0002,0002
   key(baseattr);
   if (baseattr->e!=0x00020002) exit(exitNotDICM);
   clock_gettime(CLOCK_MONOTONIC, &inputtime);
   dicmDataset(baseattr,0,DICMsize,0xfffcfffc);
   clock_gettime(CLOCK_MONOTONIC, &parsetime);
   utrail(argc, argv);
   clock_gettime(CLOCK_MONOTONIC, &finishtime);

   fprintf(stderr,"total 0.%09ld\n", finishtime.tv_nsec - starttime.tv_nsec);
   fprintf(stderr,"input 0.%09ld\n", inputtime.tv_nsec - starttime.tv_nsec);
   fprintf(stderr,"parse 0.%09ld\n", parsetime.tv_nsec - inputtime.tv_nsec);
   fprintf(stderr,"trail 0.%09ld\n", finishtime.tv_nsec - parsetime.tv_nsec);

 exit(exitZeroError);
}
