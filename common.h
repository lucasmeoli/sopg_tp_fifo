#ifndef COMMON_H
#define COMMON_H

/* FIFO description */
#define FIFO_NAME               "myfifo"
#define FIFO_MODE               (0666) 

#define INPUT_DATA_LENGHT       10

/*Both header must be the same size */
#define DATA_HEADER             "DATA:"
#define SIGNALS_HEADER          "SIGN:"
#define HEADER_LENGHT           (sizeof(DATA_HEADER) - 1)

int max(int a, int b);

#endif // COMMON_H





