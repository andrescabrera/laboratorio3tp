#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
		perror("MODO DE USO: ./binario NUMERO_ENTERO");
        exit(EXIT_FAILURE);
	}
	
	//Debug
	printf("Nombre del programa: %s\n", argv[0]);
	printf("Argumento 1: %s\n", argv[1]);

	//Si el número recibido es múltiplo de 5
	if(argv[1] % 5 == 0)
	{
		//se deberán crear 5 procesos hijos.
		int i;
		int proceso[5];
		
		for(i=0; i<5; i++)
		{
			if((proceso[i] = fork()) == -1)
			{
				perror("ERROR en fork() multiplos de 5.");
				exit(EXIT_FAILURE);
			}
			if(proceso == 0)
			{
				//Estoy en el hijo
			} 
			else
			{
				//Estoy en el padre
				
				int i;
				for(i = 0; i < 5; i++)
					wait();
					
			}
		}
	} 
	else if (argv[1] % 2 == 0) //Si el número es múltiplo de 2
	{
		//se deberán crear 4 procesos hijos.
	} else 
	{
		//Si no cumple ninguna de las condiciones anteriores, 
		//el programa deberá salir escribiendo en un archivo un mensaje de error.
		escribirMensajeDeError("El número ingresado no es multiplo de 5 ni de 2");
		exit(EXIT_SUCCESS);
	}
	
    return 0;
}
