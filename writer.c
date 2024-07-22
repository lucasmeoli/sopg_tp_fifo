#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

const char data_header[] = "DATA:";

int main(void) {
    pid_t pid = getpid();
    
    printf("WRITER: My PID is %d\n", pid);

    if (access(FIFO_NAME, F_OK) == -1) {
        // FIFO does not exist, create it
        int fifo_status = mkfifo(FIFO_NAME, FIFO_MODE);
        if (fifo_status == -1) {
            perror("FIFO creation error");
            return 1;
        } else {
            printf("FIFO created successfully.\n");
        }
    } else {
        // FIFO already exists
        printf("Not creating FIFO, it already exists.\n");
    }


    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("FIFO: open error");
        return 1;
    }

    char buf[INPUT_DATA_LENGHT];
    //char data[strlen(data_header) + INPUT_DATA_LENGHT];

    while (1) {
        char * p_error = fgets(buf, INPUT_DATA_LENGHT, stdin);
        if (p_error == NULL) {
            if (feof(stdin)) {
                printf("END OF FILE");
                break;
            }
            printf("Error: fgets");
            return -1;
        }

        char data[strlen(data_header) + strlen(buf) + 1];
        /* chequear valores*/
        // Copy DATA:
        strcpy(data, data_header);
        // Concatenar el contenido de buf después de "DATA:"
        strcat(data, buf);
        int num;

        if ((num = write(fd, data, strlen(data))) == -1) {
            perror("Error writing");
            close(fd);
            return 1;
        } else {
            printf("Escritura correcta: %s", data);
        }
    }

    close(fd);

    return 0;
}

