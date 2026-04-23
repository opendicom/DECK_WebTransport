// project: dicm2deck
// targets: dicm2cda,dicmstructdump,...
// file: uapi.h
// created by jacquesfauquex on 2024-04-04.

#ifndef uapi_h
#define uapi_h

#include "datatypes.h"


#pragma mark - propietary vr

enum kvVRcategory{
   
//vl enum
kvFD,//FD 1 floating point double
kvFL,//FL 2 floating point single
kvSL,//SL 3 signed long
kvSS,//SS 4 signed short
kvUL,//UL 5 unsigned long
kvUS,//US 6 unsigned short
                kvspp,//7 US
                kvrows,//8 US
                kvcols,//9 US
                kvalloc,//10 US
                kvstored,//11 US
                kvhigh,//12 US
                kvpixrep,//13 US
                kvplanar,//14 US

kvAT,//AT 15 attribute tag, 2 u16 hexa
   
kvUI,//UI 16 unique ID eventualmente terminado por 0x00
     kveuid,//UI 17 StudyInstanceUID
          kvsuid,//UI 18 SeriesInstanceUID
               kviuid,//UI 19 SOPInstanceUID
               kvpuid,//UI 20 00080019 PyramidUID
               kvcuid,//UI 21 SOP​Instance​UID​Of​Concatenation​Source

kvTP,//22 AS DT TM DA text short ascii pair length
     kvpbirth,//23 DA patient birthdate
     kvedate,//24 DA StudyDate
          kvsdate,//25 DA
          kvstime,//26 TM

kvTA,//27 CS AE DS IS 13 text short ascii
     kvpsex,//28 CS patient sex
     kveat,//29 CS AccessionNumber type 00080051.00400033 (DNS,EUI64,ISO,URI,UUID,X400,X500,...)
          kvsmod,//30 CS 14 Modality
               kvitype,//31 CS 00080008 ImageType
               kvphotocode,//32 CS
   
          kvsnumber,//33 IS SeriesNumber
               kvianumber,//34 IS AcquisitionNumber
               kvinumber,//35 IS InstanceNumber
               kvframesnumber,//36 IS NumberOfFrames

kvTS,//37 LO LT SH ST text short charset
    kvpay,//38 LO insurance
    kvpide,//39 SH patient id extension
    kvpidr,//40 LO patient id root issuer
    kvimg,//41 LO InstitutionName 00080080
    kvedesc,//42 LO Study name
    kveid,//43 SH StudyID
    kvean,//44 SH AccessionNumber 00080050
    kvecode,//45 SQ/SH Study code 00080100,00080102
        kvsdesc,//46 LO Series name
        kvscdaid,//47 ST HL7InstanceIdentifier 0040E001  root^extension
        kvsdoctitle,//48 ST DocumentTitle 00420010
            kvicomment,//49 LO

kvPN,//50 PN person name has a special treatment in json and xml
    kvpname,//51 PN patient name
    kvcda,//52 PN CDA
    kvref,//53 PN referring
    kvreq,//54 PN requesting
   
kv01,  //55 other OB OD OF OL OV OW SV UV binary
   kvsdocument,//56 OB Encapsulated​Document 00420011 xml cda o pdf
   kvpixelOF, //57 OF 0x7FE00008
   kvpixelOD, //58 OD 0x7FE00009
   kvpixelOB, //59 OB 0x7FE00010
   kvpixelOW, //60 OW 0x7FE00010
   kvpixelOL, //61 OL 0x7E000010
   kvpixelOV, //62 OV 0x7E000010
   kvfo,//63 OV Extended​Offset​Table fragments offset 7FE00001
   kvfl,//64 OV Extended​Offset​TableLengths fragments offset 7FE00002
   kvft,//65 UV Encapsulated​Pixel​Data​Value​Total​Length 7FE00003
   
kvTL,//66 UC UT text long charset
     kveal,//67 UT AccessionNumberIssuer local 00080051.00400031
     kveau,//68 UT AccessionNumberIssuer universal 00080051.00400032

kvTU,//69 UR text long url-encoded
   
kvSA,//70 SQ head
kvIA,//71 item head
kvIZ,//72 item tail
kvSZ,//73 SQ tail

kvUN,//UN 74

};


#pragma mark - read blocking code

bool ifread(u32 bytesaskedfor);
bool ifreadattr(u8 kloc);


#pragma mark - TO BE IMPLEMENTED FOR EACH TARGET

//called after preliminar parsing of class, sop instance and transfer syntax
bool uCreate(
   u64 soloc,         // offset in vbuf for sop class
   u16 solen,         // length in vbuf for sop class
   u16 soidx,         // index in const char *scstr[]
   u64 siloc,         // offset in vbuf for sop instance uid
   u16 silen,         // length in vbuf for sop instance uid
   u64 stloc,         // offset in vbuf for transfer syntax
   u16 stlen,         // length in vbuf for transfer syntax
   u16 stidx          // index in const char *csstr[]
);

//called after parsing successfully all the attributes
bool uCommit(bool hastrailing);//aplica a todos los kv

//finalizes the opened tx
bool uClose(void);//aplica a todos los kv

//_DKVappend called for each parsed attribute, with value already read in vbuf or not.
//vbuf is 0xFFFE bytes long (máx short DICOM vl).

bool uAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);

#endif /* uapi_h */
