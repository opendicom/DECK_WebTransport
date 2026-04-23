// project: dicm2deck
// file: decktypes.c
// created by jacquesfauquex on 2024-04-04.

#include "datatypes.h"

#include <stdint.h>

#pragma mark - endianness

u8 intdecsize(int i)
{
   //4294967295 negative ...11 char
   if (i<0)
   {
      if (i>-10000)
      {
         if (i>-100)
         {
            if (i>-10) return 2;
            return 3;
         }
         else
         {
            if (i>-1000) return 4;
            return 5;
         }
      }
      else
      {
         if (i>-100000000)
         {
            if (i>-1000000)
            {
               if (i>-100000) return 6;
               return 7;
            }
            else
            {
               if (i>-10000000) return 8;
               return 9;
            }
         }
         else
         {
            if (i>-1000000000) return 10;
            return 11;
         }
      }
   }
   else
   {
      if (i<10000)
      {
         if (i<100)
         {
            if (i<10) return 1;
            return 2;
         }
         else
         {
            if (i<1000) return 3;
            return 4;
         }
      }
      else
      {
         if (i<100000000)
         {
            if (i<1000000)
            {
               if (i<100000) return 5;
               return 6;
            }
            else
            {
               if (i<10000000) return 7;
               return 8;
            }
         }
         else
         {
            if (i<1000000000) return 9;
            return 10;
         }
      }
   }
}
u64 u64swap(u64 x)
{
   return ((x>>56) & 0xff)
         |((x>>40) & 0xff00)
         |((x>>24) & 0xff0000)
         |((x>>8)  & 0xff000000)
         |((x<<8)  & 0xff00000000)
         |((x<<24) & 0xff0000000000)
         |((x<<40) & 0xff000000000000)
         |((x<<56) & 0xff00000000000000);
}

u32 u32swap(u32 x)
{
   return ((x>>24) & 0xff)
         |((x>>8)  & 0xff00)
         |((x<<8)  & 0xff0000)
         |((x<<24) & 0xff000000);
}

u16 u16swap(u16 x)
{
   return ((x>>8) & 0xff)
         |((x<<8) & 0xff00);
}

u32 tagswap(u32 x)
{
   return ((x & 0xff       <<16))
         |((x & 0xff00    << 8))
         |((x & 0xff0000  >>16))
         |((x & 0xff000000 >>24));
}

#pragma mark - uid shrink


const char  b64char[64]="-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

char u8u4( const char *byte_array, u8 *idx) {
    // returns half_byte corresponding to one char
    // updates idx
    
    // u4=half byte (0-15)
   
    // 0x0   1.2.840.10008.
    // 0x1   .
    // 0x2   0.
    // 0x3   0
    // 0x4   1.
    // 0x5   1
    // 0x6   2.
    // 0x7   2
    // 0x8   3.
    // 0x9   3
    // 0xA   4
    // 0xB   5
    // 0xC   6
    // 0xD   7
    // 0xE   8
    // 0xF   9
    
    char cur_byte = byte_array[*idx];
    switch (cur_byte) {
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *idx += 1;
            return cur_byte - 0x2A;
        case '.':
            *idx += 1;
            return 0x01;
        case '0':
        case '2':
        case '3': {
            if (byte_array[*idx+1] == '.')  {
                *idx += 2;
                return cur_byte + cur_byte - 0x5E;
            } else {
                *idx += 1;
                return cur_byte + cur_byte - 0x5D;
            }
        }
        case '1': {
            if (byte_array[*idx+1] == '.') {
                if (   sizeof(byte_array) - *idx > 14
                    && byte_array[*idx+2]  == '2'
                    && byte_array[*idx+3]  == '.'
                    && byte_array[*idx+4]  == '8'
                    && byte_array[*idx+5]  == '4'
                    && byte_array[*idx+6]  == '0'
                    && byte_array[*idx+7]  == '.'
                    && byte_array[*idx+8]  == '1'
                    && byte_array[*idx+9]  == '0'
                    && byte_array[*idx+10] == '0'
                    && byte_array[*idx+11] == '0'
                    && byte_array[*idx+12] == '8'
                    && byte_array[*idx+13] == '.'
                    )
                {
                    *idx += 14;
                    return 0x0;
                }
                else
                {
                    *idx += 2;
                    return 0x4;
                }
            }
            else
            {
                *idx += 1;
                return cur_byte + cur_byte - 0x5D;
            }
        }
       case ' ':
          *idx += 1;
          return 0x01;

        default:
            return 0xFF;//bad input char
    }
}

//buffer uid 66 bytes length
//buffer b64 44 bytes
bool ui2b64( char *ui, u8 uilength, char *b64, u8 *b64length )
{
   //input size limitations
   if ((uilength == 0) || (uilength > 64))
   {
      b64length = 0;
      return false;
   }
   
   //normalize overflow chars of last triad
   ui[uilength]=' ';
   ui[uilength+1]=' ';
   
   //loop
   *b64length=0;
   unsigned char u4a,u4b,u4c;
   u8 uiidx=0;
   while (uiidx < uilength)
   {
      //read 3 half chars
      u4a=u8u4(ui,&uiidx);
      if (u4a > 0x10) return false;
      u4b=u8u4(ui,&uiidx);
      if (u4b > 0x10) return false;
      u4c=u8u4(ui,&uiidx);
      if (u4c > 0x10) return false;
      
      //write 3 b64 chars
      b64[*b64length]=b64char[(u4a<<2) + (u4b>>2)];
      *b64length+=1;
      b64[*b64length]=b64char[((u4b & 0x03) << 4) + u4c];
      *b64length+=1;
   }
   return true;
}


#pragma mark - main & log

enum DIWEFenum DIWEF;
bool loglevel(const char * logletter)
{
   switch ((int)*logletter) {
      case 'D':
         DIWEF=D;
         break;
      case 'I':
         DIWEF=I;
         break;
      case 'W':
         DIWEF=W;
         break;
      case 'E':
         DIWEF=E;
         break;
      case 'F':
         DIWEF=F;
         break;
      default:
         return false;
   }
   return true;
}


#pragma mark - repertoires

const char *repertoirestr[]={
   //one code
   "",                //empty
   "ISO_IR 100",      //latin1
   "ISO_IR 101",      //latin2
   "ISO_IR 109",      //latin3
   "ISO_IR 110",      //latin4
   "ISO_IR 148",      //latin5
   "ISO_IR 126",      //greek
   "ISO_IR 127",      //arabic

   "ISO_IR 192",      //utf-8 (multi byte)
   "RFC3986 ",         //for UR (not a DICOM defined code)
   "ISO_IR 13 ",      //japanese
   "ISO_IR 144",      //cyrilic
   "ISO_IR 138",      //hebrew
   "ISO_IR 166",      //thai
   "GB18030 ",        //chinese  (multi byte)
   "GBK ",            //chinese simplified  (multi byte)
   //code extension
   "ISO 2022 IR 6",  //default
   "ISO 2022 IR 100",//latin1
   "ISO 2022 IR 101",//latin2
   "ISO 2022 IR 109",//latin3
   "ISO 2022 IR 110",//latin4
   "ISO 2022 IR 148",//latin5
   "ISO 2022 IR 126",//greek
   "ISO 2022 IR 127",//arabic
   "ISO 2022 IR 87",  //japanese (multi byte)
   "ISO 2022 IR 159",//japanese (multi byte)
   "ISO 2022 IR 13",  //japanese
   "ISO 2022 IR 144",//cyrilic
   "ISO 2022 IR 138",//hebrew
   "ISO 2022 IR 166",//thai
   "ISO 2022 IR 149",//korean (multi byte)
   "ISO 2022 IR 58"   //chinese simplified (multi byte)
};

u32 repertoireidx( char *vbuf, u16 vallength )
{
   u32 idx=0x9;//error
   switch (vallength){
      case 0: idx=REPERTOIRE_GL;break;
      case 4: idx=GBK;break;
      case 8: idx=GB18030;break;
      case 10:{
         switch ((vbuf[9]<<8) | vbuf[8]) {
            case 0x3030: idx=ISO_IR100;break;//latin1
            case 0x3130: idx=ISO_IR101;break;//latin2
            case 0x3930: idx=ISO_IR109;break;//latin3
            case 0x3031: idx=ISO_IR110;break;//latin4
            case 0x3834: idx=ISO_IR148;break;//latin5
            case 0x3632: idx=ISO_IR126;break;//greek
            case 0x3732: idx=ISO_IR127;break;//arabic
            case 0x3239: idx=ISO_IR192;break;//utf-8
            case 0x2033: idx=ISO_IR13;break;//japones
               //return RFC3986
            case 0x3434: idx=ISO_IR144;break;//cyrilic
            case 0x3833: idx=ISO_IR138;break;//hebrew
            case 0x3636: idx=ISO_IR166;break;//thai
         }
      }
      case 14:{
#pragma mark TODO
      }
      case 16:{
#pragma mark TODO
      }
      default:{
#pragma mark TODO encoding extension
      }
   }
   if (strncmp(repertoirestr[idx],(char*)vbuf,vallength)==0) return idx;
   else return 0x9;//error
}

/*
 0 REPERTOIRE_GL,
 1 ISO_IR100,
 2 ISO_IR101,
 3 ISO_IR109,
 4 ISO_IR110,
 5 ISO_IR148,
 6 ISO_IR126,
 7 ISO_IR127,
 
 8 ISO_IR192,
 9 RFC3986,
 10 ISO_IR13,
 11 ISO_IR144,
 12 ISO_IR138,
 13 ISO_IR166,
 14 GB18030,
 15 GBK,
 
 16 ISO2022IR6,
 17 ISO2022IR100,
 18 ISO2022IR101,
 19 ISO2022IR109,
 20 ISO2022IR110,
 21 ISO2022IR148,
 22 ISO2022IR126,
 23 ISO2022IR127,
 
 24 ISO2022IR87,
 25 ISO2022IR159,
 26 ISO2022IR13,
 27 ISO2022IR144
 28 ISO2022IR138,
 29 ISO2022IR166,
 30 ISO2022IR149,
 31 ISO2022IR58

 */

#pragma mark TODO special treatment for utf-8 and others
const uint16_t repertoires2utf8[16][256] =
{
   //0 REPERTOIRE_GL
   {
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
  0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
  0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
  0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
  0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
  0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
  0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
   },   //8859-1 ISO_IR100
   {
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
  0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
  0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
  0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
  0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
  0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
  0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
   },
   //8859-2 ISO_IR101
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0x0104, 0x02d8, 0x0141, 0x00a4, 0x013d, 0x015a, 0x00a7, 0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,
     0x00b0, 0x0105, 0x02db, 0x0142, 0x00b4, 0x013e, 0x015b, 0x02c7, 0x00b8, 0x0161, 0x015f, 0x0165, 0x017a, 0x02dd, 0x017e, 0x017c,
     0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
     0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7, 0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
     0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
     0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7, 0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
   },
   //8859-3 ISO_IR109
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0x0126, 0x02d8, 0x00a3, 0x00a4, 0xffff, 0x0124, 0x00a7, 0x00a8, 0x0130, 0x015e, 0x011e, 0x0134, 0x00ad, 0xffff, 0x017b,
     0x00b0, 0x0127, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x0125, 0x00b7, 0x00b8, 0x0131, 0x015f, 0x011f, 0x0135, 0x00bd, 0xffff, 0x017c,
     0x00c0, 0x00c1, 0x00c2, 0xffff, 0x00c4, 0x010a, 0x0108, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
     0xffff, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x0120, 0x00d6, 0x00d7, 0x011c, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x016c, 0x015c, 0x00df,
     0x00e0, 0x00e1, 0x00e2, 0xffff, 0x00e4, 0x010b, 0x0109, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
     0xffff, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x0121, 0x00f6, 0x00f7, 0x011d, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x016d, 0x015d, 0x02d9,
   },
   //8859-4 ISO_IR110
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0x0104, 0x0138, 0x0156, 0x00a4, 0x0128, 0x013b, 0x00a7, 0x00a8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00ad, 0x017d, 0x00af,
     0x00b0, 0x0105, 0x02db, 0x0157, 0x00b4, 0x0129, 0x013c, 0x02c7, 0x00b8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014a, 0x017e, 0x014b,
     0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x012a,
     0x0110, 0x0145, 0x014c, 0x0136, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x0168, 0x016a, 0x00df,
     0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x012b,
     0x0111, 0x0146, 0x014d, 0x0137, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x0169, 0x016b, 0x02d9,
   },
   //5 ISO_IR148,
   {},
   //6 ISO_IR126,greek latin7
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0x2018, 0x2019, 0x00a3, 0x20ac, 0x20af, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x037a, 0x00ab, 0x00ac, 0x00ad, 0xffff, 0x2015,
     0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x0385, 0x0386, 0x00b7, 0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
     0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
     0x03a0, 0x03a1, 0xffff, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7, 0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
     0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
     0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0xffff,
   },
   //7 ISO_IR127,arabic latin6
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0xffff, 0xffff, 0xffff, 0x00a4, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x060c, 0x00ad, 0xffff, 0xffff,
     0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x061b, 0xffff, 0xffff, 0xffff, 0x061f,
     0xffff, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
     0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063a, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
     0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064a, 0x064b, 0x064c, 0x064d, 0x064e, 0x064f,
     0x0650, 0x0651, 0x0652, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
   },
   //8 ISO_IR192,
   {},
   //9 RFC3986,
   {},
   //10 ISO_IR13,
   {},
   //11 ISO_IR144,cyrilic latin5
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x00ad, 0x040e, 0x040f,
     0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
     0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
     0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
     0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
     0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x00a7, 0x045e, 0x045f,
   },
   //12 ISO_IR138,hebrew latin8
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0xffff, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
     0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0xffff,
     0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
     0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x2017,
     0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7, 0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
     0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7, 0x05e8, 0x05e9, 0x05ea, 0xffff, 0xffff, 0x200e, 0x200f, 0xffff,
   },
   //13 ISO_IR166,thai latin11
   {
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
     0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
     0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
     0x00a0, 0x0e01, 0x0e02, 0x0e03, 0x0e04, 0x0e05, 0x0e06, 0x0e07, 0x0e08, 0x0e09, 0x0e0a, 0x0e0b, 0x0e0c, 0x0e0d, 0x0e0e, 0x0e0f,
     0x0e10, 0x0e11, 0x0e12, 0x0e13, 0x0e14, 0x0e15, 0x0e16, 0x0e17, 0x0e18, 0x0e19, 0x0e1a, 0x0e1b, 0x0e1c, 0x0e1d, 0x0e1e, 0x0e1f,
     0x0e20, 0x0e21, 0x0e22, 0x0e23, 0x0e24, 0x0e25, 0x0e26, 0x0e27, 0x0e28, 0x0e29, 0x0e2a, 0x0e2b, 0x0e2c, 0x0e2d, 0x0e2e, 0x0e2f,
     0x0e30, 0x0e31, 0x0e32, 0x0e33, 0x0e34, 0x0e35, 0x0e36, 0x0e37, 0x0e38, 0x0e39, 0x0e3a, 0xffff, 0xffff, 0xffff, 0xffff, 0x0e3f,
     0x0e40, 0x0e41, 0x0e42, 0x0e43, 0x0e44, 0x0e45, 0x0e46, 0x0e47, 0x0e48, 0x0e49, 0x0e4a, 0x0e4b, 0x0e4c, 0x0e4d, 0x0e4e, 0x0e4f,
     0x0e50, 0x0e51, 0x0e52, 0x0e53, 0x0e54, 0x0e55, 0x0e56, 0x0e57, 0x0e58, 0x0e59, 0x0e5a, 0x0e5b, 0xffff, 0xffff, 0xffff, 0xffff,
   }

   //14 GB18030,
   //15 GBK,
   
   //16 ISO2022IR6,
   //17 ISO2022IR100,
   //18 ISO2022IR101,
   //19 ISO2022IR109,
   //20 ISO2022IR110,
   //21 ISO2022IR148,
   //22 ISO2022IR126,
   //23 ISO2022IR127,
   
   //24 ISO2022IR87,
   //25 ISO2022IR159,
   //26 ISO2022IR13,
   //27 ISO2022IR144,
   //28 ISO2022IR138,
   //29 ISO2022IR166,
   //30 ISO2022IR149,
   //31 ISO2022IR58
};

