# ESIPbasetags

https://dicom.nema.org/dicom/2013/output/chtml/part03/chapter_A.html

Attribute category (defined at base dataset level)

- E exam
- S series generic
- I instance
- P manufacturer specific (private)

Useful for relational database containing the dataset

category identifiers (which are unfortunately burried within the dataset) are made available in global variables. 
These use uib64 format which outputs url safe weighting two thirds of the original length.
