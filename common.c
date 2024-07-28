/********************** inclusions *******************************************/
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

/********************** Function Implementation **************************/
int create_and_open_FIFO(char *name, int mode, int flags, int *fd_out) {
    // Check if the FIFO exists
    if (access(name, F_OK) == -1) {
        // If FIFO does not exist, create it
        int fifo_status = mkfifo(name, mode);
        if (fifo_status == -1) {
            perror("FIFO creation:");
            return 1;
        }
    } 

    // Open the FIFO for reading/writing
    *fd_out = open(name, flags);
    if (*fd_out == -1) {
        perror("FIFO open:");
        return 1;
    }

    return 0;
}