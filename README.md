# DECK_WebTransport

W3C WebTransport DICOM imaging. 

Server side, DECK (Dicom Exam Contextualized Keys) representation of DICOM dataset optimizes the storage. 

Client side, a carefully tailored use of UDP Channels from the web page optimizes the number of datagrams required. 

The primary clients are the cell phone and tablet over 5G/satelite/wifi.

## Goals

- first: complex large imaging study diagnostic from high quality cell phone, tablet or any device conected by air (5G/satelite/Wifi). This is a challenging goal, because current http rest DICOMweb technology fails this use case. To pass it, the viewing conditions need to be similar to a local workstation, with almost no latency between a command by the radiologist and the visualization of the corresponding frames. The transport of very large files or mime multi-part contents in DICOMweb is problematic on complex network with route and provider commutation. A new strategy based on reactive web page managing just in time datagram communication is our innovative solution. It comes with its metrics: the number of datagrams necessary to perform a visualization.

- second: based on this metrics, we aim to create the most savvy, and consequently most reactive, viewer of this kind in the world.

- third: as a corollary, our representation DECK of DICOM datasets and transport of selected attributes will improve greatly the learning performance of Artificial Inteligence models.

## Keywords

DICOM:        Digital Imaging Communication in Medicine. Universal standard.
DICM:         "Magic name" of the original serialization of datasets into files
JSON:         serialization into json lenguage
XML:          serialization into xml lenguage
DECK:         Dicom Exam Contextualized Keys. Serialization into a key-value dictionary.

http/3:       web protocol which switches to UDP communication after the shakehand and encription 
WebTransport: https://www.w3.org/TR/webtransport/
UDP:          https://datatracker.ietf.org/doc/html/rfc768

rust:         https://rust-lang.org/ lenguage of our server
tokio:        asynchronous runtime for the Rust programming language, 
              used as the backbone of both our http3 and udp communication
              https://tokio.rs/ 
turso:        https://docs.turso.tech/introduction
            
javascript:   official name ecmascript, lenguage of our client
              https://ecma-international.org/publications-and-standards/standards/ecma-262/
              https://developer.mozilla.org/en-US/docs/Web/JavaScript
IndexedDB:    https://www.w3.org/TR/IndexedDB/
OPFS:         https://developer.mozilla.org/en-US/docs/Web/API/File_System_API/Origin_private_file_system
webgl:        https://www.khronos.org/webgl/
FOVIA         web page javascript library for the rendering of 2D and 3D of medical imaging
              https://fovia.com
fovia.ai      transforms AI algorithm results into interactive visualization
              https://fovia.ai
---

Though this project is born in the mercosur, contributions shall be written in english to facilitate communication with Fovia, provider of the visualization sdk for our project.
