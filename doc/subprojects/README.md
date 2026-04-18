DECK WebTransport is a server - zero footprint reactive web page solution for the viewing of DICOM exams.

The web page is provided by the PACS server, so that the system is independent and only requires a modern browser. It was not designed for interoperability.

DECK is a DICOM representation of DICOM information object model (IOD) and dictionary. It can be transformed back and forth to/from DICM, XML, and JSON representations. Being a new UDP datagram Application Entity, it does not implement part 10, nor DICOMweb, nor DIMSE.

DECK WebTransport accepts part 10 (file formated) DICOM instances as input. It requires a canonicalized version of them (explicit little endian, undefined lengths) and transforms the files into dictionaries key-value. When necessary, a preliminar canonicalization can be performed by the tool dcmodify of the dcmtk toolkit.

PACS is the complete server project. It includes MIRO (the web pages executed on the device) and uses DECK formatted resources.
