// project: dicm2deck
// targets: dicm2decksqlite
// file: capi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"
#include "deck_sqlite3.h"
#include "sqlite3.h"
#include "../../thirdparty/blake3/blake3.h"
#include "../../thirdparty/openjpeg/opj_compress.h"

extern char *DICMbuf;
extern u64 DICMidx;
extern uint8_t *kbuf;

static u32 DICMlen;

static char relativepath[256];
static u8 relativepathlength=0;
static FILE *fileptr;

static u32 vlenNoPadding;
static u32 utf8length;
static char utf8bytes[256];

/*
//blake3 provides merkle-tree, incremental and fast hash (usefull to compare buffer)
static blake3_hasher hasher;
static uint8_t hashbytes[BLAKE3_OUT_LEN];//32 bytes
static u8 *registeredhashbytes;//32 bytes
*blake3_hasher_reset(&hasher);
*blake3_hasher_update(&hasher, inbuffer, inOffset);
*blake3_hasher_finalize(&hasher, hashbytes, BLAKE3_OUT_LEN);
*/


#pragma mark -


int cPrerequisite(u64 filesize, int argc, char *argv[]){
   return exitZeroError;
}

int cCreate(FILE *inFILE, int argc, char *argv[])
{
   sqlite3open();//opens the db and instantiates the statements
   return exitZeroError;
}

int cCommit(bool hastrailing,int argc, char *argv[])
{
   return  exitZeroError;
}

extern sqlite3      *db;
extern char         *dberr = 0;
extern int           dbrc  = 0;//return code

void cClose(int argc, char *argv[])
{
   sqlite3_finalize(eblake3stmt);
   sqlite3_finalize(einsertstmt);
   sqlite3_free(dberr);
   sqlite3_close(db);
   return;
}

#pragma mark -
static u16  spp;//sample per plane = components
static u16  photocode;
static u16  rows;
static u16  cols;
static u16  alloc;
static u16  stored;
static u16  high;
static u16  pixrep;
static u16  planar;//0 = RGB del pixel; 1 = componentes RGB
static u16 fnumber;

