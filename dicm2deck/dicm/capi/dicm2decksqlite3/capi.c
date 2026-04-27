// project: dicm2deck
// targets: dicm2decksqlite
// file: capi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"
#include "deck_sqlite3.h"
#include "../../thirdparty/blake3/blake3.h"
#include "../../thirdparty/openjpeg/opj_compress.h"

extern char *DICMbuf;
extern u64 DICMidx;   classidx=soidx;//class
   syntaxidx=stidx;//transfer syntax

extern uint8_t *kbuf;
static u32 DICMlen;
static u16  syntaxidx;
static u16  classidx;

static char relativepath[256];
static u8 relativepathlength=0;
static FILE *fileptr;

static int stepreturnstatus;
static bool notRegistered;

static u32 vlenNoPadding;


//buffers ...
//start with 0xFFFF size
//and is reallocated with same increment in size each time the current space is filled up
static char *Ebuf;
static char *Sbuf;
static char *Ibuf;
static char *Pbuf;
static char *Fbuf;

static u32 Eidx=0;
static u32 Sidx=0;
static u32 Iidx=0;
static u32 Pidx=0;

static u32 Emax=0;
static u32 Smax=0;
static u32 Imax=0;
static u32 Pmax=0;
static u32 Fmax=0;

static u64 Fcidx=0;
static u64 Ffidx=0;
static u64 Fhidx=0;
static u64 Foidx=0;
static u64 Feidx=0;

static u32 utf8length;
static char utf8bytes[256];

static u64 prefix=0x00;
static int iterator;

//blake3 provides merkle-tree, incremental and fast hash (usefull to compare buffer)
static blake3_hasher hasher;
static uint8_t hashbytes[BLAKE3_OUT_LEN];//32 bytes
static u8 *registeredhashbytes;//32 bytes
/*
*blake3_hasher_reset(&hasher);
*blake3_hasher_update(&hasher, inbuffer, inOffset);
*blake3_hasher_finalize(&hasher, hashbytes, BLAKE3_OUT_LEN);
*/

//prefix components
static u8  sversion=0;

#pragma mark TODO
static u8  rversion=0;
static u16 rnumber=0;
static u8  iversion=0;
static u8  concat=0;



#pragma mark -



bool morebuf(enum eFamily f, u32 vlen)
{
   char *newbuf;
   switch (f) {
      case EDKV:{
         if (vlen > 0xFF00) {
            newbuf=realloc(Ebuf,Emax+vlen+0x0FFF);
            if (newbuf == NULL) return false;
            Emax+=vlen+0x0FFF;
         } else {
            newbuf=realloc(Ebuf,Emax+0xFFFF);
            if (newbuf == NULL) return false;
            Emax+=0xFFFF;
         }
         Ebuf=newbuf;
      } break;
      case SDKV:{
         if (vlen > 0xFF00) {
            newbuf=realloc(Sbuf,Smax+vlen+0x0FFF);
            if (newbuf == NULL) return false;
            Smax+=vlen+0x0FFF;
         } else {
            newbuf=realloc(Sbuf,Smax+0xFFFF);
            if (newbuf == NULL) return false;
            Smax+=0xFFFF;
         }
         Sbuf=newbuf;
      } break;
      case PDKV:{
         if (vlen > 0xFF00) {
            newbuf=realloc(Pbuf,Pmax+vlen+0x0FFF);
            if (newbuf == NULL) return false;
            Pmax+=vlen+0x0FFF;
         } else {
            newbuf=realloc(Pbuf,Pmax+0xFFFF);
            if (newbuf == NULL) return false;
            Pmax+=0xFFFF;
         }
         Pbuf=newbuf;
      } break;
      case IDKV:{
         if (vlen > 0xFF00) {
            newbuf=realloc(Ibuf,Imax+vlen+0x0FFF);
            if (newbuf == NULL) return false;
            Imax+=vlen+0x0FFF;
         } else {
            newbuf=realloc(Ibuf,Imax+0xFFFF);
            if (newbuf == NULL) return false;
            Imax+=0xFFFF;
         }
         Ibuf=newbuf;
      } break;
      default:
         return false;
         break;
   }
   return true;
}