//latin no dicom
/*
 //turkish_iso_8859_9
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
   0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
   0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff,
 }

 
 //latin 10
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
   0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
   0x00a0, 0x0104, 0x0112, 0x0122, 0x012a, 0x0128, 0x0136, 0x00a7, 0x013b, 0x0110, 0x0160, 0x0166, 0x017d, 0x00ad, 0x016a, 0x014a,
   0x00b0, 0x0105, 0x0113, 0x0123, 0x012b, 0x0129, 0x0137, 0x00b7, 0x013c, 0x0111, 0x0161, 0x0167, 0x017e, 0x2015, 0x016b, 0x014b,
   0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x00cf,
   0x00d0, 0x0145, 0x014c, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x0168, 0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
   0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x00ef,
   0x00f0, 0x0146, 0x014d, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x0169, 0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x0138,
 },
 

 //baltic rim_iso_8859_13
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
   0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
   0x00a0, 0x201d, 0x00a2, 0x00a3, 0x00a4, 0x201e, 0x00a6, 0x00a7, 0x00d8, 0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x201c, 0x00b5, 0x00b6, 0x00b7, 0x00f8, 0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6,
   0x0104, 0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112, 0x010c, 0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b,
   0x0160, 0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7, 0x0172, 0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df,
   0x0105, 0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113, 0x010d, 0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c,
   0x0161, 0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7, 0x0173, 0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x2019,
 }

 //celtic_iso_8859_14
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
   0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
   0x00a0, 0x1e02, 0x1e03, 0x00a3, 0x010a, 0x010b, 0x1e0a, 0x00a7, 0x1e80, 0x00a9, 0x1e82, 0x1e0b, 0x1ef2, 0x00ad, 0x00ae, 0x0178,
   0x1e1e, 0x1e1f, 0x0120, 0x0121, 0x1e40, 0x1e41, 0x00b6, 0x1e56, 0x1e81, 0x1e57, 0x1e83, 0x1e60, 0x1ef3, 0x1e84, 0x1e85, 0x1e61,
   0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   0x0174, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x1e6a, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x0176, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   0x0175, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x1e6b, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x0177, 0x00ff,
 }

 //latin9_iso_8859_15_utf8[256] (A revision of 8859-1 that removes some little-used symbols, replacing them with the euro sign € and the letters Š, š, Ž, ž, Œ, œ, and Ÿ, which completes the coverage of French, Finnish and Estonian)
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
   0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
   0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20ac, 0x00a5, 0x0160, 0x00a7, 0x0161, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x017d, 0x00b5, 0x00b6, 0x00b7, 0x017e, 0x00b9, 0x00ba, 0x00bb, 0x0152, 0x0153, 0x0178, 0x00bf,
   0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,
 }

 //latin10_iso_8859_16 ( Albanian, Croatian, Hungarian, Italian, Polish, Romanian and Slovene, but also Finnish, French, German and Irish Gaelic (new orthography). The focus lies more on letters than symbols. The generic currency sign is replaced with the euro sign.)
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
   0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
   0x00a0, 0x0104, 0x0105, 0x0141, 0x20ac, 0x201e, 0x0160, 0x00a7, 0x0161, 0x00a9, 0x0218, 0x00ab, 0x0179, 0x00ad, 0x017a, 0x017b,
   0x00b0, 0x00b1, 0x010c, 0x0142, 0x017d, 0x201d, 0x00b6, 0x00b7, 0x017e, 0x010d, 0x0219, 0x00bb, 0x0152, 0x0153, 0x0178, 0x017c,
   0x00c0, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0106, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   0x0110, 0x0143, 0x00d2, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x015a, 0x0170, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0118, 0x021a, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x0107, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   0x0111, 0x0144, 0x00f2, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x015b, 0x0171, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0119, 0x021b, 0x00ff,
 }

 */

//windows no dicom
/*
 windows_1250
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0xffff, 0x201e, 0x2026, 0x2020, 0x2021, 0xffff, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0xffff, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
   0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
   0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
   0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
   0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7, 0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
   0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
   0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7, 0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
 };

 windows_1251
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021, 0x20ac, 0x2030, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
   0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0xffff, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
   0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7, 0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407,
   0x00b0, 0x00b1, 0x0406, 0x0456, 0x0491, 0x00b5, 0x00b6, 0x00b7, 0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457,
   0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
   0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
   0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
   0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
 };

 windows_1252
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xffff, 0x017d, 0xffff,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xffff, 0x017e, 0x0178,
   0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,
 };

 windows_1253
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0xffff, 0x2030, 0xffff, 0x2039, 0xffff, 0xffff, 0xffff, 0xffff,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0xffff, 0x2122, 0xffff, 0x203a, 0xffff, 0xffff, 0xffff, 0xffff,
   0x00a0, 0x0385, 0x0386, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0xffff, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x2015,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x00b5, 0x00b6, 0x00b7, 0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
   0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
   0x03a0, 0x03a1, 0xffff, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7, 0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
   0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
   0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0xffff,
 };

 windows_1254
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xffff, 0xffff, 0xffff,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xffff, 0xffff, 0x0178,
   0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
   0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
   0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff,
 };

 windows_1255
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0xffff, 0x2039, 0xffff, 0xffff, 0xffff, 0xffff,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0xffff, 0x203a, 0xffff, 0xffff, 0xffff, 0xffff,
   0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20aa, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   0x05b0, 0x05b1, 0x05b2, 0x05b3, 0x05b4, 0x05b5, 0x05b6, 0x05b7, 0x05b8, 0x05b9, 0xffff, 0x05bb, 0x05bc, 0x05bd, 0x05be, 0x05bf,
   0x05c0, 0x05c1, 0x05c2, 0x05c3, 0x05f0, 0x05f1, 0x05f2, 0x05f3, 0x05f4, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
   0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7, 0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
   0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7, 0x05e8, 0x05e9, 0x05ea, 0xffff, 0xffff, 0x200e, 0x200f, 0xffff,
 };

 windows_1256
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0x067e, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
   0x06af, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x06a9, 0x2122, 0x0691, 0x203a, 0x0153, 0x200c, 0x200d, 0x06ba,
   0x00a0, 0x060c, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x06be, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x061b, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x061f,
   0x06c1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
   0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00d7, 0x0637, 0x0638, 0x0639, 0x063a, 0x0640, 0x0641, 0x0642, 0x0643,
   0x00e0, 0x0644, 0x00e2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x0649, 0x064a, 0x00ee, 0x00ef,
   0x064b, 0x064c, 0x064d, 0x064e, 0x00f4, 0x064f, 0x0650, 0x00f7, 0x0651, 0x00f9, 0x0652, 0x00fb, 0x00fc, 0x200e, 0x200f, 0x06d2,
 };

 windows_1257
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0xffff, 0x201e, 0x2026, 0x2020, 0x2021, 0xffff, 0x2030, 0xffff, 0x2039, 0xffff, 0x00a8, 0x02c7, 0x00b8,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0xffff, 0x2122, 0xffff, 0x203a, 0xffff, 0x00af, 0x02db, 0xffff,
   0x00a0, 0xffff, 0x00a2, 0x00a3, 0x00a4, 0xffff, 0x00a6, 0x00a7, 0x00d8, 0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00f8, 0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6,
   0x0104, 0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112, 0x010c, 0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b,
   0x0160, 0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7, 0x0172, 0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df,
   0x0105, 0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113, 0x010d, 0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c,
   0x0161, 0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7, 0x0173, 0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x02d9,
 };

 windows_1258
 {
   0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
   0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
   0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
   0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
   0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
   0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
   0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
   0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
   0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0xffff, 0x2039, 0x0152, 0xffff, 0xffff, 0xffff,
   0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0xffff, 0x203a, 0x0153, 0xffff, 0xffff, 0x0178,
   0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
   0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
   0x00c0, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x0300, 0x00cd, 0x00ce, 0x00cf,
   0x0110, 0x00d1, 0x0309, 0x00d3, 0x00d4, 0x01a0, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x01af, 0x0303, 0x00df,
   0x00e0, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x0301, 0x00ed, 0x00ee, 0x00ef,
   0x0111, 0x00f1, 0x0323, 0x00f3, 0x00f4, 0x01a1, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x01b0, 0x20ab, 0x00ff,
 };

 */

//https://github.com/nigels-com/tutf8e/blob/master/src/tutf8e.c
bool utf8
(
 u32 repidx,
 const char *charbytes,
 const u32   charstart,
 size_t      charlength,
 
 char      *utf8bytes,
 u32        utf8start,
 u32       *utf8length
)
{
  *utf8length=0;
  uint16_t u;
  for (size_t cursor = 0; cursor < charlength; cursor++) {
    u = repertoires2utf8[repidx][(u8)charbytes[charstart + cursor]];
    if (u<0x80) {
      utf8bytes[utf8start + (*utf8length)++] = u;
      continue;
    }
    if (u<0x800) {
      utf8bytes[utf8start + (*utf8length)++] = 0xc0 | (u>>6);
      utf8bytes[utf8start + (*utf8length)++] = 0x80 | (u&0x3f);
      continue;
    }
    if (u<0xffff) {
      utf8bytes[utf8start + (*utf8length)++] = 0xe0 | (u>>12);
      utf8bytes[utf8start + (*utf8length)++] = 0x80 | ((u>>6)&0x3f);
      utf8bytes[utf8start + (*utf8length)++] = 0x80 | (u&0x3f);
    }
  }
  utf8bytes[utf8start + (*utf8length)] = 0x00;
  return true;
}

#pragma mark - SopClasses

