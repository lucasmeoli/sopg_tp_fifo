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

static int _create_and_open_FIFO(int *fd);
static void _close_files(int fifo, int out_data, int out_signals);

int main(void) {
    pid_t pid = getpid();
    printf("Reader: My PID is %d\n", pid);

    if (strlen(DATA_HEADER) != HEADER_LENGHT || strlen(SIGNALS_HEADER) != HEADER_LENGHT) {
        printf("ERROR: header lenght\n");
        exit(EXIT_FAILURE);
    }

    int fd_fifo; 
    if (_create_and_open_FIFO(&fd_fifo)) {
        exit(EXIT_FAILURE);
    }

    int fd_out_data = open(OUTPUT_DATA_FILE, (O_CREAT | O_WRONLY), OUTPUT_DATA_MODE);
    if (fd_out_data == -1) {
        perror("Output open:");
        _close_files(fd_fifo, -1, -1);
        exit(EXIT_FAILURE);
    }

    int fd_out_signal = open(OUTPUT_SIGNAL_FILE, (O_CREAT | O_WRONLY), OUTPUT_SIGNAL_MODE);
    if (fd_out_signal == -1) {
        perror("Signal open:");
        _close_files(fd_fifo, fd_out_data, -1);
        exit(EXIT_FAILURE);
    }

    char input_header[HEADER_LENGHT+1];
    int input_buf_lenght = HEADER_LENGHT + INPUT_DATA_LENGHT;
    char input_buf[input_buf_lenght];
    int fd_output;
    char * p_output_buf;
    int output_buf_lenght;
    ssize_t bytes_read;
    ssize_t bytes_written;

    while (1) {
        bytes_read = read(fd_fifo, input_buf, input_buf_lenght-1);
        if (bytes_read == -1) {
            perror("FIFO read:");
            _close_files(fd_fifo, fd_out_data, fd_out_signal);
            exit(EXIT_FAILURE);
        } else if(bytes_read == 0) {
            break;
        }
        input_buf[bytes_read] = '\0';

        
        if (NULL == strncpy(input_header, input_buf, HEADER_LENGHT)) {
            printf("ERROR: header copy");
            _close_files(fd_fifo, fd_out_data, fd_out_signal);
            exit(EXIT_FAILURE);
        }
        input_header[HEADER_LENGHT] = '\0';

        if (strcmp(input_header,DATA_HEADER) == 0){
            fd_output = fd_out_data;
        } else if(strcmp(input_header,SIGNALS_HEADER) == 0) {
            fd_output = fd_out_signal;
        } else {
            printf("ERROR: invalid header");
            _close_files(fd_fifo, fd_out_data, fd_out_signal);
            exit(EXIT_FAILURE);
        }

        p_output_buf = input_buf + HEADER_LENGHT;
        output_buf_lenght = bytes_read - HEADER_LENGHT;

        bytes_written = write(fd_output, p_output_buf, output_buf_lenght);
        if (bytes_written == -1) {
            perror("Write:");
            _close_files(fd_fifo, fd_out_data, fd_out_signal);
            exit(EXIT_FAILURE);
        }
    }

    _close_files(fd_fifo, fd_out_data, fd_out_signal);
    exit(EXIT_SUCCESS);
}


static int _create_and_open_FIFO(int *fd) {
    if (access(FIFO_NAME, F_OK) == -1) {
        // If FIFO does not exist, create it
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

static void _close_files(int fd_fifo, int fd_out_data, int fd_out_signals) {
    if (fd_fifo >= 0) {
        close(fd_fifo);
    }

    if (fd_out_data >= 0){
        close(fd_out_data);
    }

    if (fd_out_signals >= 0){
        close(fd_out_signals);
    }
}