The entry point of dicm2deck is a dicm receiver. 
We use the opensource [dcmtk](https://support.dcmtk.org/docs/storescp.html) C++ implementation of it.

dicm2deck is streamable between stdin and stdout. Normally storescp writes files.
(1) We transformed it to write its output on stdout.
(2) we modified storescp so that it appends an empty trailing padding attribute. 
This last attribute is used by dicm2deck as marker only and does not make its way to the 
key-value list.

- (1) stdout
dcmtk writes files after opening them with the command freopen(filename), which can also open stdout:
freopen(stdout). the easy hack consists in changing the filename.

- (2) trailing padding
Before closing the file, storescp-side, a 0xFFFCFFFC Trailing Padding Attribute is append before the last flush.

The files to be modified are:
- dcmtk/dcmdata/libsrc/dcostrmf.cc
- dcmtk/ofstd/include/dcmtk/ofstd/offile.h

## parameters
```` storescp -dhl -pm -od /tmp +te -e -pdu 65534 11112 | ````

- dhl (optional, no network check)

- pm (optional, promiscuous, from any AET)

- od (directory where to write. Compulsory, but not used after the stdout hack.

- te (compulsory, write explicit little endian.

- e (compulsory. undefined length for sequences and items).

- pdu 65534 (=0xFFFE) matches reception buffer size.



## testing
```` storescu -R +C -pdu 65534 127.0.0.1 11112 file.dcm ... ````