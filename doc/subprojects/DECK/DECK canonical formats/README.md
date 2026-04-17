# key
The contextualized binary key is essential for the whole project.

# databases
It can be ordered into one or more memory hashes or key value databases. The use of the plural in the previous sentence refers to the possibility to subdivide the the exam atributes in categories, each of which is embedded in its database. For instance study attributes, private attributes, etc. This is defined in the database of the exam. Server side, categories are used, to improve latency. Client side, for the sake of simplicity, all the selected attributes of an exam share one dictionary only.

# commands
The copy of attributes from the server to the client requires serialization and packetization (datagrams). We defined formats for each of the commands which adjusts the contents to the max size of a datagram, about 65000 bytes. 
