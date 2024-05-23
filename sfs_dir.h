#ifndef SFS_DIR_H
#define SFS_DIR_H

/* We define a very simple directory structure.  Each directory entry
   consists of a null-terminated string of up to 28 characters,
   followed by an inode number.
*/

#include <cstdint>

#define SFS_NAME_MAX (32-sizeof(uint32_t))

typedef struct {
  char name[SFS_NAME_MAX];
  uint32_t inode;
}sfs_dirent;



#endif