const char *scstr[]={
"1.2.840.10008.1.1",//Verification
"1.2.840.10008.1.3.10",//MediaStorageDirectoryStorage
"1.2.840.10008.1.9",//BasicStudyContentNotification
"1.2.840.10008.1.20.1",//StorageCommitmentPushModel
"1.2.840.10008.1.20.2",//StorageCommitmentPullModel (Retired)
"1.2.840.10008.1.40",//ProceduralEventLogging"
"1.2.840.10008.1.42",//SubstanceAdministrationLogging
"1.2.840.10008.3.1.2.1.1",//DetachedPatientManagement (Retired)
"1.2.840.10008.3.1.2.1.4",//DetachedPatientManagementMeta (Retired)
"1.2.840.10008.3.1.2.2.1",//DetachedVisitManagement (Retired)
"1.2.840.10008.3.1.2.3.1",//DetachedStudyManagement (Retired)
"1.2.840.10008.3.1.2.3.2",//StudyComponentManagement (Retired)
"1.2.840.10008.3.1.2.3.3",//ModalityPerformedProcedureStep
"1.2.840.10008.3.1.2.3.4",//ModalityPerformedProcedureStepRetrieve
"1.2.840.10008.3.1.2.3.5",//ModalityPerformedProcedureStepNotification
"1.2.840.10008.3.1.2.5.1",//DetachedResultsManagement (Retired)
"1.2.840.10008.3.1.2.5.4",//DetachedResultsManagementMeta (Retired)
"1.2.840.10008.3.1.2.5.5",//DetachedStudyManagementMeta (Retired)
"1.2.840.10008.3.1.2.6.1",//DetachedInterpretationManagement (Retired)
"1.2.840.10008.5.1.1.1",//BasicFilmSession
"1.2.840.10008.5.1.1.2",//BasicFilmBox
"1.2.840.10008.5.1.1.4",//BasicGrayscaleImageBox
"1.2.840.10008.5.1.1.4.1",//BasicColorImageBox
"1.2.840.10008.5.1.1.4.2",//ReferencedImageBox
"1.2.840.10008.5.1.1.9",//BasicGrayscalePrintManagementMeta
"1.2.840.10008.5.1.1.9.1",//ReferencedGrayscalePrintManagementMeta
"1.2.840.10008.5.1.1.14",//PrintJob
"1.2.840.10008.5.1.1.15",//BasicAnnotationBox
"1.2.840.10008.5.1.1.16",//Printer
"1.2.840.10008.5.1.1.16.376",//PrinterConfigurationRetrieval
"1.2.840.10008.5.1.1.18",//BasicColorPrintManagementMeta
"1.2.840.10008.5.1.1.18.1",//ReferencedColorPrintManagementMeta (Retired)
"1.2.840.10008.5.1.1.22",//VOILUTBox
"1.2.840.10008.5.1.1.23",//PresentationLUT
"1.2.840.10008.5.1.1.24",//ImageOverlayBox (Retired)
"1.2.840.10008.5.1.1.24.1",//BasicPrintImageOverlayBox (Retired)
"1.2.840.10008.5.1.1.26",//PrintQueueManagement (Retired)
"1.2.840.10008.5.1.1.27",//StoredPrintStorage (Retired)
"1.2.840.10008.5.1.1.29",//HardcopyGrayscaleImageStorage (Retired)
"1.2.840.10008.5.1.1.30",//HardcopyColorImageStorage (Retired)
"1.2.840.10008.5.1.1.31",//PullPrintRequest (Retired)
"1.2.840.10008.5.1.1.32",//PullStoredPrintManagementMeta (Retired)
"1.2.840.10008.5.1.1.33",//MediaCreationManagement
"1.2.840.10008.5.1.1.40",//DisplaySystem
"1.2.840.10008.5.1.4.1.1.1",//ComputedRadiographyImageStorage
"1.2.840.10008.5.1.4.1.1.1.1",//DigitalXRayImageStorageForPresentation
"1.2.840.10008.5.1.4.1.1.1.1.1",//DigitalXRayImageStorageForProcessing
"1.2.840.10008.5.1.4.1.1.1.2",//DigitalMammographyXRayImageStorageForPresentation
"1.2.840.10008.5.1.4.1.1.1.2.1",//DigitalMammographyXRayImageStorageForProcessing
"1.2.840.10008.5.1.4.1.1.1.3",//DigitalIntraOralXRayImageStorageForPresentation
"1.2.840.10008.5.1.4.1.1.1.3.1",//DigitalIntraOralXRayImageStorageForProcessing
"1.2.840.10008.5.1.4.1.1.2",//CTImageStorage
"1.2.840.10008.5.1.4.1.1.2.1",//EnhancedCTImageStorage
"1.2.840.10008.5.1.4.1.1.2.2",//LegacyConvertedEnhancedCTImageStorage
"1.2.840.10008.5.1.4.1.1.3",//UltrasoundMultiFrameImageStorageRetired (Retired)
"1.2.840.10008.5.1.4.1.1.3.1",//UltrasoundMultiFrameImageStorage
"1.2.840.10008.5.1.4.1.1.4",//MRImageStorage
"1.2.840.10008.5.1.4.1.1.4.1",//EnhancedMRImageStorage
"1.2.840.10008.5.1.4.1.1.4.2",//MRSpectroscopyStorage
"1.2.840.10008.5.1.4.1.1.4.3",//EnhancedMRColorImageStorage
"1.2.840.10008.5.1.4.1.1.4.4",//LegacyConvertedEnhancedMRImageStorage
"1.2.840.10008.5.1.4.1.1.5",//NuclearMedicineImageStorageRetired (Retired)
"1.2.840.10008.5.1.4.1.1.6",//UltrasoundImageStorageRetired (Retired)
"1.2.840.10008.5.1.4.1.1.6.1",//UltrasoundImageStorage
"1.2.840.10008.5.1.4.1.1.6.2",//EnhancedUSVolumeStorage
"1.2.840.10008.5.1.4.1.1.6.3",//PhotoacousticImageStorage
"1.2.840.10008.5.1.4.1.1.7",//SecondaryCaptureImageStorage
"1.2.840.10008.5.1.4.1.1.7.1",//MultiFrameSingleBitSecondaryCaptureImageStorage
"1.2.840.10008.5.1.4.1.1.7.2",//MultiFrameGrayscaleByteSecondaryCaptureImageStorage
"1.2.840.10008.5.1.4.1.1.7.3",//MultiFrameGrayscaleWordSecondaryCaptureImageStorage
"1.2.840.10008.5.1.4.1.1.7.4",//MultiFrameTrueColorSecondaryCaptureImageStorage
"1.2.840.10008.5.1.4.1.1.8",//StandaloneOverlayStorage (Retired)
"1.2.840.10008.5.1.4.1.1.9",//StandaloneCurveStorage (Retired)
"1.2.840.10008.5.1.4.1.1.9.1",//WaveformStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.9.1.1",//TwelveLeadECGWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.1.2",//GeneralECGWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.1.3",//AmbulatoryECGWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.1.4",//General32bitECGWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.2.1",//HemodynamicWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.3.1",//CardiacElectrophysiologyWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.4.1",//BasicVoiceAudioWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.4.2",//GeneralAudioWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.5.1",//ArterialPulseWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.6.1",//RespiratoryWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.6.2",//MultichannelRespiratoryWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.7.1",//RoutineScalpElectroencephalogramWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.7.2",//ElectromyogramWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.7.3",//ElectrooculogramWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.7.4",//SleepElectroencephalogramWaveformStorage
"1.2.840.10008.5.1.4.1.1.9.8.1",//BodyPositionWaveformStorage
"1.2.840.10008.5.1.4.1.1.10",//StandaloneModalityLUTStorage (Retired)
"1.2.840.10008.5.1.4.1.1.11",//StandaloneVOILUTStorage (Retired)
"1.2.840.10008.5.1.4.1.1.11.1",//GrayscaleSoftcopyPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.2",//ColorSoftcopyPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.3",//PseudoColorSoftcopyPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.4",//BlendingSoftcopyPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.5",//XAXRFGrayscaleSoftcopyPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.6",//GrayscalePlanarMPRVolumetricPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.7",//CompositingPlanarMPRVolumetricPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.8",//AdvancedBlendingPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.9",//VolumeRenderingVolumetricPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.10",//SegmentedVolumeRenderingVolumetricPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.11",//MultipleVolumeRenderingVolumetricPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.11.12",//VariableModalityLUTSoftcopyPresentationStateStorage
"1.2.840.10008.5.1.4.1.1.12.1",//XRayAngiographicImageStorage
"1.2.840.10008.5.1.4.1.1.12.1.1",//EnhancedXAImageStorage
"1.2.840.10008.5.1.4.1.1.12.2",//XRayRadiofluoroscopicImageStorage
"1.2.840.10008.5.1.4.1.1.12.2.1",//EnhancedXRFImageStorage
"1.2.840.10008.5.1.4.1.1.12.3",//XRayAngiographicBiPlaneImageStorage (Retired)
"1.2.840.10008.5.1.4.1.1.12.77",//(Retired)
"1.2.840.10008.5.1.4.1.1.13.1.1",//XRay3DAngiographicImageStorage
"1.2.840.10008.5.1.4.1.1.13.1.2",//XRay3DCraniofacialImageStorage
"1.2.840.10008.5.1.4.1.1.13.1.3",//BreastTomosynthesisImageStorage
"1.2.840.10008.5.1.4.1.1.13.1.4",//BreastProjectionXRayImageStorageForPresentation
"1.2.840.10008.5.1.4.1.1.13.1.5",//BreastProjectionXRayImageStorageForProcessing
"1.2.840.10008.5.1.4.1.1.14.1",//IntravascularOpticalCoherenceTomographyImageStorageForPresentation
"1.2.840.10008.5.1.4.1.1.14.2",//IntravascularOpticalCoherenceTomographyImageStorageForProcessing
"1.2.840.10008.5.1.4.1.1.20",//NuclearMedicineImageStorage
"1.2.840.10008.5.1.4.1.1.30",//ParametricMapStorage
"1.2.840.10008.5.1.4.1.1.40",//(Retired)
"1.2.840.10008.5.1.4.1.1.66",//RawDataStorage
"1.2.840.10008.5.1.4.1.1.66.1",//SpatialRegistrationStorage
"1.2.840.10008.5.1.4.1.1.66.2",//SpatialFiducialsStorage
"1.2.840.10008.5.1.4.1.1.66.3",//DeformableSpatialRegistrationStorage
"1.2.840.10008.5.1.4.1.1.66.4",//SegmentationStorage
"1.2.840.10008.5.1.4.1.1.66.5",//SurfaceSegmentationStorage
"1.2.840.10008.5.1.4.1.1.66.6",//TractographyResultsStorage
"1.2.840.10008.5.1.4.1.1.67",//RealWorldValueMappingStorage
"1.2.840.10008.5.1.4.1.1.68.1",//SurfaceScanMeshStorage
"1.2.840.10008.5.1.4.1.1.68.2",//SurfaceScanPointCloudStorage
"1.2.840.10008.5.1.4.1.1.77.1",//VLImageStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.77.2",//VLMultiFrameImageStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.77.1.1",//VLEndoscopicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.1.1",//VideoEndoscopicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.2",//VLMicroscopicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.2.1",//VideoMicroscopicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.3",//VLSlideCoordinatesMicroscopicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.4",//VLPhotographicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.4.1",//VideoPhotographicImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.1",//OphthalmicPhotography8BitImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.2",//OphthalmicPhotography16BitImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.3",//StereometricRelationshipStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.4",//OphthalmicTomographyImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.5",//WideFieldOphthalmicPhotographyStereographicProjectionImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.6",//WideFieldOphthalmicPhotography3DCoordinatesImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.7",//OphthalmicOpticalCoherenceTomographyEnFaceImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.5.8",//OphthalmicOpticalCoherenceTomographyBscanVolumeAnalysisStorage
"1.2.840.10008.5.1.4.1.1.77.1.6",//VLWholeSlideMicroscopyImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.7",//DermoscopicPhotographyImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.8",//ConfocalMicroscopyImageStorage
"1.2.840.10008.5.1.4.1.1.77.1.9",//ConfocalMicroscopyTiledPyramidalImageStorage
"1.2.840.10008.5.1.4.1.1.78.1",//LensometryMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.78.2",//AutorefractionMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.78.3",//KeratometryMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.78.4",//SubjectiveRefractionMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.78.5",//VisualAcuityMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.78.6",//SpectaclePrescriptionReportStorage
"1.2.840.10008.5.1.4.1.1.78.7",//OphthalmicAxialMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.78.8",//IntraocularLensCalculationsStorage
"1.2.840.10008.5.1.4.1.1.79.1",//MacularGridThicknessAndVolumeReportStorage
"1.2.840.10008.5.1.4.1.1.80.1",//OphthalmicVisualFieldStaticPerimetryMeasurementsStorage
"1.2.840.10008.5.1.4.1.1.81.1",//OphthalmicThicknessMapStorage
"1.2.840.10008.5.1.4.1.1.82.1",//CornealTopographyMapStorage
"1.2.840.10008.5.1.4.1.1.88.1",//TextSRStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.88.2",//AudioSRStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.88.3",//DetailSRStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.88.4",//ComprehensiveSRStorageTrial (Retired)
"1.2.840.10008.5.1.4.1.1.88.11",//BasicTextSRStorage
"1.2.840.10008.5.1.4.1.1.88.22",//EnhancedSRStorage
"1.2.840.10008.5.1.4.1.1.88.33",//ComprehensiveSRStorage
"1.2.840.10008.5.1.4.1.1.88.34",//Comprehensive3DSRStorage
"1.2.840.10008.5.1.4.1.1.88.35",//ExtensibleSRStorage
"1.2.840.10008.5.1.4.1.1.88.40",//ProcedureLogStorage
"1.2.840.10008.5.1.4.1.1.88.50",//MammographyCADSRStorage
"1.2.840.10008.5.1.4.1.1.88.59",//KeyObjectSelectionDocumentStorage
"1.2.840.10008.5.1.4.1.1.88.65",//ChestCADSRStorage
"1.2.840.10008.5.1.4.1.1.88.67",//XRayRadiationDoseSRStorage
"1.2.840.10008.5.1.4.1.1.88.68",//RadiopharmaceuticalRadiationDoseSRStorage
"1.2.840.10008.5.1.4.1.1.88.69",//ColonCADSRStorage
"1.2.840.10008.5.1.4.1.1.88.70",//ImplantationPlanSRStorage
"1.2.840.10008.5.1.4.1.1.88.71",//AcquisitionContextSRStorage
"1.2.840.10008.5.1.4.1.1.88.72",//SimplifiedAdultEchoSRStorage
"1.2.840.10008.5.1.4.1.1.88.73",//PatientRadiationDoseSRStorage
"1.2.840.10008.5.1.4.1.1.88.74",//PlannedImagingAgentAdministrationSRStorage
"1.2.840.10008.5.1.4.1.1.88.75",//PerformedImagingAgentAdministrationSRStorage
"1.2.840.10008.5.1.4.1.1.88.76",//EnhancedXRayRadiationDoseSRStorage
"1.2.840.10008.5.1.4.1.1.90.1",//ContentAssessmentResultsStorage
"1.2.840.10008.5.1.4.1.1.91.1",//MicroscopyBulkSimpleAnnotationsStorage
"1.2.840.10008.5.1.4.1.1.104.1",//EncapsulatedPDFStorage
"1.2.840.10008.5.1.4.1.1.104.2",//EncapsulatedCDAStorage
"1.2.840.10008.5.1.4.1.1.104.3",//EncapsulatedSTLStorage
"1.2.840.10008.5.1.4.1.1.104.4",//EncapsulatedOBJStorage
"1.2.840.10008.5.1.4.1.1.104.5",//EncapsulatedMTLStorage
"1.2.840.10008.5.1.4.1.1.128",//PositronEmissionTomographyImageStorage
"1.2.840.10008.5.1.4.1.1.128.1",//LegacyConvertedEnhancedPETImageStorage
"1.2.840.10008.5.1.4.1.1.129",//StandalonePETCurveStorage
"1.2.840.10008.5.1.4.1.1.130",//EnhancedPETImageStorage
"1.2.840.10008.5.1.4.1.1.131",//BasicStructuredDisplayStorage
"1.2.840.10008.5.1.4.1.1.200.1",//CTDefinedProcedureProtocolStorage
"1.2.840.10008.5.1.4.1.1.200.2",//CTPerformedProcedureProtocolStorage
"1.2.840.10008.5.1.4.1.1.200.3",//ProtocolApprovalStorage
"1.2.840.10008.5.1.4.1.1.200.4",//ProtocolApprovalInformationModelFind
"1.2.840.10008.5.1.4.1.1.200.5",//ProtocolApprovalInformationModelMove
"1.2.840.10008.5.1.4.1.1.200.6",//ProtocolApprovalInformationModelGet
"1.2.840.10008.5.1.4.1.1.200.7",//XADefinedProcedureProtocolStorage
"1.2.840.10008.5.1.4.1.1.200.8",//XAPerformedProcedureProtocolStorage
"1.2.840.10008.5.1.4.1.1.201.1",//InventoryStorage
"1.2.840.10008.5.1.4.1.1.201.2",//InventoryFind
"1.2.840.10008.5.1.4.1.1.201.3",//InventoryMove
"1.2.840.10008.5.1.4.1.1.201.4",//InventoryGet
"1.2.840.10008.5.1.4.1.1.201.5",//InventoryCreation
"1.2.840.10008.5.1.4.1.1.201.6",//RepositoryQuery
"1.2.840.10008.5.1.4.1.1.481.1",//RTImageStorage
"1.2.840.10008.5.1.4.1.1.481.2",//RTDoseStorage
"1.2.840.10008.5.1.4.1.1.481.3",//RTStructureSetStorage
"1.2.840.10008.5.1.4.1.1.481.4",//RTBeamsTreatmentRecordStorage
"1.2.840.10008.5.1.4.1.1.481.5",//RTPlanStorage
"1.2.840.10008.5.1.4.1.1.481.6",//RTBrachyTreatmentRecordStorage
"1.2.840.10008.5.1.4.1.1.481.7",//RTTreatmentSummaryRecordStorage
"1.2.840.10008.5.1.4.1.1.481.8",//RTIonPlanStorage
"1.2.840.10008.5.1.4.1.1.481.9",//RTIonBeamsTreatmentRecordStorage
"1.2.840.10008.5.1.4.1.1.481.10",//RTPhysicianIntentStorage
"1.2.840.10008.5.1.4.1.1.481.11",//RTSegmentAnnotationStorage
"1.2.840.10008.5.1.4.1.1.481.12",//RTRadiationSetStorage
"1.2.840.10008.5.1.4.1.1.481.13",//CArmPhotonElectronRadiationStorage
"1.2.840.10008.5.1.4.1.1.481.14",//TomotherapeuticRadiationStorage
"1.2.840.10008.5.1.4.1.1.481.15",//RoboticArmRadiationStorage
"1.2.840.10008.5.1.4.1.1.481.16",//RTRadiationRecordSetStorage
"1.2.840.10008.5.1.4.1.1.481.17",//RTRadiationSalvageRecordStorage
"1.2.840.10008.5.1.4.1.1.481.18",//TomotherapeuticRadiationRecordStorage
"1.2.840.10008.5.1.4.1.1.481.19",//CArmPhotonElectronRadiationRecordStorage
"1.2.840.10008.5.1.4.1.1.481.20",//RoboticRadiationRecordStorage
"1.2.840.10008.5.1.4.1.1.481.21",//RTRadiationSetDeliveryInstructionStorage
"1.2.840.10008.5.1.4.1.1.481.22",//RTTreatmentPreparationStorage
"1.2.840.10008.5.1.4.1.1.481.23",//EnhancedRTImageStorage
"1.2.840.10008.5.1.4.1.1.481.24",//EnhancedContinuousRTImageStorage
"1.2.840.10008.5.1.4.1.1.481.25",//RTPatientPositionAcquisitionInstructionStorage
"1.2.840.10008.5.1.4.1.1.501.1",//DICOSCTImageStorage
"1.2.840.10008.5.1.4.1.1.501.2.1",//DICOSDigitalXRayImageStorageForPresentation
"1.2.840.10008.5.1.4.1.1.501.2.2",//DICOSDigitalXRayImageStorageForProcessing
"1.2.840.10008.5.1.4.1.1.501.3",//DICOSThreatDetectionReportStorage
"1.2.840.10008.5.1.4.1.1.501.4",//DICOS2DAITStorage
"1.2.840.10008.5.1.4.1.1.501.5",//DICOS3DAITStorage
"1.2.840.10008.5.1.4.1.1.501.6",//DICOSQuadrupoleResonanceStorage
"1.2.840.10008.5.1.4.1.1.601.1",//EddyCurrentImageStorage
"1.2.840.10008.5.1.4.1.1.601.2",//EddyCurrentMultiFrameImageStorage
"1.2.840.10008.5.1.4.1.2.1.1",//PatientRootQueryRetrieveInformationModelFind
"1.2.840.10008.5.1.4.1.2.1.2",//PatientRootQueryRetrieveInformationModelMove
"1.2.840.10008.5.1.4.1.2.1.3",//PatientRootQueryRetrieveInformationModelGet
"1.2.840.10008.5.1.4.1.2.2.1",//StudyRootQueryRetrieveInformationModelFind
"1.2.840.10008.5.1.4.1.2.2.2",//StudyRootQueryRetrieveInformationModelMove
"1.2.840.10008.5.1.4.1.2.2.3",//StudyRootQueryRetrieveInformationModelGet
"1.2.840.10008.5.1.4.1.2.3.1",//PatientStudyOnlyQueryRetrieveInformationModelFind
"1.2.840.10008.5.1.4.1.2.3.2",//PatientStudyOnlyQueryRetrieveInformationModelMove
"1.2.840.10008.5.1.4.1.2.3.3",//PatientStudyOnlyQueryRetrieveInformationModelGet
"1.2.840.10008.5.1.4.1.2.4.2",//CompositeInstanceRootRetrieveMove
"1.2.840.10008.5.1.4.1.2.4.3",//CompositeInstanceRootRetrieveGet
"1.2.840.10008.5.1.4.1.2.5.3",//CompositeInstanceRetrieveWithoutBulkDataGet
"1.2.840.10008.5.1.4.20.1",//DefinedProcedureProtocolInformationModelFind
"1.2.840.10008.5.1.4.20.2",//DefinedProcedureProtocolInformationModelMove
"1.2.840.10008.5.1.4.20.3",//DefinedProcedureProtocolInformationModelGet
"1.2.840.10008.5.1.4.31",//ModalityWorklistInformationModelFind
"1.2.840.10008.5.1.4.32",//GeneralPurposeWorklistManagementMeta
"1.2.840.10008.5.1.4.32.1",//GeneralPurposeWorklistInformationModelFind
"1.2.840.10008.5.1.4.32.2",//GeneralPurposeScheduledProcedureStep
"1.2.840.10008.5.1.4.32.3",//GeneralPurposePerformedProcedureStep
"1.2.840.10008.5.1.4.33",//InstanceAvailabilityNotification
"1.2.840.10008.5.1.4.34.1",//RTBeamsDeliveryInstructionStorageTrial
"1.2.840.10008.5.1.4.34.2",//RTConventionalMachineVerificationTrial
"1.2.840.10008.5.1.4.34.3",//RTIonMachineVerificationTrial (Retired)
"1.2.840.10008.5.1.4.34.4",//UnifiedWorklistAndProcedureStepTrial (Retired)
"1.2.840.10008.5.1.4.34.4.1",//UnifiedProcedureStepPushTrial (Retired)
"1.2.840.10008.5.1.4.34.4.2",//UnifiedProcedureStepWatchTrial (Retired)
"1.2.840.10008.5.1.4.34.4.3",//UnifiedProcedureStepPullTrial (Retired)
"1.2.840.10008.5.1.4.34.4.4",//UnifiedProcedureStepEventTrial
"1.2.840.10008.5.1.4.34.5",//UPSGlobalSubscriptionInstance
"1.2.840.10008.5.1.4.34.5.1",//UPSFilteredGlobalSubscriptionInstance
"1.2.840.10008.5.1.4.34.6",//UnifiedWorklistAndProcedureStep
"1.2.840.10008.5.1.4.34.6.1",//UnifiedProcedureStepPush
"1.2.840.10008.5.1.4.34.6.2",//UnifiedProcedureStepWatch
"1.2.840.10008.5.1.4.34.6.3",//UnifiedProcedureStepPull
"1.2.840.10008.5.1.4.34.6.4",//UnifiedProcedureStepEvent
"1.2.840.10008.5.1.4.34.6.5",//UnifiedProcedureStepQuery
"1.2.840.10008.5.1.4.34.7",//RTBeamsDeliveryInstructionStorage
"1.2.840.10008.5.1.4.34.8",//RTConventionalMachineVerification
"1.2.840.10008.5.1.4.34.9",//RTIonMachineVerification
"1.2.840.10008.5.1.4.34.10",//RTBrachyApplicationSetupDeliveryInstructionStorage
"1.2.840.10008.5.1.4.37.1",//GeneralRelevantPatientInformationQuery
"1.2.840.10008.5.1.4.37.2",//BreastImagingRelevantPatientInformationQuery
"1.2.840.10008.5.1.4.37.3",//CardiacRelevantPatientInformationQuery
"1.2.840.10008.5.1.4.38.1",//HangingProtocolStorage
"1.2.840.10008.5.1.4.38.2",//HangingProtocolInformationModelFind
"1.2.840.10008.5.1.4.38.3",//HangingProtocolInformationModelMove
"1.2.840.10008.5.1.4.38.4",//HangingProtocolInformationModelGet
"1.2.840.10008.5.1.4.39.1",//ColorPaletteStorage
"1.2.840.10008.5.1.4.39.2",//ColorPaletteQueryRetrieveInformationModelFind
"1.2.840.10008.5.1.4.39.3",//ColorPaletteQueryRetrieveInformationModelMove
"1.2.840.10008.5.1.4.39.4",//ColorPaletteQueryRetrieveInformationModelGet
"1.2.840.10008.5.1.4.41",//ProductCharacteristicsQuery
"1.2.840.10008.5.1.4.42",//SubstanceApprovalQuery
"1.2.840.10008.5.1.4.43.1",//GenericImplantTemplateStorage
"1.2.840.10008.5.1.4.43.2",//GenericImplantTemplateInformationModelFind
"1.2.840.10008.5.1.4.43.3",//GenericImplantTemplateInformationModelMove
"1.2.840.10008.5.1.4.43.4",//GenericImplantTemplateInformationModelGet
"1.2.840.10008.5.1.4.44.1",//ImplantAssemblyTemplateStorage
"1.2.840.10008.5.1.4.44.2",//ImplantAssemblyTemplateInformationModelFind
"1.2.840.10008.5.1.4.44.3",//ImplantAssemblyTemplateInformationModelMove
"1.2.840.10008.5.1.4.44.4",//ImplantAssemblyTemplateInformationModelGet
"1.2.840.10008.5.1.4.45.1",//ImplantTemplateGroupStorage
"1.2.840.10008.5.1.4.45.2",//ImplantTemplateGroupInformationModelFind
"1.2.840.10008.5.1.4.45.3",//ImplantTemplateGroupInformationModelMove
"1.2.840.10008.5.1.4.45.4",//ImplantTemplateGroupInformationModelGet
"1.2.840.10008.10.1",//VideoEndoscopicImageRealTimeCommunication
"1.2.840.10008.10.2",//VideoPhotographicImageRealTimeCommunication
"1.2.840.10008.10.3",//AudioWaveformRealTimeCommunication
"1.2.840.10008.10.4"//RenditionSelectionDocumentRealTimeCommunication
};

