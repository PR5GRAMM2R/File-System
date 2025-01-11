#include <stdio.h>      // 2020203011 배주환 ( Bae Juhwan )
#include <stdlib.h>     // Kwangwoon Univ. Software Department
#include "buf.h"
#include "fs.h"         // This is made in Ubuntu 22.04.3 LTS

#include <string.h>

void FileSysInit(void)
{
    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    memset(tempBlock, 0, BLOCK_SIZE);

    FileSysInfo* initFileSysInfo = malloc((size_t)sizeof(FileSysInfo));     // FileSysInfo 초기화

    initFileSysInfo->blocks = 0;
    initFileSysInfo->rootInodeNum = 0;
    initFileSysInfo->diskCapacity = 0;
    initFileSysInfo->numAllocBlocks = 0;
    initFileSysInfo->numFreeBlocks = 0;
    initFileSysInfo->numAllocInodes = 0;
    initFileSysInfo->blockBitmapBlock = 0;
    initFileSysInfo->inodeBitmapBlock = 0;
    initFileSysInfo->inodeListBlock = 0;
    initFileSysInfo->dataRegionBlock = 0;

    memcpy(tempBlock, initFileSysInfo, sizeof(FileSysInfo));
    
    BufWrite(FILESYS_INFO_BLOCK, tempBlock);

    free(initFileSysInfo);

    memset(tempBlock, 0, BLOCK_SIZE);                               // Inode Bitmap 초기화
    BufWrite(INODE_BITMAP_BLOCK_NUM, tempBlock);

    memset(tempBlock, 0, BLOCK_SIZE);                               // Block Bitmap 초기화
    BufWrite(BLOCK_BITMAP_BLOCK_NUM, tempBlock);

    memset(tempBlock, 0, BLOCK_SIZE);
    for(int i = INODELIST_BLOCK_FIRST; i < INODELIST_BLOCK_FIRST + INODELIST_BLOCKS; i++){  // Inode Bitmap 초기화
        // Inode* inode = malloc((size_t)sizeof(Inode));
        // inode->allocBlocks = 0;
        // inode->size = 0;
        // for(int j = 0; j < NUM_OF_INODE_PER_BLOCK; j++){
        //     memcpy(tempBlock + j * sizeof(Inode), inode, sizeof(Inode));
        // }
        // BufWrite(i, tempBlock);

        BufWrite(i, tempBlock);
    }

    memset(tempBlock, 0, BLOCK_SIZE);
    for(int i = INODELIST_BLOCK_FIRST + INODELIST_BLOCKS; i < 512; i++){
        BufWrite(i, tempBlock);
    }

    free(tempBlock);

    // char* blockByteMap = malloc(BLOCK_SIZE);
    // memset(blockByteMap, 0, BLOCK_SIZE);

    // for(int i = 0; i < 512; i++){
    //     BufWrite(i, blockByteMap);
    // }

    // free(blockByteMap);

    BufSync();

    return;
}

// void SetInodeBytemap(int inodeno)
// {
//     char* inodeFromByteMap = malloc(BLOCK_SIZE);
//     BufRead(INODE_BYTEMAP_BLOCK_NUM, inodeFromByteMap);

//     *(inodeFromByteMap + sizeof(char) * inodeno) = 1;

//     BufWrite(INODE_BYTEMAP_BLOCK_NUM, inodeFromByteMap);
//     free(inodeFromByteMap);

//     BufSyncBlock(INODE_BYTEMAP_BLOCK_NUM);

//     // char * fileSysInfoBlock = malloc(BLOCK_SIZE);
//     // BufRead(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // FileSysInfo * fileSysInfo = malloc(sizeof(FileSysInfo));
//     // memcpy(fileSysInfo, fileSysInfoBlock, sizeof(FileSysInfo));
//     // fileSysInfo->numAllocInodes++;
//     // memcpy(fileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo));
//     // BufWrite(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // free(fileSysInfo);
//     // free(fileSysInfoBlock);

//     return;
// }


// void ResetInodeBytemap(int inodeno)
// {
//     char* inodeFromByteMap = malloc(BLOCK_SIZE);
//     BufRead(INODE_BYTEMAP_BLOCK_NUM, inodeFromByteMap);
//     *(inodeFromByteMap + sizeof(char) * inodeno) = 0;
//     BufWrite(INODE_BYTEMAP_BLOCK_NUM, inodeFromByteMap);
//     free(inodeFromByteMap);

//     BufSyncBlock(INODE_BYTEMAP_BLOCK_NUM);

//     // char * fileSysInfoBlock = malloc(BLOCK_SIZE);
//     // BufRead(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // FileSysInfo * fileSysInfo = malloc(sizeof(FileSysInfo));
//     // memcpy(fileSysInfo, fileSysInfoBlock, sizeof(FileSysInfo));
//     // fileSysInfo->numAllocInodes--;
//     // memcpy(fileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo));
//     // BufWrite(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // free(fileSysInfo);
//     // free(fileSysInfoBlock);