bool eAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   //freeing buffer necesary?
   if ((vlen + 21 + kloc + Eidx > Emax) && !morebuf(EDKV,vlen)) return false;

   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
   Ebuf[Eidx++]=kloc+16;
   
   //prefix
   memcpy(Ebuf+Eidx, &prefix, 8);
   Eidx+=8;
   
   //key
   memcpy(Ebuf+Eidx, kbuf, kloc+8);
   Eidx+=kloc+8;
   
   //value length
   memcpy(Ebuf+Eidx, &vlen, 4);
   Eidx+=4;
   if (vlen==0) return true;
   if (!ufread(vlen)) return false;
   memcpy(Ebuf+Eidx, DICMbuf+DICMidx-vlen, vlen);


   case UI: {
      attr->c=REPERTOIRE_GL;
      switch (attr->t) {
         case B00080018: if (!vrAppend(kloc,kviuid,attr->l)) return false; break;
         case B0020000D: if (!vrAppend(kloc,kveuid,attr->l)) return false; break;
         case B0020000E: if (!vrAppend(kloc,kvsuid,attr->l)) return false; break;
         case B00080019: if (!vrAppend(kloc,kvpuid,attr->l)) return false; break;//pyramid
         case B00200242: if (!vrAppend(kloc,kvcuid,attr->l)) return false; break;//SOP​Instance​UID​Of​Concatenation​Source
            //case B00081150:
         default:        if (!vrAppend(kloc,kvUI,  attr->l)) return false;break;
      }
      if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
   } break;
   case DA: {
      attr->c=REPERTOIRE_GL;
      switch (attr->t) {
         case B00080020: if (!vrAppend(kloc,kvedate, attr->l)) return false;break;
         case B00080021: if (!vrAppend(kloc,kvsdate, attr->l)) return false;break;
         case B00100030: if (!vrAppend(kloc,kvpbirth,attr->l)) return false;break;
         default:        if (!vrAppend(kloc,kvTP,    attr->l)) return false;break;
      }
      if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
   } break;

   switch (vrcat)
   {
      case kveuid: {//StudyInstanceUID
         if (!ui2b64( Ebuf+Eidx, vlen - (Ebuf[Eidx+vlen-1] == 0), euidb64, &euidb64length )) return false;
      };break;
      case kvUI://unique ID
      case kvFD://floating point double
      case kvFL://floating point single
      case kvSL://signed long
      case kvSS://signed short
      case kvUL://unsigned long
      case kvUS://unsigned short
      case kvAT://attribute tag, 2 u16 hexa
      case kvTP://AS DT TM DA 11 text short ascii pair length
      case kvTA://AE DS IS CS 13 text short ascii
      case kvTS://LO LT SH ST 19 text short charset
      case kvTL://UC UT 25 text long charset
      case kvTU://url encoded
      //   kvUN only private
      case kv01://OB OD OF OL OV OW SV UV
      case kvPN://person name
         break;
      case kvedate: edate=atoi(Ebuf+Eidx);break;
      case kvpname: {//Patient name
         pname[0]=Eidx;
         pname[1]=vlen;
         pname[2]=kbuf[kloc+6];
      };break;
      case kvpide: {//Patient id extension
         pide[0]=Eidx;
         pide[1]=vlen;
         pide[2]=kbuf[kloc+6];
      };break;
      case kvpidr: {//Patient root id issuer
         pidr[0]=Eidx;
         pidr[1]=vlen;
         pidr[2]=kbuf[kloc+6];
      };break;
      case kvpbirth: pbirth=atoi(Ebuf+Eidx); break;
      case kvpsex: {//Patient sex
         switch (*(Ebuf+Eidx)) {
            case 'M':psex=1;
               break;
            case 'F':psex=2;
               break;
            case 'O':psex=9;
               break;
            default:psex=0;
               break;
         }
      };break;
      case kveid: {//StudyID
         eid[0]=Eidx;
         eid[1]=vlen;
         eid[2]=kbuf[kloc+6];
      };break;
      case kvean: {//AccessionNumber
         ean[0]=Eidx;
         ean[1]=vlen;
         ean[2]=kbuf[kloc+6];
      };break;
      case kveal: {//AccessionNumberIssuer local 00080051.00400031
         eal[0]=Eidx;
         eal[1]=vlen;
         eal[2]=kbuf[kloc+6];
      };break;
      case kveau: {//AccessionNumberIssuer universal 00080051.00400032
         eau[0]=Eidx;
         eau[1]=vlen;
         eau[2]=kbuf[kloc+6];
      };break;
      case kveat: {//AccessionNumberType
         eat[0]=Eidx;
         eat[1]=vlen;
         eat[2]=kbuf[kloc+6];
      };break;
      case kvimg: {//InstitutionName (placed in exam instead of series)
         img[0]=Eidx;
         img[1]=vlen;
         img[2]=kbuf[kloc+6];
      };break;
      case kvcda: {//study CDA (reading)
         cda[0]=Eidx;
         cda[1]=vlen;
         cda[2]=kbuf[kloc+6];
      };break;
      case kvreq: {//study requesting
         req[0]=Eidx;
         req[1]=vlen;
         req[2]=kbuf[kloc+6];
      };break;
      case kvref: {//study referring
         ref[0]=Eidx;
         ref[1]=vlen;
         ref[2]=kbuf[kloc+6];
      };break;
      case kvpay: {//00101050 kvpay LO pay insurance plan identification
         pay[0]=Eidx;
         pay[1]=vlen;
         pay[2]=kbuf[kloc+6];
      };break;
      case kvedesc: {//study description
         edesc[0]=Eidx;
         edesc[1]=vlen;
         edesc[2]=kbuf[kloc+6];
      };break;
      case kvecode: {//study code
         utf8(kbuf[kloc+6],Ebuf,Eidx,vlen,ecode,ecodelength,&utf8length);
         ecodelength+=utf8length;
         ecode[ecodelength++]='^';
         ecodecharset=kbuf[kloc+6];
      };break;
      default: return false;
   }
   Eidx+=vlen;
   return true;
}

