#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define NARGS 4  // Número de argumentos por paquete
#ifndef NARGS
#endif
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <comando>\n", argv[0]);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char *command = argv[1];
    char *args[NARGS + 2];  // NARGS + 1 argumentos + NULL

    int arg_count = 0;
    args[arg_count++] = command;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        // Eliminar el caracter de nueva línea
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        args[arg_count++] = strdup(line); // Guardar una copia del argumento
        if (arg_count >= NARGS + 1) {
            args[arg_count] = NULL;
            pid_t pid = fork();
            if (pid == -1) {
                perror("Error al crear el proceso hijo");
                return 1;
            } else if (pid == 0) {
                // Proceso hijo
                execvp(command, args);
                // Solo llega aquí si execvp falla
                perror("Error en execvp");
                return 1;
            } else {
                // Proceso padre
                int status;
                waitpid(pid, &status, 0);

                // Liberar la memoria de los argumentos
                for (int i = 1; i < arg_count; i++) {
                  //  free(args[i]);
                }
                arg_count = 1; // Reiniciar el contador de argumentos
            }
        }
    }

    // Ejecutar el comando para los argumentos restantes
    if (arg_count > 1) {
        args[arg_count] = NULL;

        // Ejecutar el comando
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error al crear el proceso hijo");
            return 1;
        } else if (pid == 0) {
            // Proceso hijo
            execvp(command, args);
            // Solo llega aquí si execvp falla
            perror("Error en execvp");
            return 1;
        } else {
            // Proceso padre
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}