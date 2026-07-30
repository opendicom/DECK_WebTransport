// project: cdicm2deck
// target: ...
// file: main.c
// created by jacquesfauquex on 20260715.
#include "uapi.h"


#include <stdint.h>
#include <sys/stat.h>
#include <time.h>

//defined global
struct timespec start,append,commit,finish;
FILE *inFile;
char *DICMbuf=NULL;// ....accumulator of stream registering original binary DICM. Referred by external declarations everywhere
u64 DICMidx;//CDICM pointer
char *kbuf=NULL;//keys buffer
/*
 *size defined in uCreate
 *(max use 16 bytes x 10 encapsulation levels) min size 0xFF
 *is also used as kv buffer. 0x3000 bytes can buffer LT 10280 bytes before converting it to utf-8
 **/
int exitValue=exitZeroError;

//recursive
int dicmDataset(
   u32 kloc,         // current offset
   struct trcl *attr,// read attr up to before value
   u16 keycs,        // key charset
   u64 beforebyte,   // read up to byte
   u32 beforetag     // read up to attr. On return, attr is read and found in kbuf
)
{
   while (
       (DICMidx < beforebyte)
    && (u32swap(attr->t) < beforetag)
   )
   {
      switch (attr->r) {
         case FD: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvFD,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case FL: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvFL,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case SL: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvSL,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case SS: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvSS,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UL: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvUL,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case US: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvUS,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case AT: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvAT,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UI: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvUI,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case AS:
         case DT: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvTP,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case DA: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvTP,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case TM: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvTP,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case CS: {
            attr->c=REPERTOIRE_GL;
            if (!kvRead(attr->l,kloc+12)) return false;
            if (attr->t == 0x05000800){
               u16 repidxs=repertoireidx(kbuf+kloc+12,attr->l);
               if (repidxs==0x09)
               {
                  E("bad repertoire %.*s",attr->l,kbuf+kloc+12);
                  return exitBadRepertoire;
               }
               else
               {
                  keycs=(keycs & 0x8000) | repidxs;
                  attr->c=repidxs;
               }
            }
            if (!vrAppend(kloc,kvCS,attr->l)) return false;
            if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
         } break;
         case AE:
         case DS: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvTA,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case IS: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvTA,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case LO:
         case LT:
         case SH:
         case ST: { attr->c=keycs;         if (!vrAppend(kloc,kvTS,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case PN: { attr->c=keycs;         if (!vrAppend(kloc,kvPN,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case OF:
         case OD:
         case OB:
         case OW:
         case OL:
         case OV:
         case SV:
         case UV: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kv01,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UC: { attr->c=keycs;         if (!vrAppend(kloc,kvTL,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UT: { attr->c=keycs;         if (!vrAppend(kloc,kvTL,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UR: { attr->c=ISO_IR192;     if (!vrAppend(kloc,kvTU,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;//RFC3986
#pragma mark SQ
         case SQ://sequence
         {
            u64 beforebyteSQ;
            if      (attr->l==0xFFFFFFFF) beforebyteSQ=beforebyte;//SQ undefined, byte limit will be of the dataset
            else if (beforebyte==0xFFFFFFFF) beforebyteSQ= DICMidx + attr->l;//dataset undefined, byte limit will be of the SQ
            else if (DICMidx + attr->l > beforebyte) {
               E("%s","SQ incomplete input");
               return false;
            }
            else beforebyteSQ=DICMidx + attr->l;


            if (!vrAppend(kloc,kvSA, attr->l)) return false;


#pragma mark SQ empty
            if (attr->l==0)
            {
               if (!vrAppend(kloc,kvSZ, attr->l)) return false;
            }
            else
            {
#pragma mark SQ item number
               u32 *itemnumber=(u32 *)(kbuf+kloc+4);
               *itemnumber=u32swap(1);

#pragma mark item level
               kloc+=8;
               //for each first attr fffee000 of any new item
               if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
               struct trcl * itemattr=(struct trcl*) (kbuf+kloc);
               u32 *delimiter=(u32*) (kbuf+kloc);//tag
               while ((DICMidx < beforebyteSQ) && (*delimiter==0x00E0FEFF)) //itemstart compulsory
               {
                  //for each item
                  delimiter++;//length
                  u64 beforebyteIT;//to be computed from after item start
                  if (*delimiter==0xFFFFFFFF) beforebyteIT=beforebyteSQ;
                  else if (beforebyteSQ==0xFFFFFFFF) beforebyteIT=DICMidx + *delimiter;
                  else if (DICMidx + *delimiter > beforebyteSQ) {
                     E("%s","IT incomplete input");
                     return false;
                  }
                  else beforebyteIT=DICMidx + *delimiter;

                  if (!vrAppend(kloc,kvIA, 0)) return false;
                  if (!kkRead(kloc))return false;
                  if (exitValue=dicmDataset(kloc,itemattr,keycs,(u32)beforebyteIT,0xfffee00d)) return exitValue;
                  //write IZ
                  if (u32swap(   itemattr->t)==0xfffee00d)
                  {
                        itemattr->t=0xFFFFFFFF;
                        itemattr->r=IZ;
                        itemattr->l=0;
                     if (!vrAppend(kloc,kvIZ, itemattr->l)) return false;
                     if (!kkRead(kloc))  {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;};
                  }
                  else
                  {
                     if (itemattr->r==0xFFFF) return false;//could not read next itemattr with item dataset
                     if (!vrAppend(kloc,kvIZ, itemattr->l)) return false;
                  }
                  *itemnumber=u32swap(u32swap(*itemnumber)+1);
               }//end while item
               kloc-=8;
#pragma mark item level end
               if (!vrAppend(kloc,kvSZ, 0)) return false;

               //   itemattr may be SZ or post SQ
               if (u32swap(itemattr->t)==0xfffee0dd)
               {
                  if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
               }
               else
               {
                  //should not read new attr after end of switch
                  //should transfer itemattr to attr instead
                  attr->t=   itemattr->t;
                  attr->r=   itemattr->r;
                  attr->l=   itemattr->l;
               }
            }
         } break;


            //unknown
            // https://dicom.nema.org/medical/dicom/current/output/html/part05.html#sect_6.2.2
            //5. The Value Length Field of VR UN may contain Undefined Length (FFFFFFFFH), in which case the contents can be assumed to be encoded with Implicit VR. See Section 7.5.1 to determine how to parse Data Elements with an Undefined Length.
         case UN: { attr->l=REPERTOIRE_GL; if (!vrAppend(kloc,kvUN, attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;

         case 0xFFFF:return true;//end of buffer

         default:
         {
            if (attr->t==0 && attr->r==0 && attr->l==0) return true;
            E("error unknown vr at index %lu %08x %c%c %d",DICMidx, attr->t,attr->r % 0x100,attr->r / 0x100,attr->l);
            return false;
         }
      //---------
      }//end switch
   }//end while (*index < beforebyte)
   if (attr->t == 0xFCFFFCFF)
   {
      if (attr->l==0) DICMidx-=12;
      else
      {
         //trailing padding
         attr->c=REPERTOIRE_GL;
         if (!vvRead(attr->l)) {
            E("%s","trailling padding");
            return false;
         }
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

//environment CDICM2DECKloglevel
   const char* loglevel = getenv("DICM2DECKloglevel");
   if (loglevel==NULL) loglevel="D";
   //[ D | I | W | E | F ] ( Debug, Info, Warning, Error, Fault )

//environment CDICM2DECKbeforebyte
   u32 beforebyte=0xFFFFFFFF;
   //const char* abeforebyte = getenv("DICM2DECKbeforebyte");
   //if (abeforebyte!=NULL) beforebyte=(u32)strtoll(abeforebyte, NULL, 16);
   
//environment CDICM2DECKbeforetag (using trailling padding attr as marker)
   // agradado en dcmtk-storescp storescp al final de cada instancia, para delimitarla dentro del stream
   u32 beforetag=0xFFFCFFFC;
   //const char* abeforetag = getenv("DICM2DECKbeforetag");
   //if (abeforetag!=NULL) beforetag=(u32)strtoll(abeforetag, NULL, 16);

//CDICM size
   struct stat st;
   stat(argv[1], &st);
   u64 size = st.st_size;
   D("cdicm file: %s (%lu bytes)", argv[1], size);
   if (size < 140) exit(exitNoDataset);

//uPrerequisite
   if ((exitValue=uPrerequisite(size, argc, argv))!=exitZeroError) exit(exitValue);

   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL) exit (exitErrorIn);

//uCreate
   if ((exitValue=uCreate(argc, argv))!=exitZeroError) exit(exitValue);

//read first tag after group 2 length
//   if (fseek(inFile, DICMidx, SEEK_SET)!=0) exit(exitNotDICM);//0x9E 0002,0002
   struct trcl * baseattr=(struct trcl*) kbuf;
   if (kkRead(0) && (baseattr->t==0x02000200))
   {
//uAppend (repeated call within dicmDataset) and uCommit
      clock_gettime(CLOCK_MONOTONIC, &append);
      if ((exitValue=dicmDataset(0,baseattr,0,beforebyte,beforetag))==exitZeroError) {
         //successfull parsing (exitValue==0, everything OK)
         clock_gettime(CLOCK_MONOTONIC, &commit);
         exitValue=uCommit(baseattr,argc,argv);
      }
//uClose
      uClose(argc, argv);
   }
   else exitValue=exitNotDICM;
   fclose(inFile);
   clock_gettime(CLOCK_MONOTONIC, &finish);
   D(" total= nanoseconds: %09lld", finish.tv_nsec - start.tv_nsec);
   D("%s","------------------------------");
   D(" before nanoseconds: %09lld", append.tv_nsec - start.tv_nsec);
   D(" parse  nanoseconds: %09lld", commit.tv_nsec - append.tv_nsec);
   D(" commit nanoseconds: %09lld", finish.tv_nsec - commit.tv_nsec);
 exit(exitValue);
}