#pragma mark -
static u32  edate;
static char euidb64[44];
static u8   euidb64length;
static u32  pname[3];// patient name [0] offset [1] length [2] charset
static u32  pide[3];// patient id LO
static u32  pidr[3];// patient id issuer LO
static u32  pbirth;//
static u8   psex=0;// CS
static u32  eid[3];// study id
static u32  ean[3];// accession number
static u32  eal[3];// issuer local
static u32  eau[3];// issuer universal
static u32  eat[3];// issuer type
static u32  img[3];// realisationstatic
static u32  cda[3];// cda (reading)
static u32  req[3];// requesting
static u32  ref[3];// requesting
static u32  pay[3];// patient id LO
static u32  edesc[3];// patient id LO
static char ecode[256];// code^lexique^title
static u8   ecodelength;
static u8   ecodecharset;

static u32  sdate;
static u32  stime;
static char suidb64[44];
static u8   suidb64length;
static u32  sxml[2];
//static u32  spdf[3];
static u16  snumber;//unsigned because we sum up 0x8000
//static u32  doctitle[3];
static u32  smod[2];
static u32  sdesc[3];
//sframes

static char iuidb64[44];
static u8   iuidb64length;
static u16  inumber;//unsigned because we sum up 0x8000
static u16  ianumber;//unsigned because we sum up 0x8000
//classidx
static char itype[64];
static u8   itypelength;
static char icomment[64];
static u8   icommentlength;
static u16  iframes;
//pdckv
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
static u64 cidx=0;
static u64 fidx=0;
static u64 hidx=0;
static u64 oidx=0;
static u64 zidx=0;

bool cCreate(
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
   classidx=soidx;//class
   syntaxidx=stidx;//transfer syntax
   sqlite3open();//opens the db and instantiates the statements

//create pid dir if it does not exist
   sprintf(relativepath, "%d", getpid());
   relativepathlength=intdecsize(getpid());
   relativepath[relativepathlength++]='/';
   relativepath[relativepathlength]=0x00;
   if ((stat(relativepath, &st)==-1) && (mkdir(relativepath, 0777)==-1)) return false;

#pragma mark resets
   euidb64length=0;

   //buffers init
   if (Emax==0) {
      Emax=0xFFFF;
      Ebuf=malloc(Emax);
   }
   Eidx=0;

   if (Smax==0) {
      Smax=0xFFFF;
      Sbuf=malloc(Smax);
   }
   Sidx=0;

   if (Pmax==0) {
      Pmax=0xFFFF;
      Pbuf=malloc(Pmax);
   }
   Pidx=0;

   if (Imax==0) {
      Imax=0xFFFF;
      Ibuf=malloc(Imax);
   }
   Iidx=0;

   if (Fmax==0) {
         Fmax=0x3000000;
         Fbuf=malloc(Fmax);
   }
   iframes=0;

   return true;
}

bool cCommit(bool hastrailing)
{
   if (relativepathlength<10) sqliteESIP();//no pixel handler called the method
   if (hastrailing)
   {
      I("%s","hastrailing");
      tinsert();
   }
#pragma mark write DICM
   relativepath[relativepathlength+iuidb64length]=0x00;
   fileptr=fopen(relativepath, "w");
   if (fileptr == NULL) return false;
   if (fwrite(DICMbuf ,1, DICMidx, fileptr)!=DICMidx) return false;
   fclose(fileptr);
   snumber=0;
   inumber=0;
   return uClose();
}


bool cClose(void)
{
   sqlite3_finalize(eblake3stmt);
   sqlite3_finalize(einsertstmt);
   sqlite3_free(dberr);
   sqlite3_close(db);
   DICMidx=0;
   return true;
}

#pragma mark -

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
