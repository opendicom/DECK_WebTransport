# uid2b64
The compression of uid may be significant for databases which uses them as primary key.

# key
The binary contextualized-key is essential for the whole project.
https://github.com/opendicom/DECK_WebTransport/blob/main/doc/subprojects/DECK/DECK%20canonical%20formats/ordered%20contextual-keys%20format.md

# value
Follows DICOM value representation defined in part 5, with some modifications described in the last column of table https://github.com/opendicom/DECK_WebTransport/blob/main/doc/subprojects/DECK/DECK%20canonical%20formats/DECK%20value%20representation.md
.

https://dicom.nema.org/medical/dicom/current/output/html/part05.html#sect_6.2

# databases
The attributes of an exam can be ordered into one or more memory hashes or key value databases. 
The use of the plural in the previous sentence refers to the possibility to subdivide the exam attributes 
into categories, each of which is embedded in its own database (study attributes, private attributes, etc). 
Server side, categories are used, to improve latency. 
Client side, for the sake of simplicity, all the selected attributes of an exam share one dictionary only.

# commands
The copy of attributes from the server to the client requires serialization and packetization (datagrams). 
We defined formats for each of the commands which adjusts the contents to the max size of a datagram, 
about 65000 bytes. 
