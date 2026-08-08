# dicm2cda

This implementation of uapi.h does nothing with the values of the attributes
(one fread for each attribute followed by fseek to skip the reading of the value),
except for encapsulatedDocument, which it writes to stdout

The alternative version reads the full CDICM at then works in memory

many little reads   1 big read               
total 0.000739696   0.000470684
pre   0.000019042   0.000055035
parse 0.000719799   0.000413749
post  0.000000855   0.000001900

Notable variability in time measured, which depends on read system availability.
Repeated observation shows one big read (using more memory) up to 50% faster than multiple small ones.
Smallest total time for the CDA example around 0.0003 seconds (a third of millisecond)