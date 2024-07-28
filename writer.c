#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

static int _setup_signal_handler();
static int _create_and_open_FIFO(int *fd);
static int _max(int a, int b);

volatile sig_atomic_t g_signal_called = 0;

void sigint_handler(int sig) {
    if ((SIGUSR1 == sig) || (SIGUSR2 == sig)) {
        g_signal_called = sig;
    }
}

int main(void) {
    pid_t pid = getpid();
    printf("WRITER: My PID is %d\n", pid);  

    if(_setup_signal_handler()){
        exit(EXIT_FAILURE);
    }

    int fd_fifo; 
    if (_create_and_open_FIFO(&fd_fifo)) {
        exit(EXIT_FAILURE);
    }

    char buf[INPUT_DATA_LENGHT];
    char data_buf[(_max(strlen(c_data_header), strlen(c_signals_header)) + INPUT_DATA_LENGHT)];
    ssize_t bytes_written;

    //TODO(lmeoli): ver bien que pasa con write cuando se cierra del otro lado
    while (1) {
        char * p_error = fgets(buf, INPUT_DATA_LENGHT, stdin);
        if (p_error == NULL) {
            if (feof(stdin)) {
                break;
            } else if (!g_signal_called) {
                printf("Error: fgets");
                close(fd_fifo);
                exit(EXIT_FAILURE);
            }
        }

        if (g_signal_called) {
            int usr_signal = g_signal_called;
            g_signal_called = 0;

            strcpy(data_buf, c_signals_header);
            if (SIGUSR1 == usr_signal) {
                strcat(data_buf, "1\n");
            } else if (SIGUSR2 == usr_signal) {
                strcat(data_buf, "2\n");
            }   
        } else {
            // TODO:chequear valores
            strcpy(data_buf, c_data_header);
            strcat(data_buf, buf);
            printf("Palabra recibida: %s", data_buf);
        }

        //sigpipe y termina el programa
        bytes_written = write(fd_fifo, data_buf, strlen(data_buf));
        if (bytes_written == -1) {
            perror("FIFO write");
            exit(EXIT_FAILURE);
        } else if(bytes_written == 0) {
            printf("EOF");
            break;
        }
    }

    close(fd_fifo);

    return 0;
}


static int _setup_signal_handler() {
    struct sigaction s_usr_sa;
    s_usr_sa.sa_handler = sigint_handler;
    s_usr_sa.sa_flags = 0; // SA_RESTART;
    sigemptyset(&s_usr_sa.sa_mask);

    if (sigaction(SIGUSR1, &s_usr_sa, NULL) == -1) {
        perror("Sigaction:");
        return 1;
    }

    if (sigaction(SIGUSR2, &s_usr_sa, NULL) == -1) {
        perror("Sigaction:");
        return 1;
    }

    return 0;
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

    *fd = open(FIFO_NAME, O_WRONLY);
    if (*fd == -1) {
        perror("FIFO open:");
        return 1;
    }

    return 0;
}

static int _max(int a, int b) {
    return (a > b) ? a : b;
}