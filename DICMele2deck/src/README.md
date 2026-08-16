# DICMele2DECK

- **DICMele** refers to the binary file format of DICOM (part 10) in explicit little endian.

- **DECK** refers to  DICOM exam contextual key-value format and relates one-to-one key bytechains to value bytechains within an ordered set.

- DICMele2DECK is a framework  written in c which parses DICM and outputs various products, depending on the target


## command

"cdicm2deck" is a placeholder for the name of the exe corresponding to a target.
Many parameters are available when triggered from dcmtk storescp xcr option.
The first (file path) only is mandatory.
````
xcr.sh #p/#f #u #v #s #k #i #o #t #l #a #r #c'
````

