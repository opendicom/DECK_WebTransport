// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

#include <stdint.h>

extern char *DICMbuf;
extern u64 DICMidx;
extern s16 siidx;
extern uint8_t *kbuf;

#pragma mark - read

static u64 bytesreceived;
bool ifread(u32 bytesaskedfor)
{
   bytesreceived=fread(DICMbuf+DICMidx,1,bytesaskedfor,stdin);
   if (bytesreceived>0xFFFFFFFF)return 0;
   DICMidx+=bytesreceived;
   return (bytesaskedfor==bytesreceived);
}

//returns true when 8(+4) bytes were read
bool ifreadattr(u8 kloc)
{
   if (fread(DICMbuf+DICMidx,1,8,stdin)!=8)
   {
      if (ferror(stdin)) D("%s","stdin error");
      return false;
   }
   
   //group LE>BE
   kbuf[kloc]=DICMbuf[DICMidx+1];
   kbuf[kloc+1]=DICMbuf[DICMidx];
   //element LE>BE
   kbuf[kloc+2]=DICMbuf[DICMidx+3];
   kbuf[kloc+3]=DICMbuf[DICMidx+2];
   //vr vl copied (LE)
   kbuf[kloc+4]=DICMbuf[DICMidx+4];
   kbuf[kloc+5]=DICMbuf[DICMidx+5];
   kbuf[kloc+6]=DICMbuf[DICMidx+6];
   kbuf[kloc+7]=DICMbuf[DICMidx+7];

   switch ((DICMbuf[DICMidx+5]<<8)|(DICMbuf[DICMidx+4])) {
      case OB://other byte
      case OW://other word
      case OD://other double
      case OF://other float
      case OL://other long
      case SV://signed 64-bit very long
      case OV://other 64-bit very long
      case UV://unsigned 64-bit very long
      case UC://unlimited characters
      case UT://unlimited text
      case UR://universal resrcurl identifier/locator
      case SQ://sequence
      {
         DICMidx+=8;
         if (fread(DICMbuf+DICMidx,1,4,stdin)!=4)
         {
            if (ferror(stdin)) E("%s","stdin error");
            return false;
         }
         memcpy(kbuf+kloc+8, DICMbuf+DICMidx, 4);
         DICMidx+=4;
      }break;
      default:
      {
         //IA,IZ,SZ require postprocessing in dicm2dckv
         DICMidx+=8;
         memcpy(kbuf+kloc+8, DICMbuf+DICMidx-2, 2);
         kbuf[kloc+10]=0;
         kbuf[kloc+11]=0;
      }break;
   }
   
   return true;
}

#pragma mark - instance transactions
static char *dbpath;
static FILE *outFile;

bool uCreate(
   u64 soloc,         // offset in valbyes for sop class
   u16 solen,         // length in valbyes for sop class
   u16 soidx,         // index in const char *scstr[]
   u64 siloc,         // offset in valbyes for sop instance uid
   u16 silen,         // length in valbyes for sop instance uid
   u64 stloc,         // offset in valbyes for transfer syntax
   u16 stlen,         // length in valbyes for transfer syntax
   u16 stidx          // index in const char *csstr[]
){
   dbpath=malloc(0xFF);
   char *ibuf = malloc(silen);
   memcpy(ibuf, DICMbuf+siloc, silen);
   strcat(dbpath,ibuf);
   strcat(dbpath, ".dscd.xml");
   outFile=fopen(dbpath, "w");
   return outFile==NULL?false:true;
}

bool uClose(void){
   fclose(outFile);
   return true;
}

static u32 titlerepidx=0;
static u32 titleoffset=0;
static u32 titlelength=0;
static u32 documentoffset=0;
static u32 documentlength=0;
bool uCommit(bool hastrailing){
   //title charset -> utf-8
   u32 utf8length=0;
   utf8(titlerepidx,DICMbuf,titleoffset,titlelength,DICMbuf,(u32)DICMidx,&utf8length);
   printf( "%.*s\n", utf8length,DICMbuf+DICMidx );
   
   //write document
   if (!fwrite(DICMbuf+documentoffset ,1, documentlength , outFile)) return false;

   return uClose();
}

#pragma mark - write

bool uAppend(int kloc, enum kvVRcategory  vrcat, u32 vlen)
{
   switch (vrcat) {
      case kvSA:
      case kvSZ:
      case kvIA:
      case kvIZ: break;
      case kvsdoctitle: { //ST  DocumentTitle 00420010
         titlerepidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
         titleoffset=(u32)DICMidx;
         titlelength=vlen;
         if ((vlen > 0) && (!ifread(vlen))) return false;
      } break;
      case kvsdocument: { //OB encapsulaed document
         if (!ifread(vlen)) return false;
         documentoffset=(u32)(DICMidx-vlen);
         documentlength=vlen - (DICMbuf[DICMidx-1]==0);//last char 0x00 ?
      } break;
      default:if (!ifread(vlen)) return false;break;
   }
   return true;
}
