# dicm

- **dicm** refers to the binary file format of DICOM

- **deck** refers to  DICOM exam contextual-key value format of DICOM and relates one-to-one key bytechains to value bytechains within an ordered set.

- dicm2deck is a framework  written in  c which parses dicm and outputs various products, depending on the target


## command

dicm2deck uses linux streaming (stdin, stdout, stderr).

```
executable [infile]...
```
Executable name defined by target
args infiles (alternative to stdin)

## environment variables
- DICM2DECKloglevel : [ D | I | W | E | F ] ( Debug, Info, Warning, Error, Fault )
- DICM2DECKbeforebyte: limit parsing from beginning to attribute before byte x of the input
- DICM2DECKbeforetag: limit parsing from beginning to attribute y

dcmtk storescp agrega al final de cada instancia, para delimitarla dentro del stream 0xFFFCFFFC

- DICM2DECKworkingdir: where to write the files
- DICM2DECKmegamax: max size of DICM input file


## How it works?

Using cascading handlers.
The targets differ only by the implementation of the api of the lowest layer of handlers.

Here are the layers, from top to bottom:
(1) main
(2) dicm
(3) uapi
(4) capi

### (1) main
Initiates stdin, or loops and initiates all the files listed as arguments.
For each of them, calls dicm functions:
- dicmuptosopts (which reads sopclass, sopuid and sopts (transfer syntax))
- dicmInstance (which reads the remaining attributes in the file)
These functions are separated because the first one fetches metadata required by the second. If sopclass, sopuid or transfer syntax can´t be read, the parsing is aborted.

### (2) dicm
- implements dicmuptosopts and when sopclass, sopuid and sopts are available, calls uapi:uCreate
- implements dicmInstance, which calls dicmDataset (a recursive function) with its initial parameters. When dicmDataset is successfull, uapi:uCommit is called, and consequently uapi:uClose. In case of failure, uapi:uClose only is called.
- implements dicmDataset, which parses the structure of the dataset, diferenciating the attributes by datatype (which is an extended list in comparison to the DICOM vr one, where pacs classification attributes are registered with their own specialized category). For each of the attributes uapi:uAppend is invoked with three parameters relative to the value of the attribute: offset, datatype and length. At that point the value has not been read. This operation is reserved for the implementation of uAppend.

- besides, dicmuptosopts and dicmDataset invoke uapi:ifread and uapi:ifreadattr for its reading of the stream. This allows override of the exposed functions, if necessary.

### (3) uapi (uncategorized)
implements two IO functions
- ufread
- ufreadattr
and processes DICM instances and their attributes with 4 "transactional" handlers:

### uCreate
- initializes instance variables before any attribute processing
- performs whichever other task previous to parsing

### uCommit
- finalizes successfull parsing before closing
- whichever task to be performed when the parsing was successfull

### uClose
- automatically called by uCommit, or executed alone
- memory deallocation.
- whichever other task posterior to parsing

### uAppend
invoked for the value of each of the attributes to do something with it, but at least reading it, or jumping to the start of the following attribute.

## (4) capi (categorized)
Overrides uapi: 
- forwards uCreate, uCommit and uClose to cCreate, cCommit and cClose
- analyzes the attributes passed to uAppend, classifies them into categories, and depending on the category, calls capi specific functions:

(1) ignores sequences and items delimiters;
(2) calls pAppend for private attributes;
(3) calls bAppend (byte pixel), wAppend (short pixel) for pixel;
(4) calls eAppend for patient and study attributes
(5) calls sAppend for series attributes
(6) calls iAppend for any other attribute.
