// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

extern u64 DICMidx;
extern char *CKEY;
extern char *BUFF;
u64 bytesreceived;

#pragma mark ----------------------------- file read

//DICM not buffered
void DICMread(u32 bytesaskedfor){
   if (fseek(stdin, bytesaskedfor, SEEK_CUR)!=0) {
      if (ferror(stdin)) {
         fprintf(stderr,"DICMread [DICMidx %lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"DICMread [%lu] read %u bytes truncated (%d)\n", DICMidx, bytesaskedfor, exitReadTruncated);
      exit(exitReadTruncated);
   }
   DICMidx+=bytesaskedfor;
}

//small 16 bytes buffer (CS size)
void BUFFread(u32 bytesaskedfor){
   bytesreceived=fread(BUFF,1,bytesaskedfor,stdin);
   if (bytesaskedfor!=bytesreceived) {
      if (ferror(stdin)) {
         fprintf(stderr,"BUFFread [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"BUFFread [%lu] read %u bytes truncated (%d)\n", DICMidx, bytesaskedfor, exitReadTruncated);
      exit(exitReadTruncated);
   }
   DICMidx+=bytesaskedfor;
}

void CKEYread(struct Ercle* attr) {
   if (fread(attr,1,8,stdin)!=8) {
      if (ferror(stdin)) {
         fprintf(stderr,"CKEYread [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"CKEYread [%lu] read truncated (%d)\n", DICMidx, exitReadTruncated);
      exit(exitReadTruncated);
   }
   //fprintf(stderr,"%8lu %08X\n",DICMidx,attr->e);

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
         if (fread(&attr->l,1,4,stdin)!=4)
         {
            if (ferror(stdin)) {
               fprintf(stderr,"CKEYread ll [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
               exit(errno);
            }
            fprintf(stderr,"CKEYread ll [%lu] read truncated (%d)\n", DICMidx, exitReadTruncated);
            exit(exitReadTruncated);
         }
         DICMidx+=4;
      }break;
      default:break;
   }
}

#pragma mark ----------------------------- SOP instance transactions
void uPrerequisite(int argc, char *argv[]) {
   if ((argc >4) && (strcmp(argv[4],"1.2.840.10008.5.1.4.1.1.104.2")!=0)) return exitNotEncapsulatedCDA;
   return exitZeroError;
}

void uCreate(int argc, char *argv[])
{
   DICMidx=0x9E;//0x9E 0002,0002
   if (fseek(stdin, (s64)DICMidx, SEEK_SET)!=0) return exitNotDICM;//0x9E 0002,0002
   BUFF = malloc(256);//up to 15 16 bytes CS
   return exitZeroError;
}

void uClose(int argc, char *argv[]){
   return exitZeroError;
}

#pragma mark ---------------------------- attributes processing
//const unsigned long B0040E001=0x0E002000;//ST CDA root^extension
//const unsigned long B00080060=0x60000800;//CS Modality
//const unsigned long B0008103E=0x3E100800;//LO Series name
//const unsigned long B00420010=0x10004200;//ST DocumentTitle
const unsigned long B00420011=0x11004200;//OB EncapsulatedDocument
//const unsigned long B00420012=0x12004200;//LO MIME of EncapsulatedDocument
void vrAppend(u32 kloc, enum kvVRcategory  vrcat, u32 vlen)
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
            char *CDA= malloc(vlen);
            if (fread(CDA,1,vlen,stdin)!=vlen) {
               //CKEY[0] is base level, as the enclosed CDA is
               if (ferror(stdin)) {
                  fprintf(stderr,"vrAppend kv01 CDA [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
                  exit(errno);
               }
               fprintf(stderr,"vrAppend kv01 CDA  [%lu] read truncated (%d)\n", DICMidx, exitReadTruncated);
               exit(exitReadTruncated);
            }
            fprintf(stdout,"%s",CDA);
            DICMidx+=vlen;
         }
         else DICMread(vlen);
      } break;
      default: DICMread(vlen);//fseek
   }
}