//     return;
// }


// void SetBlockBytemap(int blkno)
// {
//     char* blockFromByteMap = malloc(BLOCK_SIZE);
//     BufRead(BLOCK_BYTEMAP_BLOCK_NUM, blockFromByteMap);
//     blockFromByteMap[blkno] = 1;
//     BufWrite(BLOCK_BYTEMAP_BLOCK_NUM, blockFromByteMap);
//     free(blockFromByteMap);

//     BufSyncBlock(BLOCK_BYTEMAP_BLOCK_NUM);

//     // char * fileSysInfoBlock = malloc(BLOCK_SIZE);
//     // BufRead(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // FileSysInfo * fileSysInfo = malloc(sizeof(FileSysInfo));
//     // memcpy(fileSysInfo, fileSysInfoBlock, sizeof(FileSysInfo));
//     // fileSysInfo->numAllocBlocks++;
//     // fileSysInfo->numFreeBlocks--;
//     // memcpy(fileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo));
//     // BufWrite(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // free(fileSysInfo);
//     // free(fileSysInfoBlock);

//     return;
// }


// void ResetBlockBytemap(int blkno)
// {
//     char* blockFromByteMap = malloc(BLOCK_SIZE);
//     BufRead(BLOCK_BYTEMAP_BLOCK_NUM, blockFromByteMap);
//     blockFromByteMap[blkno] = 0;
//     BufWrite(BLOCK_BYTEMAP_BLOCK_NUM, blockFromByteMap);
//     free(blockFromByteMap);

//     BufSyncBlock(BLOCK_BYTEMAP_BLOCK_NUM);

//     // char * fileSysInfoBlock = malloc(BLOCK_SIZE);
//     // BufRead(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // FileSysInfo * fileSysInfo = malloc(sizeof(FileSysInfo));
//     // memcpy(fileSysInfo, fileSysInfoBlock, sizeof(FileSysInfo));
//     // fileSysInfo->numAllocBlocks--;
//     // fileSysInfo->numFreeBlocks++;
//     // memcpy(fileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo));
//     // BufWrite(FILESYS_INFO_BLOCK, fileSysInfoBlock);
//     // free(fileSysInfo);
//     // free(fileSysInfoBlock);

//     return;
// }


unsigned char SetBitmap(unsigned char offsetByte, int bitNum)
{
    unsigned char temp = 128;
    
    for(int i = 0; i < bitNum; i++)
        temp /= 2;

    offsetByte += temp;

    return offsetByte;
}


unsigned char ResetBitmap(unsigned char offsetByte, int bitNum)
{
    unsigned char temp = 128;
    
    for(int i = 0; i < bitNum; i++)
        temp /= 2;

    offsetByte -= temp;

    return offsetByte;
}


void SetInodeBitmap(int inodeno)
{
    int inodeByteNum = inodeno / 8;
    int inodeBitNum = inodeno % 8;
    
    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(INODE_BITMAP_BLOCK_NUM, tempBlock);

    // for(int i = 0; i < 512; i++){
    //     printf("%d ", (char)*(tempBlock + i));
    // }
    // printf("\n");

    unsigned char inodeByte = (unsigned char)*(tempBlock + (size_t)sizeof(char) * inodeByteNum);

    inodeByte = SetBitmap(inodeByte, inodeBitNum);

    memcpy(tempBlock + sizeof(char) * inodeByteNum, &inodeByte, (size_t)sizeof(char));

    // for(int i = 0; i < 512; i++){
    //     printf("%d ", (char)*(tempBlock + i));
    // }
    // printf("\n");

    BufWrite(INODE_BITMAP_BLOCK_NUM, tempBlock);
    free(tempBlock);

    BufSyncBlock(INODE_BITMAP_BLOCK_NUM);

    return;
}


void ResetInodeBitmap(int inodeno)
{
    int inodeByteNum = inodeno / 8;
    int inodeBitNum = inodeno % 8;
    
    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(INODE_BITMAP_BLOCK_NUM, tempBlock);

    unsigned char inodeByte = (unsigned char)*(tempBlock + (size_t)sizeof(char) * inodeByteNum);

    inodeByte = ResetBitmap(inodeByte, inodeBitNum);

    memcpy(tempBlock + (size_t)sizeof(char) * inodeByteNum, &inodeByte, (size_t)sizeof(char));

    BufWrite(INODE_BITMAP_BLOCK_NUM, tempBlock);
    free(tempBlock);

    BufSyncBlock(INODE_BITMAP_BLOCK_NUM);

    return;
}


