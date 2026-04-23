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
   u64 beforebyte, // limite superior de lectura
   u32 beforetag   // limite superior attr. first traspassing attr is read en kbuf
);


#endif /* parseDICM_h */
