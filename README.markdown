"Torrente"
==========
Trabajo Práctico para la materia Laboratorio III, Universidad de Palermo
------------------------------------------------------------------------
###Participantes
- Andrés Cabrera
- Juan Lopreiatto
- Isidoro Schmunck

####Notas de la versión
@acabrera - 14hs, Sabado 28/5/2011:

####BUILD 17 - CAMBIOS PRINCIPALES
Se soluciono el problema de los archivos: se enviaban 256bytes con el tamaño desde c1 a srv, y server enviaba a su vez 256 a c2, y se recibian 255 en c2.

Se elimino el thread threadDescargarArchivos, ya que su funcionalidad principal la realizaba ahora el thread threadEnviarArchivos.
Se cambio el nombre de threadEnviarArchivos, debido a que ahora cumple varias funcionalidades.
Como descargar (transferir de helper.c) y enviar archivos.

El envío de la opción CLIENTE_SOLICITA_DESCARGA hacia srv, y demás tareas que realizaba el thread threadDescargarArchivos,
ahora se realizan en cliente.c, funcion lanzador - opción 4.

Ahora el nombre del archivo se genera parseando la ruta, en el mismo cliente,
guardando la ruta solicitada actual como parametro del thread threadEnviarDescargarArchivos.
Los argumentos del thread se deben actualizar cuando se ingresa por teclado,
por tanto al estar dentro de la funcion lanzador, se tuvo que hacer global la estructura
de argumentos del thread.

Se agrego una opcion nueva al protocolo, SERVIDOR_ENVIA_LISTA_COMPARTIDOS,
esto para solucionar otros problemas de sincronización con la lectura de opciones.

Se elimino threadRecibirMostrarCompartidos en funcionesCliente.c,
las tareas que este realizaba estan en el lanzador y en threadEnviarDescargarArchivos.

#####CAMBIOS PENDIENTES
El nombre del threadEnviarDescargarArchivos(): no es coherente con las tareas que realiza.

######En servidor
- Ver la lista de cliente conectados
- Desconectar a un cliente
######En cliente
- Algunos mensajes que estan en log, deberían por pantalla igualmente, o solo por pantalla.

#####ERRORES CONOCIDOS
(Seguir los pasos para comprobarlo)
1. Un cliente comparte una carpeta. Lista OK en cliente y servidor.
2. Un cliente pide un archivo. Lista OK en cliente y servidor.
3. Un cliente pide ver la lista de compartidos.

Se arruina la lista pero la cantidad de nodos es la misma.

#####PROBLEMAS CONOCIDOS
Tamaños de las variables de las rutas y otras, escogidos al azar...
El cliente solicita un archivo por su ruta en vez de su ID.