bool sAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   //freeing buffer necesary?
   if ((vlen + 21 + kloc + Sidx > Smax) && !morebuf(SDKV,vlen)) return false;

   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
   Sbuf[Sidx++]=kloc+16;

   //prefix
   memcpy(Sbuf+Sidx, &prefix, 8);
   Sidx+=8;
   
   //key
   memcpy(Sbuf+Sidx, kbuf, kloc+8);
   Sidx+=kloc+8;
   
   //value length
   memcpy(Sbuf+Sidx, &vlen, 4);
   Sidx+=4;
   if (vlen==0) return true;
   if (!ufread(vlen)) return false;
   memcpy(Sbuf+Sidx, DICMbuf+DICMidx-vlen, vlen);

   case DS: { attr->c=REPERTOIRE_GL; if (!vrAppend(kloc,kvTA,attr->l)) return false; if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}} break;
   case IS: {
      attr->c=REPERTOIRE_GL;
      switch (attr->t) {
         case B00200011:if (!vrAppend(kloc,kvsnumber, attr->l)) return false; break;
         case B00200012:if (!vrAppend(kloc,kvianumber,attr->l)) return false; break;
         case B00200013:if (!vrAppend(kloc,kvinumber, attr->l)) return false; break;
         case B00280008:if (!vrAppend(kloc,kvframesnumber, attr->l)) return false; break;
         default:       if (!vrAppend(kloc,kvTA,      attr->l)) return false; break;
      }
      if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
   } break;


   switch (vrcat)
   {
      case kvUI://unique ID
      case kvFD://floating point double
      case kvFL://floating point single
      case kvSL://signed long
      case kvSS://signed short
      case kvUL://unsigned long
      case kvUS://unsigned short
      case kvAT://attribute tag, 2 u16 hexa
      case kvTP://AS DT TM DA 11 text short ascii pair length
      case kvTA://AE DS IS CS 13 text short ascii
      case kvTS://LO LT SH ST 19 text short charset
      case kvTL://UC UT 25 text long charset
      case kvTU://url encoded
      case kvPN://person name
      //   kvUN only private
      case kv01://OB OD OF OL OV OW SV UV
      case kvsdoctitle://ST  DocumentTitle 00420010
      case kvscdaid://HL7InstanceIdentifier
         break;
      case kvsuid: {//SeriesInstanceUID
         if (!ui2b64( Sbuf+Sidx, vlen - (Sbuf[Sidx+vlen-1] == 0), suidb64, &suidb64length )) return false;
      };break;
      case kvsdate: sdate=atoi(Sbuf+Sidx); break;
      case kvstime: stime=atoi(Sbuf+Sidx); break;
      case kvsdocument: {//OB Encapsulated​Document 00420011
         //replace everything after last > with spaces
         vlenNoPadding=Sidx+vlen-1;
         while (Sbuf[vlenNoPadding]!='>')
         {
            D("document tail (%d) %02X\n",vlenNoPadding,Sbuf[vlenNoPadding]);
            
            Sbuf[vlenNoPadding]=' ';
            vlenNoPadding--;
         }
         sxml[0]=Sidx;
         sxml[1]=vlenNoPadding-Sidx+1;
         
      } break;
      case kvsnumber: snumber=atoi(Sbuf+Sidx)+0x8000; break;
      case kvsmod: {//Modality
         smod[0]=Sidx;
         smod[1]=vlen;
      } break;
      case kvsdesc: {
         sdesc[0]=Sidx;
         sdesc[1]=vlen;
         sdesc[2]=kbuf[kloc+6];
      } break;
      default: return false;
   }
   Sidx+=vlen;
   return true;
}

bool pAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   //freeing buffer necesary?
   if ((vlen + 21 + kloc + Pidx > Pmax) && !morebuf(PDKV,vlen)) return false;
   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
   Pbuf[Pidx++]=kloc+16;
   //prefix
   memcpy(Pbuf+Pidx, &prefix, 8);
   Pidx+=8;
   //key
   memcpy(Pbuf+Pidx, kbuf, kloc+8);
   Pidx+=kloc+8;
   //value length
   memcpy(Pbuf+Pidx, &vlen, 4);
   Pidx+=4;
   if (vlen==0) return true;
   if (!ufread(vlen)) return false;
   memcpy(Pbuf+Pidx, DICMbuf+DICMidx-vlen, vlen);
   Pidx+=vlen;
   return true;
}

