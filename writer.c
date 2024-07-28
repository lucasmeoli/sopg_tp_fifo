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

#define STR_OUTPUT_SIGNAL_SIGUSR1       "1\n"
#define STR_OUTPUT_SINGAL_SIGUSR2       "2\n"

static int _setup_signal_handler();
static int _create_and_open_FIFO(int *fd);
static int _prepare_data_buffer(char *input_buf, char *header, char *output_buf);

volatile sig_atomic_t g_signal_called = 0;

void sigint_handler(int sig) {
    if ((SIGUSR1 == sig) || (SIGUSR2 == sig)) {
        g_signal_called = sig;
    }
}

int main(void) {
    pid_t pid = getpid();
    printf("WRITER: My PID is %d\n", pid);

    if (strlen(DATA_HEADER) != HEADER_LENGHT || strlen(SIGNALS_HEADER) != HEADER_LENGHT) {
        printf("ERROR: header lenght\n");
        exit(EXIT_FAILURE);
    }

    if(_setup_signal_handler()){
        exit(EXIT_FAILURE);
    }

    int fd_fifo; 
    if (_create_and_open_FIFO(&fd_fifo)) {
        exit(EXIT_FAILURE);
    }

    char input_buf[INPUT_DATA_LENGHT];
    char output_buf[HEADER_LENGHT + INPUT_DATA_LENGHT];
    char *p_output_header;

    while (1) {
        char * p_str_error = fgets(input_buf, INPUT_DATA_LENGHT, stdin);
        if (p_str_error == NULL) {
            if (feof(stdin)) {
                break;
            } else if (!g_signal_called) {
                printf("NULL Error: fgets");
                close(fd_fifo);
                exit(EXIT_FAILURE);
            }
        }

        if (g_signal_called) {
            int usr_signal = g_signal_called;
            g_signal_called = 0;

            p_output_header = SIGNALS_HEADER;

            char * p_str_signal_out;
            if (SIGUSR1 == usr_signal) {
                p_str_signal_out = STR_OUTPUT_SIGNAL_SIGUSR1;
            } else if (SIGUSR2 == usr_signal) {
                p_str_signal_out = STR_OUTPUT_SINGAL_SIGUSR2;
            }

            if (NULL == strcpy(input_buf, p_str_signal_out)) {
                close(fd_fifo);
                exit(EXIT_FAILURE);
            }
        } else {
            p_output_header = DATA_HEADER;
        }

        if (_prepare_data_buffer(input_buf, p_output_header, output_buf)) {
            close(fd_fifo);
            exit(EXIT_FAILURE);
        } 

        ssize_t bytes_written = write(fd_fifo, output_buf, strlen(output_buf));
        if (bytes_written == -1) {
            perror("FIFO write");
            close(fd_fifo);
            exit(EXIT_FAILURE);
        } else if(bytes_written == 0) {
            printf("EOF");
            break;
        }
    }

    close(fd_fifo);

    exit(EXIT_SUCCESS);
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

static int _prepare_data_buffer(char *input_buf, char *header, char *output_buf) {
    char *p_error = strcpy(output_buf, header);
    if (p_error == NULL) {
        printf("NULL Error: strcpy");
        return 1;
    }

    p_error = strcat(output_buf, input_buf);
    if (p_error == NULL) {
        printf("NULL Error: strcat");
        return 1;
    }

    return 0;
}