# dicm2cda

This implementation of uapi.h does nothing with the values of the attributes, except for 
- title, which it writes to console
- SOPInstanceUID, which it uses to form the file name with a suffix ".dscd.xml"
- encapsulatedDocument which it writes to the file (created or previously truncated in the working directory)
