DECK WebTransport is a zero footprint web page - server solution for the viewing of DICOM exams.

The web page is provided by the server. This system is independent and only requires a modern browser. It was not designed for interoperability.

DECK is partially DICOM compliant (information object model IOD and dictionary). Being a new Application Entity, it does not implement part 10, nor DICOMweb, nor DIMSE.

DECK WebTransport accepts part 10 (file formated) DICOM instances as input. It requires a canonicalized version of it (explicit little endian, undefined lengths) and transforms the files in dictionaries key-value.

