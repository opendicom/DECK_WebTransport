// project: dicm2deck
// targets: dicm2decksqlite
// file: capi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"

extern FILE *inFile;

int cPrerequisite(u64 filesize, int argc, char *argv[]){
   return exitZeroError;
}

int cCreate(FILE *inFILE, int argc, char *argv[])
{
   return exitZeroError;
}

int cCommit(bool hastrailing,int argc, char *argv[])
{
   return  exitZeroError;
}

void cClose(int argc, char *argv[])
{
    return;
}


#pragma mark append

//         printf("%8lu%*s%02X%02X%02X%02X %c%c %04X ",DICMidx-12,kloc+kloc+(kloc!=0),space, kbuf[kloc],kbuf[kloc+1],kbuf[kloc+2],kbuf[kloc+3],kbuf[kloc+4],kbuf[kloc+5],kbuf[kloc+6] + (kbuf[kloc+7] << 8));

bool eAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen)
{
   if (vlen==0) return true;
   if (!fseek(inFile, vlen, SEEK_CUR)) return false;
   return true;
}

bool sAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen)
{
   if (vlen==0) return true;
   if (!fseek(inFile, vlen, SEEK_CUR)) return false;
   return true;
}

bool pAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen)
{
   if (vlen==0) return true;
   if (fseek(inFile, vlen, SEEK_CUR)!=0) return false;
   return true;
}


bool iAppend(u32 kloc,enum kvVRcategory vrcat,u32 vlen)
{
   if (vlen==0) return true;
   if (fseek(inFile, vlen, SEEK_CUR)!=0) return(exitNotDICM);
   return true;
}
