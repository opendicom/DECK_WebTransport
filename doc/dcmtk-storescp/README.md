# stream dcmtk storescp to dicm2deck

````
storescp [> bash] > exe
compression
````


The entry point of dicm2deck is a DICM receiver. 
We use the opensource [dcmtk](https://support.dcmtk.org/docs/storescp.html) C++ implementation of it.
storescp writes files exactly as received (option --bit-preserving). 
This option disables DCMTK DICM parsing. STORESCP:
- receives packets through DIMSE protol, 
- assembles them to recreate a DICM file, 
- writes the result to the storage, 
- triggers an executable, passing it parameters of the association on which the packets were received

## available parameters
- u theirImplementationClassUID
- v theirImplementationVersion
- s max pdu size
- k sop class dcmtk key (padded with _ up to 4 chars)
- i ID presentation context negotiated 
- o sop class uid
- t transfer syntax uid
- l list of presentation context id.class-transfer_ (no spaces between elements)
- p directory path/file name
- a sender aet
- r sender ip
- c called aet

## debug script
````
#!/bin/sh
#debug storescu association
#add storescp parameter
#-xcr 'xcr.sh #p/#f /dest/ #u #v #s #k #i #o #t #l #a #r #c'
#p source dir
#f file name
#u sending app uid
#v sending app version
#s pdu size
#k sop class dcmtk key
#i presentation context id
#o sop class uid
#t transfer syntax
#l list of presentation contexts
#a aet calling
#r remote ip
#c called aet

echo "${1} ${2} ${9}"
echo `date +%Y%m%d_%H%M%S.%3N`" ${11}@${12}>${13}"
echo "app=${3} name=${4} pdu=${5}"
echo "key=${6} id=${7} uid=${8} ts=${9}"
echo "contexts=${10}"
````

The debug script can call another executable

## sop parser to memory sqlite executable 
requires 3 xcr parameters: 
- source file
- destination folder (ending with /)
- error folder (ending with /)
````
-xcr 'memorysqlite #p/#f /dst/ /err/
````
explicit little endian is the only accepted syntax.

the executable
- parses DICM to DECK into a in memory sop sqlite
- consolidates the sop sqlite into a study sqlite found in /dest/aaaammdd/studyiuid/_.sql
- moves the instance from /received/SopIUID to /dest/aaaammdd/studyiuid/sopiuid

Asynchronously, when the imaging part of the study is completed, 
another script encodes the pixels.

## Modifications to storescp
- new xcr parameters
- empty trailing padding attribute to simplify parsing
- #k is dcmtk key corresponding to sop classes.
  We transformed into magic signatures,
  padding them with _
  to make them 4 char length.
  The complete table of keys and correspondences is found [here](../dcmtk_modality-sopclass.csv).

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
-xcr 'xcr.sh #p/#f /dest/ #u #v #s #k #i #o #t #l #a #r #c' \
11111
````

- -lc storescp.log.cfg   (log config)
sample:
````
# log to the console
log4cplus.rootLogger = DEBUG, logfile
log4cplus.appender.logfile = log4cplus::FileAppender
log4cplus.appender.logfile.File = /tmp/storescp.log
log4cplus.appender.logfile.Append = true
log4cplus.appender.logfile.ImmediateFlush = true
log4cplus.appender.logfile.layout = log4cplus::PatternLayout
log4cplus.appender.logfile.layout.ConversionPattern = %D{%H:%M:%S.%q}  %5p [%-8c{1}]  %m%n
````
- -dhl   (disable lookup)
- -pdu 65534    (=0xFFFE matches reception buffer size)
- -pm   (promiscuous, accepts any association)

- -od tmpMountPoint   (where received packets are written before parsing)

- +B    --bit-preserving (as received, no parsing)
- +F    --write-file (including file format part 10)

- -xcr 'storescp.xcr.sh #p #o #f #a #r #c' (script to execute for each file)
- 11111   reception port

## Instance triggered script 

Performs:
- parsing
- find or create study sqlite
- integrate the new attributes into sqlite
- move the original to /called/calling^ip/study^patientID/series/instance

## end of study series compression application

Also triggered by dcmtk.

When it is detected that a calling aet stopped sending instances of a study, 
an application uses the parsed data in order to create a compressed imagening resource.

The compression is differed to the end of production of the imaging part of the study
so that it can be transversal to all the instances of a series. 
It is performed in the PCS before the transmission of compressed resources and sqlite to SIRIUS webtransport pacs