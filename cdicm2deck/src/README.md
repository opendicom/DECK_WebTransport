# CDICM2DECK

- **CDICM** refers to the binary file format of DICOM (part 10) in explicit little endian with an empty trailing padding attribute

- **DECK** refers to  DICOM exam contextual-key value format of DICOM and relates one-to-one key bytechains to value bytechains within an ordered set.

- cdicm2deck is a framework  written in c which parses DICM and outputs various products, depending on the target


## command

"cdicm2deck" is a placeholder for the name of the exe corresponding to a target.
Many parameters are available when "dcicm2deck" is triggered from dcmtk storescp xcr option.
The first only is mandatory: CDICM file path
````
xcr.sh #p/#f #u #v #s #k #i #o #t #l #a #r #c'
````

## environment variables
- CDICM2DECKloglevel : [ D | I | W | E | F ] ( Debug, Info, Warning, Error, Fault )
- CDICM2DECKwoutdir: where to write or move the files on successfull parsing
- CDICM2DECKerrdir: where to move the files on failed parsing

## hardcoded selectors
- CDICM2DECKbeforebyte: limit parsing from beginning to attribute before byte x of the input
- CDICM2DECKbeforetag: limit parsing from beginning to attribute y

## How it works?

Using cascading handlers:
(1) main
(2) uapi
(4) capi

### (1) main
Open the file and reads dicm structure

### (3) uapi (uncategorized)
Reads the attribute values in dicm order
Read and append are performed by distinct functions:
- vvread (valueBuffer value read)
- kvread (keyBuffer value read)
- kkRead (keyBuffer key read)

- vrAppend
- csAppend


## (3) capi (categorized)
Overrides uapi and dispatch the attributes into patient/study, series, instance, pixel and private categories
