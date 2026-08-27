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
    kvFD,// 0 FD floating point double
    kvFL,// 1 FL floating point single
    kvSL,// 2 SL signed long
    kvSS,// 3 SS signed short
    kvUL,// 4 UL unsigned long
    kvUS,// 5 US unsigned short
    kvAT,// 6 AT attribute tag, 2 u16 hexa
    kvUI,// 7 UI unique ID possibly padded with 0x00
    kvUe,// 8 UI unique ID possibly padded with 0x00 for tag 0020000D study UID
    kvUs,// 9 UI unique ID possibly padded with 0x00 for tag 0020000E series UID
    kvUi,//10 UI unique ID possibly padded with 0x00 for tag 00080008 Instance UID
    kvUp,//11 UI unique ID possibly padded with 0x00 for tag 00080019 Pyramid UID
    kvTP,//12 AS DT TM DA text short ascii pair length
    kvTA,//13 AE DS IS 13 text short ascii
    kvCS,//14 CS Code String
    kvCs,//15 Cs Code String 00080005 read in main to enable charset control
    kvTS,//16 LO LT SH ST text short charset
    kvPN,//17 PN person name has a special treatment in json and xml
    kv01,//18 other OB OD OF OL OV OW SV UV binary
    kvTL,//19 UC UT text long charset
    kvTU,//20 UR text long url-encoded
    kvSA,//21 SQ head with length 0xffffffff terminated by end tag
    kvSa,//22 SQ head with specified length
    kvIA,//23 item head with length 0xffffffff terminated by end tag
    kvIa,//24 item head with specified length
    kvIZ,//25 item tail tag of item of undefined size
    kvIz,//26 item tail added
    kvSZ,//27 SQ tail tag of SQ of undefined size
    kvSz,//28 SQ tail added
    kvUN,//29 UN
    };


void input(int argc, char *argv[]);//opening of DICM
void trail(int argc, char *argv[]);//after dataset read

void key(struct Ercle* attr);//returns little endian of the key or 0 if error
void val(enum kvVRcategory vrcat,struct Ercle* attr);

#endif /* uapi_h */
