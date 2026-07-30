# dicm2cda

This implementation of uapi.h does nothing with the values of the attributes, except for encapsulatedDocument,
which it writes to stdout

one fread for each attribute followd by fseek to skip the reading of the value.
