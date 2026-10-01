# ESCIPFserialized

https://dicom.nema.org/dicom/2013/output/chtml/part03/chapter_A.html

Based on ESIPbasetags and KVserialized 
Serializes attributes into groups that follow a simplified model of information entities:
- exam (patient+study)
- series (+equipment)
- capsule (encapsulated instance content treated as series, that is, stored in the database when there is one)
- instance (everything else but frames)
- frames (made of pixels)

00080005 removed. Everything textual is always written using UTF-8 coding

## groups of attributes
ESCIPFserialized lists groups of KVattributes, belonging to one only information entity.
They may be more than one group into an information entity. 
The groups are prefixed by one head KV which is not part of the original datataset.

DECK groups could be seen as a new and different implementation of the group length attributes in the original DICOM standard.
With respect to the latter, the differences are:
- attribute tag and vr (8 bytes) are replaced by a key length (1 byte) and a key (odd size less than 256)
- the key uses only url safe chars and slash (which repeats the semantics of directory contents in reference to the information entities)
- the group itself is essentially a macro of specifically related attributes, (instead of the gathering of tags with same first two bytes prefix).

````
• 1 byte: length of the key (KL)
• KL bytes: the key
• 4 bytes: length of the value (VL)
• VL bytes: the value, exactly as in the DICM
representation.
````
with the exception the VL refers to the sum of all the KV attributes of the group. 
This length can be used as a pointer to skip parsing part of the serialization. 

To be sure that the same information was already parsed and can be skipped, the key ends with a blake3 hash of the group. 
If the hash present in the key head of the group corresponds to the one already registered, there remains no doubt.
Besides the hash can also be used to control that the following attributes serialized were not altered.

The encapsulated objects and frames are exceptions with no blake3. Being first class objects of other standards, 
the inner coherence is secured by the corresponding standard.
In these cases, the group does contain the encapsulated object (instead of a list of attributes). 
The object length is exact, with no padding null char in case the length is odd.

## group key components
| category             | key prefix         | key name         |  key suffix  | comment                       |
|----------------------|--------------------|------------------|--------------|-------------------------------|
| Exam+patient         | date/E/     (even) | --           (2) | .blake3 (65) |                               |
| Series               | date/E/S/    (odd) | ---          (3) | .blake3 (65) |                               |
| PDF                  | date/E/S/    (odd) | ps           (2) | .pdf    (4)  | 1.2.840.10008.5.1.4.1.1.104.1 |
| CDA                  | date/E/S/    (odd) | uc sc ds ot  (2) | .xml    (4)  | 1.2.840.10008.5.1.4.1.1.104.2 |
| STL                  | date/E/S/    (odd) | 3d           (2) | .stl    (4)  | 1.2.840.10008.5.1.4.1.1.104.3 |
| OBJ                  | date/E/S/    (odd) | tx           (2) | .obj    (4)  | 1.2.840.10008.5.1.4.1.1.104.4 |
| MTL                  | date/E/S/    (odd) | tx           (2) | .mtl    (4)  | 1.2.840.10008.5.1.4.1.1.104.5 |
| AV1                  | date/E/S/    (odd) | sp tm        (2) | .av1    (4)  | spatial or temporal           |
| Instance+sop+private | date/E/S/I/ (even) | priv ----    (4) | .blake3 (65) | private + 0002                |
| Frame                | date/E/S/I/ (even) | 00001..99999 (5) | .xxx    (4)  | one frame one file            |


- date, E, S, codified uibb64 ("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~") are url safe
- they are of even length, lower than 45.
- av1 concatenates uncompressed frames (both concatenation and discrete frames are available)

## possible use by the receptor
- process one group at a time
  - separate the group key into proper key and blake3
  - checks the existence of a register with same path and hash
    - checks the hash with the contents
      - bad -> list the group for recall
      - good
        - write the content of the group in a file at the path written in the first work of the key
        - registers the key path, the hash and some of the attributes of the group into a database 

## FOVIA 

### study

patientBirthDate
patientID
patientName
patientSex
studyAccessionNumber
studyDate
studyDescription
studyID
studyInstanceUID
studyTime

### series

modality
seriesDate
seriesDescription
seriesInstanceUID
seriesNumber
seriesTime

### image

acquisitionNumber
acquisitionTime
axis	"Z"
bitsAllocated
bitsStored
bluePaletteLUTDescriptor
cols
contrastAdministrationRouteSequence	0
contrastAgent	""
contrastAgentSequence	0
contrastRoute	""
convertedToUnsiged	true
echoNumbers	""
echoTime	0
fieldOfViewDimensions	""
forceServerRenderToUpdateSignedState	true
frameIncrementPointer	""
frameNumber	1
frameOfReferenceUID	"1.2.840.113619.2.55.3.314599446.5388.1126625110.487.7100.0.11"
greenPaletteLUTDescriptor	""
headerPhotometricInterpretation	""
highBit	15
imageCreationDate	"20050914"
imageCreationTime	"132245"
imageLocation	144.5
imageNumber	1
imageOrientationPatient	"1.000\\0.000\\0.000\\0.000\\1.000\\0.000"
imagePath	"/fovia/data/democases/DICOM2/949580.dcm"
imagePositionPatient	"-110.000\\-92.400\\144.500"
imageType	"ORIGINAL\\PRIMARY\\AXIAL"
imagerPixelSpacing	"0.430\\0.430"
lossyImageCompression	""
lossyImageCompressionMethod	""
lossyImageCompressionRatio	""
lutExplanation	[]
manufacturer	"GE MEDICAL SYSTEMS"
modality	"CT"
numberOfFrames	1
numberOfModalityLUTEntries	0
numberOfOverlayEntries	0
numberOfVOILUTEntries	0
patientOrientation	""
patientPosition	"HFS "
photometricInterpretation	"MONOCHROME2 "
pixelPaddingValue	0
pixelRepresentation	0
pixelSpacing	"0.430\\0.430"
pixelSpacingCalibrationDescription	""
pixelSpacingCalibrationType	""
plannarConfiguration	0
redPaletteLUTDescriptor	""
repetitionTime	0
rescaleIntercept	-33792
rescaleSlope	1
rescaleType	"HU"
rows	512
samplesPerPixel	1
sharedMemoryFileName	"/usr/memory-manager/temp/fs_255f7c2e-4a62-4ef1-a0d5-b9b800750894_1.2.840.113619.2.55.3.314599446.5388.1126625111.914.1"
sliceLocation	144.5
sliceThickness	0.625
sopClassUID	"1.2.840.10008.5.1.4.1.1.2"
sopInstanceUID	"1.2.840.113619.2.55.3.314599446.5388.1126625111.914.1"
sourceImageSequence	[]
spacingBetweenSlices	0
spatialLocationPreserved	true
totalFrames	1
tpiNumber	0
voiLUTFunction	""
windowCenter	"40.000000"
windowWidth