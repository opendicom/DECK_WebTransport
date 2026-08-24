// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"
#include <locale.h>

extern FILE * inFile;

extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern char *CKEY;
extern u8   CKEYidx;
static char *UTF8;

u32 utf8size=0;
FILE *KVserializedFILE;
const u32 CSutf8size=10;

#pragma mark ---------------------------- SOP instance

void input( int argc, char *argv[])
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

   DICM=malloc(DICMsize);
   if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   fclose(inFile);

   KVserializedFILE = fopen("serialized.bin", "w");

   setlocale(LC_ALL, "");//output in UTF-8
   UTF8=malloc(0x4000);
   //LT max 10240,UT max 2^32 !!!
   //16K covers any UTF8 size increase for LT, but eventually requires larger buffer  for LT
}


void trail(int count, char *vector[]) {
   fclose(KVserializedFILE);
}

#pragma mark ---------------------------- attributes

void key(struct Ercle* attr)
{
   //reads at CKEYidx+8 the new 8 first bytes of the attribute
   //reorders group and unit at CKEYidx
   // copies the VR and empty bytes at CKEYidx+4
   // eventually reads the four additional bytes of long length at CKEYidx+8
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

void val(enum kvVRcategory vrcat,struct Ercle* attr)
{
   switch (vrcat) {
#pragma mark - sequence and item with end tag
      case kvIA:
      case kvSA:
      case kvSZ:
      case kvIZ:
      case kvIa:
      case kvSa:
      case kvSz:
      case kvIz:
         break;


#pragma mark -charset

      case kvCs:
      {//change to "ISO_IR 192"
         //serialize CKEY
         if (fwrite(CKEY, 1, CKEY[0]+1, KVserializedFILE) != CKEY[0]+1) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         if (fwrite(&CSutf8size, 1, 4, KVserializedFILE) != 4) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         if (fwrite("ISO_IR 192", 1, 10, KVserializedFILE) != 10) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         DICMidx+=attr->l;
      } break;

      case kvTL:
      case kvTS://LO LT SH ST
      case kvPN:
      {
         //serialize CKEY
         if (fwrite(CKEY, 1, CKEY[0]+1, KVserializedFILE) != CKEY[0]+1) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         //convert to utf-8
         utf8size=utf8(CKEY[CKEYidx+6],DICM+DICMidx,attr->l,UTF8);
         if (fwrite(&utf8size, 1, 4, KVserializedFILE) != 4) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         if (attr->l>0) {
            if (fwrite(UTF8, 1, utf8size, KVserializedFILE) != utf8size) {
               printf("%s", "cannot write KVserializedFILE\n");
               exit(-33);
            };
         }
         DICMidx+=attr->l;
      };break;

      default:{
         //serialize CKEY
         if (fwrite(CKEY, 1, CKEY[0]+1, KVserializedFILE) != CKEY[0]+1) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         if (fwrite(&attr->l, 1, 4, KVserializedFILE) != 4) {
            printf("%s", "cannot write KVserializedFILE\n");
            exit(-33);
         };
         if (attr->l > 0) {
            if (fwrite(DICM+DICMidx, 1, attr->l, KVserializedFILE) != attr->l) {
               printf("%s", "cannot write KVserializedFILE\n");
               exit(-33);
            };
            DICMidx+=attr->l;
         }
      } break;
   }
}