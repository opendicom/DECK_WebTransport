// project: dicm2deck
// target: dicmstructdump
// file: uapi.c
// created by jacquesfauquex on 2024-04-04.


#include "uapi.h"
#include <locale.h>

extern FILE * inFile;

extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern char *CKEY;
extern u8   CKEYidx;

//prepare output in memory
char *SERIALIZE;//is also buffer for utf-8
u64 Doffset=0;//where to copy from
u64 Soffset=0;//where to copy from
u32 utf8size=0;


u8 isImage=false;
//needed as input to grok
u16 *columns;
u16 *rows;
u16 *samples;
u16 *bits;
u16 *sign;


//for blake3 of non compressed image
const u_int64_t tpaBlake3attr=0x424FFCFFCFF;
const u_int32_t tpaBlake3size=32;


#pragma mark ---------------------------- SOP instance

void input( int argc, char *argv[])
{
   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL)
   {
      if (ferror(stdin)) {
         fprintf(stderr,"inFile rb %s : %s (%d)\n", argv[1], strerror(errno), errno);
         exit(errno);
      }
      exit(exitErrorFropenDICM);
   }

   DICM=malloc(DICMsize+44);
   if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   fclose(inFile);

   SERIALIZE=malloc(DICMsize + 0x1000);

   setlocale(LC_ALL, "");//output in UTF-8

   u64 *offset190=(u64*)(DICM+190);
   isImage=isItImage(*offset190,32,16);//zero means no image
}

void trail(int count, char *vector[]) {
   memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset);
   Soffset+=DICMidx-Doffset;
   //memcpy(DICM+DICMsize,&tpaBlake3attr,8);
   //memcpy(DICM+DICMsize,&tpaBlake3size,4);

   FILE *fileptr = fopen("result.utf8.cdicm", "w");
   if ((fileptr == NULL) || (fwrite(SERIALIZE, 1, Soffset, fileptr) != Soffset)) {
      printf("%s", "cannot write result.utf8.cdicm\n");
      exit(-33);
   };
   fclose(fileptr);
}

#pragma mark ---------------------------- attributes

void key(struct Ercle* attr)
{
   //reads at CKEYidx+8 the new 8 first bytes of the attribute
   //reorders group and unit at CKEYidx
   // copies the VR and empty bytes at CKEYidx+4
   // eventually reads the four additional bytes of long length at CKEYidx+8
   memcpy(attr,DICM+DICMidx,8);
   DICMidx+=8;
   attr->E=((attr->E & 0xFF00FF00) >> 8)|((attr->E & 0xFF00FF) << 8); //transform to pure big endian
   attr->e=u32swap(attr->E);//tag in little endian

   //attr->r is ascii (left to right)
   attr->l=attr->c;//short length is transferred to long length (bytes 8-11)
   //attr->c short length will be modified before parsing the value to register repositoire index
   switch (attr->r) {
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
      case UR://universal resource url identifier/locator
      case SQ://sequence
      {
         memcpy(&attr->l,DICM+DICMidx,4);
         DICMidx+=4;
      }break;
      default:break;
   }
}

void val(enum kvVRcategory vrcat,struct Ercle* attr)
{
   switch (vrcat) {

#pragma mark - sequence and item with end tag
      case kvIA:
      case kvSA:
      case kvSZ:
      case kvIZ:
         break;

#pragma mark - sequence and item with end tag created
      case kvIa:
      case kvSa:
      {
         //change SQ and item length to undefined
         //SQ starts at -8
         //if -4 not ffffffff change it
         memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset-4);
         Soffset+=DICMidx-Doffset;
         SERIALIZE[Soffset-4]=SERIALIZE[Soffset-3]=SERIALIZE[Soffset-2]=SERIALIZE[Soffset-1]=(char)0xFF;
         Doffset=DICMidx;
      }break;
      case kvSz:
      {
         //if 0xfffee0dd nothing to do
         //else add 0xfffee0dd00000000
         memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset-8);//before new tag post sequence
         Soffset+=DICMidx-Doffset;
         SERIALIZE[Soffset-8]=(char)0xfe;
         SERIALIZE[Soffset-7]=(char)0xff;
         SERIALIZE[Soffset-6]=(char)0xdd;
         SERIALIZE[Soffset-5]=(char)0xe0;
         SERIALIZE[Soffset-4]=SERIALIZE[Soffset-3]=SERIALIZE[Soffset-2]=SERIALIZE[Soffset-1]=(char)0x0;
         Doffset=DICMidx-8;
      }break;
      case kvIz:
      {
         //if 0xfffee00d nothing to do
         //else add 0xfffee0dd00000000
         memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset-8);//before new tag post sequence
         Soffset+=DICMidx-Doffset;
         SERIALIZE[Soffset-8]=(char)0xfe;
         SERIALIZE[Soffset-7]=(char)0xff;
         SERIALIZE[Soffset-6]=(char)0x0d;
         SERIALIZE[Soffset-5]=(char)0xe0;
         SERIALIZE[Soffset-4]=SERIALIZE[Soffset-3]=SERIALIZE[Soffset-2]=SERIALIZE[Soffset-1]=(char)0x0;
         Doffset=DICMidx-8;
      }break;

