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
   instance, //instance
   frame
};

void eAttribute(enum kvVRcategory vrcat,struct Ercle* attr);
void sAttribute(enum kvVRcategory vrcat,struct Ercle* attr);
void pAttribute(enum kvVRcategory vrcat,struct Ercle* attr);
void iAttribute(enum kvVRcategory vrcat,struct Ercle* attr);

void cinput(int argc, char *argv[]);//opening of DICM
void ctrail(int argc, char *argv[]);//after dataset read


#endif /* capi_h */
