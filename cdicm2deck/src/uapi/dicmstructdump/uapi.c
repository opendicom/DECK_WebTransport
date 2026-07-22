// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"

extern char *DICMbuf;
extern u64 DICMidx;
extern uint8_t *kbuf;

#pragma mark - read

static u64 bytesreceived;
bool ifread(u32 bytesaskedfor)
{
   bytesreceived=fread(DICMbuf+DICMidx,1,bytesaskedfor,stdin);
   DICMidx+=bytesreceived;
   return (bytesaskedfor==bytesreceived);
}


//returns true when 8(+4) bytes were read
bool ifreadattr(u8 kloc)
{
   if (fread(DICMbuf+DICMidx,1,8,stdin)!=8)
   {
      if (ferror(stdin)) E("%s","stdin error");
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
      case UR://universal resrcurl identifier/locator
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

const char *space=" ";

#pragma mark - instance transactions

int uPrerequisite(u64 filesize, int argc, char *argv[]) {
   return exitZeroError;
}

int uCreate(FILE *inFILE, int argc, char *argv[])
{
   return exitZeroError;
}
int uCommit(bool hastrailing,int argc, char *argv[])
{
   FILE *fileptr=fopen("dicmstructdump.dcm", "w");
   if (fileptr == NULL) return false;
   if (fwrite(DICMbuf ,1, DICMidx , fileptr)!=DICMidx) return false;
   fclose(fileptr);
}

void uClose(int count, char *vector[])
{
   return;
}

#pragma mark - write

bool uAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen)
{
   switch (vrcat) {
      case kvSA: printf("%8lu%*s%02X%02X%02X%02X+\n",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3]);break;
      case kvSZ: printf("%8lu%*s%02X%02X%02X%02X~\n",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3]);break;
      case kvIA: printf("%8lu %*s%02X%02X%02X%02X+\n",DICMidx-8,kloc+kloc-8,space,kbuf[kloc-4],kbuf[kloc-3],kbuf[kloc-2],kbuf[kloc-1]);break;
      case kvIZ: printf("%8lu %*s%02X%02X%02X%02X~\n",DICMidx-8,kloc+kloc-8,space,kbuf[kloc-4],kbuf[kloc-3],kbuf[kloc-2],kbuf[kloc-1]);break;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (!ifread(vlen)) return false;
         printf("{%lu,%u}\n",DICMidx-vlen,vlen);
      }break;
      case kvTL://UC
      //UT AccessionNumberIssuer local 00080051.00400031
      //UT AccessionNumberIssuer universal 00080051.00400032
      case kvTU: { //UR
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen>0)
         {
            //charset -> utf-8
            u32 repidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
            u32 charstart=(u32)DICMidx;
            u32 utf8length=0;
            if (!ifread(vlen)) return false;
            utf8(repidx,DICMbuf,charstart,vlen,DICMbuf,(u32)DICMidx,&utf8length);
            printf( "\"%.*s\"\n", utf8length,DICMbuf+DICMidx );
         }
      } break;
      case kvFD: { //floating point double
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            double d;
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=8)
            {
               memcpy(&d, DICMbuf+idx, 8);
               printf(" %f",d);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvFL: { //floating point single
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            float f;
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=4)
            {
               memcpy(&f, DICMbuf+idx, 4);
               printf(" %f",f);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvSL: { //signed long
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            s32 s4B;
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=4)
            {
               memcpy(&s4B, DICMbuf+idx, 4);
               printf(" %d",s4B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvSS: { //signed short
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            s16 s2B;
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=2)
            {
               memcpy(&s2B, DICMbuf+idx, 2);
               printf(" %hd",s2B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUL: { //unsigned long
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            u32 u4B;
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=4)
            {
               memcpy(&u4B, DICMbuf+idx, 4);
               printf(" %u",u4B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUS:{ //unsigned short
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            u16 u2B;
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=2)
            {
               memcpy(&u2B, DICMbuf+idx, 2);
               printf(" %hu",u2B);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvAT: { //attribute tag
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            if (!ifread(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=2)
            {
               printf(" %04x%04x",*DICMbuf+idx,*DICMbuf+idx+1);
            }
            printf(" )");
         }
         printf("\n");
      }break;
      case kvUI://unique ID
      //00080019 PyramidUID
      case kvTP:
      case kvTA://AE DS IS CS
      //ST HL7InstanceIdentifier 0040E001  root^extension
      {  printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
        if (vlen > 0)
        {
           if (!ifread(vlen)) return false;
           printf( "\"%.*s\"\n", vlen,DICMbuf+DICMidx-vlen );
        }
        else printf("\"\"\n");
     } break;
      //charset
      case kvTS://LO LT SH ST
      //ST  DocumentTitle 00420010
      case kvPN:
      {
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            //charset -> utf-8
            u32 repidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
            u32 charstart=(u32)DICMidx;
            u32 utf8length=0;
            if (!ifread(vlen)) return false;
            utf8(repidx,DICMbuf,charstart,vlen,DICMbuf,(u32)DICMidx,&utf8length);
            printf( "\"%.*s\"\n", utf8length,DICMbuf+DICMidx );
         }
         else printf("\"\"\n");
      } break;
         
      default: return false;
   
   }
   return true;
}
