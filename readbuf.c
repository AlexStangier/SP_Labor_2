#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "readbuf.h"
#include <unistd.h>

#define BUFFER_SIZE 5000

ssize_t read_buffered(int fd, void *buf, size_t nbytes) {
    //contain info about buffer and read write pos
    static char sysbuf[BUFFER_SIZE];        // Systembuffer 5000 Byte length
    static int bytes_read_total = 0;        // read bytes
    static int bytes_left_stored = 0;        // remaining bytes in buffer

    //contain info during each run
    int bytes_read = 0;                // bytes read during this run
    int bytes_copied = 0;            // bytes copied during this run
    int missing_nbytes = 0;
    int seek = 0;

    while (bytes_copied < nbytes) {
        //CASE 1 sysbuf is empty <- first read
        if (bytes_left_stored == 0) {
            if ((bytes_read = read(fd, sysbuf, BUFFER_SIZE)) > 0) {
                bytes_left_stored = bytes_read;
                bytes_read_total = 0;
            }
                //Handle failed reads
            else {
                if (bytes_read == 0) {
                    return bytes_read;
                }
                if (bytes_read == -1) {
                    return EXIT_FAILURE;
                }
            }
        } else {
            //CASE 2 btr is bigger than buff size <- bytes to read exceeds buffer
            if (nbytes >= BUFFER_SIZE) {
                //CASE 2.1 Read missing bytes to satisfy btr
                if ((bytes_copied + bytes_left_stored) > nbytes) {
                    missing_nbytes = nbytes - bytes_copied;
                    memcpy(buf + bytes_copied, sysbuf, missing_nbytes);
                    bytes_copied += missing_nbytes;
                    bytes_read_total += missing_nbytes;

                    //Adjust file offset
                    if ((seek = lseek(fd, -(bytes_left_stored - missing_nbytes), SEEK_CUR)) < 0) {
                        perror("Error while resetting the Read Pointer\n");
                    }

                    bytes_left_stored = 0;
                    return bytes_copied;
                }

                //CASE 2.2 reached EOF
                if (bytes_left_stored < BUFFER_SIZE) {
                    memcpy(buf + bytes_copied, sysbuf, bytes_left_stored);
                    bytes_copied += bytes_left_stored;
                    bytes_read_total += bytes_left_stored;
                    bytes_left_stored = 0;

                    return bytes_copied;
                }
                memcpy(buf + bytes_copied, sysbuf, BUFFER_SIZE);
                bytes_copied += BUFFER_SIZE;
                bytes_read_total += BUFFER_SIZE;
                bytes_left_stored -= BUFFER_SIZE;
                //CASE 2 btr is smaller than buff size
            } else {
                //CASE 3.1 bytes left cant satisfy btr <- buffer too small
                if (bytes_left_stored < nbytes) {
                    memcpy(buf + bytes_copied, sysbuf + bytes_read_total, bytes_left_stored);
                    bytes_copied += bytes_left_stored;
                    bytes_read_total += bytes_left_stored;
                    bytes_left_stored = 0;

                    return bytes_copied;
                }
                //CASE 3.2 normal read
                memcpy(buf + bytes_copied, sysbuf + bytes_read_total, nbytes);
                bytes_copied += nbytes;
                bytes_read_total += nbytes;
                bytes_left_stored -= nbytes;
            }

        }
    }
    return bytes_copied;
}
