# KVserialized

Derived from canonical
Outputs a basic key value streamed format described in the 2026 MEMEA paper:

````
The key-value list response format is a
sequence of zero or more quadrinommal key-value structure
in key ascending order, without any bytes before, between or
after them. The 4 components are:
• 1 byte: length of the key (KL)
• KL bytes: the key
• 4 bytes: length of the value (VL)
• VL bytes: the value, exactly as in the DICM
representation.
The list is ended with 1 byte: 1=complete
````

The list starts at attribute (0002,0002)
It ignores sequence and item start and end
Since Keys and Values are binary, KVserialized outputs to file for now (instead of stdout)