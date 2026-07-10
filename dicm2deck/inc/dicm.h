// project: dicm2deck
// file: dicm.h
// created by jacquesfauquex on 2024-04-04.

#ifndef parseDICM_h
#define parseDICM_h

#import "uapi.h"


//parse until transfer syntax and create transaction
bool dicmuptosopts(void);

//parse from transfer syntax
bool dicmInstance(
   u64 beforebyte, // read up to byte
   u32 beforetag   // read up to attr. first next attr is read en kbuf
);


#endif /* parseDICM_h */
