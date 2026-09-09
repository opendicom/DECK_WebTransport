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
const u32 CSutf8size=10;

extern u16  photocode;//photometric interpretation
extern u16  rows;
extern u16  cols;
extern u16  alloc;
extern u16  bits;
extern u16  high;
extern u16  sign;//pixrep 0028013 0=unsigned 1=signed
extern u16  comp;//planar 0 = RGB del pixel; 1 = componentes RGB (samples)

extern char eDA[4];
extern u32 eDAlength;
extern char eUI[48];
extern u32 eUIlength;
extern char sUI[48];
extern u32 sUIlength;
extern char cUI[48];
extern u32 cUIlength;
extern char iUI[48];
extern u32 iUIlength;
extern char pUI[48];//pyramid
extern u32 pUIlength;

struct KV {
    u8    Kl;
    char *K ;//key length (1 byte) followed by key
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

    printf("photocode %u\n",photocode);
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

    //largest group
    u32 KVlength=iKVlength>eKVlength?iKVlength:eKVlength;
    if (KVlength<sKVlength) KVlength=sKVlength;
    if (KVlength<pKVlength) KVlength=pKVlength;
    char groupBytes[KVlength];
    uint8_t *groupKey=malloc(218);
    groupKey[0]=eUIlength+39;
    memcpy(groupKey+1,eDA,4);
    groupKey[5]='/';
    memcpy(groupKey+6,eUI,eUIlength);

    //---------------------------- exam ----------------------------
    groupKey[eUIlength+6]=' ';
    groupKey[eUIlength+7]=' ';
    u32 cursor=0;
    for (int idx=0;idx<elast;idx++) {
        memcpy(groupBytes+cursor,&(eKVs[idx].K),eKVs[idx].Kl);
        cursor+=eKVs[idx].Kl;
        memcpy(groupBytes+cursor,&(eKVs[idx].Vl),4);
        cursor+=4;
        memcpy(groupBytes+cursor,eKVs[idx].Vp,eKVs[idx].Vl);
        cursor+=eKVs[idx].Vl;
    }

    blake3_hasher_update(&hasher, groupBytes, eKVlength);
    blake3_hasher_finalize(&hasher,(groupKey+eUIlength+8), BLAKE3_OUT_LEN);

    memcpy(groupKey+eUIlength+40,&eKVlength,4);


    //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
    if (  (fwrite(groupKey, 1, eUIlength+44, fileptr) != eUIlength+44)
        ||(fwrite(&groupBytes, 1, eKVlength, fileptr) != eKVlength)
        ) {
        printf("%s", "cannot write E\n");
        exit(-33);
    }


    //---------------------------- series ----------------------------
    groupKey[0]=eUIlength+sUIlength+39;
    groupKey[eUIlength+6]='/';
    memcpy(groupKey+eUIlength+7,sUI,sUIlength);
    groupKey[eUIlength+7+sUIlength]=' ';
    cursor=0;
    for (int idx=0;idx<slast;idx++) {
        memcpy(groupBytes+cursor,&(sKVs[idx].K),sKVs[idx].Kl);
        cursor+=sKVs[idx].Kl;
        memcpy(groupBytes+cursor,&(sKVs[idx].Vl),4);
        cursor+=4;
        memcpy(groupBytes+cursor,sKVs[idx].Vp,sKVs[idx].Vl);
        cursor+=sKVs[idx].Vl;
    }
    blake3_hasher_reset(&hasher);
    blake3_hasher_update(&hasher, groupBytes, sKVlength);
    blake3_hasher_finalize(&hasher,(groupKey+eUIlength+sUIlength+8), BLAKE3_OUT_LEN);

    memcpy(groupKey+eUIlength+sUIlength+40,&sKVlength,4);

    //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
    if (  (fwrite(groupKey, 1, eUIlength+sUIlength+44, fileptr) != eUIlength+sUIlength+44)
        ||(fwrite(&groupBytes, 1, sKVlength, fileptr) != sKVlength)
        ) {
        printf("%s", "cannot write E\n");
        exit(-33);
    }


