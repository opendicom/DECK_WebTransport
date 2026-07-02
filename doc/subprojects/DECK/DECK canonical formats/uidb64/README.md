# uidb64

Usage: stdin | uidb64 | stdout

Created by jacquesfauquex on 2026-05-01.

Converts uid into its reduced form uidb
- each character is transformed in half byte
- the resulting stream is then transformed into a special base64, 
friendly to urls (which does not contain the reserved chars / or =)

Here are the bash shrinker and expander.

The corresponding shrinker C code is found in storescp.cc, just before storeSCPCallback, which uses it.