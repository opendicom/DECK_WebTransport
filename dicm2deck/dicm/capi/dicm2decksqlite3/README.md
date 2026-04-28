# edckv sqlite

Para

- deduplicación

- control de integridad

- vinculación agil de instancias con series y series con estudio

- necesitamos consultas  del conjunto de instancias de un estudio. 
- se logra gracias a una base de datos sqlite C **embedida**.

Esta base de datos recibe todos los objetos del estudio hasta la autenticación del informe. 
Una vez concluido el estudio (no se reciben más objetos luego del informe), se consolida su contenido en 
otra base de datos a definir, multi nucleo, distribuida, optimizada para concurrencia. 
Será otro proyecto que complete el PACS.
