// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

#include <stdint.h>
#include <string.h>

extern char *DICMbuf;
extern u64 DICMidx;
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
      case UR://universal ressource rcurl identifier/locator
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


int uPrerequisite(u64 filesize, int argc, char *argv[]) {
   if ((argc >4) && (strcmp(argv[4],"1.2.840.10008.5.1.4.1.1.104.2")!=0)) return exitNotEncapsulatedCDA;
   return exitZeroError;
}

int uCreate(FILE *inFILE, int argc, char *argv[])
{
   return exitZeroError;
}

void uClose(int argc, char *argv[]){
   return;
}

static u32 titlerepidx=0;
static u32 documentoffset=0;
static u32 documentlength=0;



int uCommit(bool hastrailing,int argc, char *argv[]){
    FILE *outFile=fopen("dscd.xml", "w");
   if (outFile==NULL) return exitErrorOutPath;
   if (!fwrite(DICMbuf+documentoffset ,1, documentlength , outFile)) return exitErrorFwrite;
   fclose(outFile);
   I("%s","dscd.xml written");
   return exitZeroError;
}

#pragma mark - write
//const unsigned long B0040E001=0x0E002000;//ST CDA root^extension
//const unsigned long B00080060=0x60000800;//CS Modality
//const unsigned long B0008103E=0x3E100800;//LO Series name
const unsigned long B00420010=0x10004200;//ST DocumentTitle
const unsigned long B00420011=0x11004200;//OB EncapsulatedDocument
//const unsigned long B00420012=0x12004200;//LO MIME of EncapsulatedDocument
bool uAppend(u32 kloc, enum kvVRcategory  vrcat, u32 vlen)
{
   switch (vrcat) {
      case kvSA:
      case kvSZ:
      case kvIA:
      case kvIZ: break;

      case kvTS: {
         if ((vlen > 0) && (!ifread(vlen))) return false;

         //ST DocumentTitle 00420010
         if (!memcmp(kbuf, &B00420010, 4)) {
            titlerepidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
            u32 utf8length=0;
            utf8(titlerepidx,DICMbuf,DICMidx-vlen,vlen,DICMbuf,(u32)DICMidx,&utf8length);
            printf( "%.*s\n", utf8length,DICMbuf+DICMidx );
         }

         //LO MIME Type of Encapsulated Document 00420012 xml cda o pdf
//TODO
      } break;
      case kv01: {
         //OB encapsulaed document 00420011 xml cda o pdf
         if (!ifread(vlen)) return false;
         if (!memcmp(kbuf, &B00420011, 4))
         {
            documentoffset=(u32)(DICMidx-vlen);
            documentlength=vlen - (DICMbuf[DICMidx-1]==0);//last char 0x00 ?
         }
      } break;
      default:if (!ifread(vlen)) return false;break;
   }
   return true;
}