enum {
   L00020002=0x00020002,
   L00020003=0x00030002,
   L00020010=0x00100002,

   B00080005=0x05000800,//charset

   B00080100=0x00010800,//code
   B00080102=0x02010800,//domain
   B00080104=0x04010800,//title

   B00080018=0x18000800,//kviuid UI SOPInstanceUID
   B00080019=0x19000800,//pyramid

   B0020000D=0x0D002000,//kveuid UI StudyInstanceUID
   B0020000E=0x0E002000,//kvsuid UI SeriesInstanceUID

   B0040E001=0x0E002000,//kvscdaid ST CDA root^extension

   B00100010=0x10001000,//kvpname PN Patient nanme
   B00100020=0x20001000,//kvpide LO Patient id
   B00100021=0x21001000,//kvpidr LO Patient id issuer
   B00100030=0x30001000,//kvpbirth DA Patient bBirthdate
   B00100040=0x40001000,//kvpsex CS Patient sex

   B00080020=0x20000800,//kvedate DA StudyDate
   B00080021=0x21000800,//kvsdate DA SeriesDate
   B00080031=0x31000800,//kvstime DA SeriesTime
   B00200010=0x10002000,//kveid SH StudyID
   B00081030=0x30100800,//kvedesc LO Study name
   B00081032=0x32100800,//kvecode SQ Study code
   B00080090=0x90000800,//kvref PN referring
   B00321032=0x32103200,//kvreq PN requesting

   B00081060=0x60100800,//kvcda PN CDA writer (reading)
   B00101050=0x50101000,//kvpay LO pay insurance plan identification
   B00080060=0x60000800,//kvsmod CS Modality

   B00200011=0x11002000,//kvsnumber IS SeriesNumber
   B00200012=0x12002000,//kvianumber IS AcquisitionNumber
   B00200013=0x13002000,//kvinumber IS InstanceNumber
   B00200242=0x42022000,//SOP​Instance​UID​Of​Concatenation​Source

   B0008103E=0x3E100800,//kvedesc LO Series name

   B00080050=0x50000800,//kvean SH Accession​Number
   B00080051=0x51000800,//kvean SQ Accession​NumberIssuer
   B00080080=0x80000800,//kvimg LO InstitutionName

   B00400031=0x31004000,//kveal UT Accession​Number local
   B00400032=0x32004000,//kveau UT Accession​Number universal
   B00400033=0x33004000,//kveat CS Accession​Number type

   B00081150=0x50110800,

   B00420010=0x10004200,//kvsdoctitle ST DocumentTitle
   B00420011=0x11004200,//kvsxml OB EncapsulatedDocument

   B7FE00001=0x0100E07F,//kvfo Extended​Offset​Table
   B7FE00002=0x0200E07F,//kvfl Extended​Offset​TableLengths
   B7FE00003=0x0300E07F,//kvft Encapsulated​Pixel​Data​Value​Total​Length

   B00020010=0x10000200,//kvC or kvC UI transfert syntax
   B00082111=0x11210800,//kvC        ST derivation description
   };

enum {
B00080008=0x08000800,//CS image type itype
B00280004=0x04002800,//CS photocode (photometric interpretation)

   B00280008=0x08002800,//IS numberOfFrames

