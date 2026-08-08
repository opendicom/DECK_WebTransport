// project: dicm2deck
// targets: eDECKinline,dicm2mdbx,...
// file: capi.h
// created by jacquesfauquex on 2024-04-04.

#ifndef capi_h
#define capi_h

#include "uapi.h"

#include "blake3.h"
//#include <errno.h>

enum categories{
   patient, //patient
   exam, //exam
   series, //series generic
   modality, //modality specific
   unknown, //manufacturer private
   instance //instance
};

#pragma mark - transaction hooks

//called with parameters before opening file
int cPrerequisite(int argc, char *argv[]);

//called after opening file
int cCreate(FILE *inFILE, int argc, char *argv[]);

//called after parsing successfully all the attributes
int cCommit(bool hastrailing,int argc, char *argv[]);

//finalizes the opened tx
void cClose(int argc, char *argv[]);

bool eAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen);
bool sAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen);
bool pAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen);
bool iAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen);

#endif /* capi_h */
