#include <iostream>
#include <fstream>
#include <cstring>
#include "sfs_dir.h"
#include "sfs_inode.h"
#include "sfs_superblock.h"
#include <sys/stat.h>

extern "C" {
#include "driver.h"
#include "bitmap.h"
}

using namespace std;
void writeFileBlock(sfs_inode_t * node, size_t blockNumber, void* data);
int getFreeSpot(sfs_superblock *super, uint8_t pos, int numBlocks);
int allocateBlock(sfs_inode *node, sfs_superblock *super, int blockNum);
int allocateIndirect(sfs_inode *node, sfs_superblock *super, int blockNum);
int allocateDouble(sfs_inode *node, sfs_superblock *super, int dindirectNum);
int allocateTriple(sfs_inode *node, sfs_superblock *super);
void GetFileBlock(sfs_inode_t * node, size_t blockNumber, void* data);

int main(int argv, char** argc) {
    int i = 0;
    ifstream fin;
    struct stat fileInfo;
    int superBlockPos = 0;

    if (argv != 3) {
        cout << "Incorrect number of arguments." << endl;
        return -1;
    }

    fin.open(argc[2]);

    if(!fin.is_open())
    {
        cout << "Error: Could not open file." << endl;
        return -1;
    }

    if(stat(argc[2], &fileInfo) == -1)
    {
        cout << "Error obtaining file information." << endl;
        return -1;
    }

    // Declare a buffer that is the same size as a filesystem block.
    char raw_superblock[128];

    sfs_superblock *super = (sfs_superblock *) raw_superblock;
    sfs_inode inode[128/sizeof(sfs_inode)];

    string diskImage = argc[1];
    char* disk = const_cast<char *>(diskImage.c_str());

    /* open the disk image and get it ready to read/write blocks */
    driver_attach_disk_image(disk, 128);

    // Finding the super block.
    while(true) {

        // Read a block from the disk image.
        driver_read(super,i);

        // Is it the filesystem superblock?
        if(super->fsmagic == VMLARIX_SFS_MAGIC && !strcmp(super->fstypestr,VMLARIX_SFS_TYPESTR))
        {
            superBlockPos = i;
            break;
        }
        i++;
    }

    sfs_dirent data[super->block_size/sizeof(sfs_dirent)];

    driver_read(inode, super->inodes);

    // Finding the number of new blocks needed.
    int newBlocks = (int)fileInfo.st_size / (int)super->block_size;
    newBlocks += (int)fileInfo.st_size % super->block_size == 0 ? 0 : 1;

    char blockBitmap[super->block_size];
    char inodeBitChar[super->block_size];

    auto* inodeBitmap = (bitmap_t *) inodeBitChar;


    int inodebit = getFreeSpot(super, super->fi_bitmap, (int)super->fi_bitmapblocks);
    super->inodes_free = super->inodes_free - 1;
    driver_write(super, superBlockPos);

    sfs_inode freeInode[super->block_size/sizeof(sfs_inode)];
    sfs_dirent theFile[super->block_size/sizeof(sfs_dirent)];

    driver_read(freeInode, inodebit / 2 + super->inodes);
    int fileLoc = ((int)inode[0].size / 32) % 4;

    if(inode[0].size % 128 == 0)
    {
        int empty[32] = {0};
        int block = allocateBlock(inode, super, (int)inode[0].size / 128);
        driver_write(empty, block);
        driver_read(theFile, block);
        strcpy(theFile[fileLoc].name, argc[2]);
        theFile[fileLoc].inode = inodebit;
        driver_write(theFile, block);
    }
    else
    {
        GetFileBlock(&inode[0], inode[0].size / 128, theFile);
        strcpy(theFile[fileLoc].name, argc[2]);
        theFile[fileLoc].inode = inodebit;
        writeFileBlock(&inode[0], inode[0].size / 128, theFile);
    }

    inode[0].size += 32;

    int freeNum = inodebit % 2;

    // Setting up the inode for the file.
    freeInode[freeNum].size = fileInfo.st_size;
    freeInode[freeNum].owner = fileInfo.st_uid;
    freeInode[freeNum].group = fileInfo.st_gid;
    freeInode[freeNum].atime = fileInfo.st_atime;
    freeInode[freeNum].mtime = fileInfo.st_mtime;
    freeInode[freeNum].ctime = fileInfo.st_ctime;
    freeInode[freeNum].refcount = fileInfo.st_nlink;
    freeInode[freeNum].perm = fileInfo.st_mode;
    freeInode[freeNum].type = S_ISDIR(fileInfo.st_mode);
    for(i = 0; i < 5; i++)
    {
        freeInode[freeNum].direct[i] = 0;
    }
    freeInode[freeNum].indirect = 0;
    freeInode[freeNum].dindirect = 0;
    freeInode[freeNum].tindirect = 0;

    char rawdata[128] = {0};
    int freeBlockbit = 0;
    for(i = 0; i < fileInfo.st_size / super->block_size; i++)
    {
        freeBlockbit = allocateBlock(&freeInode[freeNum], super, i);
        fin.read(rawdata, 128);
        driver_write(rawdata, freeBlockbit);
    }
    freeBlockbit = allocateBlock(&freeInode[freeNum], super, i);
    fin.read(rawdata, (int)freeInode->size % super->blocks_free);
    driver_write(rawdata, freeBlockbit);
    driver_write(freeInode, inodebit / 2 + super->inodes);

    driver_write(super, superBlockPos);
    driver_write(inode, super->inodes);

    // Closing the file.
    fin.close();

    // Close the disk image.
    driver_detach_disk_image();

    return 0;
}

