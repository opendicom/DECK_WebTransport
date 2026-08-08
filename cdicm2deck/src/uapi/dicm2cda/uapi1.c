// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

extern u64 DICMsize;
extern u64 DICMidx;
extern char *CKEY;
extern char *DICM;
extern char *BUFF;
u64 bytesreceived;

#pragma mark ----------------------------- file read

//DICM buffered
void DICMread(u64 bytesaskedfor){
   DICMidx+=bytesaskedfor;
}

//small 16 bytes buffer (CS size)
char * BUFFread(u32 bytesaskedfor){
   DICMidx+=bytesaskedfor;
   return DICM+DICMidx-bytesaskedfor;
}

void CKEYread(struct Ercle* attr) {
   memcpy(attr,DICM+DICMidx,8);
   DICMidx+=8;
   attr->E=((attr->E & 0xFF00FF00) >> 8)|((attr->E & 0xFF00FF) << 8); //transform to pure big endian
   attr->e=u32swap(attr->E);//tag in little endian

   //attr->r is ascii (left to right)
   attr->l=attr->c;//short length is transferred to long length (bytes 8-11)
   //attr->c short length will be modified before parsing the value to register repositoire index
   switch (attr->r) {
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
      case UR://universal resource url identifier/locator
      case SQ://sequence
      {
         memcpy(&attr->l,DICM+DICMidx,4);
         DICMidx+=4;
      }break;
      default:break;
   }
}

#pragma mark ----------------------------- SOP instance transactions
void uPrerequisite(int argc, char *argv[]) {
   if ((argc >4) && (strcmp(argv[4],"1.2.840.10008.5.1.4.1.1.104.2")!=0)) exit(exitNotEncapsulatedCDA);
}

void uCreate(int argc, char *argv[])
{
   DICM = malloc(DICMsize);
   bytesreceived=fread(DICM,1,DICMsize,stdin);
   if (DICMsize!=bytesreceived) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %u bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   DICMidx=0x9E;//0x9E 0002,0002
//BUFF malloc(256);//up to 15 16 bytes CS NOT NECESSARY (written in DICM)
}

void uClose(int argc, char *argv[]){
}

#pragma mark ---------------------------- attributes processing
//const unsigned long B0040E001=0x0E002000;//ST CDA root^extension
//const unsigned long B00080060=0x60000800;//CS Modality
//const unsigned long B0008103E=0x3E100800;//LO Series name
//const unsigned long B00420010=0x10004200;//ST DocumentTitle
const unsigned long B00420011=0x11004200;//OB EncapsulatedDocument
//const unsigned long B00420012=0x12004200;//LO MIME of EncapsulatedDocument
void vrAppend(u32 keyoffset, enum kvVRcategory  vrcat, u32 vlen) //keyoffset=keyidx
{
   switch (vrcat) {
      case kvSA:
      case kvSZ:
      case kvIA:
      case kvIZ:
      case kvCS://no DICMread. BUFread already performed when reading key. DICMidx already adjusted
         break;
      case kv01: {
         //OB encapsulaed document 00420011 xml cda o pdf
         if (!memcmp(CKEY, &B00420011, 4)) {
            fprintf(stdout,"%.*s",vlen,DICM+DICMidx);
            DICMidx+=vlen;
         }
         else DICMread((u64)vlen);
      } break;
      default: DICMread((u64)vlen);//fseek
   }
}
