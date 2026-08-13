// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"
#include <locale.h>

extern FILE * inFile;


const u_int64_t tpaBlake3attr=0x424FFCFFCFF;
const u_int32_t tpaBlake3size=32;


extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern char *CKEY;
extern u8   CKEYidx;

static const char *space=" ";
static char *UTF8;

//prepare output in memory
char *SERIALIZE;
u64   SERIALIZEidx;



#pragma mark ----------------------------- file read


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


#pragma mark ----------------------------- SOP instance transactions

void input( int argc, char *argv[])
{
   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL)
   {
      if (ferror(stdin)) {
         fprintf(stderr,"inFile rb %s : %s (%d)\n", argv[1], strerror(errno), errno);
         exit(errno);
      }
      exit(exitErrorFropenCDICM);
   }

   DICM=malloc(DICMsize+44);
   if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   fclose(inFile);

   SERIALIZE=malloc(DICMsize + 0x1000);

   setlocale(LC_ALL, "");//output in UTF-8
   UTF8=malloc(0x4000);
   //LT max 10240,UT max 2^32 !!!
   //16K covers any UTF8 size increase for LT, but eventually requires larger buffer  for LT

   memcpy(DICM+DICMsize,&tpaBlake3attr,8);
   memcpy(DICM+DICMsize,&tpaBlake3size,4);

//TODO selection of pixel sop class
   u64 *offset190=(u64*)(DICM+190);
   if (isImage(*offset190,32,16)) printf("is image\n");
   else printf("is not Image\n");


}

void trail(int count, char *vector[]) {

   FILE *fileptr = fopen("dscd.utf8.cdicm", "w");
   if ((fileptr == NULL) || (fwrite(SERIALIZE, 1, SERIALIZEidx, fileptr) != SERIALIZEidx)) {
      printf("%s", "cannot write dscd.utf8.cdicm\n");
      exit(-33);
   };
   fclose(fileptr);
}


#pragma mark ---------------------------- attributes
void val(enum kvVRcategory vrcat,struct Ercle* attr)
{
   switch (vrcat) {
#pragma mark -sequence
      case kvSA: {
         //SQ starts at -8
         //if -4 not ffffffff change it
         //else nothing
      }break;
      case kvSZ: {
         //if 0xfffee0dd nothing to do
         //else add 0xfffee0dd00000000
      }break;
      case kvIA: {
         //starts at -8
         //0xfffee000
         //if -4 not ffffffff change it
         //else nothing
      }break;
      case kvIZ: {
         //if 0xfffee00d nothing to do
         //else add 0xfffee0dd00000000
      }break;
#pragma mark -long length
      case kv01://OB OD OF OL OV OW SV UV
         //TODO
         //OB Encapsulated​Document 00420011 xml cda o pdf
         //OF 0x7FE00008
         //OD 0x7FE00009
         //OB 0x7FE00010
         //OW 0x7FE00010
         //OB 0x7E000010
         //OB 0x7E000010
         //OV Extended​Offset​Table fragments offset 7FE00001
         //OV Extended​Offset​TableLengths fragments offset 7FE00002
         //UV Encapsulated​Pixel​Data​Value​Total​Length 7FE00003
      case kvUN: { DICMidx+=attr->l;}break;
      case kvTL:{//UC
         //UT AccessionNumberIssuer local 00080051.00400031
         //UT AccessionNumberIssuer universal 00080051.00400032
      // convert to utf-8

      };break;
      case kvTU: {DICMidx+=attr->l;} break;//UR
#pragma mark -numbers
      case kvFD:
      case kvFL:
      case kvSL:
      case kvSS:
      case kvUL:
      case kvUS:
      case kvAT: {DICMidx+=attr->l;}break;
#pragma mark -ascii
      case kvUI: {
         if (attr->e == 0x20010) {
            //option to modify the transfer syntax
            //copy everything before value
            memcpy(SERIALIZE,DICM,DICMidx);
            //adjust size and new syntax
            //old 0x14 0x00 1.2.840.10008.1.2.1 0x00
            //new 0x18 0x00 1.2.840.10008.1.2.4.202 0x08C
            //change group2 size (starting in 0x
            //may end in 1 (Lossless),2 (Lossless RPCL - resolution progressive) or 3 (not specified)
            SERIALIZE[DICMidx-2]=0x18;
            const char *htj2krpcl="1.2.840.10008.1.2.4.202";
            memcpy(SERIALIZE,htj2krpcl,0x18);
            SERIALIZEidx=DICMidx+0x18;
            //adjust DICMidx
         }
         DICMidx+=attr->l;
      } break;
      case kvTP:
      case kvTA://AE DS IS
         //ST HL7InstanceIdentifier 0040E001  root^extension
      case kvCS:{DICMidx+=attr->l;} break;

#pragma mark -charset
      case kvTS://LO LT SH ST
         //ST  DocumentTitle 00420010
      case kvPN:{DICMidx+=attr->l;} break;

      default:break;
   }
}

