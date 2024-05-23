
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>

/* To find the superblock, start at the beginning of the disk and read
   sectors until you find the magic number and the type string in
   thefsmagic and fstypstr fields */
#define VMLARIX_SFS_MAGIC 0x00112233
#define VMLARIX_SFS_TYPESTR "vmlarix_sfs"

typedef struct {
  uint32_t fsmagic;         /* filesystem type             */
  char fstypestr[32];
  uint32_t block_size;      /* size of a block, in bytes   */
  uint32_t sectorsperblock; /* size of a block, in sectors */
  uint32_t superblock;      /* location of the superblock */
  uint32_t num_blocks;      /* total number of blocks in the filesystem */
  uint32_t fb_bitmap;       /* first block of the free block bitmap */
  uint32_t fb_bitmapblocks; /* number of blocks in free block bitmap*/
  uint32_t blocks_free;     /* number of blocks that are unused */
  uint32_t num_inodes;      /* total number of inodes */
  uint32_t fi_bitmap;       /* first block of the free inode bitmap */
  uint32_t fi_bitmapblocks; /* number of blocks in free inode bitmap*/
  uint32_t inodes_free;     /* number of unused inodes */
  uint32_t num_inode_blocks;/* number of blocks in the inode table */
  uint32_t inodes;          /* first block of the inode table */
  uint32_t rootdir;         /* first block of the root directory */
  uint32_t open_count;      /* number of open files on this filesystem */
}sfs_superblock;

#endif
