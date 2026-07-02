# key value

DECK is a dictionary key-value format. The binary keys are ordered. 
Their length vary between 16 bytes and 192 (most extreme case) by 8 bytes steps.

## database
- The C sqlite of the converter dicm2deck, 
- the rust Turso sqlite-compatible database of the server, 
- and the javascript indexedDB of the browser 
are all appropiate for DECK key-value dictionaries handling.

Between converter and pacs an exam sqlite dump can be created on one side 
and read on the other.
