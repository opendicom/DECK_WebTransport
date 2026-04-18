DECK is a dictionary key-value format. The binary keys are ordered. Their length vary between 16 bytes and 192 (most extreme case) by 8 bytes steps.

The Turso database of the server and the indexedDB of the browser are apropiate for DECK key-value dictionaries handling.

Inbetween a simple serialization based on UDP datagrams copies the attributes from the server to the client, which writes them into its database.

# UDP serialization

Two protocols were chosen:
- the fastest, GET, which implements request-response, limited to one datagram each, on a duplex stream opened from the client side.
- the size-unlimited datagrams PUSH, which implies a request-response on a duplex stream, and the following asynchronous datagrams push from the server to the client. The maximum value size pushed for one request in this case is 65536 x 65000 bytes, which is slightly less than 4 GB.
