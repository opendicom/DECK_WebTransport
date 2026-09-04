// project: dicm2exit
// file: exittypes.h
// created by jacquesfauquex on 2024-04-04.


#ifndef exittypes_h
#define exittypes_h

#pragma mark - C

#include <stdio.h>  //puts() printf()
#include <stdbool.h>
#include <stdlib.h> //malloc() exit()
#include <string.h> //memcpy()
#include <ctype.h>
#include <unistd.h>
//#include <sys/time.h>
//#include <uuid/uuid.h>//for uuid_generate() and uuid_unparse()
#include <sys/stat.h>//for directory creation
#include <time.h>
//#include <sqlite3.h>
#include <errno.h>



//openjpeg
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>



//errno.h (>0) completed by specific error codes (<0)
typedef enum {
  exitZeroError=0,
  exitReadTruncated=-1,
  exitErrorFropenDICM=-2,
  exitProgramError=-3,
  exitErrorSQtruncated=-4,
  exitErrorITtruncated=-5,
  exitNotExplicitLittleEndian=-5,
  exitNoArg1FilePath=-6,
  exitErrorOutPath,//2
  exitErrorWrite,//3
  exitErrorCreateKV,//4
  exitSOPinstanceRejected,//5
  exitErrorParsing,//6
  exitNotDICM,//7
  exitErrorCommit,//8
  exitErrorClose,//9
  exitBadRepertoire,//10
  exitErrorDataset,//11
  exitErrorRead,//12
  exitNoDataset,//13
  exitNotEncapsulatedCDA,//15
  exitErrorFwrite,//16
 exitErrorSqliteOpen,//17
 exitErrorGroupLength,//18
  exitBadtrailingPadding//19
} exitcode;


typedef uint32_t ugLE;

typedef   int8_t s8;//%c
typedef  uint8_t u8;//%c
typedef  int16_t s16;//%hd
typedef uint16_t u16;//%hu
typedef  int32_t s32;//%d
typedef uint32_t u32;//%u
typedef  int64_t s64;//%lld
typedef uint64_t u64;//%llu



u8 intdecsize(int i);


struct Ercle {
  u32 E;//tag big Endian
  u16 r;//representation
  u16 c;//charset
  u32 l;//length (file size may be larger than u32 !!! little endian keeps u32 in bytes 8-11
  u32 e;//tag little endian
};

enum DICMvr {
   AE=0x4541,//multivalue application entity
   AS=0x5341,//multivalue age string
   AT=0x5441,//multivalue attribute tag
   CS=0x5343,//multivalue coded string
   DA=0x4144,//multivalue date
   DS=0x5344,//multivalue decimal string
   DT=0x5444,//multivalue date time
   FD=0x4446,//multivalue floating point double
   FL=0x4C46,//multivalue floating point single
   IS=0x5349,//multivalue integer string
   LO=0x4f4c,//multivalue long string ->UTF-8
   LT=0x544c,//long text ->UTF-
   PN=0x4e50,//multivalue person name ->UTF-8
   SH=0x4853,//multivalue short string ->UTF-8
   SL=0x4C53,//signed long
   SS=0x5353,//signed short
   ST=0x5453,//short text ->UTF-8
   TM=0x4d54,//multivalue time
   UI=0x4955,//multivalue unique ID ->b64shrink
   UL=0x4C55,//unsigned long
   US=0x5355,//unsigned short
   OB=0x424F,//other byte
   OD=0x444F,//other double
   OF=0x464F,//other float
   OL=0x4C4F,//other long
   OV=0x564F,//other 64-bit very long
   OW=0x574F,//multivalue other word
   SV=0x5653,//multivalue signed 64-bit very long
   UC=0x4355,//multivalue unlimited characters ->UTF-8
   UR=0x5255,//universal resource identifier/locator
   UT=0x5455,//unlimited text ->UTF-8
   UV=0x5655,//unsigned 64-bit very long
   UN=0x4E55,
   SQ=0x5153,//replaced by SA,IA,IZ SZ
   SA=0x0000,//OPENDICOM
   IA=0x2B2B,//OPENDICOM ++
   IZ=0x5F5F,//OPENDICOM __
   SZ=0xFFFF//OPENDICOM
};

#pragma mark - endianness
u64 u64swap(u64 x);
u32 u32swap(u32 x);
u16 u16swap(u16 x);


#pragma mark - uid shrink
extern char const hb[256];
extern char const b64char[64];
extern char const  base64EncodingTable[65];
extern char const  base64DecodingTable[128];
u32 ui2b64( char *ui, u8 uilength, char *b64 );
u32 ui2b64serialized( char *ui, u8 uilength, char *b64 );

#pragma mark - main & log



#pragma mark - repertoires

enum repertoireenum{
   
   REPERTOIRE_GL,
   ISO_IR100,
   ISO_IR101,
   ISO_IR109,
   ISO_IR110,
   ISO_IR148,
   ISO_IR126,
   ISO_IR127,
   
   ISO_IR192,
   RFC3986,
   ISO_IR13,
   ISO_IR144,
   ISO_IR138,
   ISO_IR166,
   GB18030,
   GBK,
   
   ISO2022IR6,
   ISO2022IR100,
   ISO2022IR101,
   ISO2022IR109,
   ISO2022IR110,
   ISO2022IR148,
   ISO2022IR126,
   ISO2022IR127,
   
   ISO2022IR87,
   ISO2022IR159,
   ISO2022IR13,
   ISO2022IR144,
   ISO2022IR138,
   ISO2022IR166,
   ISO2022IR149,
   ISO2022IR58
};


u32 repertoireidx( unsigned char *vbuf, u16 vallength );

u32 utf8( u32 repidx, const char *charbytes, size_t charlength, char *utf8bytes);
u32 utf8serialized( u32 repidx, const char *charbytes, size_t charlength, char *utf8bytes);


#pragma mark - SopClasses

//1.2.840.10008.5.1.4.1.1.
u8 isItImage(u64 u64Code, u8 idx, u8 median);

#pragma mark - TransfertSyntaxes


u8 tsidx( char *vbuf, u16 vallength );




#endif /* exittypes_h */
