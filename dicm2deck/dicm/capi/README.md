# capi: uapi override

- sequence and item markup is ignored
- cCreate, cCommit and cClose override uCreate, uCommit and uClose
- uAppend is overriden by the call to varios capi handler depending on the category to which they belong.

The categories are:
- pAppend for private attributes;
- bAppend (byte pixel), wAppend (short pixel) for pixel;
- eAppend for patient and study attributes;
- sAppend for series attributes;
- iAppend for any other attribute.

Such categories are very usefull for indexing tasks, such as for instance the construction of a PACS database.
