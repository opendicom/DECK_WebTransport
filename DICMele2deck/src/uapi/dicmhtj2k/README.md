# dicmhtj2k
CDICM parsed and serialized with:
- utf8 instead of any other charset
- sequence end tag
- series end tag
- pixels converted to htj2k rplc
- trailing padding tag with blake3 of uncompressed pixels

Needs explicit little endian as input (does not parse implicit or big)
