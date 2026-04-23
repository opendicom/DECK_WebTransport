// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"

extern char *DICMbuf;
extern u64 DICMidx;
extern s16 siidx;
extern uint8_t *kbuf;

#pragma mark - read

static u64 bytesreceived;
bool ifread(u32 bytesaskedfor)
{
   bytesreceived=fread(DICMbuf+DICMidx,1,bytesaskedfor,stdin);
   //if (bytesreceived>0xFFFFFFFF)return 0;
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

bool uCreate(
   u64 soloc,         // offset in valbyes for sop class
   u16 solen,         // length in valbyes for sop class
   u16 soidx,         // index in const char *scstr[]
   u64 siloc,         // offset in valbyes for sop instance uid
   u16 silen,         // length in valbyes for sop instance uid
   u64 stloc,         // offset in valbyes for transfer syntax
   u16 stlen,         // length in valbyes for transfer syntax
   u16 stidx          // index in const char *csstr[]
)
{
   D("dump create #%d",siidx);
   printf("     144 %s\n","00020001 OB 0000 {156,2}");
   printf("%8llu 00020002 UI 0000 \"%s\" [%hu]\n",soloc,DICMbuf+soloc,soidx);
   printf("%8llu 00020003 UI 0000 \"%s\"\n",siloc,DICMbuf+siloc);
   printf("%8llu 00020010 UI 0000 \"%s\" [%hu]\n",stloc,DICMbuf+stloc,stidx);
   return true;
}
bool uCommit(bool hastrailing)
{
   FILE *fileptr=fopen("dicmstructdump.dcm", "w");
   if (fileptr == NULL) return false;
   if (fwrite(DICMbuf ,1, DICMidx , fileptr)!=DICMidx) return false;
   fclose(fileptr);
   return uClose();
}
bool uClose(void)
{
   I("!#%d", siidx);
   return true;
}

#pragma mark - write

bool uAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   switch (vrcat) {
      case kvSA: printf("%8llu%*s%02X%02X%02X%02X+\n",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3]);break;
      case kvSZ: printf("%8llu%*s%02X%02X%02X%02X~\n",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3]);break;
      case kvIA: printf("%8llu %*s%02X%02X%02X%02X+\n",DICMidx-8,kloc+kloc-8,space,kbuf[kloc-4],kbuf[kloc-3],kbuf[kloc-2],kbuf[kloc-1]);break;
      case kvIZ: printf("%8llu %*s%02X%02X%02X%02X~\n",DICMidx-8,kloc+kloc-8,space,kbuf[kloc-4],kbuf[kloc-3],kbuf[kloc-2],kbuf[kloc-1]);break;
      case kv01://OB OD OF OL OV OW SV UV
      case kvsdocument://OB Encapsulated​Document 00420011 xml cda o pdf
      case kvpixelOF: //OF 0x7FE00008
      case kvpixelOD: //OD 0x7FE00009
      case kvpixelOB: //OB 0x7FE00010
      case kvpixelOW: //OW 0x7FE00010
      case kvpixelOL: //OB 0x7E000010
      case kvpixelOV: //OB 0x7E000010
      case kvfo://OV Extended​Offset​Table fragments offset 7FE00001
      case kvfl://OV Extended​Offset​TableLengths fragments offset 7FE00002
      case kvft://UV Encapsulated​Pixel​Data​Value​Total​Length 7FE00003
      case kvUN: {
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
         if (!ifread(vlen)) return false;
         printf("{%llu,%u}\n",DICMidx-vlen,vlen);
      }break;
      case kvTL://UC
      case kveal://UT AccessionNumberIssuer local 00080051.00400031
      case kveau://UT AccessionNumberIssuer universal 00080051.00400032
      case kvTU: { //UR
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
      case kvUS://unsigned short
      case kvspp://7 US
      case kvrows://8 US
      case kvcols://9 US
      case kvalloc://10 US
      case kvstored://11 US
      case kvhigh://12 US
      case kvpixrep://13 US
      case kvplanar: { //14 US
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
      case kveuid://StudyInstanceUID
      case kvsuid://SeriesInstanceUID
      case kviuid://SOPInstanceUID
      case kvpuid://00080019 PyramidUID
      case kvcuid://SOP​Instance​UID​Of​Concatenation​Source
      case kvTP:
      case kvpbirth://Patient birthdate
      case kvedate://StudyDate
      case kvsdate://SeriesDate
      case kvstime://SeriesTime
      case kvTA://AE DS IS CS
      case kvpsex://CS patient sex
      case kveat://AccessionNumberType
      case kvsmod://Modality
      case kvitype://CS 00080008 ImageType
      case kvphotocode://CS
      case kvsnumber://SeriesNumber
      case kvianumber://AcquisitionNumber
      case kvinumber://InstanceNumber
      case kvframesnumber://Number of frames
      case kvscdaid:{ //ST HL7InstanceIdentifier 0040E001  root^extension
        printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
        if (vlen > 0)
        {
           if (!ifread(vlen)) return false;
           printf( "\"%.*s\"\n", vlen,DICMbuf+DICMidx-vlen );
        }
        else printf("\"\"\n");
     } break;
      //charset
      case kvTS://LO LT SH ST
      case kvpay://LO insurance
      case kvpide://SH patient id extension
      case kvpidr://LO patient id root issuer
      case kvimg://InstitutionName
      case kvedesc://LO Study name
      case kveid://StudyID
      case kvean://AccessionNumber
      case kvecode://SQ/SH Study code 00080100,00080102
      case kvsdesc://LO Series name
      case kvsdoctitle://ST  DocumentTitle 00420010
      case kvicomment://LO
      case kvPN:
      case kvpname://PN patient name
      case kvcda://PN CDA
      case kvref://PN referring
      case kvreq: { //PN requesting
         printf("%8llu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-8,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));
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
