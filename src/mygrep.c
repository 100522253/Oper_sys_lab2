#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


int main(int argc, char ** argv) {
    if (argc != 3) { // Check the correct number of inputs
        printf("Usage: %s <ruta_fichero> <cadena_busqueda>\n", argv[0]);
        return -1;
    }

    // Open the file
    int fd = open(argv[1], O_RDWR);
    if (fd == -1){
        // Check if the file was opended correctly
        perror("Error opening input script");
        return -1;
    }

    size_t byte_read;
    char char_read = 0;     // Char read by the read syscall
    int line_count = 0;     // Counter of lines
    int input_idx = 0;      // Index of the input string searched
    char string_found = 0;  // Flag whether for the seeked string found

    while ((byte_read = read(fd, &char_read, sizeof(char))) > 0){
        if (char_read == '\n'){ // New line -> reset counters
            line_count++;
            input_idx = 0;
        } else if (char_read == argv[2][input_idx]){ // Read char is in seeked string
            input_idx++;
        } else {
            input_idx = 0; // Char not in seeked string
        }

        if (input_idx >= strlen(argv[2])){
            string_found = 1;
            char line_char = (char)line_count;
            if(write(STDOUT_FILENO, &line_char, sizeof(line_char)) < 0){
                perror("Error writing in the standart output");
                return -1;
            };
            input_idx = 0;
            //printf("%d\n", line_count);
        }
    }
    // Check write -1


    if (close(fd) < 0){
        perror("Error closing the input file");
        return -1;
    }

    if (!string_found){ // String not found so message displayed
        printf("\"%s\" not found\n", argv[2]);
    }

    return 0;
}