u16 sopclassidx( char *vbuf, u16 vallength )
{
   u16 idx=0x0;//verification / error
   switch (vallength-(vbuf[vallength-1]==0)){
         
      case 17: {
         switch (vbuf[16]){
            case 0x31: idx=sc_1_1;break;//Verification
            case 0x39: idx=sc_1_9;break;//BasicStudyContentNotification
         }};break;
         
      case 18: {
         switch ((vbuf[17]<<8) | vbuf[16]) {
            case 0x3034: idx=sc_1_40;break;//ProceduralEventLogging"
            case 0x3234: idx=sc_1_42;break;//SubstanceAdministrationLogging
            case 0x312E: idx=sc_10_1;break;//VideoEndoscopicImageRealTimeCommunication
            case 0x322E: idx=sc_10_2;break;//VideoPhotographicImageRealTimeCommunication
            case 0x332E: idx=sc_10_3;break;//AudioWaveformRealTimeCommunication
            case 0x342E: idx=sc_10_4;break;//RenditionSelectionDocumentRealTimeCommunication
         }};break;
         
      case 20:{
         switch (vbuf[19]){
            case 0x30: idx=sc_1_3_10;break;//MediaStorageDirectoryStorage
            case 0x31: idx=sc_1_20_1;break;//StorageCommitmentPushModel
            case 0x32: idx=sc_1_20_2;break;//StorageCommitmentPullModel (Retired)
         }};break;
         
      case 21:{
         switch (vbuf[20]){
            case 0x31: idx=sc_5_1_1_1;break;//BasicFilmSession
            case 0x32: idx=sc_5_1_1_2;break;//BasicFilmBox
            case 0x34: idx=sc_5_1_1_4;break;//BasicGrayscaleImageBox
            case 0x39: idx=sc_5_1_1_9;break;//BasicGrayscalePrintManagementMeta
        }};break;
         
      case 22:{
         switch ((vbuf[21] << 24) | (vbuf[20]<<16) | (vbuf[19]<<8) | vbuf[18]) {
            case 0x34312E31: idx=sc_5_1_1_14;break;//PrintJob
            case 0x35312E31: idx=sc_5_1_1_15;break;//BasicAnnotationBox
            case 0x36312E31: idx=sc_5_1_1_16;break;//Printer
            case 0x38312E31: idx=sc_5_1_1_18;break;//BasicColorPrintManagementMeta
            case 0x32322E31: idx=sc_5_1_1_22;break;//VOILUTBox
            case 0x33322E31: idx=sc_5_1_1_23;break;//PresentationLUT
            case 0x34322E31: idx=sc_5_1_1_24;break;//ImageOverlayBox (Retired)
            case 0x36322E31: idx=sc_5_1_1_26;break;//PrintQueueManagement (Retired)
            case 0x37322E31: idx=sc_5_1_1_27;break;//StoredPrintStorage (Retired)
            case 0x39322E31: idx=sc_5_1_1_29;break;//HardcopyGrayscaleImageStorage (Retired)
            case 0x30332E31: idx=sc_5_1_1_30;break;//HardcopyColorImageStorage (Retired)
            case 0x31332E31: idx=sc_5_1_1_31;break;//PullPrintRequest (Retired)
            case 0x32332E31: idx=sc_5_1_1_32;break;//PullStoredPrintManagementMeta (Retired)
            case 0x33332E31: idx=sc_5_1_1_33;break;//MediaCreationManagement
            case 0x30342E31: idx=sc_5_1_1_40;break;//DisplaySystem
            case 0x31332E34: idx=sc_5_1_4_31;break;//ModalityWorklistInformationModelFind
            case 0x32332E34: idx=sc_5_1_4_32;break;//GeneralPurposeWorklistManagementMeta
            case 0x33332E34: idx=sc_5_1_4_33;break;//InstanceAvailabilityNotification
            case 0x31342E34: idx=sc_5_1_4_41;break;//ProductCharacteristicsQuery
            case 0x32342E34: idx=sc_5_1_4_42;break;//SubstanceApprovalQuery
         }};break;
         
      case 23:{
         switch ((vbuf[22] << 24) | (vbuf[21]<<16) | (vbuf[20]<<8) | vbuf[19]) {
            case 0x312E312E: idx=sc_3_1_2_1_1;break;//DetachedPatientManagement (Retired)
            case 0x342E312E: idx=sc_3_1_2_1_4;break;//DetachedPatientManagementMeta (Retired)
            case 0x312E322E: idx=sc_3_1_2_2_1;break;//DetachedVisitManagement (Retired)
            case 0x312E332E: idx=sc_3_1_2_3_1;break;//DetachedStudyManagement (Retired)
            case 0x322E332E: idx=sc_3_1_2_3_2;break;//StudyComponentManagement (Retired)
            case 0x332E332E: idx=sc_3_1_2_3_3;break;//ModalityPerformedProcedureStep
            case 0x342E332E: idx=sc_3_1_2_3_4;break;//ModalityPerformedProcedureStepRetrieve
            case 0x352E332E: idx=sc_3_1_2_3_5;break;//ModalityPerformedProcedureStepNotification
            case 0x312E352E: idx=sc_3_1_2_5_1;break;//DetachedResultsManagement (Retired)
            case 0x342E352E: idx=sc_3_1_2_5_4;break;//DetachedResultsManagementMeta (Retired)
            case 0x352E352E: idx=sc_3_1_2_5_5;break;//DetachedStudyManagementMeta (Retired)
            case 0x312E362E: idx=sc_3_1_2_6_1;break;//DetachedInterpretationManagement (Retired)
            case 0x312E342E: idx=sc_5_1_1_4_1;break;//BasicColorImageBox
            case 0x322E342E: idx=sc_5_1_1_4_2;break;//ReferencedImageBox
            case 0x312E392E: idx=sc_5_1_1_9_1;break;//ReferencedGrayscalePrintManagementMeta
         }};break;
         
      case 24:{
         switch ((vbuf[23] << 24) | (vbuf[22]<<16) | (vbuf[21]<<8) | vbuf[20]) {
            case 0x312E3831: idx=sc_5_1_1_18_1;break;//ReferencedColorPrintManagementMeta (Retired)
            case 0x312E3432: idx=sc_5_1_1_24_1;break;//BasicPrintImageOverlayBox (Retired)
            case 0x312E3032: idx=sc_5_1_4_20_1;break;//DefinedProcedureProtocolInformationModelFind
            case 0x322E3032: idx=sc_5_1_4_20_2;break;//DefinedProcedureProtocolInformationModelMove
            case 0x332E3032: idx=sc_5_1_4_20_3;break;//DefinedProcedureProtocolInformationModelGet
            case 0x312E3233: idx=sc_5_1_4_32_1;break;//GeneralPurposeWorklistInformationModelFind
            case 0x322E3233: idx=sc_5_1_4_32_2;break;//GeneralPurposeScheduledProcedureStep
            case 0x332E3233: idx=sc_5_1_4_32_3;break;//GeneralPurposePerformedProcedureStep
            case 0x312E3433: idx=sc_5_1_4_34_1;break;//RTBeamsDeliveryInstructionStorageTrial
            case 0x322E3433: idx=sc_5_1_4_34_2;break;//RTConventionalMachineVerificationTrial
            case 0x332E3433: idx=sc_5_1_4_34_3;break;//RTIonMachineVerificationTrial (Retired)
            case 0x342E3433: idx=sc_5_1_4_34_4;break;//UnifiedWorklistAndProcedureStepTrial (Retired)
            case 0x352E3433: idx=sc_5_1_4_34_5;break;//UPSGlobalSubscriptionInstance
            case 0x362E3433: idx=sc_5_1_4_34_6;break;//UnifiedWorklistAndProcedureStep
            case 0x372E3433: idx=sc_5_1_4_34_7;break;//RTBeamsDeliveryInstructionStorage
            case 0x382E3433: idx=sc_5_1_4_34_8;break;//RTConventionalMachineVerification
            case 0x392E3433: idx=sc_5_1_4_34_9;break;//RTIonMachineVerification
            case 0x312E3733: idx=sc_5_1_4_37_1;break;//GeneralRelevantPatientInformationQuery
            case 0x322E3733: idx=sc_5_1_4_37_2;break;//BreastImagingRelevantPatientInformationQuery
            case 0x332E3733: idx=sc_5_1_4_37_3;break;//CardiacRelevantPatientInformationQuery
            case 0x312E3833: idx=sc_5_1_4_38_1;break;//HangingProtocolStorage
            case 0x322E3833: idx=sc_5_1_4_38_2;break;//HangingProtocolInformationModelFind
            case 0x332E3833: idx=sc_5_1_4_38_3;break;//HangingProtocolInformationModelMove
            case 0x342E3833: idx=sc_5_1_4_38_4;break;//HangingProtocolInformationModelGet
            case 0x312E3933: idx=sc_5_1_4_39_1;break;//ColorPaletteStorage
            case 0x322E3933: idx=sc_5_1_4_39_2;break;//ColorPaletteQueryRetrieveInformationModelFind
            case 0x332E3933: idx=sc_5_1_4_39_3;break;//ColorPaletteQueryRetrieveInformationModelMove
            case 0x342E3933: idx=sc_5_1_4_39_4;break;//ColorPaletteQueryRetrieveInformationModelGet
            case 0x312E3334: idx=sc_5_1_4_43_1;break;//GenericImplantTemplateStorage
            case 0x322E3334: idx=sc_5_1_4_43_2;break;//GenericImplantTemplateInformationModelFind
            case 0x332E3334: idx=sc_5_1_4_43_3;break;//GenericImplantTemplateInformationModelMove
            case 0x342E3334: idx=sc_5_1_4_43_4;break;//GenericImplantTemplateInformationModelGet
            case 0x312E3434: idx=sc_5_1_4_44_1;break;//ImplantAssemblyTemplateStorage
            case 0x322E3434: idx=sc_5_1_4_44_2;break;//ImplantAssemblyTemplateInformationModelFind
            case 0x332E3434: idx=sc_5_1_4_44_3;break;//ImplantAssemblyTemplateInformationModelMove
            case 0x342E3434: idx=sc_5_1_4_44_4;break;//ImplantAssemblyTemplateInformationModelGet
            case 0x312E3534: idx=sc_5_1_4_45_1;break;//ImplantTemplateGroupStorage
            case 0x322E3534: idx=sc_5_1_4_45_2;break;//ImplantTemplateGroupInformationModelFind
            case 0x332E3534: idx=sc_5_1_4_45_3;break;//ImplantTemplateGroupInformationModelMove
            case 0x342E3534: idx=sc_5_1_4_45_4;break;//ImplantTemplateGroupInformationModelGet
         }};break;

      case 25:{
         switch (vbuf[24]){
            case 0x31: idx=sc_5_1_4_1_1_1;break;//ComputedRadiographyImageStorage
            case 0x32: idx=sc_5_1_4_1_1_2;break;//CTImageStorage
            case 0x33: idx=sc_5_1_4_1_1_3;break;//UltrasoundMultiFrameImageStorageRetired (Retired)
            case 0x34: idx=sc_5_1_4_1_1_4;break;//MRImageStorage
            case 0x35: idx=sc_5_1_4_1_1_5;break;//NuclearMedicineImageStorageRetired (Retired)
            case 0x36: idx=sc_5_1_4_1_1_6;break;//UltrasoundImageStorageRetired (Retired)
            case 0x37: idx=sc_5_1_4_1_1_7;break;//SecondaryCaptureImageStorage
            case 0x38: idx=sc_5_1_4_1_1_8;break;//StandaloneOverlayStorage (Retired)
            case 0x39: idx=sc_5_1_4_1_1_9;break;//StandaloneCurveStorage (Retired)
            case 0x30: idx=sc_5_1_4_34_10;break;//RTBrachyApplicationSetupDeliveryInstructionStorage
         }};break;

      case 26:{
         switch ((vbuf[25] << 24) | (vbuf[24]<<16) | (vbuf[23]<<8) | vbuf[22]) {
            case 0x3637332E: idx=sc_5_1_1_16_376;break;//PrinterConfigurationRetrieval
            case 0x30312E31: idx=sc_5_1_4_1_1_10;break;//StandaloneModalityLUTStorage (Retired)
            case 0x31312E31: idx=sc_5_1_4_1_1_11;break;//StandaloneVOILUTStorage (Retired)
            case 0x30322E31: idx=sc_5_1_4_1_1_20;break;//NuclearMedicineImageStorage
            case 0x30332E31: idx=sc_5_1_4_1_1_30;break;//ParametricMapStorage
            case 0x30342E31: idx=sc_5_1_4_1_1_40;break;//(Retired)
            case 0x36362E31: idx=sc_5_1_4_1_1_66;break;//RawDataStorage
            case 0x37362E31: idx=sc_5_1_4_1_1_67;break;//RealWorldValueMappingStorage
            case 0x312E342E: idx=sc_5_1_4_34_4_1;break;//UnifiedProcedureStepPushTrial (Retired)
            case 0x322E342E: idx=sc_5_1_4_34_4_2;break;//UnifiedProcedureStepWatchTrial (Retired)
            case 0x332E342E: idx=sc_5_1_4_34_4_3;break;//UnifiedProcedureStepPullTrial (Retired)
            case 0x342E342E: idx=sc_5_1_4_34_4_4;break;//UnifiedProcedureStepEventTrial
            case 0x312E352E: idx=sc_5_1_4_34_5_1;break;//UPSFilteredGlobalSubscriptionInstance
            case 0x312E362E: idx=sc_5_1_4_34_6_1;break;//UnifiedProcedureStepPush
            case 0x322E362E: idx=sc_5_1_4_34_6_2;break;//UnifiedProcedureStepWatch
            case 0x332E362E: idx=sc_5_1_4_34_6_3;break;//UnifiedProcedureStepPull
            case 0x342E362E: idx=sc_5_1_4_34_6_4;break;//UnifiedProcedureStepEvent
            case 0x352E362E: idx=sc_5_1_4_34_6_5;break;//UnifiedProcedureStepQuery
         }};break;
         
      case 27:{
         switch ((vbuf[26] << 24) | (vbuf[25]<<16) | (vbuf[24]<<8) | vbuf[22]) {
            case 0x312E3131: idx=sc_5_1_4_1_1_1_1;break;//DigitalXRayImageStorageForPresentation
            case 0x322E3131: idx=sc_5_1_4_1_1_1_2;break;//DigitalMammographyXRayImageStorageForPresentation
            case 0x332E3131: idx=sc_5_1_4_1_1_1_3;break;//DigitalIntraOralXRayImageStorageForPresentation
            case 0x38323131: idx=sc_5_1_4_1_1_128;break;//PositronEmissionTomographyImageStorage
            case 0x39323131: idx=sc_5_1_4_1_1_129;break;//StandalonePETCurveStorage
            case 0x30333131: idx=sc_5_1_4_1_1_130;break;//EnhancedPETImageStorage
            case 0x31333131: idx=sc_5_1_4_1_1_131;break;//BasicStructuredDisplayStorage
            case 0x312E3231: idx=sc_5_1_4_1_1_2_1;break;//EnhancedCTImageStorage
            case 0x322E3231: idx=sc_5_1_4_1_1_2_2;break;//LegacyConvertedEnhancedCTImageStorage
            case 0x312E3331: idx=sc_5_1_4_1_1_3_1;break;//UltrasoundMultiFrameImageStorage
            case 0x312E3431: idx=sc_5_1_4_1_1_4_1;break;//EnhancedMRImageStorage
            case 0x322E3431: idx=sc_5_1_4_1_1_4_2;break;//MRSpectroscopyStorage
            case 0x332E3431: idx=sc_5_1_4_1_1_4_3;break;//EnhancedMRColorImageStorage
            case 0x342E3431: idx=sc_5_1_4_1_1_4_4;break;//LegacyConvertedEnhancedMRImageStorage
            case 0x312E3631: idx=sc_5_1_4_1_1_6_1;break;//UltrasoundImageStorage
            case 0x322E3631: idx=sc_5_1_4_1_1_6_2;break;//EnhancedUSVolumeStorage
            case 0x332E3631: idx=sc_5_1_4_1_1_6_3;break;//PhotoacousticImageStorage
            case 0x312E3731: idx=sc_5_1_4_1_1_7_1;break;//MultiFrameSingleBitSecondaryCaptureImageStorage
            case 0x322E3731: idx=sc_5_1_4_1_1_7_2;break;//MultiFrameGrayscaleByteSecondaryCaptureImageStorage
            case 0x332E3731: idx=sc_5_1_4_1_1_7_3;break;//MultiFrameGrayscaleWordSecondaryCaptureImageStorage
            case 0x342E3731: idx=sc_5_1_4_1_1_7_4;break;//MultiFrameTrueColorSecondaryCaptureImageStorage
            case 0x312E3931: idx=sc_5_1_4_1_1_9_1;break;//WaveformStorageTrial (Retired)
            case 0x312E3132: idx=sc_5_1_4_1_2_1_1;break;//PatientRootQueryRetrieveInformationModelFind
            case 0x322E3132: idx=sc_5_1_4_1_2_1_2;break;//PatientRootQueryRetrieveInformationModelMove
            case 0x332E3132: idx=sc_5_1_4_1_2_1_3;break;//PatientRootQueryRetrieveInformationModelGet
            case 0x312E3232: idx=sc_5_1_4_1_2_2_1;break;//StudyRootQueryRetrieveInformationModelFind
            case 0x322E3232: idx=sc_5_1_4_1_2_2_2;break;//StudyRootQueryRetrieveInformationModelMove
            case 0x332E3232: idx=sc_5_1_4_1_2_2_3;break;//StudyRootQueryRetrieveInformationModelGet
            case 0x312E3332: idx=sc_5_1_4_1_2_3_1;break;//PatientStudyOnlyQueryRetrieveInformationModelFind
            case 0x322E3332: idx=sc_5_1_4_1_2_3_2;break;//PatientStudyOnlyQueryRetrieveInformationModelMove
            case 0x332E3332: idx=sc_5_1_4_1_2_3_3;break;//PatientStudyOnlyQueryRetrieveInformationModelGet
            case 0x322E3432: idx=sc_5_1_4_1_2_4_2;break;//CompositeInstanceRootRetrieveMove
            case 0x332E3432: idx=sc_5_1_4_1_2_4_3;break;//CompositeInstanceRootRetrieveGet
            case 0x332E3532: idx=sc_5_1_4_1_2_5_3;break;//CompositeInstanceRetrieveWithoutBulkDataGet
         }};break;
         
      case 28:{
         switch ((vbuf[27] << 24) | (vbuf[26]<<16) | (vbuf[25]<<8) | vbuf[24]) {
            case 0x312E3131: idx=sc_5_1_4_1_1_11_1;break;//GrayscaleSoftcopyPresentationStateStorage
            case 0x322E3131: idx=sc_5_1_4_1_1_11_2;break;//ColorSoftcopyPresentationStateStorage
            case 0x332E3131: idx=sc_5_1_4_1_1_11_3;break;//PseudoColorSoftcopyPresentationStateStorage
            case 0x342E3131: idx=sc_5_1_4_1_1_11_4;break;//BlendingSoftcopyPresentationStateStorage
            case 0x352E3131: idx=sc_5_1_4_1_1_11_5;break;//XAXRFGrayscaleSoftcopyPresentationStateStorage
            case 0x362E3131: idx=sc_5_1_4_1_1_11_6;break;//GrayscalePlanarMPRVolumetricPresentationStateStorage
            case 0x372E3131: idx=sc_5_1_4_1_1_11_7;break;//CompositingPlanarMPRVolumetricPresentationStateStorage
            case 0x382E3131: idx=sc_5_1_4_1_1_11_8;break;//AdvancedBlendingPresentationStateStorage
            case 0x392E3131: idx=sc_5_1_4_1_1_11_9;break;//VolumeRenderingVolumetricPresentationStateStorage
            case 0x312E3231: idx=sc_5_1_4_1_1_12_1;break;//XRayAngiographicImageStorage
            case 0x322E3231: idx=sc_5_1_4_1_1_12_2;break;//XRayRadiofluoroscopicImageStorage
            case 0x332E3231: idx=sc_5_1_4_1_1_12_3;break;//XRayAngiographicBiPlaneImageStorage (Retired)
            case 0x312E3431: idx=sc_5_1_4_1_1_14_1;break;//IntravascularOpticalCoherenceTomographyImageStorageForPresentation
            case 0x322E3431: idx=sc_5_1_4_1_1_14_2;break;//IntravascularOpticalCoherenceTomographyImageStorageForProcessing
            case 0x312E3636: idx=sc_5_1_4_1_1_66_1;break;//SpatialRegistrationStorage
            case 0x322E3636: idx=sc_5_1_4_1_1_66_2;break;//SpatialFiducialsStorage
            case 0x332E3636: idx=sc_5_1_4_1_1_66_3;break;//DeformableSpatialRegistrationStorage
            case 0x342E3636: idx=sc_5_1_4_1_1_66_4;break;//SegmentationStorage
            case 0x352E3636: idx=sc_5_1_4_1_1_66_5;break;//SurfaceSegmentationStorage
            case 0x362E3636: idx=sc_5_1_4_1_1_66_6;break;//TractographyResultsStorage
            case 0x312E3836: idx=sc_5_1_4_1_1_68_1;break;//SurfaceScanMeshStorage
            case 0x322E3836: idx=sc_5_1_4_1_1_68_2;break;//SurfaceScanPointCloudStorage
            case 0x312E3737: idx=sc_5_1_4_1_1_77_1;break;//VLImageStorageTrial (Retired)
            case 0x322E3737: idx=sc_5_1_4_1_1_77_2;break;//VLMultiFrameImageStorageTrial (Retired)
            case 0x312E3837: idx=sc_5_1_4_1_1_78_1;break;//LensometryMeasurementsStorage
            case 0x322E3837: idx=sc_5_1_4_1_1_78_2;break;//AutorefractionMeasurementsStorage
            case 0x332E3837: idx=sc_5_1_4_1_1_78_3;break;//KeratometryMeasurementsStorage
            case 0x342E3837: idx=sc_5_1_4_1_1_78_4;break;//SubjectiveRefractionMeasurementsStorage
            case 0x352E3837: idx=sc_5_1_4_1_1_78_5;break;//VisualAcuityMeasurementsStorage
            case 0x362E3837: idx=sc_5_1_4_1_1_78_6;break;//SpectaclePrescriptionReportStorage
            case 0x372E3837: idx=sc_5_1_4_1_1_78_7;break;//OphthalmicAxialMeasurementsStorage
            case 0x382E3837: idx=sc_5_1_4_1_1_78_8;break;//IntraocularLensCalculationsStorage
            case 0x312E3937: idx=sc_5_1_4_1_1_79_1;break;//MacularGridThicknessAndVolumeReportStorage
            case 0x312E3038: idx=sc_5_1_4_1_1_80_1;break;//OphthalmicVisualFieldStaticPerimetryMeasurementsStorage
            case 0x312E3138: idx=sc_5_1_4_1_1_81_1;break;//OphthalmicThicknessMapStorage
            case 0x312E3238: idx=sc_5_1_4_1_1_82_1;break;//CornealTopographyMapStorage
            case 0x312E3838: idx=sc_5_1_4_1_1_88_1;break;//TextSRStorageTrial (Retired)
            case 0x322E3838: idx=sc_5_1_4_1_1_88_2;break;//AudioSRStorageTrial (Retired)
            case 0x332E3838: idx=sc_5_1_4_1_1_88_3;break;//DetailSRStorageTrial (Retired)
            case 0x342E3838: idx=sc_5_1_4_1_1_88_4;break;//ComprehensiveSRStorageTrial (Retired)
            case 0x312E3039: idx=sc_5_1_4_1_1_90_1;break;//ContentAssessmentResultsStorage
            case 0x312E3139: idx=sc_5_1_4_1_1_91_1;break;//MicroscopyBulkSimpleAnnotationsStorage
         }};break;

      case 29:{
         switch ((vbuf[28] << 24) | (vbuf[27]<<16) | (vbuf[26]<<8) | vbuf[24]) {
            case 0x312E3131: idx=sc_5_1_4_1_1_1_1_1;break;//DigitalXRayImageStorageForProcessing
            case 0x312E3231: idx=sc_5_1_4_1_1_1_2_1;break;//DigitalMammographyXRayImageStorageForProcessing
            case 0x312E3331: idx=sc_5_1_4_1_1_1_3_1;break;//DigitalIntraOralXRayImageStorageForProcessing
            case 0x312E3139: idx=sc_5_1_4_1_1_9_1_1;break;//TwelveLeadECGWaveformStorage
            case 0x322E3139: idx=sc_5_1_4_1_1_9_1_2;break;//GeneralECGWaveformStorage
            case 0x332E3139: idx=sc_5_1_4_1_1_9_1_3;break;//AmbulatoryECGWaveformStorage
            case 0x342E3139: idx=sc_5_1_4_1_1_9_1_4;break;//General32bitECGWaveformStorage
            case 0x312E3239: idx=sc_5_1_4_1_1_9_2_1;break;//HemodynamicWaveformStorage
            case 0x312E3339: idx=sc_5_1_4_1_1_9_3_1;break;//CardiacElectrophysiologyWaveformStorage
            case 0x312E3439: idx=sc_5_1_4_1_1_9_4_1;break;//BasicVoiceAudioWaveformStorage
            case 0x322E3439: idx=sc_5_1_4_1_1_9_4_2;break;//GeneralAudioWaveformStorage
            case 0x312E3539: idx=sc_5_1_4_1_1_9_5_1;break;//ArterialPulseWaveformStorage
            case 0x312E3639: idx=sc_5_1_4_1_1_9_6_1;break;//RespiratoryWaveformStorage
            case 0x322E3639: idx=sc_5_1_4_1_1_9_6_2;break;//MultichannelRespiratoryWaveformStorage
            case 0x312E3739: idx=sc_5_1_4_1_1_9_7_1;break;//RoutineScalpElectroencephalogramWaveformStorage
            case 0x322E3739: idx=sc_5_1_4_1_1_9_7_2;break;//ElectromyogramWaveformStorage
            case 0x332E3739: idx=sc_5_1_4_1_1_9_7_3;break;//ElectrooculogramWaveformStorage
            case 0x342E3739: idx=sc_5_1_4_1_1_9_7_4;break;//SleepElectroencephalogramWaveformStorage
            case 0x312E3839: idx=sc_5_1_4_1_1_9_8_1;break;//BodyPositionWaveformStorage
            case 0x30312E31: idx=sc_5_1_4_1_1_11_10;break;//SegmentedVolumeRenderingVolumetricPresentationStateStorage
            case 0x31312E31: idx=sc_5_1_4_1_1_11_11;break;//MultipleVolumeRenderingVolumetricPresentationStateStorage
            case 0x32312E31: idx=sc_5_1_4_1_1_11_12;break;//VariableModalityLUTSoftcopyPresentationStateStorage
            case 0x37372E31: idx=sc_5_1_4_1_1_12_77;break;//(Retired)
            case 0x31312E38: idx=sc_5_1_4_1_1_88_11;break;//BasicTextSRStorage
            case 0x32322E38: idx=sc_5_1_4_1_1_88_22;break;//EnhancedSRStorage
            case 0x33332E38: idx=sc_5_1_4_1_1_88_33;break;//ComprehensiveSRStorage
            case 0x34332E38: idx=sc_5_1_4_1_1_88_34;break;//Comprehensive3DSRStorage
            case 0x35332E38: idx=sc_5_1_4_1_1_88_35;break;//ExtensibleSRStorage
            case 0x30342E38: idx=sc_5_1_4_1_1_88_40;break;//ProcedureLogStorage
            case 0x30352E38: idx=sc_5_1_4_1_1_88_50;break;//MammographyCADSRStorage
            case 0x39352E38: idx=sc_5_1_4_1_1_88_59;break;//KeyObjectSelectionDocumentStorage
            case 0x35362E38: idx=sc_5_1_4_1_1_88_65;break;//ChestCADSRStorage
            case 0x37362E38: idx=sc_5_1_4_1_1_88_67;break;//XRayRadiationDoseSRStorage
            case 0x38362E38: idx=sc_5_1_4_1_1_88_68;break;//RadiopharmaceuticalRadiationDoseSRStorage
            case 0x39362E38: idx=sc_5_1_4_1_1_88_69;break;//ColonCADSRStorage
            case 0x30372E38: idx=sc_5_1_4_1_1_88_70;break;//ImplantationPlanSRStorage
            case 0x31372E38: idx=sc_5_1_4_1_1_88_71;break;//AcquisitionContextSRStorage
            case 0x32372E38: idx=sc_5_1_4_1_1_88_72;break;//SimplifiedAdultEchoSRStorage
            case 0x33372E38: idx=sc_5_1_4_1_1_88_73;break;//PatientRadiationDoseSRStorage
            case 0x34372E38: idx=sc_5_1_4_1_1_88_74;break;//PlannedImagingAgentAdministrationSRStorage
            case 0x35372E38: idx=sc_5_1_4_1_1_88_75;break;//PerformedImagingAgentAdministrationSRStorage
            case 0x36372E38: idx=sc_5_1_4_1_1_88_76;break;//EnhancedXRayRadiationDoseSRStorage
            case 0x312E3431: idx=sc_5_1_4_1_1_104_1;break;//EncapsulatedPDFStorage
            case 0x322E3431: idx=sc_5_1_4_1_1_104_2;break;//EncapsulatedCDAStorage
            case 0x332E3431: idx=sc_5_1_4_1_1_104_3;break;//EncapsulatedSTLStorage
            case 0x342E3431: idx=sc_5_1_4_1_1_104_4;break;//EncapsulatedOBJStorage
            case 0x352E3431: idx=sc_5_1_4_1_1_104_5;break;//EncapsulatedMTLStorage
            case 0x312E3831: idx=sc_5_1_4_1_1_128_1;break;//LegacyConvertedEnhancedPETImageStorage
            case 0x312E3032: idx=sc_5_1_4_1_1_200_1;break;//CTDefinedProcedureProtocolStorage
            case 0x322E3032: idx=sc_5_1_4_1_1_200_2;break;//CTPerformedProcedureProtocolStorage
            case 0x332E3032: idx=sc_5_1_4_1_1_200_3;break;//ProtocolApprovalStorage
            case 0x342E3032: idx=sc_5_1_4_1_1_200_4;break;//ProtocolApprovalInformationModelFind
            case 0x352E3032: idx=sc_5_1_4_1_1_200_5;break;//ProtocolApprovalInformationModelMove
            case 0x362E3032: idx=sc_5_1_4_1_1_200_6;break;//ProtocolApprovalInformationModelGet
            case 0x372E3032: idx=sc_5_1_4_1_1_200_7;break;//XADefinedProcedureProtocolStorage
            case 0x382E3032: idx=sc_5_1_4_1_1_200_8;break;//XAPerformedProcedureProtocolStorage
            case 0x312E3132: idx=sc_5_1_4_1_1_201_1;break;//InventoryStorage
            case 0x322E3132: idx=sc_5_1_4_1_1_201_2;break;//InventoryFind
            case 0x332E3132: idx=sc_5_1_4_1_1_201_3;break;//InventoryMove
            case 0x342E3132: idx=sc_5_1_4_1_1_201_4;break;//InventoryGet
            case 0x352E3132: idx=sc_5_1_4_1_1_201_5;break;//InventoryCreation
            case 0x362E3132: idx=sc_5_1_4_1_1_201_6;break;//RepositoryQuery
            case 0x312E3134: idx=sc_5_1_4_1_1_481_1;break;//RTImageStorage
            case 0x322E3134: idx=sc_5_1_4_1_1_481_2;break;//RTDoseStorage
            case 0x332E3134: idx=sc_5_1_4_1_1_481_3;break;//RTStructureSetStorage
            case 0x342E3134: idx=sc_5_1_4_1_1_481_4;break;//RTBeamsTreatmentRecordStorage
            case 0x352E3134: idx=sc_5_1_4_1_1_481_5;break;//RTPlanStorage
            case 0x362E3134: idx=sc_5_1_4_1_1_481_6;break;//RTBrachyTreatmentRecordStorage
            case 0x372E3134: idx=sc_5_1_4_1_1_481_7;break;//RTTreatmentSummaryRecordStorage
            case 0x382E3134: idx=sc_5_1_4_1_1_481_8;break;//RTIonPlanStorage
            case 0x392E3134: idx=sc_5_1_4_1_1_481_9;break;//RTIonBeamsTreatmentRecordStorage
            case 0x312E3135: idx=sc_5_1_4_1_1_501_1;break;//DICOSCTImageStorage
            case 0x332E3135: idx=sc_5_1_4_1_1_501_3;break;//DICOSThreatDetectionReportStorage
            case 0x342E3135: idx=sc_5_1_4_1_1_501_4;break;//DICOS2DAITStorage
            case 0x352E3135: idx=sc_5_1_4_1_1_501_5;break;//DICOS3DAITStorage
            case 0x362E3135: idx=sc_5_1_4_1_1_501_6;break;//DICOSQuadrupoleResonanceStorage
            case 0x312E3136: idx=sc_5_1_4_1_1_601_1;break;//EddyCurrentImageStorage
            case 0x322E3136: idx=sc_5_1_4_1_1_601_1;break;//EddyCurrentMultiFrameImageStorage
         }};break;
         
      case 30:{
         switch ((vbuf[29] << 24) | (vbuf[28]<<16) | (vbuf[27]<<8) | vbuf[25]) {
            case 0x312E3132: idx=sc_5_1_4_1_1_12_1_1;break;//EnhancedXAImageStorage
            case 0x312E3232: idx=sc_5_1_4_1_1_12_2_1;break;//EnhancedXRFImageStorage
            case 0x312E3133: idx=sc_5_1_4_1_1_13_1_1;break;//XRay3DAngiographicImageStorage
            case 0x322E3133: idx=sc_5_1_4_1_1_13_1_2;break;//XRay3DCraniofacialImageStorage
            case 0x332E3133: idx=sc_5_1_4_1_1_13_1_3;break;//BreastTomosynthesisImageStorage
            case 0x342E3133: idx=sc_5_1_4_1_1_13_1_4;break;//BreastProjectionXRayImageStorageForPresentation
            case 0x352E3133: idx=sc_5_1_4_1_1_13_1_5;break;//BreastProjectionXRayImageStorageForProcessing
            case 0x312E3137: idx=sc_5_1_4_1_1_77_1_1;break;//VLEndoscopicImageStorage
            case 0x322E3137: idx=sc_5_1_4_1_1_77_1_2;break;//VLMicroscopicImageStorage
            case 0x332E3137: idx=sc_5_1_4_1_1_77_1_3;break;//VLSlideCoordinatesMicroscopicImageStorage
            case 0x342E3137: idx=sc_5_1_4_1_1_77_1_4;break;//VLPhotographicImageStorage
            case 0x362E3137: idx=sc_5_1_4_1_1_77_1_6;break;//VLWholeSlideMicroscopyImageStorage
            case 0x372E3137: idx=sc_5_1_4_1_1_77_1_7;break;//DermoscopicPhotographyImageStorage
            case 0x382E3137: idx=sc_5_1_4_1_1_77_1_8;break;//ConfocalMicroscopyImageStorage
            case 0x392E3137: idx=sc_5_1_4_1_1_77_1_9;break;//ConfocalMicroscopyTiledPyramidalImageStorage
            case 0x30312E38: idx=sc_5_1_4_1_1_481_10;break;//RTPhysicianIntentStorage
            case 0x31312E38: idx=sc_5_1_4_1_1_481_11;break;//RTSegmentAnnotationStorage
            case 0x32312E38: idx=sc_5_1_4_1_1_481_12;break;//RTRadiationSetStorage
            case 0x33312E38: idx=sc_5_1_4_1_1_481_13;break;//CArmPhotonElectronRadiationStorage
            case 0x34312E38: idx=sc_5_1_4_1_1_481_14;break;//TomotherapeuticRadiationStorage
            case 0x35312E38: idx=sc_5_1_4_1_1_481_15;break;//RoboticArmRadiationStorage
            case 0x36312E38: idx=sc_5_1_4_1_1_481_16;break;//RTRadiationRecordSetStorage
            case 0x37312E38: idx=sc_5_1_4_1_1_481_17;break;//RTRadiationSalvageRecordStorage
            case 0x38312E38: idx=sc_5_1_4_1_1_481_18;break;//TomotherapeuticRadiationRecordStorage
            case 0x39312E38: idx=sc_5_1_4_1_1_481_19;break;//CArmPhotonElectronRadiationRecordStorage
            case 0x30322E38: idx=sc_5_1_4_1_1_481_20;break;//RoboticRadiationRecordStorage
            case 0x31322E38: idx=sc_5_1_4_1_1_481_21;break;//RTRadiationSetDeliveryInstructionStorage
            case 0x32322E38: idx=sc_5_1_4_1_1_481_22;break;//RTTreatmentPreparationStorage
            case 0x33322E38: idx=sc_5_1_4_1_1_481_23;break;//EnhancedRTImageStorage
            case 0x34322E38: idx=sc_5_1_4_1_1_481_24;break;//EnhancedContinuousRTImageStorage
            case 0x35322E38: idx=sc_5_1_4_1_1_481_25;break;//RTPatientPositionAcquisitionInstructionStorage
         }};break;

      case 31:{
         switch (vbuf[30]){
            case 0x31: idx=sc_5_1_4_1_1_501_2_1;break;//DICOSDigitalXRayImageStorageForPresentation
            case 0x32: idx=sc_5_1_4_1_1_501_2_2;break;//DICOSDigitalXRayImageStorageForProcessing
         }};break;
         
      case 32:{
         switch ((vbuf[31] << 24) | (vbuf[30]<<16) | (vbuf[29]<<8) | vbuf[28]) {
            case 0x312E312E: idx=sc_5_1_4_1_1_77_1_1_1;break;//VideoEndoscopicImageStorage
            case 0x312E322E: idx=sc_5_1_4_1_1_77_1_2_1;break;//VideoMicroscopicImageStorage
            case 0x312E342E: idx=sc_5_1_4_1_1_77_1_4_1;break;//VideoPhotographicImageStorage
            case 0x312E352E: idx=sc_5_1_4_1_1_77_1_5_1;break;//OphthalmicPhotography8BitImageStorage
            case 0x322E352E: idx=sc_5_1_4_1_1_77_1_5_2;break;//OphthalmicPhotography16BitImageStorage
            case 0x332E352E: idx=sc_5_1_4_1_1_77_1_5_3;break;//StereometricRelationshipStorage
            case 0x342E352E: idx=sc_5_1_4_1_1_77_1_5_4;break;//OphthalmicTomographyImageStorage
            case 0x352E352E: idx=sc_5_1_4_1_1_77_1_5_5;break;//WideFieldOphthalmicPhotographyStereographicProjectionImageStorage
            case 0x362E352E: idx=sc_5_1_4_1_1_77_1_5_6;break;//WideFieldOphthalmicPhotography3DCoordinatesImageStorage
            case 0x372E352E: idx=sc_5_1_4_1_1_77_1_5_7;break;//OphthalmicOpticalCoherenceTomographyEnFaceImageStorage
            case 0x382E352E: idx=sc_5_1_4_1_1_77_1_5_8;break;//OphthalmicOpticalCoherenceTomographyBscanVolumeAnalysisStorage
         }};break;
      default:{
         
      };break;//error
   };
   
   if (strncmp(scstr[idx],(char*)vbuf,strlen(scstr[idx]))==0) return idx;
   else return 0;//error (verification)
}

