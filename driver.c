
#include "driver.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int fd;
int blksize;

/* function to open the disk image and prepare for I/O */
void driver_attach_disk_image(char *image_file_name, int block_size)
{
  blksize = block_size;
  fd = open(image_file_name,O_RDWR);
  if(fd < 0)
    {
      perror(image_file_name);
      exit(1);
    }
}

/* function to flush and close the disk image */
void driver_detach_disk_image()
{
  close(fd);
}


/* function to read a block from the disk image */
void driver_read(void *ptr, uint32_t block_num)
{
  if(lseek(fd, (1+block_num) * blksize, SEEK_SET) == -1)
    {
      perror("Driver failed to read block");
      exit(1);
    }
  if(read(fd,ptr,blksize) != blksize)
    {
      perror("Driver failed to read block");
      exit(1);
    }
}


/* function to write a block to the disk image */
void driver_write(void *ptr, uint32_t block_num)
{
  if(lseek(fd, (1+block_num) * blksize, SEEK_SET) == -1)
    {
      perror("Driver failed to read block");
      exit(1);
    }
  if(write(fd,ptr,blksize) != blksize)
    {
      perror("Driver failed to read block");
      exit(1);
    }
}


