#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"



#define OUTPUT_DATA_FILE    "log.txt"
#define OUTPUT_DATA_MODE    0666

int main(void) {

    pid_t pid = getpid();
    printf("Reader: My PID is %d\n", pid);

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

    int fd_fifo = open(FIFO_NAME, O_RDONLY);
    if (fd_fifo == -1) {
        perror("FIFO: open error");
        return 1;
    }

    int fd_out_data = open(OUTPUT_DATA_FILE, (O_CREAT | O_WRONLY), OUTPUT_DATA_MODE);
    if (fd_out_data == -1) {
        perror("Output data: open error");
        return 1;
    }

    char buf[INPUT_DATA_LENGHT + 5];
    ssize_t bytes_read;
    ssize_t bytes_written;

    while ((bytes_read = read(fd_fifo, buf, INPUT_DATA_LENGHT+5-1)) > 0) {
        // TODO: Es realmente necesario agregar el /0?
        buf[bytes_read] = '\0';
        printf("Palabra leida: %s, bytes_Read: %ld\n", buf, bytes_read);
        if ((bytes_written = write(fd_out_data, buf+5, bytes_read-5)) == -1) {
            perror("Error writing");
            close(fd_out_data);
            return 1;
        } else {
            printf("reader: write %ld bytes: \"%s\"\n", bytes_read, buf);
        }
    }

    if (bytes_read == -1) {
        perror("Read error");
    }

    close(fd_fifo);
    close(fd_out_data);

    return 0;
}