const bool scisimage[]={
false,//Verification
false,//MediaStorageDirectoryStorage
false,//BasicStudyContentNotification
false,//StorageCommitmentPushModel
false,//StorageCommitmentPullModel (Retired)
false,//ProceduralEventLogging"
false,//SubstanceAdministrationLogging
false,//DetachedPatientManagement (Retired)
false,//DetachedPatientManagementMeta (Retired)
false,//DetachedVisitManagement (Retired)
false,//DetachedStudyManagement (Retired)
false,//StudyComponentManagement (Retired)
false,//ModalityPerformedProcedureStep
false,//ModalityPerformedProcedureStepRetrieve
false,//ModalityPerformedProcedureStepNotification
false,//DetachedResultsManagement (Retired)
false,//DetachedResultsManagementMeta (Retired)
false,//DetachedStudyManagementMeta (Retired)
false,//DetachedInterpretationManagement (Retired)
false,//BasicFilmSession
false,//BasicFilmBox
false,//BasicGrayscaleImageBox
false,//BasicColorImageBox
false,//ReferencedImageBox
false,//BasicGrayscalePrintManagementMeta
false,//ReferencedGrayscalePrintManagementMeta
false,//PrintJob
false,//BasicAnnotationBox
false,//Printer
false,//PrinterConfigurationRetrieval
false,//BasicColorPrintManagementMeta
false,//ReferencedColorPrintManagementMeta (Retired)
false,//VOILUTBox
false,//PresentationLUT
false,//ImageOverlayBox (Retired)
false,//BasicPrintImageOverlayBox (Retired)
false,//PrintQueueManagement (Retired)
false,//StoredPrintStorage (Retired)
false,//HardcopyGrayscaleImageStorage (Retired)
false,//HardcopyColorImageStorage (Retired)
false,//PullPrintRequest (Retired)
false,//PullStoredPrintManagementMeta (Retired)
false,//MediaCreationManagement
false,//DisplaySystem
   true,//ComputedRadiographyImageStorage
   true,//DigitalXRayImageStorageForPresentation
   true,//DigitalXRayImageStorageForProcessing
   true,//DigitalMammographyXRayImageStorageForPresentation
   true,//DigitalMammographyXRayImageStorageForProcessing
   true,//DigitalIntraOralXRayImageStorageForPresentation
   true,//DigitalIntraOralXRayImageStorageForProcessing
   true,//CTImageStorage
   true,//EnhancedCTImageStorage
   true,//LegacyConvertedEnhancedCTImageStorage
   true,//UltrasoundMultiFrameImageStorageRetired (Retired)
   true,//UltrasoundMultiFrameImageStorage
   true,//MRImageStorage
   true,//EnhancedMRImageStorage
   true,//MRSpectroscopyStorage
   true,//EnhancedMRColorImageStorage
   true,//LegacyConvertedEnhancedMRImageStorage
   true,//NuclearMedicineImageStorageRetired (Retired)
   true,//UltrasoundImageStorageRetired (Retired)
   true,//UltrasoundImageStorage
   true,//EnhancedUSVolumeStorage
   true,//PhotoacousticImageStorage
   true,//SecondaryCaptureImageStorage
   true,//MultiFrameSingleBitSecondaryCaptureImageStorage
   true,//MultiFrameGrayscaleByteSecondaryCaptureImageStorage
   true,//MultiFrameGrayscaleWordSecondaryCaptureImageStorage
   true,//MultiFrame   trueColorSecondaryCaptureImageStorage
false,//StandaloneOverlayStorage (Retired)
false,//StandaloneCurveStorage (Retired)
false,//WaveformStorageTrial (Retired)
false,//TwelveLeadECGWaveformStorage
false,//GeneralECGWaveformStorage
false,//AmbulatoryECGWaveformStorage
false,//General32bitECGWaveformStorage
false,//HemodynamicWaveformStorage
false,//CardiacElectrophysiologyWaveformStorage
false,//BasicVoiceAudioWaveformStorage
false,//GeneralAudioWaveformStorage
false,//ArterialPulseWaveformStorage
false,//RespiratoryWaveformStorage
false,//MultichannelRespiratoryWaveformStorage
false,//RoutineScalpElectroencephalogramWaveformStorage
false,//ElectromyogramWaveformStorage
false,//ElectrooculogramWaveformStorage
false,//SleepElectroencephalogramWaveformStorage
false,//BodyPositionWaveformStorage
false,//StandaloneModalityLUTStorage (Retired)
false,//StandaloneVOILUTStorage (Retired)
false,//GrayscaleSoftcopyPresentationStateStorage
false,//ColorSoftcopyPresentationStateStorage
false,//PseudoColorSoftcopyPresentationStateStorage
false,//BlendingSoftcopyPresentationStateStorage
false,//XAXRFGrayscaleSoftcopyPresentationStateStorage
false,//GrayscalePlanarMPRVolumetricPresentationStateStorage
false,//CompositingPlanarMPRVolumetricPresentationStateStorage
false,//AdvancedBlendingPresentationStateStorage
false,//VolumeRenderingVolumetricPresentationStateStorage
false,//SegmentedVolumeRenderingVolumetricPresentationStateStorage
false,//MultipleVolumeRenderingVolumetricPresentationStateStorage
false,//VariableModalityLUTSoftcopyPresentationStateStorage
   true,//XRayAngiographicImageStorage
   true,//EnhancedXAImageStorage
   true,//XRayRadiofluoroscopicImageStorage
   true,//EnhancedXRFImageStorage
   true,//XRayAngiographicBiPlaneImageStorage (Retired)
false,//(Retired)
   true,//XRay3DAngiographicImageStorage
   true,//XRay3DCraniofacialImageStorage
   true,//BreastTomosynthesisImageStorage
   true,//BreastProjectionXRayImageStorageForPresentation
   true,//BreastProjectionXRayImageStorageForProcessing
   true,//IntravascularOpticalCoherenceTomographyImageStorageForPresentation
   true,//IntravascularOpticalCoherenceTomographyImageStorageForProcessing
   true,//NuclearMedicineImageStorage
   true,//ParametricMapStorage
false,//(Retired)
false,//RawDataStorage
false,//SpatialRegistrationStorage
false,//SpatialFiducialsStorage
false,//DeformableSpatialRegistrationStorage
   true,//SegmentationStorage
false,//SurfaceSegmentationStorage
false,//TractographyResultsStorage
false,//RealWorldValueMappingStorage
false,//SurfaceScanMeshStorage
false,//SurfaceScanPointCloudStorage
   true,//VLImageStorageTrial (Retired)
   true,//VLMultiFrameImageStorageTrial (Retired)
   true,//VLEndoscopicImageStorage
   true,//VideoEndoscopicImageStorage
   true,//VLMicroscopicImageStorage
   true,//VideoMicroscopicImageStorage
   true,//VLSlideCoordinatesMicroscopicImageStorage
   true,//VLPhotographicImageStorage
   true,//VideoPhotographicImageStorage
   true,//OphthalmicPhotography8BitImageStorage
   true,//OphthalmicPhotography16BitImageStorage
false,//StereometricRelationshipStorage
   true,//OphthalmicTomographyImageStorage
   true,//WideFieldOphthalmicPhotographyStereographicProjectionImageStorage
   true,//WideFieldOphthalmicPhotography3DCoordinatesImageStorage
   true,//OphthalmicOpticalCoherenceTomographyEnFaceImageStorage
false,//OphthalmicOpticalCoherenceTomographyBscanVolumeAnalysisStorage
   true,//VLWholeSlideMicroscopyImageStorage
   true,//DermoscopicPhotographyImageStorage
   true,//ConfocalMicroscopyImageStorage
   true,//ConfocalMicroscopyTiledPyramidalImageStorage
false,//LensometryMeasurementsStorage
false,//AutorefractionMeasurementsStorage
false,//KeratometryMeasurementsStorage
false,//SubjectiveRefractionMeasurementsStorage
false,//VisualAcuityMeasurementsStorage
false,//SpectaclePrescriptionReportStorage
false,//OphthalmicAxialMeasurementsStorage
false,//IntraocularLensCalculationsStorage
false,//MacularGridThicknessAndVolumeReportStorage
false,//OphthalmicVisualFieldStaticPerimetryMeasurementsStorage
false,//OphthalmicThicknessMapStorage
false,//CornealTopographyMapStorage
false,//TextSRStorageTrial (Retired)
false,//AudioSRStorageTrial (Retired)
false,//DetailSRStorageTrial (Retired)
false,//ComprehensiveSRStorageTrial (Retired)
false,//BasicTextSRStorage
false,//EnhancedSRStorage
false,//ComprehensiveSRStorage
false,//Comprehensive3DSRStorage
false,//ExtensibleSRStorage
false,//ProcedureLogStorage
false,//MammographyCADSRStorage
false,//KeyObjectSelectionDocumentStorage
false,//ChestCADSRStorage
false,//XRayRadiationDoseSRStorage
false,//RadiopharmaceuticalRadiationDoseSRStorage
false,//ColonCADSRStorage
false,//ImplantationPlanSRStorage
false,//AcquisitionContextSRStorage
false,//SimplifiedAdultEchoSRStorage
false,//PatientRadiationDoseSRStorage
false,//PlannedImagingAgentAdministrationSRStorage
false,//PerformedImagingAgentAdministrationSRStorage
false,//EnhancedXRayRadiationDoseSRStorage
false,//ContentAssessmentResultsStorage
false,//MicroscopyBulkSimpleAnnotationsStorage
false,//EncapsulatedPDFStorage
false,//EncapsulatedCDAStorage
false,//EncapsulatedSTLStorage
false,//EncapsulatedOBJStorage
false,//EncapsulatedMTLStorage
   true,//PositronEmissionTomographyImageStorage
   true,//LegacyConvertedEnhancedPETImageStorage
false,//StandalonePETCurveStorage
   true,//EnhancedPETImageStorage
false,//BasicStructuredDisplayStorage
false,//CTDefinedProcedureProtocolStorage
false,//CTPerformedProcedureProtocolStorage
false,//ProtocolApprovalStorage
false,//ProtocolApprovalInformationModelFind
false,//ProtocolApprovalInformationModelMove
false,//ProtocolApprovalInformationModelGet
false,//XADefinedProcedureProtocolStorage
false,//XAPerformedProcedureProtocolStorage
false,//InventoryStorage
false,//InventoryFind
false,//InventoryMove
false,//InventoryGet
false,//InventoryCreation
false,//RepositoryQuery
   true,//RTImageStorage
false,//RTDoseStorage
false,//RTStructureSetStorage
false,//RTBeamsTreatmentRecordStorage
false,//RTPlanStorage
false,//RTBrachyTreatmentRecordStorage
false,//RTTreatmentSummaryRecordStorage
false,//RTIonPlanStorage
false,//RTIonBeamsTreatmentRecordStorage
false,//RTPhysicianIntentStorage
false,//RTSegmentAnnotationStorage
false,//RTRadiationSetStorage
false,//CArmPhotonElectronRadiationStorage
false,//TomotherapeuticRadiationStorage
false,//RoboticArmRadiationStorage
false,//RTRadiationRecordSetStorage
false,//RTRadiationSalvageRecordStorage
false,//TomotherapeuticRadiationRecordStorage
false,//CArmPhotonElectronRadiationRecordStorage
false,//RoboticRadiationRecordStorage
false,//RTRadiationSetDeliveryInstructionStorage
false,//RTTreatmentPreparationStorage
   true,//EnhancedRTImageStorage
   true,//EnhancedContinuousRTImageStorage
false,//RTPatientPositionAcquisitionInstructionStorage
   true,//DICOSCTImageStorage
   true,//DICOSDigitalXRayImageStorageForPresentation
   true,//DICOSDigitalXRayImageStorageForProcessing
false,//DICOSThreatDetectionReportStorage
false,//DICOS2DAITStorage
false,//DICOS3DAITStorage
false,//DICOSQuadrupoleResonanceStorage
   true,//EddyCurrentImageStorage
   true,//EddyCurrentMultiFrameImageStorage
false,//PatientRootQueryRetrieveInformationModelFind
false,//PatientRootQueryRetrieveInformationModelMove
false,//PatientRootQueryRetrieveInformationModelGet
false,//StudyRootQueryRetrieveInformationModelFind
false,//StudyRootQueryRetrieveInformationModelMove
false,//StudyRootQueryRetrieveInformationModelGet
false,//PatientStudyOnlyQueryRetrieveInformationModelFind
false,//PatientStudyOnlyQueryRetrieveInformationModelMove
false,//PatientStudyOnlyQueryRetrieveInformationModelGet
false,//CompositeInstanceRootRetrieveMove
false,//CompositeInstanceRootRetrieveGet
false,//CompositeInstanceRetrieveWithoutBulkDataGet
false,//DefinedProcedureProtocolInformationModelFind
false,//DefinedProcedureProtocolInformationModelMove
false,//DefinedProcedureProtocolInformationModelGet
false,//ModalityWorklistInformationModelFind
false,//GeneralPurposeWorklistManagementMeta
false,//GeneralPurposeWorklistInformationModelFind
false,//GeneralPurposeScheduledProcedureStep
false,//GeneralPurposePerformedProcedureStep
false,//InstanceAvailabilityNotification
false,//RTBeamsDeliveryInstructionStorageTrial
false,//RTConventionalMachineVerificationTrial
false,//RTIonMachineVerificationTrial (Retired)
false,//UnifiedWorklistAndProcedureStepTrial (Retired)
false,//UnifiedProcedureStepPushTrial (Retired)
false,//UnifiedProcedureStepWatchTrial (Retired)
false,//UnifiedProcedureStepPullTrial (Retired)
false,//UnifiedProcedureStepEventTrial
false,//UPSGlobalSubscriptionInstance
false,//UPSFilteredGlobalSubscriptionInstance
false,//UnifiedWorklistAndProcedureStep
false,//UnifiedProcedureStepPush
false,//UnifiedProcedureStepWatch
false,//UnifiedProcedureStepPull
false,//UnifiedProcedureStepEvent
false,//UnifiedProcedureStepQuery
false,//RTBeamsDeliveryInstructionStorage
false,//RTConventionalMachineVerification
false,//RTIonMachineVerification
false,//RTBrachyApplicationSetupDeliveryInstructionStorage
false,//GeneralRelevantPatientInformationQuery
false,//BreastImagingRelevantPatientInformationQuery
false,//CardiacRelevantPatientInformationQuery
false,//HangingProtocolStorage
false,//HangingProtocolInformationModelFind
false,//HangingProtocolInformationModelMove
false,//HangingProtocolInformationModelGet
false,//ColorPaletteStorage
false,//ColorPaletteQueryRetrieveInformationModelFind
false,//ColorPaletteQueryRetrieveInformationModelMove
false,//ColorPaletteQueryRetrieveInformationModelGet
false,//ProductCharacteristicsQuery
false,//SubstanceApprovalQuery
false,//GenericImplantTemplateStorage
false,//GenericImplantTemplateInformationModelFind
false,//GenericImplantTemplateInformationModelMove
false,//GenericImplantTemplateInformationModelGet
false,//ImplantAssemblyTemplateStorage
false,//ImplantAssemblyTemplateInformationModelFind
false,//ImplantAssemblyTemplateInformationModelMove
false,//ImplantAssemblyTemplateInformationModelGet
false,//ImplantTemplateGroupStorage
false,//ImplantTemplateGroupInformationModelFind
false,//ImplantTemplateGroupInformationModelMove
false,//ImplantTemplateGroupInformationModelGet
false,//VideoEndoscopicImageRealTimeCommunication
false,//VideoPhotographicImageRealTimeCommunication
false,//AudioWaveformRealTimeCommunication
false//RenditionSelectionDocumentRealTimeCommunication
};

