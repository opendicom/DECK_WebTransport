// project: dicm2deck
// target: ...
// file: main.c
// created by jacquesfauquex on 2024-04-04.

#include <stdint.h>

#include "dicm.h"

//defined global
char *DICMbuf=NULL;//accumulator of stream registering original binary DICM. Referred by external declarations everywhere
u64 DICMidx=0;//associated current pointer
s16 siidx=1;//pointer reset count
uint8_t *kbuf=NULL;//buffer (size 0xFF) for the creation of _DKV and EDKV contextual keys. max use 16 bytes x 10 encapsulation levels
/*
 main controls dicm2_dkv.
 dicm2_dkv may be used by any other app controlling dicm2_dkv
 all fread get processed by dckvapi.m
 */
int main(int argc, const char * argv[]) {
   /*  (exactly ones defined in the xcr option of dcmtk storescp:
   0 command name defined by target
   1 #p/#f dir path / dicm file name
   2 #a scu aet
   3 #r scu ip
   4 #c scp aet
   */
   
//environment variables
   
#pragma mark DICM2DECKloglevel
   const char* loglevel = getenv("DICM2DECKloglevel");
   if (loglevel==NULL) loglevel="D";
   //[ D | I | W | E | F ] ( Debug, Info, Warning, Error, Fault )
   
#pragma mark DICM2DECKbeforebyte
   u32 beforebyte=0xFFFFFFFF;
   const char* abeforebyte = getenv("DICM2DECKbeforebyte");
   if (abeforebyte!=NULL) beforebyte=(u32)strtoll(abeforebyte, NULL, 16);
   
#pragma mark DICM2DECKbeforetag
   // agradado en dcmtk-storescp storescp al final de cada instancia, para delimitarla dentro del stream
   u32 beforetag=0xFFFCFFFC;
   const char* abeforetag = getenv("DICM2DECKbeforetag");
   if (abeforetag!=NULL) beforetag=(u32)strtoll(abeforetag, NULL, 16);
   
#pragma mark DICM2DECKworkingdir
   const char* workingdir = getenv("DICM2DECKworkingdir");
   if (workingdir!=NULL) chdir(workingdir);
   else chdir("/Users/jacquesfauquex/sqlite_edckv/");
   
   char cwd[1024];
   getcwd(cwd, sizeof(cwd));
   D("working dir:  %s", cwd);

#pragma mark DICM2DECKmegamax
   //in MB, 0=no dicom binary output
   int DICMmegamax=300;
   const char* aDICMmegamax = getenv("DICM2DECKmegamax");
   if (aDICMmegamax!=NULL) DICMmegamax=(u32)atoi(aDICMmegamax);

#pragma mark - read file and process
   FILE *inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL) return dckvErrorIn;
   DICMbuf=malloc(DICMmegamax*1024*1024);
   kbuf = malloc(0xFF);
   if (!dicmuptosopts()) return dckvSOPinstanceRejected;
   if (!dicmInstance(beforebyte,beforetag)) return dckvErrorParsing;
   fclose(inFile);
}
