# key value

DECK is a dictionary key-value format. The binary keys are ordered and no longer than 256 bytes.
Their length vary by 8 bytes steps.

A blob can be used as primary key
```CREATE TABLE E (key BLOB PRIMARY KEY COLLATE BINARY, value BLOB) WITHOUT ROWID;```
When used as a key, binary data is compared byte-for-byte using memcmp().

## databases
- The C sqlite of the converter dicm2deck,
- the rust Turso sqlite-compatible database of the server,
- and the javascript indexedDB of the browser
are all appropiate for DECK key-value dictionaries handling.


In the Command Line Interface (CLI), 
you can use the -hexkey option to provide a binary key represented in hexadecimal.

The sqlite3_key() function allows you to pass raw binary data to derive the encryption key.

Using the WITHOUT ROWID optimization can improve performance 
by storing the data directly in the primary key B-Tree instead of a separate index.