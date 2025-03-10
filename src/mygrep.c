#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 2048

void close_file(int file){
    if(close(file) < 0){
        perror("Error closing the file");
        exit(-1);   
    }
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        printf("Usage: %s <ruta_fichero> <cadena_busqueda>\n", argv[0]);
        return -1;
    }

    // Open the file
    int file = open(argv[1]);
    if (file == -1){
        perror("Error opening input script");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    int bytes_read, line_index = 0;

    while ((bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || line_index >= BUFFER_SIZE - 1) {
                line[line_index] = '\0';  // Null-terminate the string
                if (strstr(line, argv[2])) {  // Check if the line contains the search term
                    write(STDOUT_FILENO, line, strlen(line));
                    write(STDOUT_FILENO, "\n", 1);
                }
                line_index = 0;  // Reset for the next line
            } else {
                line[line_index++] = buffer[i];
            }
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        close(file);
        exit(EXIT_FAILURE);
    }

    close(file);