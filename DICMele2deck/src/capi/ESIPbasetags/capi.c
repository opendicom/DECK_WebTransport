// project: dicm2deck
// targets: dicm2decksqlite
// file: capi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"

extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern u8   *CKEY;

extern u8    CKEYidx;
extern char *UTF8;
extern u32   utf8size;

void eAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("E %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void sAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("S %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void pAttribute(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("P %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void iAttribute(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("I %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}


void eUID(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("E %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void sUID(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("S %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void pUID(enum kvVRcategory vrcat,struct Ercle* attr){
    printf("P %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}

void iUID(enum kvVRcategory vrcat,struct Ercle* attr) {
    printf("I %08X\n",u32swap(*(u32*)(CKEY+1)));
    DICMidx+=attr->l;
}
