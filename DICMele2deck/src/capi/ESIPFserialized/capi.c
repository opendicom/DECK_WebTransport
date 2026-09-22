// project: dicm2deck
// targets: dicm2decksqlite
// file: capi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"
#include <locale.h>
#include "blake3.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern FILE * inFile;
FILE *KVserializedFILE;

extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern u8   *CKEY;
extern u8    CKEYidx;

static char *UTF8;
u32 utf8size=0;

extern int  fram;
extern u16  photo;//photometric interpretation
extern u16  rows;
extern u16  cols;
extern u16  alloc;
extern u16  bits;
extern u16  high;
extern u16  sign;//pixrep 0028013 0=unsigned 1=signed
extern u16  comp;//planar 0 = RGB del pixel; 1 = componentes RGB (samples)

extern char eDA[4];
extern u8 eDAlength;
extern char eUI[48];
extern u8 eUIlength;
extern char sUI[48];
extern u8 sUIlength;

extern char cUI[48];
extern u8 cUIlength;
extern char iUI[48];
extern u8 iUIlength;
extern char pUI[48];//pyramid
extern u8 pUIlength;

struct KV {
    u8    Kl;
    char *K ;//key length (1 byte) followed by key
    char Kpadding;//when charset ends with zero byte, the struct may overwrite it. Kpadding avoids it
    u32   Vl;//value length written LE
    char *Vp;//value pointer
};
//e P(patient),C(clinical study),
//s S(series), X (special series level)
//i anything else which is not private
//p private
struct KV eKVs[100];//number of base attributes registered as type P or C
struct KV sKVs[150];//number of base attributes registered as type S or X
struct KV iKVs[350];//number of base attributes registered as type S or X
struct KV pKVs[200];//number of base attributes registered as type S or X
struct KV pdf;
struct KV cda;
struct KV stl;
struct KV obj;
struct KV mtl;

u16 elast;
u16 slast;
u16 ilast;
u16 plast;
u32 eKVlength=0;
u32 sKVlength=0;
u32 iKVlength=0;
u32 pKVlength=0;

int sattrc=0;
char *sattrv[150];
#pragma mark ---------------------------- SOP instance

void cinput( int argc, char *argv[])
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

    DICM=malloc(DICMsize);
    if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
        if (ferror(stdin)) {
            fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
            exit(errno);
        }
        fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
        exit(exitReadTruncated);
    };
    fclose(inFile);

    KVserializedFILE = fopen("serialized.bin", "w");

    setlocale(LC_ALL, "");//output in UTF-8
    UTF8=malloc(0x4000);
    //LT max 10240,UT max 2^32 !!!
    //16K covers any UTF8 size increase for LT, but eventually requires larger buffer  for LT
}

