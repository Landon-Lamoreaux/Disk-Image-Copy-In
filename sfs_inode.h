#ifndef INODE_H
#define INODE_H

/* Every file has an inode, which contains the permissions, owner,
   size, pointers to the data, etc */

#include <cstdint>

/* The BLOCK_SIZE is specified in the superblock.  The superblock data
   structure must fit within a block, so the minimim block size is set
   to 128.  The default filesystem block size is the natural block
   size of the device that the filesystem is on, or 128, whichever is
   greater.  For testing, we use a disk image file with 128 byte
   blocks. This small block size allows us to test all of the code
   with relatively small files.

   We want the size of the inode structure to be a power of 2, so it
   fits nicely in a disk block.  We can adjust the number of direct
   blocks in order to adjust the size of the structure. */

#define NUM_DIRECT 5

/* 
  For a filesystem with 128 byte blocks: 
   - Five direct blocks will handle files up to 128*5 = 640 bytes.
   - Five direct and one indirect block will handle files up to 128*5
     + 128*128/4 = 4736 bytes.
   - Five direct, one indirect, and one double indirect will cover
     files up to 128*5 + 128*128/4 + 128*128/4*128/4 = 135808 bytes.
   - Five direct, one indirect, one double indirect, and one triple
     indirect will cover files up to 128*5 + 128*128/4 +
     128*128/4*128/4 + 128*128/4*128/4*128/4 = 4330112 bytes.

   For a filesystem with 256 byte blocks:
   - Five direct blocks will handle files up to 256*5 = 1280 bytes.
   - Five direct and one indirect block will handle files up to 256*5
     + 256*256/4 = 17644 bytes.
   - Five direct, one indirect, and one double indirect will cover
     files up to 256*5 + 256*256/4 + 256*256/4*256/4 = 1066220 bytes.
   - Five direct, one indirect, one double indirect, and one triple
     indirect will cover files up to 256*5 + 256*256/4 +
     256*256/4*256/4 + 256*256/4*256/4*256/4 = 68,175,084 bytes.

   A filesystem with a 4K block size can have files up to 4096*5 +
   4096*4096/4 + 4096*4096/4*4096/4 + 4096*4096/4*4096/4*4096/4 = 
   4,402,345,693,184 bytes in length.  That should be enough for
   any sane person.
*/

/* defines for different types of files */
#define FT_NORMAL 0
#define FT_DIR 1
#define FT_CHAR_SPEC 2
#define FT_BLOCK_SPEC 3
#define FT_PIPE 4
#define FT_SOCKET 5
#define FT_SYMLINK 6

/* this is the structure of an inode */
typedef struct sfs_inode{
  uint32_t owner;    /* UID of owner */
  uint32_t group;    /* GID of group */
  uint32_t ctime;    /* time that the file was created */
  uint32_t mtime;    /* time that the file was last modified */
  uint32_t atime;    /* time that the file was last accessed */
  uint16_t perm;     /* file permissions */
  uint8_t  type;     /* one of the FT_ values defined above for
			different types of files.  If type is
			FT_CHAR_SPEC or FT_BLOCK_SPEC, then the major
			driver number is stored in direct[0] and the
			minor driver number is stored in direct[1] */
  uint8_t refcount;  /* number of hard links to this file, limit is 255 */
  uint64_t size;     /* since size is an unsigned 64-bit int, it
		        limits us to enormous files (2^64 bytes). The
		        block size is also a limiting factor, as
		        discussed above. */
  /* block numbers for direct blocks */
  uint32_t direct[NUM_DIRECT]; /* with 5 direct, an inode is 64 bytes */
  /* block number for indirect block */
  uint32_t indirect;
  /* block number for double indirect block */
  uint32_t dindirect;
  /* block number for triple indirect block */
  uint32_t tindirect;
}sfs_inode_t;


/* Max file sizes

block size  |  triple   double  single direct
128 byte    |  4M +     128K +  4K +    640b
256 byte    |  64M +    1M +    16K +   1280b
1K          |  16G +    64M +   256K +  5K
4K          |  4T +     4G +    4M +    20K
8K          |  64T +    32G +   16M +   40K
16K         |  256T +   256G +  64M +   80K
*/ 

#endif