#pragma mark - long length
      case kv01://OB OD OF OL OV OW SV UV
         printf("%u %u %u %u %u",*columns,*rows,*samples,*bits,*sign);
         //TODO
         /*
          * compression
          * frames 00280008 IS
          * RGB interleave corrected
          * -F c,r,s,b,s
          * columns
          * rows
          * samples
          * bits 15=16, 13=14, 11=12
          * sign?"s":"u"
          * -i
          * -o
          */
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


      case kvUS: {
         switch (attr->e) {
            case 0x280011: columns=(u16*)DICM+(DICMidx>>1); break;
            case 0x280010: rows=(u16*)DICM+(DICMidx>>1); break;
            case 0x280002: samples=(u16*)DICM+(DICMidx>>1); break;
            case 0x280101: bits=(u16*)DICM+(DICMidx>>1); break;//15=16, 13=14, 11=12
            case 0x280103: sign=(u16*)DICM+(DICMidx>>1); break;//?"s":"u"
         }
         DICMidx+=attr->l;
      } break;


#pragma mark -ascii
      case kvUI: {
         if (attr->e == 0x20010 && (isImage!=0)) {// modify the transfer syntax
            //copy everything before value
            memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx);
            Soffset+=DICMidx;

            //adjust size and new syntax
            //old 0x14 0x00 1.2.840.10008.1.2.1 0x00
            //new 0x18 0x00 1.2.840.10008.1.2.4.202 0x08C
            //may end in 1 (Lossless),2 (Lossless RPCL - resolution progressive) or 3 (not specified)
            SERIALIZE[DICMidx-2]=0x18;//new size
            const char *htj2krpcl="1.2.840.10008.1.2.4.202";
            memcpy(SERIALIZE+Soffset,htj2krpcl,0x18);//new UID
            Soffset+=0x18;
            DICMidx+=attr->l;
            Doffset=DICMidx;
         }
         else DICMidx+=attr->l;//do not copy yet
      } break;



#pragma mark -charset

      case kvCs:
      {//change to "ISO_IR 192"
         //copy everything before value
         memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset);
         Soffset+=DICMidx-Doffset;

         //adjust size and new syntax
         SERIALIZE[Soffset-2]=0x0A;//size of "ISO_IR 192"
         memcpy(SERIALIZE+Soffset,"ISO_IR 192",0x0A);//new UID
         Soffset+=0x0A;
         DICMidx+=attr->l;
         Doffset=DICMidx;
      } break;

      case kvTS://LO LT SH ST
      case kvPN:
      {//convert to utf-8
         if (attr->l!=0) {
            //copy up to utf8 value
            memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset);
            Soffset+=DICMidx-Doffset;
            Doffset=DICMidx+attr->l;

            //write utf8 value
            utf8size=utf8(CKEY[CKEYidx+6],DICM+DICMidx,attr->l,SERIALIZE+Soffset);
            memcpy(SERIALIZE+Soffset-2,&utf8size,2);//short length
            Soffset+=utf8size;
         }
         DICMidx+=attr->l;
      };break;

      case kvTL:
      {//convert to utf-8
         if (attr->l!=0) {
            //copy up to utf8 value
            memcpy(SERIALIZE+Soffset,DICM+Doffset,DICMidx-Doffset);
            Soffset+=DICMidx-Doffset;
            Doffset=DICMidx+attr->l;

            //write utf8 value
            utf8size=utf8(CKEY[CKEYidx+6],DICM+DICMidx,attr->l,SERIALIZE+Soffset);
            memcpy(SERIALIZE+Soffset-4,&utf8size,4);//long length
            Soffset+=utf8size;
         }
         DICMidx+=attr->l;
      };break;


      default:DICMidx+=attr->l;break;
   }
}