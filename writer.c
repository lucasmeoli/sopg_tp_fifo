/********************** inclusions *******************************************/
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

/********************** macros and definitions *******************************/
#define STR_OUTPUT_SIGNAL_SIGUSR1       "1\n"
#define STR_OUTPUT_SINGAL_SIGUSR2       "2\n"

/********************** internal data definition *****************************/
volatile sig_atomic_t g_signal_called = 0; // indicates which signal has been received.

/********************** internal functions declaration ************************/
static int _setup_signal_handler();
static int _prepare_data_buffer(char *input_buf, char *header, char *output_buf);

/********************** external functions definition ************************/
void sigint_handler(int sig) {
    if ((SIGUSR1 == sig) || (SIGUSR2 == sig)) {
        g_signal_called = sig;
    }
}

int main(void) {
    pid_t pid = getpid();
    printf("WRITER: My PID is %d\n", pid);

    // Verify that the lengths of the data and signals headers match the expected length.
    if (strlen(DATA_HEADER) != HEADER_LENGHT || strlen(SIGNALS_HEADER) != HEADER_LENGHT) {
        printf("ERROR: header lenght\n");
        exit(EXIT_FAILURE);
    }

    // Set up signal handlers for SIGUSR1 and SIGUSR2.
    if(_setup_signal_handler()){
        exit(EXIT_FAILURE);
    }

    // Create and open the FIFO for writing.
    int fd_fifo; 
    if (create_and_open_FIFO(FIFO_NAME, FIFO_MODE, O_WRONLY, &fd_fifo)) {
        exit(EXIT_FAILURE);
    }

    char input_buf[INPUT_DATA_LENGHT];
    char output_buf[HEADER_LENGHT + INPUT_DATA_LENGHT];
    char *p_output_header;

    while (1) {
        // Read the input from stdin.
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

        // Determine if stdin input data or a signal was received.
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

        // Prepare the output buffer by concatenating the input buffer with the header.
        if (_prepare_data_buffer(input_buf, p_output_header, output_buf)) {
            close(fd_fifo);
            exit(EXIT_FAILURE);
        } 

        // Write the processed output data to the FIFO.
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

/********************** internal functions definition ***********************/

/*
 * Sets up the signal handlers for SIGUSR1 and SIGUSR2 signals.
 *
 * This function initializes the sigaction structure and registers the
 * signal handler for both SIGUSR1 and SIGUSR2. It returns 0 on success
 * and 1 if an error occurs during the registration of the signal handlers.
 *
 * @params 
 *   None
 * @return 
 *   0 if successful, 1 if an error occurred.
*/
static int _setup_signal_handler() {
    struct sigaction s_usr_sa;
    s_usr_sa.sa_handler = sigint_handler;
    s_usr_sa.sa_flags = 0;
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

/*
 * Prepares the output buffer by concatenating the header and input buffer.
 *
 * This function copies the specified header into the output buffer and
 * then appends the input buffer to the output buffer. It returns 0 on
 * success and 1 if an error occurs during the copying or concatenation
 * process.
 *
 * @params 
 *   input_buf - The input data buffer to be appended to the header.
 *   header - The header to be copied into the output buffer.
 *   output_buf - The buffer where the header and input buffer will be stored.
 * @return 
 *   0 if successful, 1 if an error occurred.
*/
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