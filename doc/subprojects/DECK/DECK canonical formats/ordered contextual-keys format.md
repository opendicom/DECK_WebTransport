# ordered contextual keys format.md

## ordered

The order, the classification is a prerequisite to both the DICM representation and to the dictionary key-value. The DICOM "tag" (An ordered pair of 16-bit unsigned integers representing the Group Number followed by Element Number). 

### DICM

(1) On the DICM side, unfortunately, when "big endian" was deprecated and "little endian" promoted as DICOM numbers canonical encoding, tag codification was scrambled. Byte re-ordering, pair by pair, is now required before any classification. This bad coding is somehow obfuscated by the correct order of the attributes within the serialization of the SOP instance into the file. However, to identify a tag defined in the DICOM dictionary (part 6), the filter needs to be re-ordered, pair by pair, before recognizing it in the serialization.

(2) The nightmare doesn´t stop here. DICOM promotes deeply structured datasets. One kind of attribute, the sequence (datatype SQ) contains items which are themselves datasets. Dataset embedding is recursive. Embedded datasets may contain attributes type SQ. This architecture implies a deep parsing of the complete serialization, from start, up to any attribute looked for.

### DECK

The tag is concise and its use is necessary to relate attributes to the DICOM IOD (Information Object Definition - part 3 of the standard). But its representation can fit better natural written english lenguage, from left to right, which is reflected by big endian codification. This is what DECK does in 4 bytes: group most significant byte, group less significant byte, element most significant byte, element less significant byte. This solves problem 1.

Problem 2 is solved by substituing the "tag" by the binary "contextualized-key", which contains tags, for sure, but also other ordering bytes, in order to reflect :
- instance classification within the exam,
- attributes classification within the instance,
- correct interpretation of the attribute. 

A DECK Key is composed of two or more groups of 8 bytes:
- (a) The first group is a prefix which identifies the instance and the series.
- (b) The last group exposes attribute tag and representation.
- (c) In-between, there are zero or more 8 byte strings representing an embedding sequence and the numbered item in it.

#### (a) prefix

Root tags in the dataset of a DICOM instance are not prefixed by any item number. The DICOM SOP instance is the list content of a non existent root item. In the XML representation,  such root item is mandatory. This was the occasion to create the root XML element <NativeDicomModel>, unique root to the XML representation.
In DECK, the object described is an Exam, that is, a group of SOP instances. In order to identify the SOP instance referred to by a key, we prepend to the key an 8 bytes prefix. It would be conceptually equivalent to a <NativeDicomModel SOPinstance=""> in the XML representation. The prefix both gathers the attributes of a specific instance and differentiates instances.
We carefully crafted the format of the prefix. In DICOM object model, instances (and series too) are identified by both an ordinal number (the original identification) and an UID (the new universal identifier, which conforms to the syntax of the OID). The UID is defined as universally unique, but is quite large (up to 64 bytes). The number is short, but may not be unique within the study. It is possible to observe on occasions instances with the same number and a different UID. A short prefix based on numbers needs to be able to differentiate these two instances. This is why we added an index byte before the number in the 8 bytes of the format of the prefix PF: "sSSiIIFF".
We take advantage of the special value 0 of these index to signal an attribute common to all the instances of a series or of a study, so that we can de duplicate these common attributes and register them once only.

