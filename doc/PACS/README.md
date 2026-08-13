The PACS is written in RUST on the kyber framework

It accesses the DECK resources using [Turso](https://github.com/tursodatabase/turso/tree/main/bindings/rust), a rust-embeded SQLite-compatible database library, which supports natively TOKIO async operations. 

Additionally, it implements "Prepared Statements", which are optimized query execution with parameter binding, very usefull for repetitive queries.
