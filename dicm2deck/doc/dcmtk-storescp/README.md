# stream dcmtk storescp to dicm2deck

The entry point of dicm2deck is a DICM receiver. 
We use the opensource [dcmtk](https://support.dcmtk.org/docs/storescp.html) C++ implementation of it.
storescp writes files exactly as received (option --bit-preserving). 
This option disables DCMTK DICM parsing. STORESCP:
- receives packets through DIMSE protol, 
- assembles them to recretate a DICM file, 
- writes it to storage, 
- triggers a script, passing to it parameters of the association on which the packets were received:
    - p directory path
    - o object class key
    - f file name
    - a sender aet
    - r sender ip
    - c called aet

This script 
- parses DICM to DECK into an in memory sqlite
- calls a script which consolidates the study sqlite with the data of the instance
- moves the instance
  - from : /received/SopIUID
  - to   : /aaaammdd/studyiuid/seriesiuid/Sopiuid

Asynchronously, when the imagenological part of the study is completed, 
another script encodes the pixels.

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
+B \
+F \
-xcr 'storescp.xcr.sh #p #o #f #a #r #c' \
11111
````

- -lc storescp.log.cfg   (log config)
- -dhl   (disable lookup)
- -pdu 65534    (=0xFFFE matches reception buffer size)
- -pm   (promiscuous, accepts any association)

- -od tmpMountPoint   (where received packets are written before parsing)

- +B    --bit-preserving (as received, no parsing)
- +F    --write-file (including file format part 10)

- -xcr 'storescp.xcr.sh #p #o #f #a #r #c' (script to execute for each file with parameters dir, filename, scu, scuip, scp  passed in this order)
- 11111   reception port

## Instance triggered script 

Performs:
- parsing
- find or create study sqlite
- integrate the new attributes into sqlite
- move the original to /called/calling^ip/study^patientID/series/instance

## end of study series compression application

When it is detected that a calling aet stopped sending instances of a study, 
an application uses the parsed data in order to create a compressed imagening resource.

The compression is differed to the end of production of the imaging part of the study
so that it can be transversal to all the instances of a series. 
It is performed in the PCS before the transmission of compressed resources and sqlite to SIRIUS webtransport pacs