   B00280002=0x02002800,//US spp
B00280010=0x10002800,//US rows
B00280011=0x11002800,//US cols
B00280100=0x00012800,//US alloc
B00280101=0x01012800,//US stored
B00280102=0x02012800,//US high
B00280103=0x03012800,//US pixrep
B00280106=0x06012800,//US planar
};
bool iAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   //freeing buffer necesary?
   if ((vlen + 21 + kloc + Iidx > Imax) && !morebuf(IDKV,vlen)) return false;

   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
   Ibuf[Iidx++]=kloc+16;
   
   //prefix
   memcpy(Ibuf+Iidx, &prefix, 8);
   Iidx+=8;
   
   //key
   memcpy(Ibuf+Iidx, kbuf, kloc+8);
   Iidx+=kloc+8;
   
   //value length
   memcpy(Ibuf+Iidx, &vlen, 4);
   Iidx+=4;
   if (vlen==0) return true;
   if (!ufread(vlen)) return false;
   memcpy(Ibuf+Iidx, DICMbuf+DICMidx-vlen, vlen);

   case US: {
      attr->c=REPERTOIRE_GL;
      switch (attr->t) {
         case B00280002: if (!vrAppend(kloc,kvspp,   attr->l)) return false; break;//spp
         case B00280010: if (!vrAppend(kloc,kvrows,  attr->l)) return false; break;//rows
         case B00280011: if (!vrAppend(kloc,kvcols,  attr->l)) return false; break;//cols
         case B00280100: if (!vrAppend(kloc,kvalloc, attr->l)) return false; break;//alloc
         case B00280101: if (!vrAppend(kloc,kvstored,attr->l)) return false; break;//stored
         case B00280102: if (!vrAppend(kloc,kvhigh,  attr->l)) return false; break;//high
         case B00280103: if (!vrAppend(kloc,kvpixrep,attr->l)) return false; break;//pixrep
         case B00280106: if (!vrAppend(kloc,kvplanar,attr->l)) return false; break;//planar
         default:        if (!vrAppend(kloc,kvUS,    attr->l)) return false;
      }
      if (! kkRead(kloc)) {attr->t=u32swap(beforetag);attr->r=0xFFFF;attr->l=0;}
   } break;

   case CS: {
      attr->c=REPERTOIRE_GL;
      switch (attr->t) {
         case B00100040: if (!vrAppend(kloc,kvpsex,     attr->l)) return false; break;
         case B00400033: { //kveat CS Accession​Number type
            u32 *itemtag=(u32 *)kbuf;
            if (*itemtag==B00080051)
            {
               if (!vrAppend(kloc,kveat,attr->l)) return false;
            }
            else
            {
               if (!vrAppend(kloc,kvTA,attr->l)) return false;
            }
         } break;
         case B00080060: if (!vrAppend(kloc,kvsmod,     attr->l)) return false; break;
         case B00080008: if (!vrAppend(kloc,kvitype,    attr->l)) return false; break;
         case B00280004: if (!vrAppend(kloc,kvphotocode,attr->l)) return false; break;
            // https://dicom.innolitics.com/ciods/rt-dose/image-pixel/00280004
         case B00080005: {
            if (!vrAppend(kloc,kvTA,attr->l)) return false;
            u16 repidxs=repertoireidx(DICMbuf+DICMidx-attr->l,attr->l);
            if (repidxs==0x09)
            {
               E("bad repertoire %.*s",attr->l,DICMbuf+DICMidx-attr->l);
               return false;
            }
            else
            {
               keycs=(keycs & 0x8000) | repidxs;
               attr->c=repidxs;
            }
         } break;
   switch (vrcat)
   {
      case kvFD://floating point double
      case kvFL://floating point single
      case kvSL://signed long
      case kvUS://unsigned short
      case kvSS://signed short
      case kvUL://unsigned long
      case kvAT://attribute tag, 2 u16 hexa
      case kvTP://AS DT TM DA 11 text short ascii pair length
      case kvTA://AE DS IS CS 13 text short ascii
      case kvTS://LO LT SH ST 19 text short charset
      case kvTL://UC UT 25 text long charset
      case kvTU://url encoded
      case kvPN://person name
      //   kvUN only private
      case kv01://OB OD OF OL OV OW SV UV
      case kvUI://unique ID
      case kvfo://OV 31 Extended​Offset​Table fragments offset 7FE00001
      case kvfl://OV 32 Extended​Offset​TableLengths fragments offset 7FE00002
      case kvft://UV 33 Encapsulated​Pixel​Data​Value​Total​Length 7FE00003
           break;
      case kviuid: {//SOPInstanceUID
         if (!ui2b64( Ibuf+Iidx, vlen - (Ibuf[Iidx+vlen-1] == 0), iuidb64, &iuidb64length )) return false;
      };break;
      case kvitype: break;
      case kvspp:spp=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280002 US
      case kvframesnumber:iframes=atoi(Ibuf+Iidx);break;// 00280008 IS
      case kvrows:rows=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280010 US
      case kvcols:cols=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280011 US
      case kvalloc:alloc=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280100 US
      case kvstored:stored=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280101 US
      case kvhigh:high=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280102 US
      case kvpixrep:pixrep=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280103 US
      case kvplanar:planar=Ibuf[Iidx]|(Ibuf[Iidx+1]<<8);break;// 00280106 US
//10:icomment B00204000=0x00402000;//LT compression desc (image comment)
      case kvicomment: break;
//14:photocode 00280004 CS https://dicom.innolitics.com/ciods/rt-dose/image-pixel/00280004
      case kvphotocode: break;
      case kvinumber: inumber=atoi(Ibuf+Iidx)+0x8000; break;
      case kvianumber:ianumber=atoi(Ibuf+Iidx)+0x8000;//AcquisitionNumber
           break;
      default: return false;
   }
   Iidx+=vlen;
   return true;
}

#pragma mark - alternative calls
//https://dicom.nema.org/medical/dicom/current/output/html/part05.html#sect_A.4


bool fAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   I("%s","fAppend");
     return false;
}