bool sopclassidxisimage( u16 sopclassidx)
{
   return scisimage[sopclassidx];
}

const bool scisframes[]={
false,//Verification
false,//MediaStorageDirectoryStorage
false,//BasicStudyContentNotification
false,//StorageCommitmentPushModel
false,//StorageCommitmentPullModel (Retired)
false,//ProceduralEventLogging"
false,//SubstanceAdministrationLogging
false,//DetachedPatientManagement (Retired)
false,//DetachedPatientManagementMeta (Retired)
false,//DetachedVisitManagement (Retired)
false,//DetachedStudyManagement (Retired)
false,//StudyComponentManagement (Retired)
false,//ModalityPerformedProcedureStep
false,//ModalityPerformedProcedureStepRetrieve
false,//ModalityPerformedProcedureStepNotification
false,//DetachedResultsManagement (Retired)
false,//DetachedResultsManagementMeta (Retired)
false,//DetachedStudyManagementMeta (Retired)
false,//DetachedInterpretationManagement (Retired)
false,//BasicFilmSession
false,//BasicFilmBox
false,//BasicGrayscaleImageBox
false,//BasicColorImageBox
false,//ReferencedImageBox
false,//BasicGrayscalePrintManagementMeta
false,//ReferencedGrayscalePrintManagementMeta
false,//PrintJob
false,//BasicAnnotationBox
false,//Printer
false,//PrinterConfigurationRetrieval
false,//BasicColorPrintManagementMeta
false,//ReferencedColorPrintManagementMeta (Retired)
false,//VOILUTBox
false,//PresentationLUT
false,//ImageOverlayBox (Retired)
false,//BasicPrintImageOverlayBox (Retired)
false,//PrintQueueManagement (Retired)
false,//StoredPrintStorage (Retired)
false,//HardcopyGrayscaleImageStorage (Retired)
false,//HardcopyColorImageStorage (Retired)
false,//PullPrintRequest (Retired)
false,//PullStoredPrintManagementMeta (Retired)
false,//MediaCreationManagement
false,//DisplaySystem
false,//ComputedRadiographyImageStorage
false,//DigitalXRayImageStorageForPresentation
false,//DigitalXRayImageStorageForProcessing
false,//DigitalMammographyXRayImageStorageForPresentation
false,//DigitalMammographyXRayImageStorageForProcessing
false,//DigitalIntraOralXRayImageStorageForPresentation
false,//DigitalIntraOralXRayImageStorageForProcessing
false,//CTImageStorage
      true,//Enh CT EnhancedCTImageStorage
      true,//LegacyConvertedEnhancedCTImageStorage
      true,//UltrasoundMultiFrameImageStorageRetired (Retired)
      true,//US MF UltrasoundMultiFrameImageStorage
false,//MRImageStorage
       true,//Enh MR EnhancedMRImageStorage
false,//MRSpectroscopyStorage
       true,//Enh MR Col EnhancedMRColorImageStorage
       true,//LegacyConvertedEnhancedMRImageStorage
       true,//NM NuclearMedicineImageStorageRetired (Retired)
false,//UltrasoundImageStorageRetired (Retired)
false,//UltrasoundImageStorage
       true,//Enh US Vol EnhancedUSVolumeStorage
       true,//PA PhotoacousticImageStorage
false,//SecondaryCaptureImageStorage
       true,//SC MF SB MultiFrameSingleBitSecondaryCaptureImageStorage
       true,//SC MF GB MultiFrameGrayscaleByteSecondaryCaptureImageStorage
       true,//SC MF GW MultiFrameGrayscaleWordSecondaryCaptureImageStorage
       true,//SC MF TC MultiFrameTrueColorSecondaryCaptureImageStorage
false,//StandaloneOverlayStorage (Retired)
false,//StandaloneCurveStorage (Retired)
false,//WaveformStorageTrial (Retired)
false,//TwelveLeadECGWaveformStorage
false,//GeneralECGWaveformStorage
false,//AmbulatoryECGWaveformStorage
false,//General32bitECGWaveformStorage
false,//HemodynamicWaveformStorage
false,//CardiacElectrophysiologyWaveformStorage
false,//BasicVoiceAudioWaveformStorage
false,//GeneralAudioWaveformStorage
false,//ArterialPulseWaveformStorage
false,//RespiratoryWaveformStorage
false,//MultichannelRespiratoryWaveformStorage
false,//RoutineScalpElectroencephalogramWaveformStorage
false,//ElectromyogramWaveformStorage
false,//ElectrooculogramWaveformStorage
false,//SleepElectroencephalogramWaveformStorage
false,//BodyPositionWaveformStorage
false,//StandaloneModalityLUTStorage (Retired)
false,//StandaloneVOILUTStorage (Retired)
false,//GrayscaleSoftcopyPresentationStateStorage
false,//ColorSoftcopyPresentationStateStorage
false,//PseudoColorSoftcopyPresentationStateStorage
false,//BlendingSoftcopyPresentationStateStorage
false,//XAXRFGrayscaleSoftcopyPresentationStateStorage
false,//GrayscalePlanarMPRVolumetricPresentationStateStorage
false,//CompositingPlanarMPRVolumetricPresentationStateStorage
false,//AdvancedBlendingPresentationStateStorage
false,//VolumeRenderingVolumetricPresentationStateStorage
false,//SegmentedVolumeRenderingVolumetricPresentationStateStorage
false,//MultipleVolumeRenderingVolumetricPresentationStateStorage
false,//VariableModalityLUTSoftcopyPresentationStateStorage
      true,//XA XRayAngiographicImageStorage
      true,//Enh XA EnhancedXAImageStorage
      true,//RF XRayRadiofluoroscopicImageStorage
      true,//Enh RF EnhancedXRFImageStorage
      true,//XRayAngiographicBiPlaneImageStorage (Retired)
false,//(Retired)
      true,//3D XA XRay3DAngiographicImageStorage
      true,//3S CF XRay3DCraniofacialImageStorage
      true,//Br Tomo BreastTomosynthesisImageStorage
      true,//Br Proj BreastProjectionXRayImageStorageForPresentation
      true,//Br Proj BreastProjectionXRayImageStorageForProcessing
      true,//IVOCT IntravascularOpticalCoherenceTomographyImageStorageForPresentation
      true,//IVOCT IntravascularOpticalCoherenceTomographyImageStorageForProcessing
      true,//NM NuclearMedicineImageStorage
      true,//Par Map ParametricMapStorage
false,//(Retired)
false,//RawDataStorage
false,//SpatialRegistrationStorage
false,//SpatialFiducialsStorage
false,//DeformableSpatialRegistrationStorage
      true,//Seg SegmentationStorage
false,//SurfaceSegmentationStorage
false,//TractographyResultsStorage
false,//RealWorldValueMappingStorage
false,//SurfaceScanMeshStorage
false,//SurfaceScanPointCloudStorage
false,//VLImageStorageTrial (Retired)
      true,//VLMultiFrameImageStorageTrial (Retired)
false,//VL EN VLEndoscopicImageStorage
      true,//Vid VL EN VideoEndoscopicImageStorage
false,//VLMicroscopicImageStorage
      true,//Vid VL MC VideoMicroscopicImageStorage
false,//VLSlideCoordinatesMicroscopicImageStorage
false,//VLPhotographicImageStorage
      true,//Vid VL PH VideoPhotographicImageStorage
      true,//Oph 8 bit OphthalmicPhotography8BitImageStorage
      true,//Oph 16 bit OphthalmicPhotography16BitImageStorage
false,//StereometricRelationshipStorage
      true,//OPT OphthalmicTomographyImageStorage
      true,//WF Oph St WideFieldOphthalmicPhotographyStereographicProjectionImageStorage
      true,//v 3DC WideFieldOphthalmicPhotography3DCoordinatesImageStorage
false,//OCT ENF OphthalmicOpticalCoherenceTomographyEnFaceImageStorage
      true,//OCT BSV OphthalmicOpticalCoherenceTomographyBscanVolumeAnalysisStorage
      true,//VL WS VLWholeSlideMicroscopyImageStorage
false,//DermoscopicPhotographyImageStorage
      true,//CFM ConfocalMicroscopyImageStorage
      true,//CFM TP ConfocalMicroscopyTiledPyramidalImageStorage
false,//LensometryMeasurementsStorage
false,//AutorefractionMeasurementsStorage
false,//KeratometryMeasurementsStorage
false,//SubjectiveRefractionMeasurementsStorage
false,//VisualAcuityMeasurementsStorage
false,//SpectaclePrescriptionReportStorage
false,//OphthalmicAxialMeasurementsStorage
false,//IntraocularLensCalculationsStorage
false,//MacularGridThicknessAndVolumeReportStorage
false,//OphthalmicVisualFieldStaticPerimetryMeasurementsStorage
false,//OphthalmicThicknessMapStorage
false,//CornealTopographyMapStorage
false,//TextSRStorageTrial (Retired)
false,//AudioSRStorageTrial (Retired)
false,//DetailSRStorageTrial (Retired)
false,//ComprehensiveSRStorageTrial (Retired)
false,//BasicTextSRStorage
false,//EnhancedSRStorage
false,//ComprehensiveSRStorage
false,//Comprehensive3DSRStorage
false,//ExtensibleSRStorage
false,//ProcedureLogStorage
false,//MammographyCADSRStorage
false,//KeyObjectSelectionDocumentStorage
false,//ChestCADSRStorage
false,//XRayRadiationDoseSRStorage
false,//RadiopharmaceuticalRadiationDoseSRStorage
false,//ColonCADSRStorage
false,//ImplantationPlanSRStorage
false,//AcquisitionContextSRStorage
false,//SimplifiedAdultEchoSRStorage
false,//PatientRadiationDoseSRStorage
false,//PlannedImagingAgentAdministrationSRStorage
false,//PerformedImagingAgentAdministrationSRStorage
false,//EnhancedXRayRadiationDoseSRStorage
false,//ContentAssessmentResultsStorage
false,//MicroscopyBulkSimpleAnnotationsStorage
false,//EncapsulatedPDFStorage
false,//EncapsulatedCDAStorage
false,//EncapsulatedSTLStorage
false,//EncapsulatedOBJStorage
false,//EncapsulatedMTLStorage
false,//PositronEmissionTomographyImageStorage
      true,//LegacyConvertedEnhancedPETImageStorage
false,//StandalonePETCurveStorage
      true,//Enh PT EnhancedPETImageStorage
false,//BasicStructuredDisplayStorage
false,//CTDefinedProcedureProtocolStorage
false,//CTPerformedProcedureProtocolStorage
false,//ProtocolApprovalStorage
false,//ProtocolApprovalInformationModelFind
false,//ProtocolApprovalInformationModelMove
false,//ProtocolApprovalInformationModelGet
false,//XADefinedProcedureProtocolStorage
false,//XAPerformedProcedureProtocolStorage
false,//InventoryStorage
false,//InventoryFind
false,//InventoryMove
false,//InventoryGet
false,//InventoryCreation
false,//RepositoryQuery
false,//RTImageStorage
false,//RTDoseStorage
false,//RTStructureSetStorage
false,//RTBeamsTreatmentRecordStorage
false,//RTPlanStorage
false,//RTBrachyTreatmentRecordStorage
false,//RTTreatmentSummaryRecordStorage
false,//RTIonPlanStorage
false,//RTIonBeamsTreatmentRecordStorage
false,//RTPhysicianIntentStorage
false,//RTSegmentAnnotationStorage
false,//RTRadiationSetStorage
false,//CArmPhotonElectronRadiationStorage
false,//TomotherapeuticRadiationStorage
false,//RoboticArmRadiationStorage
false,//RTRadiationRecordSetStorage
false,//RTRadiationSalvageRecordStorage
false,//TomotherapeuticRadiationRecordStorage
false,//CArmPhotonElectronRadiationRecordStorage
false,//RoboticRadiationRecordStorage
false,//RTRadiationSetDeliveryInstructionStorage
false,//RTTreatmentPreparationStorage
      true,//EnhancedRTImageStorage
      true,//EnhancedContinuousRTImageStorage
false,//RTPatientPositionAcquisitionInstructionStorage
false,//DICOSCTImageStorage
false,//DICOSDigitalXRayImageStorageForPresentation
false,//DICOSDigitalXRayImageStorageForProcessing
false,//DICOSThreatDetectionReportStorage
false,//DICOS2DAITStorage
false,//DICOS3DAITStorage
false,//DICOSQuadrupoleResonanceStorage
false,//EddyCurrentImageStorage
      true,//EddyCurrentMultiFrameImageStorage
false,//PatientRootQueryRetrieveInformationModelFind
false,//PatientRootQueryRetrieveInformationModelMove
false,//PatientRootQueryRetrieveInformationModelGet
false,//StudyRootQueryRetrieveInformationModelFind
false,//StudyRootQueryRetrieveInformationModelMove
false,//StudyRootQueryRetrieveInformationModelGet
false,//PatientStudyOnlyQueryRetrieveInformationModelFind
false,//PatientStudyOnlyQueryRetrieveInformationModelMove
false,//PatientStudyOnlyQueryRetrieveInformationModelGet
false,//CompositeInstanceRootRetrieveMove
false,//CompositeInstanceRootRetrieveGet
false,//CompositeInstanceRetrieveWithoutBulkDataGet
false,//DefinedProcedureProtocolInformationModelFind
false,//DefinedProcedureProtocolInformationModelMove
false,//DefinedProcedureProtocolInformationModelGet
false,//ModalityWorklistInformationModelFind
false,//GeneralPurposeWorklistManagementMeta
false,//GeneralPurposeWorklistInformationModelFind
false,//GeneralPurposeScheduledProcedureStep
false,//GeneralPurposePerformedProcedureStep
false,//InstanceAvailabilityNotification
false,//RTBeamsDeliveryInstructionStorageTrial
false,//RTConventionalMachineVerificationTrial
false,//RTIonMachineVerificationTrial (Retired)
false,//UnifiedWorklistAndProcedureStepTrial (Retired)
false,//UnifiedProcedureStepPushTrial (Retired)
false,//UnifiedProcedureStepWatchTrial (Retired)
false,//UnifiedProcedureStepPullTrial (Retired)
false,//UnifiedProcedureStepEventTrial
false,//UPSGlobalSubscriptionInstance
false,//UPSFilteredGlobalSubscriptionInstance
false,//UnifiedWorklistAndProcedureStep
false,//UnifiedProcedureStepPush
false,//UnifiedProcedureStepWatch
false,//UnifiedProcedureStepPull
false,//UnifiedProcedureStepEvent
false,//UnifiedProcedureStepQuery
false,//RTBeamsDeliveryInstructionStorage
false,//RTConventionalMachineVerification
false,//RTIonMachineVerification
false,//RTBrachyApplicationSetupDeliveryInstructionStorage
false,//GeneralRelevantPatientInformationQuery
false,//BreastImagingRelevantPatientInformationQuery
false,//CardiacRelevantPatientInformationQuery
false,//HangingProtocolStorage
false,//HangingProtocolInformationModelFind
false,//HangingProtocolInformationModelMove
false,//HangingProtocolInformationModelGet
false,//ColorPaletteStorage
false,//ColorPaletteQueryRetrieveInformationModelFind
false,//ColorPaletteQueryRetrieveInformationModelMove
false,//ColorPaletteQueryRetrieveInformationModelGet
false,//ProductCharacteristicsQuery
false,//SubstanceApprovalQuery
false,//GenericImplantTemplateStorage
false,//GenericImplantTemplateInformationModelFind
false,//GenericImplantTemplateInformationModelMove
false,//GenericImplantTemplateInformationModelGet
false,//ImplantAssemblyTemplateStorage
false,//ImplantAssemblyTemplateInformationModelFind
false,//ImplantAssemblyTemplateInformationModelMove
false,//ImplantAssemblyTemplateInformationModelGet
false,//ImplantTemplateGroupStorage
false,//ImplantTemplateGroupInformationModelFind
false,//ImplantTemplateGroupInformationModelMove
false,//ImplantTemplateGroupInformationModelGet
false,//VideoEndoscopicImageRealTimeCommunication
false,//VideoPhotographicImageRealTimeCommunication
false,//AudioWaveformRealTimeCommunication
false//RenditionSelectionDocumentRealTimeCommunication
};

