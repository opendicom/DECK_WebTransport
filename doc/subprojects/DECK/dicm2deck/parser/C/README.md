Usefull libraries:
- sqlite
- dcmtk
- blake3
- grok j2k
are all written in C/C++

The parser, in a previous version, has been working in production for many years,
was nicely structured, compact and very fast,
and needs only little modifications
- prefix
- htj2k
to fit into this new project.

The original was written in objective-c, 
we translated it to pure C
and will now adjust the details for it to work as a converter triggered by 
dcmtk storescp, one of the most stable and universal receiver of DICM files.