bool dAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   I("%s","dAppend");
     return false;
}


bool bAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   //freeing buffer necesary?
   if ((vlen + 21 + kloc + Iidx > Imax) && !morebuf(IDKV,vlen)) return false;

   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
   Ibuf[Iidx++]=kloc+16;
   
   //prefix
   memcpy(Ibuf+Iidx, &prefix, 8);
   Iidx+=8;
   
   //key
   memcpy(Ibuf+Iidx, kbuf, kloc+8);
   Iidx+=kloc+8;
   
   //value length
   memcpy(Ibuf+Iidx, &vlen, 4);
   Iidx+=4;

   if (iframes==0)iframes=1;
   if (!sqliteESIP()) return false;//create sql for E,S,I,P

   for (fnumber=1;fnumber <= iframes;fnumber++)
   {
      //standarize pixel representation (per component, unsigned int LE
      DICMlen=cols * rows * spp;
      if (!ufread(DICMlen)) return false;
      cidx=DICMidx;
      //write 4times bigger normalized data after the read in buffer
      if (pixrep) //signed
      {
         if ((spp==1)||planar)
         {
            for (u64 i=DICMidx - DICMlen; i < DICMidx; i++)
            {
               DICMbuf[cidx++]=(char)DICMbuf[i];
               DICMbuf[cidx++]=0;
               DICMbuf[cidx++]=0;
               DICMbuf[cidx++]=0;
            }
         }
         else //multi comp pixels
         {
            u64 j;
            u64 compsize=cols * rows;
            for (u64 i=DICMidx - DICMlen; i < DICMidx; i+=spp)
            {
               for (j=0; j<spp; j++)
               {
                  DICMbuf[cidx+(compsize*j)]=(char)DICMbuf[i];
                  DICMbuf[cidx+(compsize*j)+1]=0;
                  DICMbuf[cidx+(compsize*j)+2]=0;
                  DICMbuf[cidx+(compsize*j)+3]=0;
               }
               cidx++;
            }
            cidx+=cols * rows * (spp -1);
         }
      }
      else //unsigned
      {
         if ((spp==1)||planar)
         {
            for (u64 i=DICMidx - DICMlen; i < DICMidx; i++)
            {
               DICMbuf[cidx++]=DICMbuf[i];
               DICMbuf[cidx++]=0;
               DICMbuf[cidx++]=0;
               DICMbuf[cidx++]=0;
            }
         }
         else //multi comp pixels
         {
            u64 j;
            u64 compsize=cols * rows;
            for (u64 i=DICMidx - DICMlen; i < DICMidx; i+=spp)
            {
               for (j=0; j<spp; j++)
               {
                  DICMbuf[cidx+(compsize*j)]=DICMbuf[i];
                  DICMbuf[cidx+(compsize*j)+1]=0;
                  DICMbuf[cidx+(compsize*j)+2]=0;
                  DICMbuf[cidx+(compsize*j)+3]=0;
               }
               cidx++;
            }
            cidx+=cols * rows * (spp -1);
         }
       }

      /*
      9:syntaxidx
      11:iframes, (0:no frame objects, 1:native, n:encoded)
      13:spp
      14:photocode
      15:rows
      16:cols
      17:alloc
      18:stored
      19:high
      20:pixrep
      21:planar
      */

      //compression cfho
      u64 fidx=0;//fast offset
      u64 hidx=0;//high offset
      u64 oidx=0;//original offset
      u64 zidx=0;//first byte after original
      if (!opj_cfho(photocode ,spp,rows,cols,stored,DICMidx,cidx,&fidx,&hidx,&oidx,&zidx))
      {
         E("%s","error");
      }
      
      
      
      //4 s SS SS iu II II FF FF
      if (!finsert(0x40|sversion|u16swap(snumber)*0x100|iversion*0x100000|concat*0x1000000|u16swap(inumber)*0x100000000|u16swap(fnumber)*0x100000000000000)) return false;
   }
   /*
     call compress for each frame
    
    bool finsert(u64 prefix)
    {
        0:Ifk
          1:pk
        2:fnumber
        //3:fdckv BLOB
        4:DICMidx
        5:DICMlen
        6:syntaxidx
 
    */
   if (!ufread(vlen)) return false;

   
   
   
   memcpy(Ibuf+Iidx, DICMbuf+DICMidx-vlen, vlen);
   Iidx+=vlen;
   return true;

   
   
   
   
   
   
   

   return true;   D("%s","bAppend");

   /*
    put first E S P I to sqlite to get pk
    trailing padding is another dependance of an instance
    
    */
   if (!sqliteESIP())return false;
   
   //freeing buffer necesary?
   //if ((vlen + 21 + kloc + Fidx > Fmax) && !morebuf(FDKV,vlen)) return false;

   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
