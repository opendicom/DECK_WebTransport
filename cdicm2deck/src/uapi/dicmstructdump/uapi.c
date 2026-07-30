// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"

extern char *DICMbuf;
extern u64 DICMidx;
extern char *kbuf;

#pragma mark - read

static u64 bytesreceived;
bool vvRead(u32 bytesaskedfor)
{
   bytesreceived=fread(DICMbuf+DICMidx,1,bytesaskedfor,stdin);
   DICMidx+=bytesreceived;
   return (bytesaskedfor==bytesreceived);
}
bool kvRead(u32 bytesaskedfor, u32 kloc12)
{
   bytesreceived=fread(kbuf+kloc12,1,bytesaskedfor,stdin);
   return (bytesaskedfor==bytesreceived);
}




//returns true when 8(+4) bytes were read
bool kkRead(u8 kloc)
{
   //reads at kloc+8 the new 8 first bytes of the attribute
   //reorders group and unit at kloc
   // copies the VR and empty bytes at kloc+4
   // eventually reads the four additional bytes of long length at kloc+8
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
   //kbuf[kloc+6] for repertoire
   //kbuf[kloc+7] for repertoire
   kbuf[kloc+8]=DICMbuf[DICMidx+6];
   kbuf[kloc+9]=DICMbuf[DICMidx+7];
   kbuf[kloc+10]=0;
   kbuf[kloc+11]=0;
   DICMidx+=8;
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
         if (fread(DICMbuf+DICMidx,1,4,stdin)!=4)
         {
            if (ferror(stdin)) E("%s","stdin error");
            return false;
         }
         //replace short length by long one
         memcpy(kbuf+kloc+8,DICMbuf+DICMidx,4);
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
   return true;
}


#pragma mark - instance transactions

int uPrerequisite(u64 filesize, int argc, char *argv[]) {
   DICMbuf=malloc(filesize);

#pragma mark filesystem
   const char* outdir = getenv("CDICM2DECKoutdir");
   D("CDICM2DECKoutdir:  %s", outdir);
   chdir(outdir);

   const char* errdir = getenv("CDICM2DECKerrdir");
   D("CDICM2DECKerrdir:  %s", errdir);

   char cwd[1024];
   getcwd(cwd, sizeof(cwd));
   D("working dir:  %s", cwd);

   return exitZeroError;
}
int uCreate(FILE *inFILE, int argc, char *argv[])
{
   DICMidx=0x9E;//0x9E 0002,0002
   bytesreceived=fread(DICMbuf,1,DICMidx,stdin);
   if (DICMidx!=bytesreceived) return exitNotDICM;
   //if (fseek(stdin, DICMidx, SEEK_SET)!=0) return exitNotDICM;

   kbuf = malloc(0x3000);
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

const char *space=" ";
bool vrAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen)
{
   switch (vrcat) {
#pragma mark -sequence
      case kvSA: {printf("%8lu%*s%02X%02X%02X%02X+\n",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3]);}break;
      case kvSZ: {printf("%8lu%*s%02X%02X%02X%02X~\n",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3]);}break;
      case kvIA: {printf("%8lu %*s%02X%02X%02X%02X+\n",DICMidx,kloc+kloc-8,space,kbuf[kloc-4],kbuf[kloc-3],kbuf[kloc-2],kbuf[kloc-1]);}break;
      case kvIZ: {printf("%8lu %*s%02X%02X%02X%02X~\n",DICMidx,kloc+kloc-8,space,kbuf[kloc-4],kbuf[kloc-3],kbuf[kloc-2],kbuf[kloc-1]);}break;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (!vvRead(vlen)) return false;
         printf("{%lu,%u}\n",DICMidx-vlen,vlen);
      }break;
      case kvTL://UC
      //UT AccessionNumberIssuer local 00080051.00400031
      //UT AccessionNumberIssuer universal 00080051.00400032
      case kvTU: { //UR
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen>0)
         {
            //charset -> utf-8
            u32 repidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
            u32 charstart=(u32)DICMidx;
            u32 utf8length=0;
            if (!vvRead(vlen)) return false;
            utf8(repidx,DICMbuf,charstart,vlen,DICMbuf,(u32)DICMidx,&utf8length);
            printf( "\"%.*s\"\n", utf8length,DICMbuf+DICMidx );
         }
      } break;
#pragma mark -numbers
      case kvFD: { //floating point double
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            double d;
            if (!vvRead(vlen)) return false;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            float f;
            if (!vvRead(vlen)) return false;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            s32 s4B;
            if (!vvRead(vlen)) return false;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            s16 s2B;
            if (!vvRead(vlen)) return false;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            u32 u4B;
            if (!vvRead(vlen)) return false;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            u16 u2B;
            if (!vvRead(vlen)) return false;
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
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            printf("(");
            if (!vvRead(vlen)) return false;
            for (u16 idx=DICMidx-vlen; idx<DICMidx; idx+=2)
            {
               printf(" %04x%04x",*DICMbuf+idx,*DICMbuf+idx+1);
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
      {
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
           if (!vvRead(vlen)) return false;
           printf( "\"%.*s\"\n", vlen,DICMbuf+DICMidx-vlen );
         }
         else printf("\"\"\n");
     } break;
      case kvCS:
      {
         //no vvRead, replaced by memcpy from kv
         //the value is found in kbuf+kloc+12
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X \"%.*s\"\n",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8), vlen,kbuf+kloc+12);
         memcpy(DICMbuf+DICMidx, kbuf+kloc+12, vlen);
         DICMidx+=vlen;
      } break;
#pragma mark -charset
      case kvTS://LO LT SH ST
      //ST  DocumentTitle 00420010
      case kvPN:
      {
         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (vlen > 0)
         {
            //charset -> utf-8
            u32 repidx=kbuf[kloc+6] + (kbuf[kloc+7] << 8);
            u32 charstart=(u32)DICMidx;//corresponds to the offset of the original charset string. utf-8 will be written following the last character
            u32 utf8length=0;
            if (!vvRead(vlen)) return false;
            utf8(repidx,DICMbuf,charstart,vlen,kbuf,kloc,&utf8length);
            printf( "\"%.*s\"\n", utf8length,kbuf+kloc );

         }
         else printf("\"\"\n");
      } break;
         
      default: return false;
   
   }
   return true;
}
