// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

#include <stdint.h>
#include <string.h>


extern u64 DICMidx;
extern uint8_t *kbuf;
u64 xmlOffset=0;
u32 xmlLength=0;
const char *space=" ";
static u64 bytesreceived;

#pragma mark ----------------------------- file read
bool vvRead(u32 bytesaskedfor)
{
   DICMidx+=bytesaskedfor;
   return (fseek(stdin, bytesaskedfor, SEEK_CUR)==0);
}

bool kvRead(u32 bytesaskedfor, u32 kloc12)
{
   bytesreceived=fread(kbuf+kloc12,1,bytesaskedfor,stdin);
   DICMidx+=bytesaskedfor;
   return (bytesaskedfor==bytesreceived);
}


//returns true when 8(+4) bytes were read
bool kkRead(u8 kloc)
{
   // read 8 bytes, and, eventually, four additional bytes of long length at kloc+8

   //starts above used space (+12), to copy then byte by byte in current space
   if (fread(kbuf+kloc+12,1,8,stdin)!=8)
   {
      if (ferror(stdin)) E("%s","stdin error");
      return false;
   }
   DICMidx+=8;

   //group LE>BE
   kbuf[kloc]=kbuf[kloc+13];
   kbuf[kloc+1]=kbuf[kloc+12];
   //element LE>BE
   kbuf[kloc+2]=kbuf[kloc+15];
   kbuf[kloc+3]=kbuf[kloc+14];
   //vr vl copied (LE)
   kbuf[kloc+4]=kbuf[kloc+16];
   kbuf[kloc+5]=kbuf[kloc+17];
   //kbuf[kloc+6] for repertoire
   //kbuf[kloc+7] for repertoire
   kbuf[kloc+8]=kbuf[kloc+18];
   kbuf[kloc+9]=kbuf[kloc+19];
   kbuf[kloc+10]=0;
   kbuf[kloc+11]=0;

   switch ((kbuf[kloc+5]<<8)|(kbuf[kloc+4])) {
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
      case UR://universal ressource url identifier/locator
      case SQ://sequence
      {
         if (fread(kbuf+kloc+8,1,4,stdin)!=4)
         {
            if (ferror(stdin)) E("%s","stdin error");
            return false;
         }
         DICMidx+=4;
      }break;
      case IA:
      case IZ:
      case SZ: {
         //IA,IZ,SZ require postprocessing in dicm2dckv
         kbuf[kloc+10]=0;
         kbuf[kloc+11]=0;
      }break;
   }
   //D("%8lu%*s%02X%02X%02X%02X %c%c\n",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5]);
   return true;
}

#pragma mark ----------------------------- SOP instance transactions
int uPrerequisite(u64 filesize, int argc, char *argv[]) {
   if ((argc >4) && (strcmp(argv[4],"1.2.840.10008.5.1.4.1.1.104.2")!=0)) return exitNotEncapsulatedCDA;
   return exitZeroError;
}

int uCreate(int argc, char *argv[])
{
   DICMidx=0x9E;//0x9E 0002,0002
   if (fseek(stdin, DICMidx, SEEK_SET)!=0) return exitNotDICM;//0x9E 0002,0002
   kbuf = malloc(0xFFFF);
   return exitZeroError;
}

void uClose(int argc, char *argv[]){
   return;
}

int uCommit(bool hastrailing,int argc, char *argv[]){
   printf("%.*s",xmlLength,kbuf+32);
   return exitZeroError;
}

#pragma mark ---------------------------- attributes processing
//const unsigned long B0040E001=0x0E002000;//ST CDA root^extension
//const unsigned long B00080060=0x60000800;//CS Modality
//const unsigned long B0008103E=0x3E100800;//LO Series name
//const unsigned long B00420010=0x10004200;//ST DocumentTitle
const unsigned long B00420011=0x11004200;//OB EncapsulatedDocument
//const unsigned long B00420012=0x12004200;//LO MIME of EncapsulatedDocument
bool vrAppend(u32 kloc, enum kvVRcategory  vrcat, u32 vlen)
{
   switch (vrcat) {
      case kvSA:
      case kvSZ:
      case kvIA:
      case kvIZ: break;
/*
      case kvTS: {
         if ((vlen > 0) && (!vvread(vlen))) return false;
         //ST DocumentTitle 00420010

         if (!memcmp(kbuf, &B00420010, 4)) {
            titlerepidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
            u32 utf8length=0;
            utf8(titlerepidx,DICMbuf,DICMidx-vlen,vlen,DICMbuf,(u32)DICMidx,&utf8length);
            printf( "%.*s\n", utf8length,DICMbuf+DICMidx );
         }

      } break;*/
      case kv01: {
         //OB encapsulaed document 00420011 xml cda o pdf
         if (!memcmp(kbuf, &B00420011, 4))
         {
            if (!kvRead(vlen,kloc+32)) return false;//next CS may overwrite 16 chars after char 12
            xmlOffset=DICMidx-vlen;//was adjusted in kvReed, we need to roll it back
            xmlLength=vlen - (kbuf[kloc+vlen+11]==0);
            D("xmlOffset:%lu xmlLength:%d\n",xmlOffset,xmlLength);
         }
         else {
            if (!vvRead(vlen)) return false;
         }
      } break;
      case kvCS:
      {
         //no vvRead
         //kvRead already performed and DICMidx adjusted
      } break;
      default: {
         if (!vvRead(vlen)) return false;
      } break;
   }
   return true;
}
