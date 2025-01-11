#include <stdio.h>                          // gcc main.c fs.c fs_internal.c buf.c disk.c -o main
#include "fs.h"                             // xxd -c 64  MY_DISK
#include "buf.h"
#include "disk.h"                           // 배주환 2020203011

#include <stdlib.h>

int main(){
    printf("Hello OS World\n");

    CreateFileSystem();

    MakeDir("/tmp");

    RemoveDir("/tmp");

    // printf("0\n");

    // int blkNum, inodeNum;

    // DevCreateDisk();
    // BufInit();

    // FileSysInit();

    // printf("1\n");

    // inodeNum = GetFreeInodeNum();
    // SetInodeBitmap(inodeNum);

    // //inodeNum = GetFreeInodeNum();

    // printf("2\n");

    // blkNum = GetFreeBlockNum();
    // SetBlockBitmap(blkNum);

    // printf("%d %d\n", inodeNum, blkNum);

    // printf("3\n");

    // Inode inode;
    // inode.allocBlocks = 1;
    // inode.size = inode.allocBlocks * 512;
    // inode.type = FILE_TYPE_FILE;
    // for(int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++)
    //     inode.dirBlockPtr[i] = 0;
    // PutInode(inodeNum, &inode);

    // printf("4\n");

    // Inode* pInode = (Inode*)malloc((size_t)sizeof(Inode));
    // GetInode(inodeNum, pInode);

    // printf("5\n");

    // printf("%d %d %d %d \n", inodeNum, blkNum, (int)pInode->allocBlocks, (int)pInode->size);


    return 0;
}
