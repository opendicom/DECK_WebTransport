# DECK_WebTransport

W3C WebTransport DICOM imaging. 

DECK (Dicom Exam Contextualized Keys) representation of DICOM dataset optimizes the storage server-side. 

Client side, a carefully tailored use of UDP Channels from the web page optimizes the number of datagrams required. 

The primary clients are the cell phone and tablet.

## Goals

- first: complex large imaging study diagnostic from high quality cell phone, tablet or any device conected by air (5G/satelite/Wifi). This is a challenging goal, because current http rest DICOMweb technology fails this use case. To pass it, the viewing conditions need to be similar to a local workstation, with almost no latency between a command by the radiologist and the visualization of the corresponding frames. The transport of very large files or mime multi-part contents in DICOMweb is problematic on complex network with route and provider commutation to client with smaller memory. A new strategy based on reactive web page managing just in time datagram communication is our innovative solution. It comes with its metrics: the number of datagrams necessary to perform a visualization.

- second: based on this metrics, we aim to be the most savvy, and consequently most reactive, in the world.

- third: as a corollary, our representation DECK of DICOM datasets and transport of selected attributes will improve greatly the learning performance of Artificial Inteligence models.

---

Though this project is born in the mercosur, contributions shall be written in english.
