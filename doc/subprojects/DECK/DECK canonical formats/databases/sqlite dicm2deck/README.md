The converter dicm2deck produces key values which it writes to an C embedded sqlite. 
The DB needs to be ordered. Because it is used to avoid duplicated keys in E and S categories.

A blob can be used as primary key
```CREATE TABLE E (key BLOB PRIMARY KEY COLLATE BINARY, value BLOB) WITHOUT ROWID;```
When used as a key, binary data is compared byte-for-byte using memcmp().

In the Command Line Interface (CLI), 
you can use the -hexkey option to provide a binary key represented in hexadecimal.

The sqlite3_key() function allows you to pass raw binary data to derive the encryption key.

Using the WITHOUT ROWID optimization can improve performance 
by storing the data directly in the primary key B-Tree instead of a separate index.