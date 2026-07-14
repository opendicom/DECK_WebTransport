# b64uid
Shrunk same-order url-safe ascii codification for unique identifiers, 
with special consideration of DICOM ones.

## use case

The DICOM identifiers UI, IS and DA, made of numbers and dots is key for objects classification. 
But since it may be quite large, up to 64 chars, it results inefficient when used as file or dir name. 
Especially so, when subject to sorting or filter operations.

The OID encoding of UID is not efficient since it uses merely 13 codes ( . 0 1 2 3 4 5 6 7 8 9 multivalue padding). 
IS additionally uses + and -. 
HL7v2 defines a component separator ^ which is useful relate two or more into a complex identificator.
This sums up exactly 16 codes, which fit into a half byte.

b64uid aims at shrinking them, but cleverly. In particular, it complies with the following requirements:
- the result must be readable ascii
- the result must be url-safe ("-" / "." / DIGIT /ALPHA /  "_" / alpha / "~")
- the sorting of OIDs before and after shrinkage is unchanged
- the algorithm to shrink and expand should be simple and without dependencies to extern libraries

## algorithm

The algorithm is made of two operations :

### codes mapping to half bytes

The order of ascii is insignificant. What matter is the order of the mapping to half codes.
For ordering purposes, any separator needs to come before any digit.

| half byte | ascii | hex | special chars          |
|-----------|-------|-----|------------------------|
| 0x0       | NULL  | 00  | last value padding     |
| 0x1       | \     | 5C  | multi-value separator  |
| 0x2       | ^     | 5E  | multi-component value  |
| 0x3       | -     | 2D  | negative               |
| 0x4       | +     | 2B  | positive               |
| 0x5       | .     | 2E  | dot                    |
| 0x6       | 0     | 30  | 0                      |
| 0x7       | 1     | 31  | 1                      |
| 0x8       | 2     | 32  | 2                      |
| 0x9       | 3     | 33  | 3                      |
| 0xA       | 4     | 34  | 4                      |
| 0xB       | 5     | 35  | 5                      |
| 0xC       | 6     | 36  | 6                      | 
| 0xD       | 7     | 37  | 7                      |
| 0xE       | 8     | 38  | 8                      |
| 0xF       | 9     | 39  | 9                      |

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