int allocateBlock(sfs_inode *node, sfs_superblock *super, int blockNum)
{
    int block = 0;

    if (blockNum < 5)
    {
       node->direct[blockNum] = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
       block = (int)node->direct[blockNum];
       super->blocks_free = super->blocks_free - 1;
    }
    else if (blockNum < 5 + 32)
    {
        block = allocateIndirect(node, super, (int)node->indirect);
    }
    else if (blockNum < 5 + 32 + (32 * 32))
    {
        block = allocateDouble(node, super, (int)node->dindirect);
    }
    else if (blockNum < 5 + 32 + (32 * 32) + (32 * 32 * 32))
    {
        block = allocateTriple(node, super);
    }
    else
    {
        cout << "Error Writing File. Too big." << endl;
        exit(-1);
    }

    return block;
}

int allocateIndirect(sfs_inode *node, sfs_superblock *super, int blockNum)
{
    int empty[32] = {0};
    int arr[32] = {0};
    int freeBlock;
    int i = 0;

    if (node->indirect == 0)
    {
        node->indirect = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
        super->blocks_free -= 1;
        blockNum = (int)node->indirect;
        driver_write(arr, node->indirect);
    }
    driver_read(arr, blockNum);
    freeBlock = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
    driver_write(empty, freeBlock);
    while (arr[i] != 0)
        i++;
    arr[i] = freeBlock;

    driver_write(arr, blockNum);
    super->blocks_free = super->blocks_free - 1;

    return freeBlock;
}

int allocateDouble(sfs_inode *node, sfs_superblock *super, int dindirectNum)
{
    int empty[32] = {0};
    int arr[32] = {0};
    int freeBlock = 0;
    int indirect[32] = {0};
    int i = 0;

    if (node->dindirect == 0)
    {
        node->dindirect = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
        super->blocks_free = super->blocks_free - 1;
        dindirectNum = (int)node->dindirect;
        driver_write(arr, node->dindirect);
    }
    driver_read(arr, dindirectNum);

    // Finding the dindirect location of the indirect block to use.
    while (arr[i] != 0 && i < 32)
    {
        driver_read(indirect, arr[i]);
        if(indirect[31] == 0)
            break;
        i++;
    }
    if (arr[i] != 0 && i < 32)
        freeBlock = arr[i];
    else
    {
        freeBlock = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
        arr[i] = freeBlock;
        driver_write(empty, freeBlock);
        super->blocks_free -= 1;
    }
    driver_write(arr, dindirectNum);
    freeBlock = allocateIndirect(node, super, freeBlock);

    return freeBlock;
}


int allocateTriple(sfs_inode *node, sfs_superblock *super)
{
    int empty[32] = {0};
    int arr[32] = {0};
    int freeBlock = 0;
    int dindirect[32];
    int i = 0;

    if (node->tindirect == 0)
    {
        node->tindirect = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
        super->blocks_free = super->blocks_free - 1;
        driver_write(arr, node->tindirect);
    }
    driver_read(arr, node->tindirect);

    // Finding the tindirect location of the dindirect block to use.
    while (arr[i] != 0 && i < 32)
    {
        driver_read(dindirect, arr[i]);
        if(dindirect[31] == 0)
            break;
        i++;
    }
    if (arr[i] != 0 && i < 32)
        freeBlock = arr[i];
    else
    {
        freeBlock = getFreeSpot(super, super->fb_bitmap, (int)super->fb_bitmapblocks);
        arr[i] = freeBlock;
        driver_write(empty, freeBlock);
        super->blocks_free -= 1;
    }
    driver_write(arr, node->tindirect);
    freeBlock = allocateDouble(node, super, freeBlock);

    return freeBlock;
}



