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
    kvUI,//UI 7 unique ID possibly padded with 0x00
    kvTP,//8 AS DT TM DA text short ascii pair length
    kvTA,//9 AE DS IS 13 text short ascii
    kvCS,//10 CS Code String
    kvCs,//10 Cs Code String 00080005 read in main to enable charset control
    kvTS,//10 LO LT SH ST text short charset
    kvPN,//11 PN person name has a special treatment in json and xml
    kv01,//12 other OB OD OF OL OV OW SV UV binary
    kvTL,//13 UC UT text long charset
    kvTU,//14 UR text long url-encoded
    kvSA,//15 SQ head with length 0xffffffff terminated by end tag
    kvSa,//15 SQ head with specified length
    kvIA,//16 item head with length 0xffffffff terminated by end tag
    kvIa,//16 item head with specified length
    kvIZ,//17 item tail tag of item of undefined size
    kvIz,//17 item tail added
    kvSZ,//18 SQ tail tag of SQ of undefined size
    kvSz,//18 SQ tail added
    kvUN,//19 UN
    };


void input(int argc, char *argv[]);//CDICM opened
void key(struct Ercle* attr);//returns little endian of the key or 0 if error
void val(enum kvVRcategory vrcat,struct Ercle* attr);
void trail(int argc, char *argv[]);//after dataset read

#endif /* uapi_h */
