// project: dicm2deck
// target: dicm2cda
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.

#include "uapi.h"

const u_int64_t tpaBlake3attr=0x424FFCFFCFF;
const u_int32_t tpaBlake3size=32;

extern FILE *inFile;
extern char *DICM;
extern u64   DICMsize;
extern u64   DICMidx;
extern char *CKEY;
extern u32   CKEYidx;

#pragma mark ---------------------------- attributes

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


//const unsigned long B0040E001=0x0E002000;//ST CDA root^extension
//const unsigned long B00080060=0x60000800;//CS Modality
//const unsigned long B0008103E=0x3E100800;//LO Series name
//const unsigned long B00420010=0x10004200;//ST DocumentTitle
const unsigned long B00420011=0x11004200;//OB EncapsulatedDocument
//const unsigned long B00420012=0x12004200;//LO MIME of EncapsulatedDocument
void val(enum kvVRcategory  vrcat, struct Ercle* attr)
{
   switch (vrcat) {
      case kvSA:
      case kvSZ:
      case kvIA:
      case kvIZ:
      case kvSa:
      case kvSz:
      case kvIa:
      case kvIz:
         break;
      case kv01: {
         //OB encapsulaed document 00420011 xml cda
         if (memcmp(CKEY+1, &B00420011, 4)==0) fprintf(stdout,"%.*s",attr->l - ((DICM+DICMidx+attr->l -1 )==0x00) ,DICM+DICMidx);
         DICMidx+=attr->l;
      } break;
      default: DICMidx+=attr->l;
   }
}


#pragma mark ----------------------------- SOP instance

void input(int argc, char *argv[])
{
   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL)
   {
      if (ferror(stdin)) {
         fprintf(stderr,"inFile rb %s : %s (%d)\n", argv[1], strerror(errno), errno);
         exit(errno);
      }
      exit(exitErrorFropenDICM);
   }
   DICM = malloc(DICMsize+8);//8=preread tag vr sh of inexistent attribute after last one
   if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   fclose(inFile);
   inFile=NULL;
   if (strcmp(DICM+0xA6,"1.2.840.10008.5.1.4.1.1.104.2")!=0) exit(exitNotEncapsulatedCDA);


   memcpy(DICM+DICMsize,&tpaBlake3attr,8);
   memcpy(DICM+DICMsize,&tpaBlake3size,4);
}

void trail(int argc, char *argv[]){
}

