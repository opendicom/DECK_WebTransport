# canonical

receives an explicit little endian DICM file

- uses utf8 instead of any other charset
- adds sequence end tag instead of defined length sequence
- adds series end tag instead of defined length item
- removes meaningless spaces

writes the output to file with extension .cdicm

The result is a bytestream that can be firmed electronically,
because there is no other way to convey the exact same meaning
with a different bytestream while respecting the same rules
which specialize part 10 of the standard.

The concept of canonical DICM is parallel to the one of canonical XML.