void SetBlockBitmap(int blkno)
{
    int blockByteNum = blkno / 8;
    int blockBitNum = blkno % 8;
    
    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, tempBlock);

    unsigned char blockByte = (unsigned char)*(tempBlock + (size_t)sizeof(char) * blockByteNum);

    blockByte = SetBitmap(blockByte, blockBitNum);

    memcpy(tempBlock + (size_t)sizeof(char) * blockByteNum, &blockByte, (size_t)sizeof(char));

    BufWrite(BLOCK_BITMAP_BLOCK_NUM, tempBlock);
    free(tempBlock);

    BufSyncBlock(BLOCK_BITMAP_BLOCK_NUM);

    return;
}


void ResetBlockBitmap(int blkno)
{
    int blockByteNum = blkno / 8;
    int blockBitNum = blkno % 8;
    
    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, tempBlock);

    unsigned char blockByte = (unsigned char)*(tempBlock + (size_t)sizeof(char) * blockByteNum);

    blockByte = ResetBitmap(blockByte, blockBitNum);

    memcpy(tempBlock + (size_t)sizeof(char) * blockByteNum, &blockByte, (size_t)sizeof(char));

    BufWrite(BLOCK_BITMAP_BLOCK_NUM, tempBlock);
    free(tempBlock);

    BufSyncBlock(BLOCK_BITMAP_BLOCK_NUM);

    return;
}




void PutInode(int inodeno, Inode* pInode)
{
    int inodeBlockNum = INODELIST_BLOCK_FIRST + inodeno / NUM_OF_INODE_PER_BLOCK;
    int inodeNumInBlock = inodeno % NUM_OF_INODE_PER_BLOCK;

    // printf("- %d %d -\n", inodeBlockNum, inodeNumInBlock);       ///

    // printf("%d, %d\n", pInode->allocBlocks, pInode->size);      ///

    // printf("%ld  %ld  %ld=== \n", sizeof(char), sizeof(Inode), sizeof(FileSysInfo));

    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(inodeBlockNum, tempBlock);
    memcpy(tempBlock + sizeof(Inode) * inodeNumInBlock, pInode, (size_t)sizeof(Inode));

    //printf("==> %ld --> \n", tempBlock + (size_t)sizeof(Inode) * inodeNumInBlock);

    BufWrite(inodeBlockNum, tempBlock);
    free(tempBlock);

    BufSyncBlock(inodeBlockNum);

    return;
}


void GetInode(int inodeno, Inode* pInode)
{
    int inodeBlockNum = INODELIST_BLOCK_FIRST + inodeno / NUM_OF_INODE_PER_BLOCK;
    int inodeNumInBlock = inodeno % NUM_OF_INODE_PER_BLOCK;

    //printf("- %d %d -\n", inodeBlockNum, inodeNumInBlock);       ///

    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(inodeBlockNum, tempBlock);
    memcpy(pInode, tempBlock + (size_t)sizeof(Inode) * inodeNumInBlock, (size_t)sizeof(Inode));
    free(tempBlock);

    //printf("==> %ld\n", tempBlock + (size_t)sizeof(Inode) * inodeNumInBlock);

    //printf("%d, %d\n", pInode->allocBlocks, pInode->size);      ///

    return;
}


int GetFreeBit(unsigned char byteForFree)
{
    int freeBitOffset = 0;
    unsigned char temp = 128;

    for(int bitOffset = 0; bitOffset < 8; bitOffset++){
        if(byteForFree / temp == 1){
            byteForFree -= temp;
            temp /= 2;
        }
        else{
            freeBitOffset = bitOffset;
            break;
        }
    }

    return freeBitOffset;
}


int GetFreeInodeNum(void)
{
    int inodeByteNum = 0;

    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);

    BufRead(INODE_BITMAP_BLOCK_NUM, tempBlock);

    // for(int i = 0; i < 512; i++){
    //     char* temp = tempBlock + i;
    //     printf("%d ", *temp);
    // }
    // printf("\n");

    while((unsigned char)*(tempBlock + (size_t)sizeof(char) * inodeByteNum) == 255){
        inodeByteNum++;
    }

    unsigned char inodeByteForFree = (unsigned char)*(tempBlock + (size_t)sizeof(char) * inodeByteNum);

    int inodeBitNum = GetFreeBit(inodeByteForFree);

    free(tempBlock);

    return (inodeByteNum * 8 + inodeBitNum);
}


int GetFreeBlockNum(void)
{
    int blockByteNum = 0;

    char* tempBlock = (char*)malloc((size_t)BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, tempBlock);

    while((unsigned char)*(tempBlock + (size_t)sizeof(char) * blockByteNum) == 255){
        blockByteNum++;
    }

    unsigned char blockByteForFree = (unsigned char)*(tempBlock + (size_t)sizeof(char) * blockByteNum);

    int blockBitNum = GetFreeBit(blockByteForFree);

    free(tempBlock);

    return (blockByteNum * 8 + blockBitNum);
}