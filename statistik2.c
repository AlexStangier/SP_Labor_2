#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "readbuf.h"

int main() {
    unsigned char checksum;
    int offset = 0;

    //persist time
    struct timeval startTime, endTime;

    //read file
    short fd;
    fd = open("lab.reference", O_RDONLY);

    if (fd == -1) {
        //handle read error
        perror("failed to open file \n");
        return EXIT_FAILURE;
    } else {
        printf("__________________________________________________________________________\n");
        printf("Blocksize\t \t   #Reads\t   Time-Elapsed\t         Checksum\t \n");
        printf("__________________________________________________________________________\n");

        int i;
        for (i = 0; i <= 20; i++) {
            checksum = 0;
            //capture start time
            gettimeofday(&startTime, NULL);

            int base = 2, power = i, blockSize = 0, timeElapsed = 0;
            int reads = 0;
            ssize_t readCount = 0;
            int j;

            //calc current blocksize 2^i
            blockSize = (int) pow(base, power);

            //init buffer array
            char buffer[blockSize];
            while ((readCount = read_buffered(fd, buffer, blockSize)) > 0) {
                reads++;
                for (j = 0; j < readCount; j++) {
                    checksum += buffer[j];
                }
            }

            //capture end time
            gettimeofday(&endTime, NULL);

            //calc runtime
            timeElapsed = ((endTime.tv_sec - startTime.tv_sec) * 1000000) + ((endTime.tv_usec - startTime.tv_usec));

            printf(" %8d\t \t %8d\t  %8d ms\t\t       %d\t \n", blockSize, reads, timeElapsed, checksum);

            if ((offset = lseek(fd, 0, SEEK_SET)) == -1) {
                perror("lseek failed to reposition the pointer");
                return EXIT_FAILURE;
            }
        }
        printf("__________________________________________________________________________\n");
    }
    return EXIT_SUCCESS;
}
