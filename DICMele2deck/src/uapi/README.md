# uapi

## dicm2cda
Extracts encapsulatedCDA to stdout

Can be extended to extract any set of metadata

## dicmdump
Dumps to console the description of the attributes.
With simple attribute output modifications, dicmdump can be transformed into dicm2json or dicm2xml

## dicm2cdicm
cdicm means canonicalized dicm. The adjustments are
- utf8 instead of any other charset
- sequence end tag
- series end tag
- empty trailing padding tag

needs explicit little endian as input (does not parse implicit or big)

Can be extended to enable compression to a explicit little endian derived transfer syntax