/*
   Fbuf[Fidx++]=kloc+16;
   
   //prefix
   memcpy(Fbuf+Fidx, &prefix, 8);
   Fidx+=8;
   
   //key
   memcpy(Fbuf+Fidx, kbuf, kloc+8);
   Fidx+=kloc+8;
   
   //value length
   memcpy(Fbuf+Fidx, &vlen, 4);
   Fidx+=4;
   if (vlen==0) return true;
   if (!ufread(vlen)) return false;
   memcpy(Fbuf+Fidx, DICMbuf+DICMidx-vlen, vlen);
   Fidx+=vlen;
*/
   return false;
}

bool wAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   if (iframes==0)iframes=1;
   if (!sqliteESIP())return false;
   /*
   13:spp
   14:photocode
   15:rows
   16:cols
   17:alloc
   18:stored
   19:high
   20:pixrep
   21:planar
   */
   
   if ( (spp==1) && (alloc==16) )
   {
      //always explicit
      D("%s","wAppend");
      /*not compressed one channel */
      for (fnumber=1;fnumber <= iframes;fnumber++)
      {
         DICMlen=cols * rows * 2;
         if (!ufread(DICMlen)) return false;
        //4 s SS SS iu II II FF FF
         if (!finsert(0x40|sversion|u16swap(snumber)*0x100|iversion*0x100000|concat*0x1000000|u16swap(inumber)*0x100000000|u16swap(fnumber)*0x100000000000000)) return false;
      }
   }
   else
   {
      E("%s","wAppend");
      return false;
   }
   return true;
}

bool lAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
   if (vlen!=0xFFFFFFFF) return false; //0x7FE00010 fragments vlen is undefined

      /*
       our codification does not follow the standard
       u32 number of fragments
       u32 offset of fragment (repeated)
       u32 offset after last fragment
       fragments data
       */
      