void ctrail(int argc, char *argv[]) {

    printf("eDA %*s\n",eDAlength,eDA);
    printf("eUI %*s\n",sUIlength,sUI);
    printf("sUI %*s\n",sUIlength,sUI);
    printf("iUI %*s\n",iUIlength,iUI);
    printf("pUI %*s\n",pUIlength,pUI);

    printf("photocode %u\n",photo);
    printf("rows %u\n",rows);
    printf("cols %u\n",cols);
    printf("alloc %u\n",alloc);
    printf("bits %u\n",bits);
    printf("high %u\n",high);
    printf("sign %u\n",sign);
    printf("comp %u\n",comp);

    FILE *fileptr = fopen("ESIPF.bin", "w");
    if (fileptr == NULL) {
        printf("%s", "cannot write dscd.utf8.cdicm\n");
        exit(-33);
    }

    //blake3 provides merkle-tree, incremental and fast hash (usefull to compare buffer)
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    uint8_t blake32[32];

    //largest group
    u32 KVlength=iKVlength>eKVlength?iKVlength:eKVlength;
    if (KVlength<sKVlength) KVlength=sKVlength;
    if (KVlength<pKVlength) KVlength=pKVlength;
    char groupBytes[KVlength];
    char * groupKey=malloc(256);
    groupKey[0]=eUIlength+73;
    memcpy(groupKey+1,eDA,4);
    groupKey[5]='/';
    memcpy(groupKey+6,eUI,eUIlength);
    groupKey[eUIlength+6]='/';

    //---------------------------- exam ----------------------------
    groupKey[eUIlength+7]='-';
    groupKey[eUIlength+8]='-';
    groupKey[eUIlength+9]='.';
    u32 cursor=0;
    for (int idx=0;idx<elast;idx++) {
        memcpy(groupBytes+cursor,&(eKVs[idx].K),eKVs[idx].Kl);
        cursor+=eKVs[idx].Kl;
        if (groupBytes[cursor-2]!=0) { //UTF-8
            utf8size=utf8serialized(groupBytes[cursor-2],eKVs[idx].Vp,eKVs[idx].Vl,UTF8);
            memcpy(groupBytes+cursor,UTF8,utf8size);
            cursor+=utf8size;
        }
        else {
            memcpy(groupBytes+cursor,&(eKVs[idx].Vl),4);
            cursor+=4;
            memcpy(groupBytes+cursor,eKVs[idx].Vp,eKVs[idx].Vl);
            cursor+=eKVs[idx].Vl;
        }
    }

    blake3_hasher_update(&hasher, groupBytes, cursor);
    blake3_hasher_finalize(&hasher,blake32, BLAKE3_OUT_LEN);
    //as string hexa representation
    char* blake3offset=groupKey+eUIlength+10;
    for (u32 hexa=0;hexa<32;hexa++) { sprintf(blake3offset+hexa+hexa, "%02x", blake32[hexa]);}

    memcpy(groupKey+eUIlength+74,&eKVlength,4);

    //EXAM fwrite group E + size
    //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
    if (  (fwrite(groupKey, 1, eUIlength+78, fileptr) != eUIlength+78)
        ||(fwrite(&groupBytes, 1, cursor, fileptr) != cursor)
        ) {
        printf("%s", "cannot write E\n");
        exit(-33);
    }


    //---------------------------- series ----------------------------
    groupKey[0]=eUIlength+sUIlength+75;
    memcpy(groupKey+eUIlength+7,sUI,sUIlength);
    groupKey[eUIlength+sUIlength+7]='/';
    groupKey[eUIlength+sUIlength+8]='-';
    groupKey[eUIlength+sUIlength+9]='-';
    groupKey[eUIlength+sUIlength+10]='-';
    groupKey[eUIlength+sUIlength+11]='.';
    cursor=0;
    for (int idx=0;idx<slast;idx++) {
        memcpy(groupBytes+cursor,&(sKVs[idx].K),sKVs[idx].Kl);
        cursor+=sKVs[idx].Kl;
        if (groupBytes[cursor-2]!=0) { //UTF-8
            utf8size=utf8serialized(groupBytes[cursor-2],sKVs[idx].Vp,sKVs[idx].Vl,UTF8);
            memcpy(groupBytes+cursor,UTF8,utf8size);
            cursor+=utf8size;
        }
        else {
            memcpy(groupBytes+cursor,&(sKVs[idx].Vl),4);
            cursor+=4;
            memcpy(groupBytes+cursor,sKVs[idx].Vp,sKVs[idx].Vl);
            cursor+=sKVs[idx].Vl;
        }
    }
    blake3_hasher_reset(&hasher);
    blake3_hasher_update(&hasher, groupBytes, sKVlength);
    blake3_hasher_finalize(&hasher,blake32, BLAKE3_OUT_LEN);
    //as string hexa representation
    blake3offset=groupKey+eUIlength+sUIlength+12;
    for (u32 hexa=0;hexa<32;hexa++) { sprintf(blake3offset+hexa+hexa, "%02x", blake32[hexa]);}

    memcpy(groupKey+eUIlength+sUIlength+76,&sKVlength,4);

    //SERIES fwrite group ES + size
    //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
    if (  (fwrite(groupKey, 1, eUIlength+sUIlength+80, fileptr) != eUIlength+sUIlength+80)
        ||(fwrite(&groupBytes, 1, cursor, fileptr) != cursor)
        ) {
        printf("%s", "cannot write E\n");
        exit(-33);
    }

    //----------------------------------pdf-----------------------------
    if (pdf.Vl > 0) {
        groupKey[0]=eUIlength+sUIlength+13;
        groupKey[eUIlength+sUIlength+8]='p';
        groupKey[eUIlength+sUIlength+9]='s';
        groupKey[eUIlength+sUIlength+10]='.';
        groupKey[eUIlength+sUIlength+11]='p';
        groupKey[eUIlength+sUIlength+12]='d';
        groupKey[eUIlength+sUIlength+13]='f';
        memcpy(groupKey+eUIlength+sUIlength+14,&(pdf.Vl),4);
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+18, fileptr) != eUIlength+sUIlength+18)
             ||(fwrite(pdf.Vp, 1, pdf.Vl, fileptr) != pdf.Vl)
             ) {
            printf("%s", "cannot write pdf\n");
            exit(-33);
             }
    }

    //----------------------------------cda-----------------------------
    if (cda.Vl > 0) {
        // find <d, <C o <s within the first 160 first chars
        char a='o';
        char b='t';
        for (u8 c=0; c < 0xA0; c++) {
            if (*cda.Vp+c == 'c') {
                switch (*cda.Vp+c+1) {
                    case 'd': { a='d';b='s';c=0xA0;} break;
                    case 's': { a='s';b='c';c=0xA0;} break;
                    case 'C': { a='u';b='c';c=0xA0;} break;
                }
            }
        }
        groupKey[0]=eUIlength+sUIlength+13;
        groupKey[eUIlength+sUIlength+8]=a;
        groupKey[eUIlength+sUIlength+9]=b;
        groupKey[eUIlength+sUIlength+10]='.';
        groupKey[eUIlength+sUIlength+11]='x';
        groupKey[eUIlength+sUIlength+12]='m';
        groupKey[eUIlength+sUIlength+13]='l';
        memcpy(groupKey+eUIlength+sUIlength+14,&(cda.Vl),4);
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+18, fileptr) != eUIlength+sUIlength+18)
             ||(fwrite(cda.Vp, 1, cda.Vl, fileptr) != cda.Vl)
             ) {
            printf("%s", "cannot write cda\n");
            exit(-33);
             }
    }

    //----------------------------------stl-----------------------------
    if (stl.Vl > 0) {
        groupKey[0]=eUIlength+sUIlength+13;
        groupKey[eUIlength+sUIlength+8]='3';
        groupKey[eUIlength+sUIlength+9]='d';
        groupKey[eUIlength+sUIlength+10]='.';
        groupKey[eUIlength+sUIlength+11]='s';
        groupKey[eUIlength+sUIlength+12]='t';
        groupKey[eUIlength+sUIlength+13]='l';
        memcpy(groupKey+eUIlength+sUIlength+14,&(stl.Vl),4);
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+18, fileptr) != eUIlength+sUIlength+18)
             ||(fwrite(stl.Vp, 1, stl.Vl, fileptr) != stl.Vl)
             ) {
            printf("%s", "cannot write stl\n");
            exit(-33);
             }
    }

    //----------------------------------obj-----------------------------
    if (obj.Vl > 0) {
        groupKey[0]=eUIlength+sUIlength+13;
        groupKey[eUIlength+sUIlength+8]='t';
        groupKey[eUIlength+sUIlength+9]='x';
        groupKey[eUIlength+sUIlength+10]='.';
        groupKey[eUIlength+sUIlength+11]='o';
        groupKey[eUIlength+sUIlength+12]='b';
        groupKey[eUIlength+sUIlength+13]='j';
        memcpy(groupKey+eUIlength+sUIlength+14,&(obj.Vl),4);
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+18, fileptr) != eUIlength+sUIlength+18)
             ||(fwrite(obj.Vp, 1, obj.Vl, fileptr) != obj.Vl)
             ) {
            printf("%s", "cannot write obj\n");
            exit(-33);
             }
    }

    //----------------------------------mtl-----------------------------
    if (mtl.Vl > 0) {
        groupKey[0]=eUIlength+sUIlength+13;
        groupKey[eUIlength+sUIlength+8]='t';
        groupKey[eUIlength+sUIlength+9]='x';
        groupKey[eUIlength+sUIlength+10]='.';
        groupKey[eUIlength+sUIlength+11]='m';
        groupKey[eUIlength+sUIlength+12]='t';
        groupKey[eUIlength+sUIlength+13]='l';
        memcpy(groupKey+eUIlength+sUIlength+14,&(mtl.Vl),4);
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+18, fileptr) != eUIlength+sUIlength+18)
             ||(fwrite(mtl.Vp, 1, mtl.Vl, fileptr) != mtl.Vl)
             ) {
            printf("%s", "cannot write mtl\n");
            exit(-33);
             }
    }

    //---------------------------- instance ----------------------------
    groupKey[0]=eUIlength+sUIlength+iUIlength+77;

    memcpy(groupKey+eUIlength+sUIlength+8,iUI,iUIlength);
    groupKey[eUIlength+sUIlength+iUIlength+8]='/';
    groupKey[eUIlength+sUIlength+iUIlength+9]='-';
    groupKey[eUIlength+sUIlength+iUIlength+10]='-';
    groupKey[eUIlength+sUIlength+iUIlength+11]='-';
    groupKey[eUIlength+sUIlength+iUIlength+12]='-';
    groupKey[eUIlength+sUIlength+iUIlength+13]='.';
    cursor=0;
    for (int idx=0;idx<ilast;idx++) {
        memcpy(groupBytes+cursor,&(iKVs[idx].K),iKVs[idx].Kl);
        cursor+=iKVs[idx].Kl;
        if (groupBytes[cursor-2]!=0) { //UTF-8
            utf8size=utf8serialized(groupBytes[cursor-2],iKVs[idx].Vp,iKVs[idx].Vl,UTF8);
            memcpy(groupBytes+cursor,UTF8,utf8size);
            cursor+=utf8size;
        }
        else {
            memcpy(groupBytes+cursor,&(iKVs[idx].Vl),4);
            cursor+=4;
            memcpy(groupBytes+cursor,iKVs[idx].Vp,iKVs[idx].Vl);
            cursor+=iKVs[idx].Vl;
        }
    }
    blake3_hasher_reset(&hasher);
    blake3_hasher_update(&hasher, groupBytes, iKVlength);
    blake3_hasher_finalize(&hasher,blake32, BLAKE3_OUT_LEN);
    //as string hexa representation
    blake3offset=groupKey+eUIlength+sUIlength+iUIlength+14;
    for (u32 hexa=0;hexa<32;hexa++) { sprintf(blake3offset+hexa+hexa, "%02x", blake32[hexa]);}

    memcpy(groupKey+eUIlength+sUIlength+iUIlength+78,&iKVlength,4);

    //INSANTACE fwrite group ESI + size
    //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
    if (  (fwrite(groupKey, 1, eUIlength+sUIlength+iUIlength+82, fileptr) != eUIlength+sUIlength+iUIlength+82)
        ||(fwrite(&groupBytes, 1, cursor, fileptr) != cursor)
        ) {
        printf("%s", "cannot write E\n");
        exit(-33);
        }


    //---------------------------- private ----------------------------
    if (pKVlength>0) {
        groupKey[eUIlength+sUIlength+iUIlength+9]='p';
        groupKey[eUIlength+sUIlength+iUIlength+10]='r';
        groupKey[eUIlength+sUIlength+iUIlength+11]='i';
        groupKey[eUIlength+sUIlength+iUIlength+12]='v';
        cursor=0;
        for (int idx=0;idx<plast;idx++) {
            memcpy(groupBytes+cursor,&(pKVs[idx].K),pKVs[idx].Kl);
            cursor+=pKVs[idx].Kl;
            if (groupBytes[cursor-2]!=0) { //UTF-8
                utf8size=utf8serialized(groupBytes[cursor-2],pKVs[idx].Vp,pKVs[idx].Vl,UTF8);
                memcpy(groupBytes+cursor,UTF8,utf8size);
                cursor+=utf8size;
            }
            else {
                memcpy(groupBytes+cursor,&(pKVs[idx].Vl),4);
                cursor+=4;
                memcpy(groupBytes+cursor,pKVs[idx].Vp,pKVs[idx].Vl);
                cursor+=pKVs[idx].Vl;
            }
        }
        blake3_hasher_reset(&hasher);
        blake3_hasher_update(&hasher, groupBytes, pKVlength);
        blake3_hasher_finalize(&hasher,blake32, BLAKE3_OUT_LEN);
        //as string hexa representation
        blake3offset=groupKey+eUIlength+sUIlength+iUIlength+14;
        for (u32 hexa=0;hexa<32;hexa++) { sprintf(blake3offset+hexa+hexa, "%02x", blake32[hexa]);}

        memcpy(groupKey+eUIlength+sUIlength+iUIlength+78,&pKVlength,4);

        //PRIVATE fwrite group ESI + size
        //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+iUIlength+82, fileptr) != eUIlength+sUIlength+iUIlength+82)
            ||(fwrite(&groupBytes, 1, cursor, fileptr) != cursor)
            ) {
            printf("%s", "cannot write E\n");
            exit(-33);
            }
    }

    //--------------------------- frames -----------------------------
    // https://dicom.nema.org/medical/Dicom/2024c/output/chtml/part03/sect_C.7.6.6.html
    switch (photo) {
        case MONOCHROME1:
        case MONOCHROME2: {
            int buffersize=cols * rows * comp;//1
            for (int f=1;f <= fram;f++) {

            }
        } break;
        //case PALETTE:;
            /*
        case RGB: {
            int buffersize=cols * rows * comp;//3
        } break;
            */
        //case YBR_FULL:;//RLE
        //case YBR_FULL_422:;//JPEG 1 +1/2 + 1/2
        //case YBR_PARTIAL_420:;//mpeg 1 + 1/4 + 1/4
        //case YBR_ICT:; //jpeg2000 lossy
        //case YBR_RCT:;//jpeg2000 lossless  pal secal
        //case XYB:;//jpeg-xl
        default:;
    }




    //----------------------------------------------------------
    fclose(fileptr);
    fclose(KVserializedFILE);

    cursor=0;

}


