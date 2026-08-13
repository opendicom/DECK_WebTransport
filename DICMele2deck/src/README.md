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

## How it works?

Using cascading handlers:
(1) main
(2) uapi
(3) capi

### (1) main
- opens the file
- parses CDICM structure, including sequences and items
- echos charset code
- delegates read operations to uapi

### (3) uapi (uncategorized)
reads attributes properties and values

## (3) capi (categorized)
Overrides uapi and dispatch the attributes into patient/study, series, instance, pixel and private categories
