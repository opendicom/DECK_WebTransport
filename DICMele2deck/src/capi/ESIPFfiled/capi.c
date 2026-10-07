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

//to write to file system
#include <sys/stat.h>
#include <sys/types.h>

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
struct KV pKVs[20000];//number of base attributes registered as type S or X
struct KV data;//pdf,cda,stl,obj,mtl,pix(raw frames)
char *dataFilename;


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
    printf("pUI %*s\n",pUIlength,pUI);//pyramid

    printf("photocode %u\n",photo);
    printf("rows %u\n",rows);
    printf("cols %u\n",cols);
    printf("alloc %u\n",alloc);
    printf("bits %u\n",bits);
    printf("high %u\n",high);
    printf("sign %u\n",sign);
    printf("comp %u\n",comp);

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
    // date dir
    memcpy(groupKey+1,eDA,4);
    printf("eDA:%d\n",mkdir(groupKey+1, 0777));//0 created -1 failed
    //printf("Failed to create directory: %s\n", strerror(errno));
    groupKey[5]='/';
    memcpy(groupKey+6,eUI,eUIlength);
    //exam dir
    printf("eUI:%d\n",mkdir(groupKey+1, 0777));//0 created -1 failed
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

    //write E attributes
    FILE *examptr = fopen(groupKey+1, "w");
    if (examptr == NULL) {
        printf( "cannot touch %s\n",groupKey+1);
        exit(-33);
    }
    if (fwrite(&groupBytes, 1, cursor, examptr) != cursor)
    {
        printf("cannot write E to %s\n",groupKey+1);
        exit(-33);
    }
    fclose(examptr);

    //---------------------------- series ----------------------------
    groupKey[0]=eUIlength+sUIlength+75;
    memcpy(groupKey+eUIlength+7,sUI,sUIlength);
    groupKey[eUIlength+sUIlength+7]=0x00;
    //series dir
    printf("sUI:%d\n",mkdir(groupKey+1, 0777));//0 created -1 failed

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

    //write S attributes
    FILE *seriesptr = fopen(groupKey+1, "w");
    if (seriesptr == NULL) {
        printf("cannot touch %s\n",groupKey+1);
        exit(-33);
    }
    if (fwrite(&groupBytes, 1, cursor, seriesptr) != cursor)
    {
        printf("cannot write S to %s\n",groupKey+1);
        exit(-33);
    }
    fclose(seriesptr);

    //enclosed (level series: one instance per series)
    if (dataFilename != NULL) {
        memcpy(groupKey+eUIlength+sUIlength+8,dataFilename,7);
        FILE *dataptr = fopen(groupKey+1, "w");
        if (dataptr == NULL) {
            printf("cannot touch %s\n",groupKey+1);
            exit(-33);
        }
        if (fwrite(data.Vp, 1, data.Vl, dataptr) != data.Vl)
        {
            printf("cannot write %s\n",groupKey+1);
            exit(-33);
        }
        fclose(dataptr);
    }

    //---------------------------- instance ----------------------------
    groupKey[0]=eUIlength+sUIlength+iUIlength+77;
    memcpy(groupKey+eUIlength+sUIlength+8,iUI,iUIlength);
    groupKey[eUIlength+sUIlength+iUIlength+8]=0x00;

    //instance dir
    printf("iUI:%d\n",mkdir(groupKey+1, 0777));//0 created -1 failed

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

    //write I attributes
    FILE *instanceptr = fopen(groupKey+1, "w");
    if (instanceptr == NULL) {
        printf("cannot touch %s\n",groupKey+1);
        exit(-33);
    }
    if (fwrite(&groupBytes, 1, cursor, instanceptr) != cursor)
    {
        printf("cannot write I to %s\n",groupKey+1);
        exit(-33);
    }
    fclose(instanceptr);

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
            if (groupBytes[cursor-2]!=0) { //charset to be transformed to UTF-8
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

        //write P attributes
        FILE *privateptr = fopen(groupKey+1, "w");
        if (privateptr == NULL) {
            printf("cannot touch %s\n",groupKey+1);
            exit(-33);
        }
        if (fwrite(&groupBytes, 1, cursor, privateptr) != cursor)
        {
            printf("cannot write I to %s\n",groupKey+1);
            exit(-33);
        }
        fclose(privateptr);
    }

    //--------------------------- frames -----------------------------
    // https://dicom.nema.org/medical/Dicom/2024c/output/chtml/part03/sect_C.7.6.6.html
    groupKey[0]=eUIlength+sUIlength+iUIlength+17;
    cursor=cols * rows * comp * alloc / 8;
    memcpy(groupKey+eUIlength+sUIlength+iUIlength+18,&cursor,4);
    if (fram==0) fram=1;
    char *ffilename=groupKey+eUIlength+sUIlength+iUIlength+9;
    ffilename[9]=0;
    switch (photo) {
        case MONOCHROME1:
        case MONOCHROME2://OPC (opacity)
        {
            for (int f=0; f<fram; f++) {
                snprintf(ffilename,10,"%05d.OPC",f+1);
                FILE *frameptr = fopen(groupKey+1, "w");
                if (frameptr == NULL) {
                    printf("cannot touch %s\n",groupKey+1);
                    exit(-33);
                }
                if (fwrite(data.Vp+(f*cursor),1, cursor, frameptr) != cursor)
                {
                    printf("cannot write frame %05d.OPC",f+1);
                    exit(-33);
                }
                fclose(frameptr);
            }
        } break;

        case PALETTE://PLT
        {
            for (int f=0; f<fram; f++) {
                snprintf(ffilename,10,"%05d.PLT",f+1);
                FILE *frameptr = fopen(groupKey+1, "w");
                if (frameptr == NULL) {
                    printf("cannot touch %s\n",groupKey+1);
                    exit(-33);
                }
                if (fwrite(data.Vp+(f*cursor),1, cursor, frameptr) != cursor)
                {
                    printf("cannot write frame %05d.PLT",f+1);
                    exit(-33);
                }
                fclose(frameptr);
            }
        } break;

        case RGB: { //RGB
            //needs to interleave instead of plane by plane
            for (int f=0; f<fram; f++) {
                snprintf(ffilename,10,"%05d.RGB",f+1);
                FILE *frameptr = fopen(groupKey+1, "w");
                if (frameptr == NULL) {
                    printf("cannot touch %s\n",groupKey+1);
                    exit(-33);
                }
                if (fwrite(data.Vp+(f*cursor),1, cursor, frameptr) != cursor)
                {
                    printf("cannot write frame %05d.RGB",f+1);
                    exit(-33);
                }
                fclose(frameptr);
            }
        } break;

        case YBR_FULL:printf("%s", "YBR_FULL (RLE) not handled\n");break;//RLE
        case YBR_FULL_422:printf("%s", "YBR_FULL_422 (jpeg) not handled\n");break;//JPG 1 +1/2 + 1/2
        case YBR_PARTIAL_420:printf("%s", "YBR_PARTIAL_420 (mpeg) not handled\n");break;//MPG 1 + 1/4 + 1/4
        case YBR_ICT:printf("%s", "YBR_ICT (j2k lossy) not handled\n");break; //J2I jpeg2000 lossy
        case YBR_RCT:printf("%s", "YBR_RCT (j2k lossless, pal secam) not handled\n");break;//J2R jpeg2000 lossless  pal secam
        case XYB:printf("%s", "XYB (JPEG-XL) not handled\n");break;//JXL jpeg-xl
        default:;
    }


    //----------------------------------------------------------
    fclose(KVserializedFILE);

    cursor=0;

}