#pragma mark ---------------------------- attributes

void eAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("E %08X\n",u32swap(*(u32*)(CKEY+1)));

    eKVs[elast].Kl=CKEYidx+8;
    memcpy(&eKVs[elast].K,CKEY,CKEYidx+8);
    eKVs[elast].Vl=attr->l;
    eKVs[elast].Vp=DICM+DICMidx;
    eKVlength+=CKEYidx+12+attr->l;
    elast++;

    DICMidx+=attr->l;
}

void sAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("S %08X\n",u32swap(*(u32*)(CKEY+1)));

    sKVs[slast].Kl=CKEYidx+8;
    memcpy(&sKVs[slast].K,CKEY,CKEYidx+8);
    sKVs[slast].Vl=attr->l;
    sKVs[slast].Vp=DICM+DICMidx;
    sKVlength+=CKEYidx+12+attr->l;
    slast++;

    DICMidx+=attr->l;
}


void pdfAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("PDF %08X\n",u32swap(*(u32*)(CKEY+1)));

    pdf.Kl=CKEYidx+8;
    pdf.Vl=attr->l- (*(DICM+DICMidx+attr->l-1)== 0x00);
    pdf.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void cdaAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("CDA %08X\n",u32swap(*(u32*)(CKEY+1)));

    cda.Kl=CKEYidx+8;
    cda.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    cda.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void stlAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("SLT %08X\n",u32swap(*(u32*)(CKEY+1)));

    stl.Kl=CKEYidx+8;
    stl.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    stl.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void objAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("OBJ %08X\n",u32swap(*(u32*)(CKEY+1)));

    obj.Kl=CKEYidx+8;
    obj.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    obj.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void mtlAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("MTL %08X\n",u32swap(*(u32*)(CKEY+1)));

    mtl.Kl=CKEYidx+8;
    mtl.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    mtl.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void iAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("I %08X\n",u32swap(*(u32*)(CKEY+1)));

    iKVs[ilast].Kl=CKEYidx+8;
    memcpy(&iKVs[ilast].K,CKEY,CKEYidx+8);
    iKVs[ilast].Vl=attr->l;
    iKVs[ilast].Vp=DICM+DICMidx;
    iKVlength+=CKEYidx+12+attr->l;
    ilast++;

    DICMidx+=attr->l;
}
void pAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("P %08X\n",u32swap(*(u32*)(CKEY+1)));

    pKVs[plast].Kl=CKEYidx+8;
    memcpy(&pKVs[plast].K,CKEY,CKEYidx+8);
    pKVs[plast].Vl=attr->l;
    pKVs[plast].Vp=DICM+DICMidx;
    pKVlength+=CKEYidx+12+attr->l;
    plast++;

    DICMidx+=attr->l;
}


void fAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("F %08X\n",u32swap(*(u32*)(CKEY+1)));

    FILE *fileptr = fopen("ESIPF.bin", "w");
    if (fileptr == NULL) {
        printf("%s", "cannot write dscd.utf8.cdicm\n");
        exit(-33);
    }

    DICMidx+=attr->l;
}

