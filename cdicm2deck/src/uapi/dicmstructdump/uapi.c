// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"

extern u64   DICMidx;extern FILE * inFile;
extern u64   DICMsize;
extern char *CKEY;
extern char *DICM;
extern char *BUFF;
const char *space=" ";
static u64 bytesreceived;

#pragma mark ----------------------------- file read

void DICMread(u64 bytesaskedfor)
{
   DICMidx+=bytesaskedfor;
}

char * BUFFread(u32 bytesaskedfor)
{
   DICMidx+=bytesaskedfor;
   return DICM+DICMidx-bytesaskedfor;
}

void key(struct Ercle* attr)
{
   //reads at keyoffset+8 the new 8 first bytes of the attribute
   //reorders group and unit at keyoffset
   // copies the VR and empty bytes at keyoffset+4
   // eventually reads the four additional bytes of long length at keyoffset+8
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

   DICM=malloc(DICMsize);
   bytesreceived=fread(DICM,1,DICMsize,stdin);
   if (DICMsize!=bytesreceived) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %u bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };

   if (inFile!=NULL) {
      fclose(inFile);
      inFile=NULL;
   }
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
void val(u32 keyoffset,enum kvVRcategory vrcat,struct Ercle* attr)
{
   switch (vrcat) {
#pragma mark -sequence
      case kvSA: {printf("%8lu%*s%08X+\n",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e);}break;
      case kvSZ: {printf("%8lu%*s%08X~\n",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e);}break;
      case kvIA: {printf("%8lu %*s%02X%02X%02X%02X+\n",DICMidx,keyoffset+keyoffset-8,space,CKEY[keyoffset-4],CKEY[keyoffset-3],CKEY[keyoffset-2],CKEY[keyoffset-1]);}break;
      case kvIZ: {printf("%8lu %*s%02X%02X%02X%02X~\n",DICMidx,keyoffset+keyoffset-8,space,CKEY[keyoffset-4],CKEY[keyoffset-3],CKEY[keyoffset-2],CKEY[keyoffset-1]);}break;
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
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         DICMread(attr->l);
         printf("{%lu,%u}\n",DICMidx-attr->l,attr->l);

      }break;
      case kvTL://UC
      //UT AccessionNumberIssuer local 00080051.00400031
      //UT AccessionNumberIssuer universal 00080051.00400032
      case kvTU: { //UR
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l>0)
         {
            //charset -> utf-8
            u32 repidx=CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8);
            u32 charstart=(u32)DICMidx;
            u32 utf8length=0;
            DICMread(attr->l);
            utf8(repidx,DICM,charstart,attr->l,DICM,(u32)DICMidx,&utf8length);
            printf( "\"%.*s\"\n", utf8length,DICM+DICMidx );
         }
      } break;
#pragma mark -numbers
      case kvFD: { //floating point double
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            double d;
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=8)
            {
               memcpy(&d, DICM+idx, 8);
               printf(" %f",d);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvFL: { //floating point single
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            float f;
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=4)
            {
               memcpy(&f, DICM+idx, 4);
               printf(" %f",f);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvSL: { //signed long
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            s32 s4B;
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=4)
            {
               memcpy(&s4B, DICM+idx, 4);
               printf(" %d",s4B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvSS: { //signed short
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            s16 s2B;
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=2)
            {
               memcpy(&s2B, DICM+idx, 2);
               printf(" %hd",s2B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUL: { //unsigned long
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            u32 u4B;
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=4)
            {
               memcpy(&u4B, DICM+idx, 4);
               printf(" %u",u4B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUS:{ //unsigned short
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            u16 u2B;
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=2)
            {
               memcpy(&u2B, DICM+idx, 2);
               printf(" %hu",u2B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvAT: { //attribute tag
         printf("%8lu%*s%08X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            printf("(");
            DICMread(attr->l);
            for (u16 idx=DICMidx-attr->l; idx<DICMidx; idx+=2)
            {
               printf(" %04x%04x",*DICM+idx,*DICM+idx+1);
            }
            printf(" )");
         }
         printf("\n");
      }break;
#pragma mark -ascii
      case kvUI://unique ID
      //00080019 PyramidUID
      case kvTP:
      case kvTA://AE DS IS
      //ST HL7InstanceIdentifier 0040E001  root^extension
      case kvCS:
      {
         printf("%8lu%*s%08X %c%c %04X \"%.*s\"\n",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8), attr->l,DICM+DICMidx);
         DICMidx+=attr->l;
      } break;
#pragma mark -charset
      case kvTS://LO LT SH ST
      //ST  DocumentTitle 00420010
      case kvPN:
      {
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,keyoffset+keyoffset+(keyoffset!=0),space, attr->e,CKEY[keyoffset+4],CKEY[keyoffset+5],CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8));
         if (attr->l > 0)
         {
            //charset -> utf-8
            u32 repidx=CKEY[keyoffset+6] + (CKEY[keyoffset+7] << 8);
            u32 charstart=(u32)DICMidx;//corresponds to the offset of the original charset string. utf-8 will be written following the last character
            u32 utf8length=0;
            DICMread(attr->l);
            utf8(repidx,DICM,charstart,attr->l,CKEY,keyoffset,&utf8length);
            printf( "\"%.*s\"\n", utf8length,CKEY+keyoffset );

         }
         else printf("\"\"\n");
      } break;
         
      default:break;
   }
}
