# stream dcmtk storescp to dicm2deck

The entry point of dicm2deck is a DICM receiver. 
We use the opensource [dcmtk](https://support.dcmtk.org/docs/storescp.html) C++ implementation of it.
storescp writes files exactly as received (option --bit-preserving). 
This option disables DCMTK DICM parsing.
STORESCP then only receives packets through DIMSE protol, assembles them to
recretate a DICM file and writes it to storage.

After writing the file, STORESCP triggers a script, passing to it:
- the location of the file
- parameters of the association on which the packets were received.

This script parses DICM to DECK and the image in the appropiate format.
The parser needs to be able to read DICM SQ/item **BOTH** lengths and delimiters.
The script also moves the file from /received 
to /called/calling^ip/study^patientID/series/instance

## storescu invocation
```` storescu -R +C -pdu 65534 127.0.0.1 11111 ele.dcm ... ````

## storescp initialization
The path to dicom.dic needs to be registered into environment variables
````DCMDICTPATH=../../dcmdata/data/dicom.dic```


```` storescp \
-lc storescp.log.cfg \
-dhl \
-pdu 65534 \
-pm \
-od tmpMountPoint \
-fe '.DICM.bin' \
+B \
+F \
-xcr 'storescp.xcr.sh #a #r #c #p #f' \
11111
````

- -lc storescp.log.cfg   (log config)
- -dhl   (disable lookup)
- -pdu 65534    (=0xFFFE matches reception buffer size)
- -pm   (promiscuous, accepts any association)

- -od tmpMountPoint   (where received packets are written before parsing)
- -fe '.DICM.bin'   (extension to the filename)

- +B    --bit-preserving (as received, no parsing)
- +F    --write-file (including file format part 10)

- -xcr 'storescp.xcr.sh #a #r #c #p #f' (script to execute for each file with parameters scu, scuip, scp, dir, filename passed in this order)
- 11111   reception port

Option usefull for debug only:
- -xs   --exec-sync (script execution without starting a new proces)

## Instance triggered script 

Performs:
- parsing
- find or create study sqlite
- integrate the new attributes into sqlite
- move the original to /called/calling^ip/study^patientID/series/instance

## end of study series compression application

When it is detected that a calling aet does not send more instances of a study
for a while or started to send instances of a new study, 
an application uses the parsed data in order to create a compressed imagenological ressource.

The compression is differed to the end of production of the imagenological part of the study
so that it can be transversal to all the instances of a series. 
It is performed in the PCS before the transmission of compressed resources and sqlite to SIRIUS webtransport pacs