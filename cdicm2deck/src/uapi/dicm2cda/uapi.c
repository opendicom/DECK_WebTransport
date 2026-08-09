// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

extern FILE * inFile;
extern u64 DICMsize;
extern u64 DICMidx;
extern char *CKEY;
extern char *DICM;

void key(struct Ercle* attr) {
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

#pragma mark ----------------------------- SOP instance


void input(int argc, char *argv[])
{
   if ((argc >4) && (strcmp(argv[4],"1.2.840.10008.5.1.4.1.1.104.2")!=0)) exit(exitNotEncapsulatedCDA);

   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL)
   {
      if (ferror(stdin)) {
         fprintf(stderr,"inFile rb %s : %s (%d)\n", argv[1], strerror(errno), errno);
         exit(errno);
      }
      exit(exitErrorFropenCDICM);
   }
   DICM = malloc(DICMsize);
   if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   if (inFile!=NULL) {
      fclose(inFile);
      inFile=NULL;
   }
}

void trail(int argc, char *argv[]){
}

#pragma mark ---------------------------- attributes
//const unsigned long B0040E001=0x0E002000;//ST CDA root^extension
//const unsigned long B00080060=0x60000800;//CS Modality
//const unsigned long B0008103E=0x3E100800;//LO Series name
//const unsigned long B00420010=0x10004200;//ST DocumentTitle
const unsigned long B00420011=0x11004200;//OB EncapsulatedDocument
//const unsigned long B00420012=0x12004200;//LO MIME of EncapsulatedDocument
void val(u32 keyoffset, enum kvVRcategory  vrcat, struct Ercle* attr) //keyoffset=keyidx
{
   switch (vrcat) {
      case kvSA:
      case kvSZ:
      case kvIA:
      case kvIZ:
         break;
      case kv01: {
         //OB encapsulaed document 00420011 xml cda o pdf
         if (!memcmp(CKEY, &B00420011, 4)) fprintf(stdout,"%.*s",attr->l,DICM+DICMidx);
         DICMidx+=attr->l;
      } break;
      default: DICMidx+=attr->l;
   }
}
