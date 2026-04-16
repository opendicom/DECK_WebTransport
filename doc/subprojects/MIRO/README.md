# MIRO (Medical Imaging Reactive Online)

MIRO is the web page part of the PACS.

Thanks to javascript and the many API embedded in the browser, a web page is an application which:
- supports multi processing and inter process communication
- embeds a true database https://www.w3.org/TR/IndexedDB/
- accesses a real Origin Private File System (OPFS) which is non blocking
- renders opengl via webgl
- executes compiled C and rust code via WASM
- communicates through UDP channels with the server (WebTransport)

The browser gets the web page and then executes it like an application, with the capacity to initiate UDP streams with the server.

## FOVIA
In our case image rendering is managed by a commercial library, with javascript API , which pilots webgl.
