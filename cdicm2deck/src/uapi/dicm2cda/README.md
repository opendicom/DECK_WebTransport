# dicm2cda

This implementation of uapi.h does nothing with the values of the attributes
(one fread for each attribute followed by fseek to skip the reading of the value),
except for encapsulatedDocument, which it writes to stdout

.
