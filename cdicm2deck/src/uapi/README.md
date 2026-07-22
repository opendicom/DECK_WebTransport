# uapi

Possible override of stream reading functions:
- ufread
- ufreadattr
Transactional approach to DICM file parsing:
- uPrerequisite before opening the file, based on params received and file size
- uCreate once the file is opened
- uCommit in case parsing was successfull
- uClose called after parsing
- uAppend called for each of the attribute with offset, datatype and length as parameters. It is the responsibility of the handler to read the value, or to jump to the offset after the value

## Examples

### dicm2cda
Extracts to file the encapsulatedCDA

### dicmstructdump
Dumps to console the description of the attributes
