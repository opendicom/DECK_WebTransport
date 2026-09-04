# DICMele2deck

Dicom Exam Contextualized Keys (DECK) is a flat hashmap parser result language 
for DICM files. The goal for this presentation of the DICOM metadata is discrete access to any attribute and consumer lowest latency. 

DICMele2deck, executable commands written in C, 
parses DICM (DICOM standard part 10 file format) with explicit little endian syntax,
outputting DECK key values representation.

## dcmtk storescp -> cdicm2deck

We modified dcmtk storescp to receive DICOM DIMSE communication 
and invoque an executable with parameters to process each SOP instance.
We added the critical information of the DIMSE association as parameters to the invocation.


## dicm2deck design

dicm2deck is layered.

### main
Common to all products is the main file which:
- parses the dataset structure
- instantiates global variables
  - eDA and eDAlength (study date aammdd compressed uib64)
  - iUI and iUIlength (SOP Instance UID compressed uib64)
  - sUI and sUIlength (series Instance UID compressed uib64)
  - eUI and eUIlength (study Instance UID compressed uib64)
  - pUI and pUIlength (pyramid UID compressed uib64)
  - isImage (char 0=not image)
  - spp (sample per plane = components)
  - photocode (photometric interpretation)
  - rows 00280010
  - cols 00280011
  - word 00280100 (number of bits allocated per pixel)
  - bits 00280101
  - high 00280102
  - sign 00280103 (pixrep 0028013 0=unsigned 1=signed)
  - comp 00280106 (planar 0 = RGB del pixel; 1 = componentes RGB samples)
  - fram 08002800 IS (frame number -1 not available, 0 not an image, number of frames)
- delegates reading and writing functions to a second layer "uapi" (u meaning uncategorized):
   - input (delegates the opening and reading of the file previous to parsing. Adds a trailing padding attribute with 32 bytes value, usefull for blake3)
   - key and val (delegates parsing at attribute level)
   - trail (called when the trailing padding attribute has been reached)
   
### second level uapi
implements input (before reading), key (to get the attribute tag, vr and vm), val to do something with its value, trail
for each attribute in tag order.

### Third level capi
This class is not used by uapi executables.
It depends on a specific implementation of uapi which subdivides the function val and delegates
capi is exposed by a specific implementation of uapi which delegates val() processing to one of four parallel functions, depending on attribute category:
- eAppend for patient and study attributes
- sAppend for series attributes
- iAppend instance attributes
- pAppend private attributes

Note: 
the DICOM explicit little endian syntax represents pixels in native format, 
that is, as a succession of pixel lines without any markup between them. 
This applies also to multiframe images where the first line of the next frame 
follows immediately the last line of the previous one.

## Testing environment
The folder Testing contains canonicalized test files. 
The modified storescp outputs DICOM datasets received as files into this directory.

dicm2deck may produce files as output. We configured CMake targets 

![dicm2cda conf](../doc/testCLionConf.png)

to write them into a folder automatically created by CLion (the IDE we use):
cmake-build-debug/Testing/Temporary 

![see](../doc/CLionProject.png)


### dicm2deck targets:

- uapi: **dicm2cda** extracts the enclosed CDA from a DICM encapsulatedCDA SOP instance
- uapi: **utf8dump** dumps a textual representation of the DICM file.
- uapi: **canonical** replace syntactical variants and forms a new monosyntactical bytestream, written to file in one shot.
- uapi: **KVserialized** serialization of the former written attribute (tag, VR, VM, contents) by attribute to stdout
___

We use CMake® and JetBrain CLion® IDE for the development.

DICOM® is a Registered Mark under the copyright of 
"National Electrical Manufacturers Association" (USA) 