/*
      u64 fragmentbytes=0;
      struct t4l4 *fragmentstruct=(struct t4l4*) &fragmentbytes;
      
      
      //fragmento 0
      if (EDKVfread(&fragmentbytes, 1, 8, stdin)!=8) return false;
      if (fragmentstruct->t != 0xe000fffe) return false;
      //read (and ignore) neventually existing table
      if (   (fragmentstruct->l > 0)
          && (EDKVfread(vbuf,1,fragmentstruct->l,stdin) != fragmentstruct->l)
         ) return false;
      vloc+=20+fragmentstruct->l;

      
      //write iBuffer and reset index
      //fwriteIfile(false);
      
      //read fragment 1
      if (EDKVfread(&fragmentbytes, 1, 8, stdin)!=8) return false;
      
      while (fragmentstruct->t != 0xe0ddfffe)
      {
         if (fragmentstruct->t != 0xe000fffe) return false;
#pragma mark TODO fragment number
//Iprefix+=0x10000000000;
         Ibuf[Iidx++]=16;//key size
         //memcpy(Ibuf+Iidx, &Iprefix, 8);//copy Iprefix
         Iidx+=8;
         memcpy(Ibuf+Iidx, kbuf, 8);//copy key
         Iidx+=8;
         memcpy(Ibuf+Iidx, &(fragmentstruct->l), 4);//val length
         Iidx+=8;
         
         //write iBuffer and reset index
         //fwriteIfile(false);
         
         //D("%08lld %016llx %x %x\n",vloc,u64swap(Iprefix),fragmentstruct->t,fragmentstruct->l);
         if (fragmentstruct->l > 0)
         {
            size_t bytesremaing=fragmentstruct->l;
            while ( bytesremaing > 0xFFFD)
            {
               if (EDKVfread(vbuf,1,0xFFFE,stdin)!=0xFFFE) return false;
#pragma mark TODO                               if (fwrite(vbuf ,1, 0xFFFE , Ifile)!=0xFFFE) return false;
               bytesremaing-=0xFFFE;
            }
            if (bytesremaing > 0)
            {
               if (EDKVfread(vbuf,1,bytesremaing,stdin)!=bytesremaing) return false;
#pragma mark TODO                               if (fwrite(vbuf ,1, bytesremaing , Ifile)!=bytesremaing) return false;
            }

         }
         vloc+=8+fragmentstruct->l;//174674 en lugar de 172954 (dif 1720)
         if (EDKVfread(&fragmentbytes, 1, 8, stdin)!=8) return false;
      }
*/
   return false;
}

bool vAppend(int kloc,enum kvVRcategory vrcat,u32 vlen)
{
/*
   //freeing buffer necesary?
   if ((vlen + 21 + kloc + Fidx > Fmax) && !morebuf(FDKV,vlen)) return false;

   //key length = key path length + 8 prefix + 8 current attribute
   //idx increased by 1
   Fbuf[Fidx++]=kloc+16;

   //prefix
   memcpy(Fbuf+Fidx, &prefix, 8);
   Fidx+=8;
   
   //key
   memcpy(Fbuf+Fidx, kbuf, kloc+8);
   Fidx+=kloc+8;
   
   //value length
   memcpy(Fbuf+Fidx, &vlen, 4);
   Fidx+=4;
   if (vlen==0) return true;
   //value with contents
   switch (vrcat)
   {
#pragma mark UID
      case kvUI://unique ID (transfert syntax)
      {
         if (fromStdin){if(EDKVfread(Fbuf+Fidx,1,vlen,stdin)!=vlen) return false;}
         else memcpy(Fbuf+Fidx, vbuf, vlen);//from vbuf
         Fidx+=vlen;
      };break;
         
#pragma mark generic
      case kvUS://unsigned short
      case kvTS://LO LT SH ST 19 text short charset
      case kv01://OB OD OF OL OV OW SV UV
#pragma mark special
      case kvnativeOB://40 0x7FE00010: //OB
      case kvnativeOW://41 0x7FE00010: //OW
      case kvnativeOF://42 0x7FE00008: //OF float
      case kvnativeOD://43 0x7FE00009: //OD double
      {
         if (fromStdin){if(EDKVfread(Fbuf+Fidx,1,vlen,stdin)!=vlen) return false;}
         else memcpy(Fbuf+Fidx, vbuf, vlen);//from vbuf
         Fidx+=vlen;
      };break;

      default: return false;
   }
 */
   return false;
}


#pragma mark - trailing padding attribute
