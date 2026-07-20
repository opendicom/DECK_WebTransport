# dicm2deck

Dicom Exam Contextualized Keys (DECK) is a flat hashmap parser result language 
for DICM files. The objective of this presentation of the DICOM metadata is
consumer lowest latency. It implies discrete acceses to any attribute. 

dicm2deck, executable command written in C, 
parses DICM (DICOM standard part 10 file format),
outputting DECK key values. It requires explicit little endian representation
of the dataset terminated by an empty trailling padding attribute as input.
We call this presentation context "canonicalized".


## dcmtk storescp -> dicm2deck

We modified dcmtk storescp to receive DICOM DIMSE communication 
and forward such canonicalized format (with trailing padding attribute) to dicm2deck.
We also added parameters to the invocation of dicm2deck by storescp 
so that the critical information of the DIMSE association shall be passed to dicm2deck.

The modified dcmtk storescp and dicm2deck work as a binome.


## dicm2deck design

dicm2deck has a generic layered structure which is extended by the implementation of of one or another class
in order to create diferent products.

Common to all products is the main file which:
- reads the canonicalized file
- parses the dataset structure
- calls second layer "uapi" (u meaning uncategorized) functions:
   - uPrerequisite with association params. This delegates the possibility to abort the parsing without reading the file.
   - uCreate with association params invoked after opening the canonicalized file to enable the implementation of uapi to start a transaction to be commited or canceled later
   - uAppend called for each attribute passes its properties and delegates the reading of the value
   - uCommit is called when the trailling padding attribue is reached
   - uClose gives an oportunity to cancel the transaction, in cases where the parsing of the canonicalized file was not successfull.
 
A simple product may implement uAppend only. For instance, uapi/dicmstructdump uses the properties of the attribute and reads it to output in stdout a dump of the canonicalized dicom file.

dicm2deck comes with 2 apis:
* uapi (u meaning uncategorized) exposes DICOM attributes
* capi (c meaning categorized) exposes DICM attributes categorized as:
   - e patient and study attributes
   - s series attributes
   - p private attributes
   - i instance attributes
   - f float pixel 7FE00008 (not implemented)
   - d double pixel 7FE00009 (not implemented)
   - b byte pixel 7FE00010
   - w short pixel 7FE00010
   - l long pixel 7FE00010 (not implemented)
   - v very long pixel 7FE00010 (not implemented 64 bits)

Note: 
the DICOM explicit little endian syntax represents pixels in native format, 
that is, as a succession of lines without any markup between them. 
This applies also to multiframe images where the first line of the next frame 
follows immediately the last line of the previous one.


### Testing environment

The folder Testing contains canonicalized test files. 
The modified storescp outputs DICOM datasets received as files into this directory.

dicm2deck may produce files as output. We configured CMake targets 

![dicm2cda conf](../doc/testCLionConf.png)

to write them into a folder automatically created by CLion (the IDE we use):
cmake-build-debug/Testing/Temporary 

![see](../doc/CLionProject.png)


### dicm2deck targets:

- uapi: **dicm2cda** extracts the enclosed CDA from a DICM. 
   - Example infile: dscd.dicm
- uapi: **dicmstructdump** dumps a textual representation of the DICM file. 
   - Example infile: dscd.dicm
- capi: **sqlite** keeps the parsing into an in-memory sqlite.
- capi: **examsqlite**, builds on sqlite and on commit exports the registers to an exam sqlite server, 
after prepending a series/instance prefix to the keys.

___

We use CMake® and JetBrain CLion® IDE for the development.

DICOM® is a Registered Mark under the copyright of 
"National Electrical Manufacturers Association" (USA) 
