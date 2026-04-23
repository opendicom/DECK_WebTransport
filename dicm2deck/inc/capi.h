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
   EDKV, //exam
   SDKV, //series
   PDKV, //private
   IDKV, //instance
   nativeOB,
   nativeOW,
   nativeOF,
   nativeOD,
   nativeOC,
   framesOB,
   framesOC
};

bool cCreate(
   u64 soloc,         // offset in vbuf for sop class
   u16 solen,         // length in vbuf for sop class
   u16 soidx,         // index in const char *scstr[]
   u64 siloc,         // offset in vbuf for sop instance uid
   u16 silen,         // length in vbuf for sop instance uid
   u64 stloc,         // offset in vbuf for transfer syntax
   u16 stlen,         // length in vbuf for transfer syntax
   u16 stidx         // index in const char *csstr[]
);
bool cCommit(bool hastrailing);
bool cClose(void);

bool eAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool sAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool pAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool iAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool fAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);//7FE00008
bool dAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);//7FE00009
bool bAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);//7FE00010
bool wAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool lAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);
bool vAppend(int kloc,enum kvVRcategory vrcat,u32 vlen);

#endif /* capi_h */
