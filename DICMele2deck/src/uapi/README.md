# uapi

## dicm2cda
Extracts encapsulatedCDA to stdout. Additional parámetros may apply an XSLT stylesheet to the XML streamed.

dicm2cda can be moodified to extract a PDF or any set of metadata.

## utf8dump
Dumps to console the description of the attributes.
With simple value output modifications, utf8dump can be transformed into dicm2json or dicm2xml

## dicmhtj2k
adjusts
- utf8 instead of any other charset
- sequence end tag
- series end tag
- high throughput j2k comprensión
- trailing padding with checksum of uncomprressed pixels

Can be modified to enable compression to a explicit little endian derived transfer syntax

## dicmisqlite
- crea una base SQLite con los atributos de la instancia y la guarda dentro de un mount point aaaammdd/euid/suid/iuid