int getFreeSpot(sfs_superblock *super, uint8_t pos, int numBlocks)
{
    int i = 0;

//    auto* bitmap = (bitmap_t *) inodeBitChar;
    char bitmap[128];
    int32_t bit;

    // Finding the location of the first free block.
    for(i = 0; i < numBlocks; i++)
    {
        driver_read(bitmap, pos + i);
        bit = first_cleared(reinterpret_cast<bitmap_t *>(bitmap), super->block_size * 8);
        if(bit != -1)
            break;
    }
    if(bit == -1 && i == numBlocks)
    {
        cout << "Error: Disk is full." << endl;
        exit(0);
    }
    set_bit(reinterpret_cast<bitmap_t *>(bitmap), bit);
    driver_write(bitmap, pos + i);

    return bit + 1024 * i;
}


void writeFileBlock(sfs_inode_t * node, size_t blockNumber, void* data)
{
    uint32_t ptrs[128];

    // direct
    if (blockNumber < 5) {
        //printf("pass\n");
        driver_write(data, node->direct[blockNumber]);
    }
        // indirect
    else if (blockNumber < (5 + 32)) {
        driver_read(ptrs, node->indirect);
        driver_write(data, ptrs[blockNumber - 5]);
    }
        // double indirect
    else if (blockNumber < (5 + 32 + (32 * 32))) {
        driver_read(ptrs, node->dindirect);
        int tmp = (blockNumber - 5 - 32) / 32;
        driver_read(ptrs, ptrs[tmp]);
        tmp = (blockNumber - 5 - 32) % 32;
        driver_write(data, ptrs[tmp]);
    }
        // triple indirect
    else if (blockNumber < (5 + 32 + (32 * 32) + (32 * 32 * 32))) {
        driver_read(ptrs, node->tindirect);
        int tmp = (blockNumber - 5 - 32 - (32 * 32)) / (32 * 32);
        driver_read(ptrs, ptrs[tmp]);
        tmp = ((blockNumber - 5 - 32 - (32 * 32)) / 32) % 32;
        driver_read(ptrs, ptrs[tmp]);
        tmp = (blockNumber - 5 - 32 - (32 * 32)) % 32;
        driver_write(data, ptrs[tmp]);
    } else {
        printf("Error in block fetch, out of range\n");
        exit(1);
    }
}

void GetFileBlock(sfs_inode_t * node, size_t blockNumber, void* data)
{
    uint32_t ptrs[128];

    // direct
    if (blockNumber < 5)
    {
        //printf("pass\n");
        driver_read(data, node->direct[blockNumber]);
    }
        // indirect
    else if (blockNumber < (5 + 32))
    {
        driver_read(ptrs, node->indirect);
        driver_read(data, ptrs[blockNumber - 5]);
    }
        // double indirect
    else if (blockNumber < (5 + 32 + (32 * 32)))
    {
        driver_read(ptrs, node->dindirect);
        int tmp = (blockNumber - 5 - 32) / 32;
        driver_read(ptrs, ptrs[tmp]);
        tmp = (blockNumber - 5 - 32) % 32;
        driver_read(data, ptrs[tmp]);
    }
        // triple indirect
    else if (blockNumber < (5 + 32 + (32 * 32) + (32 * 32 * 32)))
    {
        driver_read(ptrs, node->tindirect);
        int tmp = (blockNumber - 5 - 32 - (32 * 32)) / (32 * 32);
        driver_read(ptrs, ptrs[tmp]);
        tmp = ((blockNumber - 5 - 32 - (32 * 32)) / 32) % 32;
        driver_read(ptrs, ptrs[tmp]);
        tmp = (blockNumber - 5 - 32 - (32 * 32)) % 32;
        driver_read(data, ptrs[tmp]);
    }
    else
    {
        printf("Error in block fetch, out of range\n");
        exit(1);
    }
}