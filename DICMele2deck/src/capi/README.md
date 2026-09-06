# capi: uapi override

- sequence and item markup is ignored

- val is overriden by the call to one of four capi handlers:
  - pAttribute for private attributes;
  - eAttribute for patient and study attributes;
  - sAttribute for series attributes;
  - iAttribute for any other attribute.
