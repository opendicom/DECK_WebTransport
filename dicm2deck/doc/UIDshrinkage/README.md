# b64uid
shrinked same-order url-safe ascii codification for unique identificators, with special consideration of DICOM ones

## use case

The DICOM identificator UID, made of numbers and dots is key for objects classification. But since it may be quite large, up to 64 chars, it results inefficient when used as file or dir name. Especially so, when subject to sorting or filter operations.

The OID encoding of UID is not efficient since it uses merely 11 codes ( 0 1 2 3 4 5 6 7 8 9 . ) out of the 256 available in each char.

b64uid aims at shrinking them, but cleverly. In particular, it complies with the following requirements:
- the result must be readable ascii
- the result must be url-safe
- the sorting of OIDs before and after shrinkage is unchanged
- the algorithm to shrink and expand should be simple and without dependencies to extern libraries

## contents of the repository

- bash 5 ( use: stdin | b64uid | stdout )
    - b64uid.sh
    - uidb64.sh


## algorithm

The algorithm is made of two operations :

###  UID codes mapping to half bytes

| half byte | code |
|---|---|
| 0x0 | 1.2.840.10008. |
| 0x1 | . |
| 0x2 | 0. |
| 0x3 | 0 |
| 0x4 | 1. |
| 0x5 | 1 |
| 0x6 | 2. |
| 0x7 | 2 |
| 0x8 | 3. |
| 0x9 | 3 |
| 0xA | 4 |
| 0xB | 5 |
| 0xC | 6 |
| 0xD | 7 |
| 0xE | 8 |
| 0xF | 9 |

half bytes 0x0, 0x1, 0x4, 0x6, 0x8 shrink two or more codes. "1.2.840.10008." is the root of all DICOM OID.


### base 64 like dump of groups of 6 half bytes

3 half bytes are processed together to output 2 6-bit chars from the base 64 table below:

```
-, 0, 1, 2, 3, 4, 5, 6, 7, 8,
9, A, B, C, D, E, F, G, H, I,
J, K, L, M, N, O, P, Q, R, S,
T, U, V, W, X, Y, Z, _, a, b,
c, d, e, f, g, h, i, j, k, l,
m, n, o, p, q, r, s, t, u, v,
w, x, y, z
```

The length of the result of the shrinkeage is always a multiple of 2 chars.
