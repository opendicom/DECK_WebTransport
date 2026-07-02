# b64uid
Shrunk same-order url-safe ascii codification for unique identifiers, 
with special consideration of DICOM ones.

## use case

The DICOM identifiers modality UID and IS, made of numbers and dots is key for objects classification. 
But since it may be quite large, up to 64 chars, it results inefficient when used as file or dir name. 
Especially so, when subject to sorting or filter operations.

The OID encoding of UID is not efficient since it uses merely 11 codes ( 0 1 2 3 4 5 6 7 8 9 . ) 
out of the 256 available in each char. IS uses + and -. HL7v2 defines separators ^ | ~

b64uid aims at shrinking them, but cleverly. In particular, it complies with the following requirements:
- the result must be readable ascii
- the result must be url-safe
- the sorting of OIDs before and after shrinkage is unchanged
- the algorithm to shrink and expand s hould be simple and without dependencies to extern libraries

## algorithm

The algorithm is made of two operations :

### codes mapping to half bytes

| ascii | half byte |
|-------|-----------|
| 0     | 0x0       |
| 1     | 0x1       |
| 2     | 0x2       |
| 3     | 0x3       |
| 4     | 0x4       |
| 5     | 0x5       |
| 6     | 0x6       |
| 7     | 0x7       |
| 8     | 0x8       |
| 9     | 0x9       |
| .     | 0xA       |
| -     | 0xB       |
| +     | 0xC       |
| ^     | 0xD       |
| \|    | 0xE       |
| ~     | 0xF       |

### base 64 like dump of half bytes

3 * 4-bits are equivalent to 2 * 6-bit, which are transformed in two ascii from the base 64 table below:

```
-, 0, 1, 2, 3, 4, 5, 6, 7, 8,
9, A, B, C, D, E, F, G, H, I,
J, K, L, M, N, O, P, Q, R, S,
T, U, V, W, X, Y, Z, _, a, b,
c, d, e, f, g, h, i, j, k, l,
m, n, o, p, q, r, s, t, u, v,
w, x, y, z
```

The length of the shrinkage is always a multiple of 2 ascii (which correspond to 3 ascii of the input)