#pragma mark ---------------------------- attributes

void eAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu E %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);

    eKVs[elast].Kl=CKEYidx+8;
    memcpy(&eKVs[elast].K,CKEY,CKEYidx+8);
    eKVs[elast].Vl=attr->l;
    eKVs[elast].Vp=DICM+DICMidx;
    eKVlength+=CKEYidx+12+attr->l;
    elast++;

    DICMidx+=attr->l;
}

void sAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu S %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);

    sKVs[slast].Kl=CKEYidx+8;
    memcpy(&sKVs[slast].K,CKEY,CKEYidx+8);
    sKVs[slast].Vl=attr->l;
    sKVs[slast].Vp=DICM+DICMidx;
    sKVlength+=CKEYidx+12+attr->l;
    slast++;

    DICMidx+=attr->l;
}

void pdfAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu PDF %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
    dataFilename="ps.pdf";
    data.Kl=CKEYidx+8;
    data.Vl=attr->l- (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void cdaAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu CDA %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);

    data.Kl=CKEYidx+8;
    data.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    // find <d, <C o <s within the first 160 first chars
    for (u8 c=0; c < 0xA0; c++) {
        if (*data.Vp+c == 'c') {
            switch (*data.Vp+c+1) {
                case 'd': dataFilename="dc.xml"; break;
                case 's': dataFilename="sc.xml"; break;
                case 'C': dataFilename="uc.xml"; break;
                default:  dataFilename="ot.xml"; break;
            }
        }
    }

    DICMidx+=attr->l;
}

void stlAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu SLT %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
    dataFilename="3d.stl";
    data.Kl=CKEYidx+8;
    data.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void objAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu OBJ %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
    dataFilename="tx.obj";
    data.Kl=CKEYidx+8;
    data.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void mtlAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu MTL %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
    dataFilename="tx.mtl";
    data.Kl=CKEYidx+8;
    data.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

void iAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("%8lu I %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);

    iKVs[ilast].Kl=CKEYidx+8;
    memcpy(&iKVs[ilast].K,CKEY,CKEYidx+8);
    iKVs[ilast].Vl=attr->l;
    iKVs[ilast].Vp=DICM+DICMidx;
    iKVlength+=CKEYidx+12+attr->l;
    ilast++;

    DICMidx+=attr->l;
}

void pAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("%8lu P %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);
    pKVs[plast].Kl=CKEYidx+8;
    memcpy(&pKVs[plast].K,CKEY,CKEYidx+8);
    pKVs[plast].Vl=attr->l;
    pKVs[plast].Vp=DICM+DICMidx;
    pKVlength+=CKEYidx+12+attr->l;
    plast++;

    DICMidx+=attr->l;
}

void fAttribute(enum kvVRcategory vrcat,struct Ercle* attr){//frame
    printf("%8lu F %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);

    data.Kl=CKEYidx+8;
    memcpy(&data.K,CKEY,CKEYidx+8);
    data.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}

//https://dicom.nema.org/medical/dicom/2023e/output/chtml/part03/sect_F.7.html
void iconAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    //logo icon
    printf("%8lu logo %08X %c%c\n",DICMidx,u32swap(*(u32*)(CKEY+1)),CKEY[CKEYidx+4],CKEY[CKEYidx+5]);

    data.Kl=CKEYidx+8;
    memcpy(&data.K,CKEY,CKEYidx+8);
    data.Vl=attr->l - (*(DICM+DICMidx+attr->l-1)== 0x00);
    data.Vp=DICM+DICMidx;

    DICMidx+=attr->l;
}