bool sopclassidxisframes( u16 sopclassidx)
{
   return scisimage[sopclassidx];
}


#pragma mark - TransfertSyntaxes

const char *tsstr[]={
"1.2.840.10008.1.​20",        //Papyrus​3​Implicit​VR​Little​Endian      Retired
"1.2.840.10008.1.​2",         //Implicit​VR​Little​Endian
"1.2.840.10008.1.​2.​1",       //Explicit​VR​Little​Endian
"1.2.840.10008.1.​2.​1.98",    //Encapsulated​Uncompressed​Explicit​VR​Little​Endian
"1.2.840.10008.1.2.​1.​99",    //Deflated​Explicit​VR​Little​Endian
"1.2.840.10008.1.​2.​2",       //Explicit​VR​Big​Endian     Retired
"1.2.840.10008.1.2.​4.​50",    //JPEG​Baseline​8​Bit
"1.2.840.10008.1.2.​4.​51",    //JPEG​Extended​12​Bit
"1.2.840.10008.1.2.​4.​52",    //JPEG​Extended35      Retired
"1.2.840.10008.1.2.​4.​53",    //JPEG​Spectral​Selection​Non​Hierarchical68      Retired
"1.2.840.10008.1.2.​4.​54",    //JPEG​Spectral​Selection​Non​Hierarchical79      Retired
"1.2.840.10008.1.2.​4.​55",    //JPEG​Full​Progression​Non​Hierarchical1012      Retired
"1.2.840.10008.1.2.​4.​56",    //JPEG​Full​Progression​Non​Hierarchical1113      Retired
"1.2.840.10008.1.2.​4.​57",    //JPEG​Lossless
"1.2.840.10008.1.2.​4.​58",    //JPEG​Lossless​Non​Hierarchical​15      Retired
"1.2.840.10008.1.2.​4.​59",    //JPEG​Extended​Hierarchical​1618     Retired
"1.2.840.10008.1.2.​4.​60",    //JPEG​Extended​Hierarchical​1719     Retired
"1.2.840.10008.1.2.​4.​61",    //JPEG​Spectral​Selection​Hierarchical2022     Retired
"1.2.840.10008.1.2.​4.​62",    //JPEG​Spectral​Selection​Hierarchical2123     Retired
"1.2.840.10008.1.2.​4.​63",    //JPEG​Full​Progression​Hierarchical2426    Retired
"1.2.840.10008.1.2.​4.​64",    //JPEG​Full​Progression​Hierarchical2527    Retired
"1.2.840.10008.1.2.​4.​65",    //JPEG​Lossless​Hierarchical​28    Retired
"1.2.840.10008.1.2.​4.​66",    //JPEG​Lossless​Hierarchical​29    Retired
"1.2.840.10008.1.2.​4.​70",    //JPEG​Lossless​SV1
"1.2.840.10008.1.2.​4.​80",    //JPEG​LS​Lossless
"1.2.840.10008.1.2.​4.​81",    //JPEG​LS​Near​Lossless
"1.2.840.10008.1.2.​4.​90",    //JPEG​2000Lossless
"1.2.840.10008.1.2.​4.​91",    //JPEG​2000
"1.2.840.10008.1.2.​4.​92",    //JPEG​2000MCLossless
"1.2.840.10008.1.2.​4.​93",    //JPEG​2000MC
"1.2.840.10008.1.2.​4.​94",    //JPIP​Referenced
"1.2.840.10008.1.2.​4.​95",    //JPIP​Referenced​Deflate
"1.2.840.10008.1.2.​4.​100",  //MPEG2​MPML
"1.2.840.10008.1.2.​4.​100.1",//MPEG2​MPMLF
"1.2.840.10008.1.2.​4.​101",  //MPEG2​MPHL
"1.2.840.10008.1.2.​4.​101.1",//MPEG2​MPHLF
"1.2.840.10008.1.2.​4.​102",  //MPEG4​HP41
"1.2.840.10008.1.2.​4.​102.1",//MPEG4​HP41F
"1.2.840.10008.1.2.​4.​103",  //MPEG4​HP41BD
"1.2.840.10008.1.2.​4.​103.1",//MPEG4​HP41BDF
"1.2.840.10008.1.2.​4.​104",  //MPEG4​HP422D
"1.2.840.10008.1.2.​4.​104.1",//MPEG4​HP422DF
"1.2.840.10008.1.2.​4.​105",  //MPEG4​HP423D
"1.2.840.10008.1.2.​4.​105.1",//MPEG4​HP423DF
"1.2.840.10008.1.2.​4.​106",  //MPEG4​HP42STEREO
"1.2.840.10008.1.2.​4.​106.1",//MPEG4​HP42STEREOF
"1.2.840.10008.1.2.​4.107",  //HEVC​MP51
"1.2.840.10008.1.2.​4.108",  //HEVC​M10P51
"1.2.840.10008.1.2.​4.​201",  //HTJ2K​Lossless
"1.2.840.10008.1.2.​4.202",  //HTJ2K​Lossless​RPCL
"1.2.840.10008.1.2.​4.​203",  //HTJ2K
"1.2.840.10008.1.2.​4.​204",  //JPIP​HTJ2K​Referenced
"1.2.840.10008.1.2.​4.​205",  //JPIP​HTJ2K​Referenced​Deflate
"1.2.840.10008.1.​2.​5",      //RLE​Lossless
"1.2.840.10008.1.2.​6.​1",    //RFC​2557​MIME​Encapsulation
"1.2.840.10008.1.2.​6.​2",    //XML​Encoding
"1.2.840.10008.1.2.7.1",    //SMPTE​ST​211020​Uncompressed​Progressive​Active​Video
"1.2.840.10008.1.2.7.2",    //SMPTE​ST​211020​Uncompressed​Interlaced​Active​Video
"1.2.840.10008.1.2.7.3"     //SMPTE​ST​211030​PCMDigital​Audio
};