    //---------------------------- instance ----------------------------
    groupKey[0]=eUIlength+sUIlength+iUIlength+41;

    groupKey[eUIlength+sUIlength+7]='/';
    memcpy(groupKey+eUIlength+sUIlength+8,iUI,iUIlength);
    groupKey[eUIlength+sUIlength+iUIlength+8]=' ';
    groupKey[eUIlength+sUIlength+iUIlength+9]=' ';
    cursor=0;
    for (int idx=0;idx<ilast;idx++) {
        memcpy(groupBytes+cursor,&(iKVs[idx].K),iKVs[idx].Kl);
        cursor+=iKVs[idx].Kl;
        memcpy(groupBytes+cursor,&(iKVs[idx].Vl),4);
        cursor+=4;
        memcpy(groupBytes+cursor,iKVs[idx].Vp,iKVs[idx].Vl);
        cursor+=iKVs[idx].Vl;
    }
    blake3_hasher_reset(&hasher);
    blake3_hasher_update(&hasher, groupBytes, iKVlength);
    blake3_hasher_finalize(&hasher,(groupKey+eUIlength+sUIlength+iUIlength+10), BLAKE3_OUT_LEN);

    memcpy(groupKey+eUIlength+sUIlength+iUIlength+42,&iKVlength,4);

    //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
    if (  (fwrite(groupKey, 1, eUIlength+sUIlength+iUIlength+46, fileptr) != eUIlength+sUIlength+iUIlength+46)
        ||(fwrite(&groupBytes, 1, iKVlength, fileptr) != iKVlength)
        ) {
        printf("%s", "cannot write E\n");
        exit(-33);
        }


    //---------------------------- private ----------------------------
    if (pKVlength>0) {
        groupKey[eUIlength+sUIlength+iUIlength+8]='-';
        cursor=0;
        for (int idx=0;idx<plast;idx++) {
            memcpy(groupBytes+cursor,&(pKVs[idx].K),pKVs[idx].Kl);
            cursor+=pKVs[idx].Kl;
            memcpy(groupBytes+cursor,&(pKVs[idx].Vl),4);
            cursor+=4;
            memcpy(groupBytes+cursor,pKVs[idx].Vp,pKVs[idx].Vl);
            cursor+=pKVs[idx].Vl;
        }
        blake3_hasher_reset(&hasher);
        blake3_hasher_update(&hasher, groupBytes, pKVlength);
        blake3_hasher_finalize(&hasher,(groupKey+eUIlength+sUIlength+iUIlength+10), BLAKE3_OUT_LEN);

        memcpy(groupKey+eUIlength+sUIlength+iUIlength+42,&pKVlength,4);

        //groupkey malloc makes it a pointer, char groupbyte[x] makes it a char
        if (  (fwrite(groupKey, 1, eUIlength+sUIlength+iUIlength+46, fileptr) != eUIlength+sUIlength+iUIlength+46)
            ||(fwrite(&groupBytes, 1, pKVlength, fileptr) != pKVlength)
            ) {
            printf("%s", "cannot write E\n");
            exit(-33);
            }
    }

    //--------------------------- frames -----------------------------

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
/*
    char tmp[21];
    memcpy(&tmp[0],CKEY,CKEYidx+8);
    memcpy(&tmp[9],&attr->l,4);
    memcpy(&tmp[13],DICM+DICMidx,attr->l);


    sattrv[sattrc]=malloc(CKEYidx+12+attr->l);
    memcpy(&sattrv[sattrc],CKEY,CKEYidx+8);
    memcpy((&sattrv[sattrc]+9),&attr->l,4);
    memcpy((&sattrv[sattrc])+13,DICM+DICMidx,attr->l);
    sattrc++;
*/
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
    DICMidx+=attr->l;
}