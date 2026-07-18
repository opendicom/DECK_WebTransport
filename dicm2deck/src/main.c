// project: dicm2deck
// target: ...
// file: main.c
// created by jacquesfauquex on 20260715.

#include <stdint.h>

#import "uapi.h"

//defined global
//TODO Maybe not needed !!!!
char *DICMbuf=NULL;// ....accumulator of stream registering original binary DICM. Referred by external declarations everywhere

u64 DICMidx=0;//associated current pointer
uint8_t *kbuf=NULL;//buffer (size 0xFF) for the creation of _DKV and EDKV contextual keys. max use 16 bytes x 10 encapsulation levels
int exitValue=deckZeroError;

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
         case FD: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvFD,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case FL: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvFL,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case SL: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvSL,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case SS: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvSS,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UL: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvUL,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case US: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvUS,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case AT: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvAT,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UI: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvUI,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case AS:
         case DT: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvTP,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case DA: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvTP,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case TM: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvTP,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case CS: {
            attr->c=REPERTOIRE_GL;
            if (!uAppend(kloc,kvTA,attr->l)) return false;
            if (attr->t == 0x05000800){
               u16 repidxs=repertoireidx(DICMbuf+DICMidx-attr->l,attr->l);
               if (repidxs==0x09)
               {
                  E("bad repertoire %.*s",attr->l,DICMbuf+DICMidx-attr->l);
                  return deckBadRepertoire;
               }
               else
               {
                  keycs=(keycs & 0x8000) | repidxs;
                  attr->c=repidxs;
               }
            }
            if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
         } break;
         case AE:
         case DS: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvTA,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case IS: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kvTA,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case LO:
         case LT:
         case SH:
         case ST: { attr->c=keycs;         if (!uAppend(kloc,kvTS,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case PN: { attr->c=keycs;         if (!uAppend(kloc,kvPN,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case OF:
         case OD:
         case OB:
         case OW:
         case OL:
         case OV:
         case SV:
         case UV: { attr->c=REPERTOIRE_GL; if (!uAppend(kloc,kv01,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UC: { attr->c=keycs;         if (!uAppend(kloc,kvTL,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UT: { attr->c=keycs;         if (!uAppend(kloc,kvTL,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
         case UR: { attr->c=ISO_IR192;     if (!uAppend(kloc,kvTU,attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;//RFC3986
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


            if (!uAppend(kloc,kvSA, attr->l)) return false;


#pragma mark SQ empty
            if (attr->l==0)
            {
               if (!uAppend(kloc,kvSZ, attr->l)) return false;
            }
            else
            {
#pragma mark SQ item number
               u32 *itemnumber=(u32 *)(kbuf+kloc+4);
               *itemnumber=u32swap(1);

#pragma mark item level
               kloc+=8;
               //for each first attr fffee000 of any new item
               if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
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

                  if (!uAppend(kloc,kvIA, 0)) return false;
                  if (!ifreadattr(kloc))return false;
                  if (exitValue=dicmDataset(kloc,itemattr,keycs,(u32)beforebyteIT,0xfffee00d)) return exitValue;
                  //write IZ
                  if (u32swap(   itemattr->t)==0xfffee00d)
                  {
                        itemattr->t=0xFFFFFFFF;
                        itemattr->r=IZ;
                        itemattr->l=0;
                     if (!uAppend(kloc,kvIZ, itemattr->l)) return false;
                     if (!ifreadattr(kloc))  {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;};
                  }
                  else
                  {
                     if (itemattr->r==0xFFFF) return false;//could not read next itemattr with item dataset
                     if (!uAppend(kloc,kvIZ, itemattr->l)) return false;
                  }
                  *itemnumber=u32swap(u32swap(*itemnumber)+1);
               }//end while item
               kloc-=8;
#pragma mark item level end
               if (!uAppend(kloc,kvSZ, 0)) return false;

               //   itemattr may be SZ or post SQ
               if (u32swap(itemattr->t)==0xfffee0dd)
               {
                  if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
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
         case UN: { attr->l=REPERTOIRE_GL; if (!uAppend(kloc,kvUN, attr->l)) return false; if (! ifreadattr(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;

         case 0xFFFF:return true;//end of buffer

         default:
         {
            if (attr->t==0 && attr->r==0 && attr->l==0) return true;
            E("error unknown vr at index %llu %08x %c%c %d",DICMidx, attr->t,attr->r % 0x100,attr->r / 0x100,attr->l);
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
         if (!ifread(attr->l)) {
            E("%s","trailling padding");
            return false;
         }
      }
   }
   return deckZeroError;
}

int main(int argc, const char * argv[]) {
   /*  (args defined in the calling script or directly in the xcr option of dcmtk storescp
    *  The first four are required in the same order
    *  all the args are passed to uCreate
   0 command name defined by target
   1 #p/#f dir path / dicm file name
   2 out dir
   3 err dir
   4 ...
   */
   
//environment variables
   
#pragma mark DICM2DECKloglevel
   const char* loglevel = getenv("DICM2DECKloglevel");
   if (loglevel==NULL) loglevel="D";
   //[ D | I | W | E | F ] ( Debug, Info, Warning, Error, Fault )
   
#pragma mark DICM2DECKbeforebyte
   u32 beforebyte=0xFFFFFFFF;
   //const char* abeforebyte = getenv("DICM2DECKbeforebyte");
   //if (abeforebyte!=NULL) beforebyte=(u32)strtoll(abeforebyte, NULL, 16);
   
#pragma mark DICM2DECKbeforetag
   // agradado en dcmtk-storescp storescp al final de cada instancia, para delimitarla dentro del stream
   u32 beforetag=0xFFFCFFFC;
   //const char* abeforetag = getenv("DICM2DECKbeforetag");
   //if (abeforetag!=NULL) beforetag=(u32)strtoll(abeforetag, NULL, 16);

   /*
#pragma mark DICM2DECKoutdir
   const char* outdir = getenv("DICM2DECKoutdir");
   if (outdir!=NULL) chdir(outdir);
   else chdir("/tmp");
   */
   char cwd[1024];
   getcwd(cwd, sizeof(cwd));
   D("outdir:  %s", cwd);


#pragma mark - read file and process
   FILE *inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL) exitValue=deckErrorIn;
   else
   { //file opened
      uCreate(argc, argv);
      //TODO eliminate DICMbuf ?
      DICMbuf=malloc(300*1024*1024);
      kbuf = malloc(0xFF);
      struct trcl * baseattr=(struct trcl*) kbuf;

      //read up to dicom version 0002001 (8+150 bytes)
      //D("%s","waiting 158 bytes on stdin");
      if (  ifread(132)
         && (DICMbuf[128]==0x44)
         && (DICMbuf[129]==0x49)
         && (DICMbuf[130]==0x43)
         && (DICMbuf[131]==0x4D)
         && ifreadattr(0)
      )
      {
         if (!(exitValue=dicmDataset(0,baseattr,0,beforebyte,beforetag))) exitValue=uCommit(baseattr,argc,argv); //successfull parsing (exitValue==0, everything OK)
         uClose(argc, argv);
      }
      else exitValue=deckNotDICM;
      fclose(inFile);
   }
   return exitValue;
}