//ts_papyrus3ile = 0 = código de error
u8 tsidx( char *vbuf, u16 vallength )
{
   u8 idx=0x0;//verification / error
   
   switch (vallength){
      case 17: idx=ts_;break;//Implicit​VR​Little​Endian
      case 19:{
         switch (vbuf[18]) {
            case 0x31: idx=ts_1;break;//Explicit​VR​Little​Endian
            case 0x32: idx=ts_2;break;//Explicit​VR​Big​Endian     Retired
            case 0x35: idx=ts_5;break;//RLE​Lossless
            default : idx=ts_papyrus3ile;break;//error
         }};break;
      case 22:{
         switch ((vbuf[20]<<8)+vbuf[21]) {
            case 0x3938: idx=ts_1_98;break;//Encapsulated​Uncompressed​Explicit​VR​Little​Endian
            case 0x3939: idx=ts_1_99;break;//Deflated​Explicit​VR​Little​Endian
            case 0x3530: idx=ts_4_50;break;//JPEG​Baseline​8​Bit
            case 0x3531: idx=ts_4_51;break;//JPEG​Extended​12​Bit
            case 0x3532: idx=ts_4_52;break;//JPEG​Extended35      Retired
            case 0x3533: idx=ts_4_53;break;//JPEG​Spectral​Selection​Non​Hierarchical68      Retired
            case 0x3534: idx=ts_4_54;break;//JPEG​Spectral​Selection​Non​Hierarchical79      Retired
            case 0x3535: idx=ts_4_55;break;//JPEG​Full​Progression​Non​Hierarchical1012      Retired
            case 0x3536: idx=ts_4_56;break;//JPEG​Full​Progression​Non​Hierarchical1113      Retired
            case 0x3537: idx=ts_4_57;break;//JPEG​Lossless
            case 0x3538: idx=ts_4_58;break;//JPEG​Lossless​Non​Hierarchical​15      Retired
            case 0x3539: idx=ts_4_59;break;//JPEG​Extended​Hierarchical​1618     Retired
            case 0x3630: idx=ts_4_60;break;//JPEG​Extended​Hierarchical​1719     Retired
            case 0x3631: idx=ts_4_61;break;//JPEG​Spectral​Selection​Hierarchical2022     Retired
            case 0x3632: idx=ts_4_62;break;//JPEG​Spectral​Selection​Hierarchical2123     Retired
            case 0x3633: idx=ts_4_63;break;//JPEG​Full​Progression​Hierarchical2426    Retired
            case 0x3634: idx=ts_4_64;break;//JPEG​Full​Progression​Hierarchical2527    Retired
            case 0x3635: idx=ts_4_65;break;//JPEG​Lossless​Hierarchical​28    Retired
            case 0x3636: idx=ts_4_66;break;//JPEG​Lossless​Hierarchical​29    Retired
            case 0x3730: idx=ts_4_70;break;//JPEG​Lossless​SV1
            case 0x3830: idx=ts_4_80;break;//JPEG​LS​Lossless
            case 0x3831: idx=ts_4_81;break;//JPEG​LS​Near​Lossless
            case 0x3930: idx=ts_4_90;break;//JPEG​2000Lossless
            case 0x3931: idx=ts_4_91;break;//JPEG​2000
            case 0x3932: idx=ts_4_92;break;//JPEG​2000MCLossless
            case 0x3933: idx=ts_4_93;break;//JPEG​2000MC
            case 0x3934: idx=ts_4_94;break;//JPIP​Referenced
            case 0x3935: idx=ts_4_95;break;//JPIP​Referenced​Deflate
            case 0x3120:{
               switch (vbuf[18]){
                  case 0x36: idx=ts_6_1;break;//RFC​2557​MIME​Encapsulation
                  case 0x37: idx=ts_7_1;break;//SMPTE​ST​211020​Uncompressed​Progressive​Active​Video
                  default: idx=ts_papyrus3ile;break;//error
               }};break;
            case 0x3220:{
               switch (vbuf[18]){
                  case 0x36: idx=ts_6_2;break;//XML​Encoding
                  case 0x37: idx=ts_7_2;break;//SMPTE​ST​211020​Uncompressed​Interlaced​Active​Video
                  default: idx=ts_papyrus3ile;break;//error
               }};break;
            case 0x3320: idx=ts_7_3;break;//SMPTE​ST​211030​PCMDigital​Audio
            default    : idx=ts_papyrus3ile;break;//error
         }};break;
      case 23:{
         switch (vbuf[20]){
            case 31:{
               switch (vbuf[22]){
                  case 0x30:idx=ts_4_100 ;break;//MPEG2​MPML
                  case 0x31:idx=ts_4_101 ;break;//MPEG2​MPHL
                  case 0x32:idx=ts_4_102 ;break;//MPEG4​HP41
                  case 0x33:idx=ts_4_103 ;break;//MPEG4​HP41BD
                  case 0x34:idx=ts_4_104 ;break;//MPEG4​HP422D
                  case 0x35:idx=ts_4_105 ;break;//MPEG4​HP423D
                  case 0x36:idx=ts_4_106 ;break;//MPEG4​HP42STEREO
                  case 0x37:idx=ts_4_107 ;break;//HEVC​MP51
                  case 0x38:idx=ts_4_108 ;break;//HEVC​M10P51
                  default: idx=ts_papyrus3ile;break;//error
               }};break;
            case 32:{
               switch (vbuf[22]){
                  case 0x31:idx=ts_4_201 ;break;//HTJ2K​Lossless
                  case 0x32:idx=ts_4_202 ;break;//HTJ2K​Lossless​RPCL
                  case 0x33:idx=ts_4_203 ;break;//HTJ2K
                  case 0x34:idx=ts_4_204 ;break;//JPIP​HTJ2K​Referenced
                  case 0x35:idx=ts_4_205 ;break;//JPIP​HTJ2K​Referenced​Deflate
                  default: idx=ts_papyrus3ile;break;//error
               }};break;
            default: idx=ts_papyrus3ile;break;//error
         }};break;
      case 25:{
         switch (vbuf[22]){
            case 0x30:idx=ts_4_100_1 ;break;//MPEG2​MPMLF
            case 0x31:idx=ts_4_101_1 ;break;//MPEG2​MPHLF
            case 0x32:idx=ts_4_102_1 ;break;//MPEG4​HP41F
            case 0x33:idx=ts_4_103_1 ;break;//MPEG4​HP41BDF
            case 0x34:idx=ts_4_104_1 ;break;//MPEG4​HP422DF
            case 0x35:idx=ts_4_105_1 ;break;//MPEG4​HP423DF
            case 0x36:idx=ts_4_106_1 ;break;//MPEG4​HP42STEREOF
            default: idx=ts_papyrus3ile;break;//error
         }
      };break;
      default: idx=ts_papyrus3ile;break;//error
   };
   
   if (strncmp(tsstr[idx],(char*)vbuf,strlen(tsstr[idx]))) return idx;
   else return 0;//error (verification)
}
