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
kvFD,//FD 0 floating point double
kvFL,//FL 1 floating point single
kvSL,//SL 2 signed long
kvSS,//SS 3 signed short
kvUL,//UL 4 unsigned long
kvUS,//US 5 unsigned short
kvAT,//AT 6 attribute tag, 2 u16 hexa
kvUI,//UI 7 unique ID eventualmente terminado por 0x00
kvTP,//8 AS DT TM DA text short ascii pair length
kvTA,//9 AE DS IS 13 text short ascii
kvCS,//10 CS value read in main to enable charset adjustment
kvTS,//10 LO LT SH ST text short charset
kvPN,//11 PN person name has a special treatment in json and xml
kv01,//12 other OB OD OF OL OV OW SV UV binary
kvTL,//13 UC UT text long charset
kvTU,//14 UR text long url-encoded
kvSA,//15 SQ head
kvIA,//16 item head
kvIZ,//17 item tail
kvSZ,//18 SQ tail
kvUN,//19 UN
};


#pragma mark - read blocking code

bool vvRead(u32 bytesaskedfor);
bool kvRead(u32 bytesaskedfor, u32 kloc12);
bool kkRead(u8 kloc);


#pragma mark - TO BE IMPLEMENTED FOR EACH TARGET

bool vrAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen);

#pragma mark - transaction hooks

int uPrerequisite(u64 filesize, int argc, char *argv[]);

//called after preliminar parsing of class, sop instance and transfer syntax
int uCreate(int argc, char *argv[]);

//called after parsing successfully all the attributes
int uCommit(bool hastrailing,int argc, char *argv[]);

//finalizes the opened tx
void uClose(int argc, char *argv[]);


#endif /* uapi_h */
