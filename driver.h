#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

/* function to open the disk image and prepare for I/O */
extern void driver_attach_disk_image(char *image_file_name, int block_size);
/* function to flush and close the disk image */
void driver_detach_disk_image();
/* function to read a block from the disk image */
void driver_read(void *ptr, uint32_t block_num);
/* function to write a block to the disk image */
void driver_write(void *ptr, uint32_t block_num);

#endif
