The PACS is written in RUST with the UDP capable TOKIO library as the server backbone. It serves HTTP/3 and is WebTransport capable. https://github.com/BiagioFesta/wtransport is used as prototype of the server.

It accesses the DECK resources using Turso, https://github.com/tursodatabase/turso/tree/main/bindings/rust, an embeded SQLite-compatible database library for Rust. This database manager support natively TOKIO async operations. Additionally, it implements "Prepared Statements", which are optimized query execution with parameter binding, very usefull for repetitive queries.
