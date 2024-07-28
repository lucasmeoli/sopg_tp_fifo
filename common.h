#ifndef COMMON_H
#define COMMON_H

/********************** FIFO Configuration **************************/
#define FIFO_NAME               "myfifo"      // FIFO file name
#define FIFO_MODE               (0666)        // FIFO file permissions

/********************** Input Data Configuration ********************/
#define INPUT_DATA_LENGHT       20            // Length of input data buffer

/********************** Header Definitions ***************************/
#define DATA_HEADER             "DATA:"        // Data header string
#define SIGNALS_HEADER          "SIGN:"        // Signals header string
// Length of the headers, data header lenght MUST be equal to signals header
#define HEADER_LENGHT           (sizeof(DATA_HEADER) - 1) 

/********************** Function Declarations ************************/
int create_and_open_FIFO(char *name, int mode, int flags, int *fd_out);

#endif // COMMON_H