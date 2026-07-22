// project: dicm2deck
// targets: eDECKinline,dicm2mdbx,...
// file: capi.h
// created by jacquesfauquex on 2024-04-04.

#ifndef capi_h
#define capi_h

#include "uapi.h"

#include "blake3.h"
//#include <errno.h>

enum eFamily{
   eDECK, //exam
   sDECK, //series
   pDECK, //private
   iDECK, //instance
   fDECK //frame pixel
};

#pragma mark - transaction hooks

//called with parameters before opening file
int cPrerequisite(u64 filesize, int argc, char *argv[]);

//called after opening file
int cCreate(FILE *inFILE, int argc, char *argv[]);

//called after parsing successfully all the attributes
int cCommit(bool hastrailing,int argc, char *argv[]);

//finalizes the opened tx
void cClose(int argc, char *argv[]);

bool eAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool sAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool pAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool iAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool fAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);

#endif /* capi_h */
