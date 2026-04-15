DECK WebTransport is a server - zero footprint reactive web page solution for the viewing of DICOM exams.

The web page is provided by the server, so that the system is independent and only requires a modern browser. It was not designed for interoperability.

DECK is partially DICOM compliant (information object model IOD and dictionary). Being a new Application Entity, it does not implement part 10, nor DICOMweb, nor DIMSE.

DECK WebTransport accepts part 10 (file formated) DICOM instances as input. It requires a canonicalized version of it (explicit little endian, undefined lengths) and transforms the files in dictionaries key-value. The canonicalization can be performed by the tool dcmodify of the dcmtk toolkit.

