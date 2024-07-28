#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

#define OUTPUT_DATA_FILE        "log.txt"
#define OUTPUT_DATA_MODE        0666

#define OUTPUT_SIGNAL_FILE      "signals.txt"
#define OUTPUT_SIGNAL_MODE      0666

//TODO: esto deberia algo comun donde se le pase el nombre y el modo
static int _create_and_open_FIFO(int *fd);


int main(void) {
    pid_t pid = getpid();
    printf("Reader: My PID is %d\n", pid);

    int fd_fifo; 
    if (_create_and_open_FIFO(&fd_fifo)) {
        exit(EXIT_FAILURE);
    }

    int fd_out_data = open(OUTPUT_DATA_FILE, (O_CREAT | O_WRONLY), OUTPUT_DATA_MODE);
    if (fd_out_data == -1) {
        perror("Output open:");
        close(fd_fifo);
        exit(EXIT_FAILURE);
    }

    int fd_out_signal = open(OUTPUT_SIGNAL_FILE, (O_CREAT | O_WRONLY), OUTPUT_SIGNAL_MODE);
    if (fd_out_signal == -1) {
        perror("Signal open:");
        close(fd_fifo);
        close(fd_out_data);
        exit(EXIT_FAILURE);
    }

    char buf[INPUT_DATA_LENGHT + 5];
    ssize_t bytes_read;
    ssize_t bytes_written;

    while ((bytes_read = read(fd_fifo, buf, INPUT_DATA_LENGHT+5-1)) > 0) {
        // TODO: Es realmente necesario agregar el /0?
        buf[bytes_read] = '\0';
        if (buf[0] == 'D'){
            if ((bytes_written = write(fd_out_data, buf+5, bytes_read-5)) == -1) {
                perror("Error writing");
                close(fd_fifo);
                close(fd_out_data);
                close(fd_out_signal);
                exit(EXIT_FAILURE);
            }
        } else if(buf[0] == 'S') {
            if ((bytes_written = write(fd_out_signal, buf+5, bytes_read-5)) == -1) {
                perror("Error writing");
                close(fd_fifo);
                close(fd_out_data);
                close(fd_out_signal);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (bytes_read == -1) {
        perror("Read error");
    }

    close(fd_fifo);
    close(fd_out_data);
    close(fd_out_signal);

    exit(EXIT_SUCCESS);
}


static int _create_and_open_FIFO(int *fd) {
    if (access(FIFO_NAME, F_OK) == -1) {
        // FIFO does not exist, create it
        int fifo_status = mkfifo(FIFO_NAME, FIFO_MODE);
        if (fifo_status == -1) {
            perror("FIFO creation:");
            return 1;
        }
    } 

    *fd = open(FIFO_NAME, O_RDONLY);
    if (*fd == -1) {
        perror("FIFO open:");
        return 1;
    }

    return 0;
}