DECK is a diccionary key-value format. The binary keys are ordered. Their length vary between 16 bytes and 192 (most extreme case) by 8 bytes steps.

The server Turso database and the browser indexedDB are apropiate for DECK key-value dictionaries handling.

Inbetween a simple serialization based on UDP datagrams copies the attributes from the server to the client, which writes them into its database.


