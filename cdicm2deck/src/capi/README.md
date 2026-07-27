# capi: uapi override

- sequence and item markup is ignored
- uAppend is overriden by the call to one of four capi handlers:
- pAppend for private attributes;
- eAppend for patient and study attributes;
- sAppend for series attributes;
- iAppend for any other attribute.
