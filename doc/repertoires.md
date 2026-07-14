# DICOM "repertoires"

DICOM "repertoires" can be subdivided into groups.

- (1) Fourteen don't support code extension technique. 
- (2) Sixteen do. With this technique, it becomes possible to append to the first alphabetical group
a second ideographical one and eventually a third phonetical one.
- (3) Another repertoire is used in UR attributes (escaped UTF-8).
- (4) When no repertoire is specified, latin 1 ("ISO_IR 100") is used.

It sums up to 32 repertoires, number which fits into 5 bits. 
In the case of a double code extension, alphabetic+ideographical+phonetical sum to 15 bits.
This fits into the 16 bits we reserve after value representation in our DECK key syntax.

Repertoires are applied only to DICOM value representations:
- short value length: LO,LT,SH,ST 
- long value length: UC y UT

## list

### none

| idx  | code | description |
|------|------|-------------|
| 0x00 |      | empty       |

### Single-Byte Character Sets Without Code Extensions
http://dicom.nema.org/medical/dicom/current/output/html/part03.html#table_C.12-2

| idx  | code       | description |
|------|------------|-------------|
| 0x01 | ISO_IR 100 | latin 1     |
| 0x02 | ISO_IR 101 | latin 2     |
| 0x03 | ISO_IR 109 | latin 3     |
| 0x04 | ISO_IR 110 | latin 4     |
| 0x05 | ISO_IR 148 | latin 5     |
| 0x06 | ISO_IR 126 | greek       |
| 0x07 | ISO_IR 127 | arabic      |
|      |            |             |
| 0x09 | ISO_IR 144 | cyrilic     |
| 0x0A | ISO_IR 138 | hebrew      |
| 0x0B | ISO_IR 13  | japanese    |
| 0x0C | ISO_IR 166 | thai        |


### Multi-Byte Character Sets Without Code Extensions
http://dicom.nema.org/medical/dicom/current/output/html/part03.html#table_C.12-5

| idx  | code       | description      |
|------|------------|------------------| 
| 0x08 | ISO_IR 192 | Unicode in UTF-8 |
|      |            |                  |
| 0x0D | GB18030    | GB18030          |
| 0x0E | GBK        | GBK              |


### attributes vr UR (not listed in DICOM repertoires)

| idx  | code    | description       |
|------|---------|-------------------|
| 0x0F | RFC3986 | url-encoded utf-8 |

The URI/URL (UR) VR uses a subset of the Default Character Repertoire as defined in 
[RFC3986], and shall not use any code extension or replacement techniques. 

URI/URL domain name components that in their original form use characters 
outside the permitted character set shall use the Internationalized Domain Names 
for Applications encoding in accordance with IETF RFC5890 and RFC5891. 

Other URI/URL content that uses characters outside the permitted character set 
shall use the Internationalized Resource Identifiers encoding mechanism of 
IETF RFC 3987, representing the content string in UTF-8 
and percent encoding characters as required.


### Single-Byte Character Sets with Code Extensions
http://dicom.nema.org/medical/dicom/current/output/html/part03.html#table_C.12-3

| idx  | code            | description |
|------|-----------------|-------------|
| 0x10 | ISO 2022 IR 6   | ascii       |
| 0x11 | ISO 2022 IR 100 | latin 1     |
| 0x12 | ISO 2022 IR 101 | latin 2     |
| 0x13 | ISO 2022 IR 109 | latin 3     |
| 0x14 | ISO 2022 IR 110 | latin 4     |
| 0x15 | ISO 2022 IR 144 | cyrilic     |
| 0x16 | ISO 2022 IR 127 | arabic      |
| 0x17 | ISO 2022 IR 126 | greek       |
| 0x18 | ISO 2022 IR 138 | hebrew      |
| 0x19 | ISO 2022 IR 148 | latin 5     |
| 0x1A | ISO 2022 IR 13  | japanese    |
| 0x1B | ISO 2022 IR 166 | thai        |

### Multi-Byte Character Sets with Code Extensions
http://dicom.nema.org/medical/dicom/current/output/html/part03.html#table_C.12-4

| idx  | código          | descripción        |
|------|-----------------|--------------------|
| 0x1C | ISO 2022 IR 87  | japanese           |
| 0x1D | ISO 2022 IR 159 | japanese           |
| 0x1E | ISO 2022 IR 149 | korean             |
| 0x1F | ISO 2022 IR 58  | simplified chinese |

## bit 16
Reserved to flag if the original repertoire was transformed into UTF-8, or not (originally UTF-8)

## DECK value syntax
All the values are coded UTF-8. The original syntax is used only in case 
DICM original canonicalized DICM needs to be outputted