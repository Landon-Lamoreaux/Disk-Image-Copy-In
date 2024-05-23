This program will copy a specified file into the provided disk image. The first argument to this program should be the disk image, the second should be the name of the file you want to copy into the disk image. You can view what files are in the disk images by running my Disk Image List program on that disk image. This program will only work with 128 byte blocks of data in the disk image.
When copying the file into the disk image this program updates the super block and the bitmaps, and creates a new inode for the new file in the disk image.

To run this program use the provided CMakeLists.txt file to build the project, then run it as any other executable with the required arguments.

Example/testing disk images have been included in the Disk Images folder.
