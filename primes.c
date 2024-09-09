#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>



void filter(int fds_read){
	pid_t pid;

	int lectura;
	int numero_filtrado;
	lectura = read(fds_read,&numero_filtrado, sizeof(numero_filtrado));
	if(lectura == 0){
		close(fds_read);
		exit(0);
		return;
	}
	if(lectura == -1){
		close(fds_read);
		perror("Fallo lectura");
		exit(-1);
		return;
	}

	printf("primo %i\n",numero_filtrado);
	fflush(stdout);

	int fdsNuevo[2];
	int r = pipe(fdsNuevo);
	if(r == -1){
		perror("Fallo pipe");
		exit(-1);
		return;
	}
	pid = fork();
	if(pid == -1){
		perror("Fallo fork");
		exit(-1);
		return;
	}
	if(pid == 0){
		close(fds_read);
		close(fdsNuevo[1]);
		filter(fdsNuevo[0]);
		return;
	} else {
		int numero;
		close(fdsNuevo[0]);

		while(read(fds_read,&numero, sizeof(numero)) > 0){
			if(numero % numero_filtrado != 0){
				int w = write(fdsNuevo[1], &numero, sizeof(numero));
				if(w == -1){
					perror("Error de write");
				}
			}	
		}
		
		close(fdsNuevo[1]);
		close(fds_read);
	}
	wait(NULL);
	exit(0);
	return;
}


int main(int argc, char *argv[]){

	pid_t pid;
    int fds[2];

	char* ingresoPorTerminal = argv[1];
	if(ingresoPorTerminal == NULL){
		return 0;
	}
    int conversion = 10;
    int s = sscanf(ingresoPorTerminal,"%d", &conversion);
	if(s <= 0){
		return 0;
	}
	
	int r = pipe(fds);
	if(r == -1){
		perror("Error");
		return 0;
	}

	pid = fork();
	if(pid == -1){
		return(-1);
	}

	if(pid == 0){
		close(fds[1]);
		filter(fds[0]);
	} 
	else {
		
		int i = 2;
		close(fds[0]);
		while(i <= conversion){
			int w = write(fds[1], &i, sizeof(i));
			if(w == -1){
				perror("Fallo el write");
			}
			i = i + 1;
		}
		close(fds[1]);
		wait(NULL);
		exit(0);
	}
	return 0;
}