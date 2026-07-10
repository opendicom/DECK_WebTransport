# uapi

Possible override of stream reading functions:
- ufread
- ufreadattr
Transactional approach to DICM file parsing:
- uCreate called before
- uCommit+uClose or uClose called after
- uAppend called for each of the attribute with offset, datatype and length as parameters. It is the responsibility of the handler to read the value, or to jump to the offset after the value

## Examples

### dicm2cda
Extracts to file the encapsulatedCDA

### dicmstructdump
Lists to console the description of the attributes
