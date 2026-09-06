// project: dicm2deck
// targets: dicm2decksqlite
// file: capi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"
#include <locale.h>

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
    fclose(KVserializedFILE);
}


#pragma mark ---------------------------- attributes


void eAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("E %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void sAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("S %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void iAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("I %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void pAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("P %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}
