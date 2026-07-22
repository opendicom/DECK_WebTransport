# capi: uapi override

- sequence and item markup is ignored
- uAppend is overriden by the call to distinct capi handlers depending on the category to which they belong.

The categories are:
- pAppend for private attributes;
- bAppend (byte pixel), wAppend (short pixel) for pixel and some other pixel related ones;
- eAppend for patient and study attributes;
- sAppend for series attributes;
- iAppend for any other attribute.
