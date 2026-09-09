# ESIPFserialized

Based on ESIPbasetags and KVserialized 
Serializes study, series, instances, private and framepixel attributes

## groups of attributes
ESIPFserialized lists groups of KVattributes, corresponding to the categories exam(study), series, instance, private and framepixels.
The categories are prefixed by one head KV which is not part of the original datataset.
Categories could be seen as a new and different implementation of the group length attributes in the original DICOM standard.
With respect to the latter, the difference are:
- attribute tag + vr are replaced by a key concatenating much information (as seen below)
- the 4 or 8 bytes of the tag + vr becomes a variable length, always odd and shorter than 256 bytes. 
One bit of the Key length is enough to know if the key is of a dicom attribute or the head of a group
- the key uses only url safe chars and / where the separator could be translated as the directory in a file system

## KV pattern
ESIPFserialized respects the KV pattern defined in KVserialize
````
• 1 byte: length of the key (KL)
• KL bytes: the key
• 4 bytes: length of the value (VL)
• VL bytes: the value, exactly as in the DICM
representation.
````
with the exception the VL refers to the sum of all the KV attributes of the group. 
This length can be used as a pointer to skip parsing part of the serialization.

For instance, if the attributes of the study or of the series were already processed from the serialization of another instance belonging to the study,
there is no need to parse the corresponding group again.

To be sure that the same information was already parsed and can be skipped, the key ends with a blake3 hash of the group. 
If the hash present in the key head of the group corresponds to the one already registered, there remains no doubt.
Besides the hash can also be used to control that the following attributes serialized were not altered 

## group key components
| category       | l  | key                                                                      | key size                    | contents          | comment                                |
|----------------|---|--------------------------------------------------------------------------|-----------------------------|-------------------|----------------------------------------|
| Exam           | 1 | dab64[slash]uib64E                                  [space][space]blake3 | 4+ (even up to 44)  +3  +32 | study and patient |                                        |
| Series         | 1 | dab64[slash]uib64E[slash]uib64S                            [space]blake3 | 4+ (even up to 88)  +3  +32 | series            | both generic and specific              |
| Instance       | 1 | dab64[slash]uib64E[slash]uib64S[slash]uib64I        [space][space]blake3 | 4+ (even up to 132) +5  +32 | instance          | includes per frame metadata            |
| Private        | 1 | dab64[slash]uib64E[slash]uib64S[slash]uib64I[minus]        [space]blake3 | 4+ (even up to 132) +3  +32 | private + group2  | [minus]='-' (b64 uses ~, not -)        |
| Frame          | 1 | dab64[slash]uib64E[slash]uib64S[slash]uib64I[slash]0001.xxx[space]blake3 | 4+ (even up to 132) +13 +32 | frame image       | 0000.xxx=encapsulated object not frame |

- uibb64 codes "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~" are url safe
- they move upwards the ascii scale the following characters present in uids and numeric values:
  - 5C '\'(multivalue), 
  - 5E '^'(component), 
  - 2D '-'(negative),
  - 2B '+'(positive),
  - 2E '.'(dot)
- These are liberated as lower code separators
- ascii order: space, % - . / 0-9 A-Z _ a-z ~ 
- '/' can be use within the key to simulate a route to a resource (we use it to group instance by series, series by exams, exams by date)
- 'space', '-' and '.'  can be used in keys to separate the route and the hash of the value without altering a classification

- dab64 is aammdd written en 4 b64 chars
- uib64... are uids compressed to an even (lower than 45) number of url safe chars.

- the space (or double space) within the key separates two words: a "path" and an "hash"

## possible use by the receptor
- process one group at a time
  - separate the group key into proper key and blake3
  - checks the existence of a register with same path and hash
    - checks the hash with the contents
      - bad -> list the group for recall
      - good
        - write the content of the group in a file at the path written in the first work of the key
        - registers the key path, the hash and some of the attributes of the group into a database 