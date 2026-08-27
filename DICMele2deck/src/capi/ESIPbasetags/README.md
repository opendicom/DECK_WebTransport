# ESIPbasetags

Attribute category (defined at base dataset level)

- E exam
- S series generic
- I instance
- P manufacturer specific

Useful for relational database containing the dataset

Specific functions are also created selecting the identifiers (which are unfortunately burried within the dataset):
case 0x00080008: kvUi //instance uid
case 0x0020000D: kvUe //study uid
case 0x0020000E: kvUs //series uid
case 0x00080019: kvUs //PyramidUID
