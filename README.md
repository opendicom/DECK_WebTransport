# SIRIUS WebTransport pacs

SIRIUS WebTransportPacs is a new way to consume medical imaging on internet. 
The interoperability with preexisting DICOM PACS is limited to DICOM Store SCP 
through a specialized proxy, that we call PCS (Picture Communication System).

## PCS
- converts DICM to DECK sqlite
- compresses imagenological series
- forwards DECK sqlite and corresponding resources to SIRIUS webtransport pacs,
preserving the network security of the hospital.

## PACS
SIRIUS WebTransportPacs is no standard PACS, 
neither is interoperable with Standard DICOM viewers. 
SIRIUS WebTransportPacs is an all-in-one system for storage, data mining, distribution and visualization.
It is designed starting from the specifications of modern web browsers backwards.

  
    
    
  

  
    
    
    
  

  
    
      
      
    
    
      
      
    
  

in order to offer the lowest latency in most challenging network conditions, 
such as for instance source communting (wifi/3,4,5G or satelite).
This implies streaming and datagram management over UDP.
W3C WebTransport QUIC UDP enables UDP channels creation vía javascript client side.
This also implies to distantiate the product from DICOM part 10 and DICOMweb, 
which were designed for files and adopted complex metadata structures.

## Goals
- first: complex large imaging study diagnostic from high quality cell phone, tablet 
or any device connected by air (5G/satelite/Wifi). 
This is a challenging goal, because current http rest DICOMweb technology fails this use case. 
To pass it, the viewing conditions need to be similar to a local workstation, with almost no latency 
between a command by the radiologist and the visualization of the corresponding frames. 
The transport of very large files or mime multi-part contents in DICOMweb 
is problematic on complex network with route and provider commutation. 
A new strategy based on reactive web page managing just in time datagram communication is 
our innovative solution. It comes with its metrics: 
the number of datagrams necessary to perform a visualization.

- second: this metrics observable from the server, the client, and any router inbetween 
will demonstrate SIRIUS WebTransport to be the most savvy and reactive DICOM viewer.

- third: as a corollary, our DECK representation of DICOM datasets and transport of selected attributes 
will also improve greatly the speed of learning of Artificial Intelligence models.

## Keywords

- DICOM:        Digital Imaging Communication in Medicine. Universal standard.
- DICM:         "Magic name" of the original serialization of datasets into files
- JSON:         serialization into json lenguage
- XML:          serialization into xml lenguage
- DECK:         Dicom Exam Contextualized Keys. Serialization into a key-value dictionary.

- http/3:       web protocol which switches to UDP communication after the shakehand and encription 
- [WebTransport:](https://www.w3.org/TR/webtransport/) web page javascript opens UDP channels
- [UDP:](https://datatracker.ietf.org/doc/html/rfc768)          User Datagram Protocol

- [rust:](https://rust-lang.org/)         language of our server
- [tokio:](https://tokio.rs/)        asynchronous runtime for the Rust programming language, used as the backbone of both http3 and udp communication  
- [turso:](https://docs.turso.tech/introduction)        rust embedded database, compatible with sqlite
            
- [javascript:](https://developer.mozilla.org/en-US/docs/Web/JavaScript)   official name [ecmascript](https://ecma-international.org/publications-and-standards/standards/ecma-262/) language of the web page  
- [IndexedDB:](https://www.w3.org/TR/IndexedDB/)    javascript key value database for the web page
- [OPFS:](https://developer.mozilla.org/en-US/docs/Web/API/File_System_API/Origin_private_file_system)         javascript non-blocking file system for the web page
- [webgl:](https://www.khronos.org/webgl/)        gpu acceleration of the rendering of the canvas
- [FOVIA:](https://fovia.com)        web page javascript library for the rendering of 2D and 3D of medical imaging
- [fovia.ai:](https://fovia.ai)     transforms AI algorithm results into interactive visualization
---

Though this project is born in the Mercosur, contributions shall be written in english to facilitate the communication with Fovia, provider of the visualization sdk for our project.
