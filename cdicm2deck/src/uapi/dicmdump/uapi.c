// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"
#include <locale.h>

const u_int64_t tpaBlake3attr=0x424FFCFFCFF;
const u_int32_t tpaBlake3size=32;

extern FILE * inFile;

extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern char *CKEY;
extern u8   CKEYidx;

static const char *space=" ";
static char *UTF8;


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
      exit(exitErrorFropenDICM);
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

   if (inFile!=NULL) {
      fclose(inFile);
      inFile=NULL;
   }
   setlocale(LC_ALL, "");//output in UTF-8
   UTF8=malloc(0x4000);
   //LT max 10240,UT max 2^32 !!!
   //16K covers any UTF8 size increase for LT, but eventually requires larger buffer  for LT

   memcpy(DICM+DICMsize,&tpaBlake3attr,8);
   memcpy(DICM+DICMsize,&tpaBlake3size,4);
}

void trail(int count, char *vector[]) {
   /*
   FILE *fileptr = fopen("dicmstructdump.dcm", "w");
   if (fileptr == NULL) return false;
   if (fwrite(DICM, 1, DICMidx, fileptr) != DICMidx) return false;
   fclose(fileptr);
   */
}


#pragma mark ---------------------------- attributes
void val(enum kvVRcategory vrcat,struct Ercle* attr)
{
   switch (vrcat) {
#pragma mark -sequence
      case kvSA: {printf("%8lu%*s%08X+\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e);}break;
      case kvSZ: {printf("%8lu%*s%08X~\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, u32swap(attr->E));}break;
      case kvIA: {printf("%8lu %*s%02X%02X%02X%02X+\n",DICMidx,CKEYidx+CKEYidx-8,space,CKEY[CKEYidx-4],CKEY[CKEYidx-3],CKEY[CKEYidx-2],CKEY[CKEYidx-1]);}break;
      case kvIZ: {printf("%8lu %*s%02X%02X%02X%02X~\n",DICMidx,CKEYidx+CKEYidx-8,space,CKEY[CKEYidx-4],CKEY[CKEYidx-3],CKEY[CKEYidx-2],CKEY[CKEYidx-1]);}break;
#pragma mark -long length
      case kv01://OB OD OF OL OV OW SV UV
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
      case kvUN: {
         printf("%8lu%*s%08X %c%c      {%lu,%u}\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5],DICMidx,attr->l);
         DICMidx+=attr->l;
      }break;
      case kvTL:{//UC
      //UT AccessionNumberIssuer local 00080051.00400031
      //UT AccessionNumberIssuer universal 00080051.00400032
         printf("%8lu%*s%08X %c%c %02X   \"%.*s\"\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5],CKEY[CKEYidx+6], utf8(CKEY[CKEYidx+6],DICM+DICMidx,attr->l,UTF8),UTF8);
         DICMidx+=attr->l;
      } break;
      case kvTU: { //UR originally UTF-8
         printf("%8lu%*s%08X %c%cF-8  \"%.*s\"\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5], attr->l,DICM+DICMidx);
         DICMidx+=attr->l;
      } break;
#pragma mark -numbers
      case kvFD: { //floating point double
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            double d;
            DICMidx+=attr->l;
            for (u32 idx=DICMidx-attr->l; idx<DICMidx; idx+=8)
            {
               memcpy(&d, DICM+idx, 8);
               printf(" %f",d);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvFL: { //floating point single
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            float f;
            DICMidx+=attr->l;
            for (u64 idx=DICMidx-attr->l; idx<DICMidx; idx+=4)
            {
               memcpy(&f, DICM+idx, 4);
               printf(" %f",f);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvSL: { //signed long
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            s32 s4B;
            DICMidx+=attr->l;
            for (u64 idx=DICMidx-attr->l; idx<DICMidx; idx+=4)
            {
               memcpy(&s4B, DICM+idx, 4);
               printf(" %d",s4B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvSS: { //signed short
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            s16 s2B;
            DICMidx+=attr->l;
            for (u64 idx=DICMidx-attr->l; idx<DICMidx; idx+=2)
            {
               memcpy(&s2B, DICM+idx, 2);
               printf(" %hd",s2B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUL: { //unsigned long
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            u32 u4B;
            DICMidx+=attr->l;
            for (u64 idx=DICMidx-attr->l; idx<DICMidx; idx+=4)
            {
               memcpy(&u4B, DICM+idx, 4);
               printf(" %u",u4B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUS:{ //unsigned short
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            u16 u2B;
            DICMidx+=attr->l;
            for (u64 idx=DICMidx-attr->l; idx<DICMidx; idx+=2)
            {
               memcpy(&u2B, DICM+idx, 2);
               printf(" %hu",u2B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvAT: { //attribute tag
         printf("%8lu%*s%08X %c%c      ",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
         if (attr->l > 0)
         {
            printf("(");
            for (u64 idx=DICMidx-attr->l; idx<(DICMidx+attr->l); idx+=4)
            {
               printf(" %02x%02x%02x%02x",DICM[idx+1],DICM[idx],DICM[idx+3],DICM[idx+2]);
            }
            printf(" )");
         }
         printf("\n");
         DICMidx+=attr->l;

      }break;
#pragma mark -ascii
      case kvUI://unique ID
      //00080019 PyramidUID
      case kvTP:
      case kvTA://AE DS IS
      //ST HL7InstanceIdentifier 0040E001  root^extension
      case kvCS:
      {
         printf("%8lu%*s%08X %c%c      \"%.*s\"\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5], attr->l,DICM+DICMidx);
         DICMidx+=attr->l;
      } break;
#pragma mark -charset
      case kvTS://LO LT SH ST
      //ST  DocumentTitle 00420010
      case kvPN:
      {
         printf("%8lu%*s%08X %c%c %02X   \"%.*s\"\n",DICMidx,CKEYidx+CKEYidx+(CKEYidx!=0),space, attr->e,CKEY[CKEYidx+4],CKEY[CKEYidx+5],CKEY[CKEYidx+6], utf8(CKEY[CKEYidx+6],DICM+DICMidx,attr->l,UTF8),UTF8);
         DICMidx+=attr->l;
      } break;
         
      default:break;
   }
}
