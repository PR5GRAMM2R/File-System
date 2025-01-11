#include <stdio.h>      // 2020203011 배주환 ( Bae Juhwan )
#include <stdlib.h>     // Kwangwoon Univ. Software Department
#include "buf.h"
#include "fs.h"         // This is made in Ubuntu 22.04.3 LTS

#include <string.h>     // 과제 수행 기간 : 2024년 06월 20일 (목) 오후 07시 42분 00초 ~
#include "queue.h"      //                  2024년 06월 25일 (화) 오후 10시 48분 00초

#define MAX_LENGTH (256)

FileDesc pFileDesc[MAX_FD_ENTRY_MAX];
FileSysInfo* pFileSysInfo;


// typedef struct _Path{
//     char* paths[MAX_PATHS];
//     int pathCount;
// } Path;


typedef struct Path{
    int pathNameLength;
    char pathName[MAX_NAME_LEN];
    TAILQ_ENTRY(Path) nextPath;
} Path;

TAILQ_HEAD(pathList, Path);

struct pathList pathListHead;


void GetPathsFromPath(const char* _path)        // '/' 기준으로 파싱 진행하여 TAILQ 리스트에 순차적으로 넣음.
{
    char path[MAX_LENGTH];

    strncpy(path, _path, MAX_LENGTH);

    TAILQ_INIT(&pathListHead);

    //printf("%s \n", path);

    //printf("0\n");

    int currentPos = 0;

    char tempPathName[MAX_NAME_LEN];

    for(int i = 1; i < MAX_LENGTH; i++){
        if(path[i] == '/'){
            
            //printf("2\n");

            Path* tempPath1 = malloc(sizeof(Path));

            tempPathName[currentPos] = '\0';

            strcpy(tempPath1->pathName, tempPathName);

            tempPath1->pathNameLength = currentPos;

            //printf("%s\n", tempPath1->pathName);

            TAILQ_INSERT_TAIL(&pathListHead, tempPath1, nextPath);

            currentPos = 0;
        }
        else if(path[i] == '\0'){
            //printf("3\n");

            Path* tempPath2 = malloc(sizeof(Path));

            tempPathName[currentPos] = '\0';

            strcpy(tempPath2->pathName, tempPathName);

            tempPath2->pathNameLength = currentPos;

            //printf("%s\n", tempPath2->pathName);

            TAILQ_INSERT_TAIL(&pathListHead, tempPath2, nextPath);

            break;
        }
        else{
            tempPathName[currentPos] = path[i];
            currentPos++;

            //printf("1\n");
        }
    }

    return;
}


void FreePathList()
{
    Path* tempPath;

    TAILQ_FOREACH(tempPath, &pathListHead, nextPath){
        free(tempPath);
    }
}


int SearchDirectoryPath(const char* path)   // path 의 최하위 디렉터리의 한 단계 상위의 디렉터리의 inodeNum 를 반환
{
    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    
    DirEntry* dirEntry = malloc((size_t)sizeof(DirEntry));

    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));

    //printf("<==> %d %d %d %d %d %d %d %d %d %d\n", fileSysInfo->blocks, fileSysInfo->rootInodeNum, fileSysInfo->diskCapacity, fileSysInfo->numAllocBlocks, fileSysInfo->numFreeBlocks, fileSysInfo->numAllocInodes, fileSysInfo->blockBitmapBlock, fileSysInfo->inodeBitmapBlock, fileSysInfo->inodeListBlock, fileSysInfo->dataRegionBlock);

    int inodeNum = fileSysInfo->rootInodeNum;       // Start with Root Inode
    
    GetPathsFromPath(path);         // Get Path TAILQ

    Path* tempPath;

    int dirNameCount = 0;           // I will use this to stop at the Parent Dir of Lowest Dir.

    TAILQ_FOREACH(tempPath, &pathListHead, nextPath){
        dirNameCount++;
    }

    //printf("<<<<<<--->>>>>> %d\n", dirNameCount);   ///

    int currentDirNameCount = 0;    // I will use this to stop at the Parent Dir of Lowest Dir, too.

    TAILQ_FOREACH(tempPath, &pathListHead, nextPath){
        char* dirName = tempPath->pathName;

        if(currentDirNameCount == dirNameCount - 1){

            //printf("<<<<<<+++>>>>>> %d\n", currentDirNameCount);   ///
            //printf("<<<<<<===>>>>>> %s\n", dirEntry->name);     ///

            free(tempFileSysInfoBlock);
            free(fileSysInfo);
            free(tempInode);
            free(tempBlockBlock);

            free(dirEntry);

            return inodeNum;    // Return the Dir's Inode, which is the parent of Lowest Dir.
        }

        GetInode(inodeNum, tempInode);

        for(int i = 0; i < tempInode->allocBlocks; i++){
            BufRead(tempInode->dirBlockPtr[i], tempBlockBlock);
                
            for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){   // && currentDirNameCount != dirNameCount
                memcpy(dirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));

                //printf("------------------%s\n", dirEntry->name);

                if(strcmp(dirEntry->name, tempPath->pathName) == 0){   // If pathName Found, use GOTO for continuing.
                    inodeNum = dirEntry->inodeNum;
                    goto GET_INTO_SUB_DIRECTORY;
                }
            }
        }

        free(tempFileSysInfoBlock);
        free(fileSysInfo);
        free(tempInode);
        free(tempBlockBlock);

        free(dirEntry);

        return -1;  // If there is no pathName Dir, it is considered to have no Parent Dir.

        GET_INTO_SUB_DIRECTORY:

        currentDirNameCount++;

    }
}


int     CreateFile(const char* szFileName)
{
    //printf("-=======- 0\n");    ///

    int parentDirInodeNum = SearchDirectoryPath(szFileName); // Get the inode of Parent Dir of Lowest File.

    //printf("%s -> %d\n ", szDirName, parentDirInodeNum);    ///

    //printf("-=======- 1\n");    ///

    int inodeNum;

    if(parentDirInodeNum < 0)                               // If returned Inode is minus, it returns -1.
        return -1;

    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));     // Get the FIleSysInfo.

    GetInode(parentDirInodeNum, tempParentInode);               // Get Inode of returned Inode (Parent Dir of Lowest File) .

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);       // Get Lowest File's (path) Name.

    //printf("%d \n", parentDirInodeNum);         ///
    //printf("===== %s\n", tempPath->pathName);   ///

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        //printf("=0 \n");         ///
        
        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){   // && currentDirNameCount != dirNameCount
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            //printf("%s\n", tempDirEntry->name); ///

            if(strcmp(tempDirEntry->name, "") == 0){  ///   // If this DirEntry's Name has no content, do the next stuff.
                inodeNum = GetFreeInodeNum();               // Get the new free Inode Number.

                //printf("=1 \n");         ///

                DirEntry * dirEntry = malloc((size_t)sizeof(DirEntry));
                strcpy(dirEntry->name, tempPath->pathName);
                dirEntry->inodeNum = inodeNum;              // Make thd Lowest File into the Parent Dir.

                //printf("%d %s\n", j, dirEntry->name); ///

                //printf("=2 \n");         ///
    
                memcpy(tempBlockBlock + j * sizeof(DirEntry), dirEntry, sizeof(DirEntry));
                BufWrite(tempParentInode->dirBlockPtr[i], tempBlockBlock);

                BufSyncBlock(tempParentInode->dirBlockPtr[i]);

                //printf("=3 \n");         ///

                ///

                GetInode(inodeNum, tempInode);          // Get the new free Inode for Lowest File.

                //printf("=4 \n");         ///

                tempInode->allocBlocks = 0;
                tempInode->size = tempInode->allocBlocks * BLOCK_SIZE;
                tempInode->type = FILE_TYPE_FILE;

                PutInode(inodeNum, tempInode);      //  Set the new Inode for Lowest File.

                //printf("=5 \n");         ///

                SetInodeBitmap(inodeNum);           // Set the Inode Bitmap Num for Lowest File to 1.

                //printf("=6 \n");         ///

                free(dirEntry);

                goto FINAL_CREATEFILE;     // We already made the Lowest Dir, so we have to end this function.
            }
        }

        if(i == NUM_OF_DIRECT_BLOCK_PTR - 1){
            return -1;
        }

        if(i == tempParentInode->allocBlocks - 1){  // If DirEntry is Full, make the new DirBlockPtr.
            tempParentInode->allocBlocks++;
            tempParentInode->size += BLOCK_SIZE;
            int freeBlockNum = GetFreeBlockNum();
            tempParentInode->dirBlockPtr[i + 1] = freeBlockNum;

            PutInode(parentDirInodeNum, tempParentInode);

            SetBlockBitmap(freeBlockNum);
        }
    }

    FINAL_CREATEFILE:

    //fileSysInfo->blocks++;
    //fileSysInfo->diskCapacity = fileSysInfo->blocks * BLOCK_SIZE;
    //fileSysInfo->numAllocBlocks++;
    //fileSysInfo->numFreeBlocks--;
    fileSysInfo->numAllocInodes++;

    memcpy(tempFileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo)); // Update the fileSysInfo.
    BufWrite(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);


    free(tempFileSysInfoBlock);
    free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    BufSyncBlock(FILESYS_INFO_BLOCK);
    //BufSync();      // Must do BufSync.

    FreePathList();

    // Below is for File Descriptor.

    File* file = malloc((size_t)sizeof(File));

    file->fileOffset = 0;
    file->inodeNum = inodeNum;

    int index = 0;

    for(index = 0; index < MAX_FD_ENTRY_MAX; index++){
        if(pFileDesc[index].bUsed == FALSE){
            break;
        }
    }

    pFileDesc[index].bUsed = TRUE;
    pFileDesc[index].pOpenFile = file;

    return index;
}

int     OpenFile(const char* szFileName)
{
    //printf("-=======- 0\n");    ///

    int parentDirInodeNum = SearchDirectoryPath(szFileName); // Get the inode of Parent Dir of Lowest File.

    //printf("%s -> %d\n ", szDirName, parentDirInodeNum);    ///

    //printf("-=======- 1\n");    ///

    int inodeNum;

    if(parentDirInodeNum < 0)                               // If returned Inode is minus, it returns -1.
        return -1;

    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));     // Get the FIleSysInfo.

    GetInode(parentDirInodeNum, tempParentInode);               // Get Inode of returned Inode (Parent Dir of Lowest File) .

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);       // Get Lowest File's (path) Name.

    //printf("%d \n", parentDirInodeNum);         ///
    //printf("===== %s\n", tempPath->pathName);   ///

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        //printf("=0 \n");         ///
        
        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){   // && currentDirNameCount != dirNameCount
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            //printf("%s\n", tempDirEntry->name); ///

            if(strcmp(tempDirEntry->name, tempPath->pathName) == 0){
                inodeNum = tempDirEntry->inodeNum;
                
                goto FINAL_OPENFILE;
            }
        }

        if(i == NUM_OF_DIRECT_BLOCK_PTR - 1){
            free(tempFileSysInfoBlock);
            free(fileSysInfo);
            free(tempParentInode);
            free(tempInode);
            free(tempBlockBlock);
            free(tempDirEntry);

            FreePathList();

            return -1;
        }
    }

    FINAL_OPENFILE:

    free(tempFileSysInfoBlock);
    free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    FreePathList();

    // Below is for File Descriptor.

    File* file = malloc((size_t)sizeof(File));

    file->fileOffset = 0;
    file->inodeNum = inodeNum;

    int index = 0;

    for(index = 0; index < MAX_FD_ENTRY_MAX; index++){
        if(pFileDesc[index].bUsed == FALSE){
            break;
        }
    }

    pFileDesc[index].bUsed = TRUE;
    pFileDesc[index].pOpenFile = file;

    return index;
}


int     WriteFile(int fileDesc, char* pBuffer, int length)
{
    if(pFileDesc[fileDesc].bUsed == FALSE){
        return -1;
    }

    File* file = pFileDesc[fileDesc].pOpenFile;

    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo)); 

    int inodeNum = file->inodeNum;

    int result = file->fileOffset + length;

    int startFileBlockNum = file->fileOffset / BLOCK_SIZE;

    int startFileBlockOffset = file->fileOffset % BLOCK_SIZE;

    int endFileBlockNum = (result == 0) ? 0 :  (result - 1) / BLOCK_SIZE; //(result / BLOCK_SIZE - 1 < 0) ? 0 : (result / BLOCK_SIZE);

    int endFileBlockOffset = result & BLOCK_SIZE;

    Inode* fileInode = malloc((size_t)sizeof(Inode));

    GetInode(inodeNum, fileInode);

    int ofs = 0;

    for(int i = startFileBlockNum; i <= endFileBlockNum; i++){
        if(i >= NUM_OF_DIRECT_BLOCK_PTR){
            break;
        }

        if(fileInode->dirBlockPtr[i] == 0){
            int blockNum = GetFreeBlockNum();

            fileInode->dirBlockPtr[i] = blockNum;

            fileSysInfo->numAllocBlocks++;
            fileSysInfo->numFreeBlocks--;

            fileInode->allocBlocks++;
            fileInode->size = fileInode->allocBlocks * BLOCK_SIZE;

            char* tempBlock = malloc((size_t)BLOCK_SIZE);

            BufRead(blockNum, tempBlock);

            if(i == startFileBlockNum){
                memcpy(tempBlock + startFileBlockOffset, pBuffer + ofs, (size_t)(BLOCK_SIZE - startFileBlockOffset));
                ofs += BLOCK_SIZE - startFileBlockOffset;
            }
            else if(i == endFileBlockNum){
                memcpy(tempBlock, pBuffer + ofs, (size_t)endFileBlockOffset);
            }
            else{
                memcpy(tempBlock, pBuffer + ofs, (size_t)BLOCK_SIZE);
                ofs += BLOCK_SIZE;
            }

            BufWrite(blockNum, tempBlock);

            SetBlockBitmap(blockNum);

            BufSyncBlock(blockNum);

            free(tempBlock);
        }
        else{
            int blockNum = fileInode->dirBlockPtr[i];

            char* tempBlock = malloc((size_t)BLOCK_SIZE);

            BufRead(blockNum, tempBlock);

            if(i == startFileBlockNum){
                memcpy(tempBlock + startFileBlockOffset, pBuffer + ofs, (size_t)(BLOCK_SIZE - startFileBlockOffset));
                ofs += BLOCK_SIZE - startFileBlockOffset;
            }
            else if(i == endFileBlockNum){
                memcpy(tempBlock, pBuffer + ofs, (size_t)endFileBlockOffset);
            }
            else{
                memcpy(tempBlock, pBuffer + ofs, (size_t)BLOCK_SIZE);
                ofs += BLOCK_SIZE;
            }

            BufWrite(blockNum, tempBlock);

            BufSyncBlock(blockNum);

            free(tempBlock);
        }
    }

    file->fileOffset = result;

    //printf("?? %d\n", file->fileOffset);  ///

    PutInode(inodeNum, fileInode);

    memcpy(tempFileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo)); // Update the fileSysInfo.
    BufWrite(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);

    BufSyncBlock(FILESYS_INFO_BLOCK);

    //free(file);
    free(fileInode);
    free(fileSysInfo);
    free(tempFileSysInfoBlock);

    return result;
}

int     ReadFile(int fileDesc, char* pBuffer, int length)
{
    if(pFileDesc[fileDesc].bUsed == FALSE){
        return -1;
    }

    //printf("====>> 40\n");   ///

    File* file = pFileDesc[fileDesc].pOpenFile;

    int inodeNum = file->inodeNum;

    int result = file->fileOffset + length;

    int startFileBlockNum = file->fileOffset / BLOCK_SIZE;

    int startFileBlockOffset = file->fileOffset % BLOCK_SIZE;

    int endFileBlockNum = (result == 0) ? 0 :  (result - 1) / BLOCK_SIZE; //(result / BLOCK_SIZE - 1 < 0) ? 0 : (result / BLOCK_SIZE);

    int endFileBlockOffset = result & BLOCK_SIZE;

    Inode* fileInode = malloc((size_t)sizeof(Inode));

    //printf("====>> 41\n");   ///

    GetInode(inodeNum, fileInode);

    //printf("====>> 42\n");   ///

    int ofs = 0;

    for(int i = startFileBlockNum; i <= endFileBlockNum; i++){
        if(i >= NUM_OF_DIRECT_BLOCK_PTR){
            break;
        }

        int blockNum = fileInode->dirBlockPtr[i];

        char* tempBlock = malloc((size_t)BLOCK_SIZE);

        BufRead(blockNum, tempBlock);

        //printf("==<> %d\n", blockNum);  ///

        //printf("====>> 43\n");   ///

        if(i == startFileBlockNum){
            memcpy(pBuffer + ofs, tempBlock + startFileBlockOffset, (size_t)(BLOCK_SIZE - startFileBlockOffset));
            ofs += BLOCK_SIZE - startFileBlockOffset;

            //printf("====>> 430\n");   ///
        }
        else if(i == endFileBlockNum){
            memcpy(pBuffer + ofs, tempBlock, (size_t)endFileBlockOffset);

            //printf("====>> 431\n");   ///
        }
        else{
            memcpy(pBuffer + ofs, tempBlock, (size_t)BLOCK_SIZE);
            ofs += BLOCK_SIZE;

            //printf("====>> 432\n");   ///
        }

        free(tempBlock);

        //printf("====>> 44\n");   ///
    }

    file->fileOffset = result;

    //free(file);
    free(fileInode);

    //printf("====>> 45\n");   ///

    return result;
}


int     CloseFile(int fileDesc)
{
    if(pFileDesc[fileDesc].bUsed == FALSE){
        return -1;
    }

    pFileDesc[fileDesc].bUsed = FALSE;
    File* file = pFileDesc[fileDesc].pOpenFile;
    free(file);

    return fileDesc;
}

int     RemoveFile(const char* szFileName)
{
    //printf("-=======- 0\n");    ///

    int parentDirInodeNum = SearchDirectoryPath(szFileName); // Get the inode of Parent Dir of Lowest File.

    //printf("%s -> %d\n ", szDirName, parentDirInodeNum);    ///

    //printf("-=======- 1\n");    ///

    int inodeNum;

    if(parentDirInodeNum < 0)                               // If returned Inode is minus, it returns -1.
        return -1;

    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));     // Get the FIleSysInfo.

    GetInode(parentDirInodeNum, tempParentInode);               // Get Inode of returned Inode (Parent Dir of Lowest File) .

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);       // Get Lowest File's (path) Name.

    //printf("%d \n", parentDirInodeNum);         ///
    //printf("===== %s\n", tempPath->pathName);   ///

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        //printf("=0 \n");         ///
        
        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){   // && currentDirNameCount != dirNameCount
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            //printf("%s\n", tempDirEntry->name); ///

            if(strcmp(tempDirEntry->name, tempPath->pathName) == 0){  ///   // If this DirEntry's Name is the Lowset File's name, do the next stuff.
                inodeNum = tempDirEntry->inodeNum;

                //char isThereAnyFilesOrDirectories = FALSE;

                Inode* dirInode = malloc((size_t)sizeof(Inode));
                DirEntry* dirEntry = malloc((size_t)sizeof(DirEntry));
                char* tempBlock = malloc((size_t)BLOCK_SIZE);

                GetInode(inodeNum, dirInode);

                for(int a = 0; a < dirInode->allocBlocks; a++){
                    BufRead(dirInode->dirBlockPtr[a], tempBlock);
                    memset(tempBlock, 0, (size_t)BLOCK_SIZE);
                    BufWrite(dirInode->dirBlockPtr[a], tempBlock);
                    BufSyncBlock(dirInode->dirBlockPtr[a]);
                    ResetBlockBitmap(dirInode->dirBlockPtr[a]);

                    //fileSysInfo->blocks--;
                    //fileSysInfo->diskCapacity = fileSysInfo->blocks * BLOCK_SIZE;
                    fileSysInfo->numAllocBlocks--;
                    fileSysInfo->numFreeBlocks++;

                    dirInode->dirBlockPtr[a] = 0;
                }

                dirInode->allocBlocks = 0;
                dirInode->size = 0;
                dirInode->type = 0;

                PutInode(inodeNum, dirInode);
                ResetInodeBitmap(inodeNum);

                tempDirEntry->inodeNum = 0;
                for(int a = 0; a < MAX_NAME_LEN; a++){
                    tempDirEntry->name[a] = '\0';
                }
                
                memcpy(tempBlockBlock + j * sizeof(DirEntry), tempDirEntry, sizeof(DirEntry));
                BufWrite(tempParentInode->dirBlockPtr[i], tempBlockBlock);

                BufSyncBlock(tempParentInode->dirBlockPtr[i]);

                free(dirInode);
                free(dirEntry);

                goto FINAL_REMOVEFILE;     // We already removed the Lowest File, so we have to end this function.
            }
        }
    }

    FINAL_REMOVEFILE:

    fileSysInfo->numAllocInodes--;

    memcpy(tempFileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo)); // Update the fileSysInfo.
    BufWrite(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);


    free(tempFileSysInfoBlock);
    free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    BufSyncBlock(FILESYS_INFO_BLOCK);
    //BufSync();      // Must do BufSync.

    FreePathList();

    return inodeNum;
}



int     MakeDir(const char* szDirName)
{
    //printf("-=======- 0\n");    ///

    int parentDirInodeNum = SearchDirectoryPath(szDirName); // Get the inode of Parent Dir of Lowest Dir.

    //printf("%s -> %d\n ", szDirName, parentDirInodeNum);    ///

    //printf("-=======- 1\n");    ///

    int inodeNum;

    if(parentDirInodeNum < 0)                               // If returned Inode is minus, it returns -1.
        return -1;

    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));     // Get the FIleSysInfo.

    GetInode(parentDirInodeNum, tempParentInode);               // Get Inode of returned Inode (Parent Dir of Lowest Dir) .

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);       // Get Lowest Dir's (path) Name.

    //printf("%d \n", parentDirInodeNum);         ///
    //printf("===== %s\n", tempPath->pathName);   ///

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        //printf("=0 \n");         ///
        
        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){   // && currentDirNameCount != dirNameCount
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.
        
            if(strcmp(tempDirEntry->name, tempPath->pathName) == 0){
                free(tempFileSysInfoBlock);
                free(fileSysInfo);
                free(tempParentInode);
                free(tempInode);
                free(tempBlockBlock);
                free(tempDirEntry);

                FreePathList();

                return -1;
            }
        }
    }

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        //printf("=0 \n");         ///
        
        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){   // && currentDirNameCount != dirNameCount
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            //printf("%s\n", tempDirEntry->name); ///

            if(strcmp(tempDirEntry->name, "") == 0){  ///   // If this DirEntry's Name has no content, do the next stuff.
                inodeNum = GetFreeInodeNum();               // Get the new free Inode Number.

                //printf("=1 \n");         ///

                DirEntry * dirEntry = malloc((size_t)sizeof(DirEntry));
                strcpy(dirEntry->name, tempPath->pathName);
                dirEntry->inodeNum = inodeNum;              // Make thd Lowest Dir into the Parent Dir.

                //printf("%d %s\n", j, dirEntry->name); ///

                //printf("=2 \n");         ///
    
                memcpy(tempBlockBlock + j * sizeof(DirEntry), dirEntry, sizeof(DirEntry));
                BufWrite(tempParentInode->dirBlockPtr[i], tempBlockBlock);

                BufSyncBlock(tempParentInode->dirBlockPtr[i]);

                //printf("=3 \n");         ///

                ///

                int blockNum = GetFreeBlockNum();       // Get thd new free Block Number.


                GetInode(inodeNum, tempInode);          // Get the new free Inode for Lowest Dir.

                //printf("=4 \n");         ///

                tempInode->allocBlocks = 1;
                tempInode->size = tempInode->allocBlocks * BLOCK_SIZE;
                tempInode->type = FILE_TYPE_DIR;
                tempInode->dirBlockPtr[0] = blockNum;

                PutInode(inodeNum, tempInode);      //  Set the new Inode for Lowest Dir.

                //printf("=5 \n");         ///

                SetInodeBitmap(inodeNum);           // Set the Inode Bitmap Num for Lowest Dir to 1.

                ///

                BufRead(blockNum, tempBlockBlock);      // Get the block for Lowest Dir.

                memset(tempBlockBlock, 0, BLOCK_SIZE);

                memcpy(dirEntry, tempBlockBlock, sizeof(DirEntry));

                //printf("=6 \n");         ///

                strcpy(dirEntry->name, ".");
                dirEntry->inodeNum = inodeNum;

                memcpy(tempBlockBlock, dirEntry, sizeof(DirEntry));     // Set the '.' Sub Dir into the Lowest Dir.

                // BufWrite(blockNum, tempBlockBlock);
                // BufRead(blockNum, tempBlockBlock);

                memcpy(dirEntry, tempBlockBlock + 1 * sizeof(DirEntry), sizeof(DirEntry));

                strcpy(dirEntry->name, "..");
                dirEntry->inodeNum = parentDirInodeNum;

                memcpy(tempBlockBlock + 1 * sizeof(DirEntry), dirEntry, sizeof(DirEntry));  // Set the '..' Sub Dir into the Lowest Dir.

                BufWrite(blockNum, tempBlockBlock);

                SetBlockBitmap(blockNum);

                BufSyncBlock(blockNum);

                //printf("=7 \n");         ///

                free(dirEntry);

                goto FINAL_MAKEDIR;     // We already made the Lowest Dir, so we have to end this function.
            }
        }

        if(i == NUM_OF_DIRECT_BLOCK_PTR - 1){
            return -1;
        }

        if(i == tempParentInode->allocBlocks - 1){  // If DirEntry is Full, make the new DirBlockPtr.
            tempParentInode->allocBlocks++;
            tempParentInode->size += BLOCK_SIZE;
            int freeBlockNum = GetFreeBlockNum();
            tempParentInode->dirBlockPtr[i + 1] = freeBlockNum;

            PutInode(parentDirInodeNum, tempParentInode);

            SetBlockBitmap(freeBlockNum);
        }
    }

    FINAL_MAKEDIR:

    //fileSysInfo->blocks++;
    //fileSysInfo->diskCapacity = fileSysInfo->blocks * BLOCK_SIZE;
    fileSysInfo->numAllocBlocks++;
    fileSysInfo->numFreeBlocks--;
    fileSysInfo->numAllocInodes++;

    memcpy(tempFileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo)); // Update the fileSysInfo.
    BufWrite(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);


    free(tempFileSysInfoBlock);
    free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    BufSyncBlock(FILESYS_INFO_BLOCK);
    //BufSync();      // Must do BufSync.

    FreePathList();

    // printf("%ld \n", sizeof(DirEntryInfo));

    // DirEntry * dirEntry = malloc((size_t)sizeof(DirEntry));
    // int blockNum = GetFreeBlockNum();
    // int inodeNum = GetFreeInodeNum();
    // strcpy(dirEntry->name, ".");
    // dirEntry->inodeNum = inodeNum;
    
    // // memcpy(tempBlock, dirEntry, sizeof(DirEntry));
    // // BufWrite(blockNum, tempBlock);
    // // SetBlockBitmap(blockNum);
    // // free(dirEntry);

    // const char* path = "/tmp/dir/ac.c";

    // GetPathsFromPath(path);

    // Path* tempPath;

    // TAILQ_FOREACH(tempPath, &pathListHead, nextPath){
    //     printf("%s ", tempPath->pathName);
    //     printf("%d \n", tempPath->pathNameLength);
    // }
    // printf("\n");

    // FreePathList();

    return inodeNum;
}


int     RemoveDir(const char* szDirName)
{
    int parentDirInodeNum = SearchDirectoryPath(szDirName);

    int inodeNum;

    if(parentDirInodeNum < 0)
        return -1;

    char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));

    GetInode(parentDirInodeNum, tempParentInode);       // Get the inode of Parent of Lowest Dir.

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            if(strcmp(tempDirEntry->name, tempPath->pathName) == 0){
                inodeNum = tempDirEntry->inodeNum;

                //char isThereAnyFilesOrDirectories = FALSE;

                Inode* dirInode = malloc((size_t)sizeof(Inode));
                DirEntry* dirEntry = malloc((size_t)sizeof(DirEntry));
                char* tempBlock = malloc((size_t)BLOCK_SIZE);

                GetInode(inodeNum, dirInode);

                for(int a = 0; a < dirInode->allocBlocks; a++){
                    BufRead(dirInode->dirBlockPtr[a], tempBlock);

                    if(a == 0){
                        for(int b = 2; b < NUM_OF_DIRENT_PER_BLOCK; b++){
                            memcpy(dirEntry, tempBlock + b * sizeof(DirEntry), sizeof(DirEntry));

                            if(strcmp(dirEntry->name, "") != 0){
                                return -1;
                            }
                        }
                    }
                    else{
                        for(int b = 0; b < NUM_OF_DIRENT_PER_BLOCK; b++){
                            memcpy(dirEntry, tempBlock + b * sizeof(DirEntry), sizeof(DirEntry));

                            if(strcmp(dirEntry->name, "") != 0){
                                return -1;
                            }
                        }
                    }
                }

                for(int a = 0; a < NUM_OF_DIRECT_BLOCK_PTR; a++){
                    if(dirInode->dirBlockPtr[a] != 0){
                        BufRead(dirInode->dirBlockPtr[a], tempBlock);
                        memset(tempBlock, 0, BLOCK_SIZE);
                        BufWrite(dirInode->dirBlockPtr[a], tempBlock);
                        BufSyncBlock(dirInode->dirBlockPtr[a]);
                        ResetBlockBitmap(dirInode->dirBlockPtr[a]);
                    }
                    else{
                        break;
                    }
                }


                dirInode->allocBlocks = 0;
                dirInode->size = 0;
                dirInode->type = 0;
                for(int a = 0; a < NUM_OF_DIRECT_BLOCK_PTR; a++){
                    dirInode->dirBlockPtr[a] = 0;
                }
                PutInode(inodeNum, dirInode);
                ResetInodeBitmap(inodeNum);

                tempDirEntry->inodeNum = 0;
                for(int a = 0; a < MAX_NAME_LEN; a++){
                    tempDirEntry->name[a] = '\0';
                }

                memcpy(tempBlockBlock + j * sizeof(DirEntry), tempDirEntry, sizeof(DirEntry));
                BufWrite(tempParentInode->dirBlockPtr[i], tempBlockBlock);

                BufSyncBlock(tempParentInode->dirBlockPtr[i]);

                ///

                DirEntry* temptempDirEntry = malloc((size_t)sizeof(DirEntry));

                int isNotEmpty = 0;
                for(int a = 0; a < NUM_OF_DIRENT_PER_BLOCK; a++){
                    memcpy(temptempDirEntry, tempBlockBlock + a * sizeof(DirEntry), sizeof(DirEntry));
                    if(strcmp(temptempDirEntry->name, "") != 0){
                        isNotEmpty = 1;
                        break;
                    }
                }

                free(temptempDirEntry);

                if(isNotEmpty == 0){
                    char* temp = malloc((size_t)BLOCK_SIZE);

                    memset(tempBlockBlock, 0, (size_t)BLOCK_SIZE);

                    BufWrite(tempParentInode->dirBlockPtr[i], tempBlockBlock);

                    ResetBlockBitmap(tempParentInode->dirBlockPtr[i]);

                    BufSyncBlock(tempParentInode->dirBlockPtr[i]);

                    fileSysInfo->numAllocBlocks--;
                    fileSysInfo->numFreeBlocks++;
                    
                    tempParentInode->dirBlockPtr[i] = 0;
                    tempParentInode->allocBlocks--;
                    tempParentInode->size = tempParentInode->allocBlocks * BLOCK_SIZE;
                    PutInode(parentDirInodeNum, tempParentInode);

                    goto FINAL_REMOVEDIR;
                }

                ///

                goto FINAL_REMOVEDIR;
            }
        }

    }

    FINAL_REMOVEDIR:

    //fileSysInfo->blocks--;
    //fileSysInfo->diskCapacity = fileSysInfo->blocks * BLOCK_SIZE;
    fileSysInfo->numAllocBlocks--;
    fileSysInfo->numFreeBlocks++;
    fileSysInfo->numAllocInodes--;

    memcpy(tempFileSysInfoBlock, fileSysInfo, sizeof(FileSysInfo)); // Update the fileSysInfo.
    BufWrite(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);

    free(tempFileSysInfoBlock);
    free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    BufSyncBlock(FILESYS_INFO_BLOCK);
    //BufSync();      // Must do BufSync.

    FreePathList();

    return inodeNum;
}

int   EnumerateDirStatus(const char* szDirName, DirEntryInfo* pDirEntry, int dirEntrys)
{
    int count = 0;

    int parentDirInodeNum = SearchDirectoryPath(szDirName);

    int inodeNum;

    if(parentDirInodeNum < 0)
        return -1;

    //char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    // FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    // BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    // memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));

    GetInode(parentDirInodeNum, tempParentInode);       // Get the inode of Parent of Lowest Dir.

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            if(strcmp(tempDirEntry->name, tempPath->pathName) == 0){
                inodeNum = tempDirEntry->inodeNum;

                Inode* dirInode = malloc((size_t)sizeof(Inode));
                DirEntry* dirEntry = malloc((size_t)sizeof(DirEntry));
                char* tempBlock = malloc((size_t)BLOCK_SIZE);

                GetInode(inodeNum, dirInode);

                for(int a = 0; a < dirInode->allocBlocks; a++){
                    BufRead(dirInode->dirBlockPtr[a], tempBlock);

                    for(int b = 0; b < NUM_OF_DIRENT_PER_BLOCK; b++){
                        memcpy(dirEntry, tempBlock + b * sizeof(DirEntry), sizeof(DirEntry));

                        if(strcmp(dirEntry->name, "") != 0){
                            DirEntryInfo* tempDirEntryInfo = malloc((size_t)sizeof(DirEntryInfo));

                            tempDirEntryInfo->inodeNum = dirEntry->inodeNum;
                            strcpy(tempDirEntryInfo->name, dirEntry->name);

                            Inode* tempInodeOfTheDirSub = malloc((size_t)sizeof(Inode));
                            GetInode(dirEntry->inodeNum, tempInodeOfTheDirSub);
                            tempDirEntryInfo->type = tempInodeOfTheDirSub->type;
                            free(tempInodeOfTheDirSub);

                            memcpy(&(pDirEntry[count]), tempDirEntryInfo, sizeof(DirEntryInfo));

                            free(tempDirEntryInfo);

                            count++;
                        }
                    }
                }

                free(dirInode);
                free(dirEntry);

                goto FINAL_ENUMERATEDIRSTATUS;
            }
        }
    }

    FINAL_ENUMERATEDIRSTATUS:

    // free(tempFileSysInfoBlock);
    // free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    FreePathList();

    return count;
}


void    CreateFileSystem()
{
    DevCreateDisk();
    BufInit();

    FileSysInit();

    for(int i = 0; i < INODELIST_BLOCK_FIRST + INODELIST_BLOCKS; i++)
        SetBlockBitmap(i);

    char * tempBlock = malloc((size_t)BLOCK_SIZE);

    // Create Root Directory First

    DirEntry * dirEntry = malloc((size_t)sizeof(DirEntry));     // Make '.' Sub Dir into the Root Dir.
    int blockNum = GetFreeBlockNum();
    int inodeNum = GetFreeInodeNum();
    strcpy(dirEntry->name, ".");
    dirEntry->inodeNum = inodeNum;
    
    memcpy(tempBlock, dirEntry, sizeof(DirEntry));
    BufWrite(blockNum, tempBlock);
    SetBlockBitmap(blockNum);
    free(dirEntry);

    Inode * inode = malloc((size_t)sizeof(Inode));      // Initialize the root Inode.
    GetInode(inodeNum, inode);
    inode->allocBlocks = 1;
    inode->size = inode->allocBlocks * BLOCK_SIZE;
    inode->type = FILE_TYPE_DIR;
    inode->dirBlockPtr[0] = INODELIST_BLOCK_FIRST + INODELIST_BLOCKS;
    PutInode(inodeNum, inode);
    SetInodeBitmap(inodeNum);
    free(inode);


    // Initialize File System information

    BufRead(FILESYS_INFO_BLOCK, tempBlock);
    FileSysInfo * initFileSysInfo = malloc((size_t)sizeof(FileSysInfo));

    //FileSysInfo* fileSysInfo = initFileSysInfo;
    //printf("<==> %d %d %d %d %d %d %d %d %d %d\n", fileSysInfo->blocks, fileSysInfo->rootInodeNum, fileSysInfo->diskCapacity, fileSysInfo->numAllocBlocks, fileSysInfo->numFreeBlocks, fileSysInfo->numAllocInodes, fileSysInfo->blockBitmapBlock, fileSysInfo->inodeBitmapBlock, fileSysInfo->inodeListBlock, fileSysInfo->dataRegionBlock);
    ///

    initFileSysInfo->blocks = 512;     // Initialize the fileSysInfo.
    initFileSysInfo->rootInodeNum = 0;
    initFileSysInfo->diskCapacity = initFileSysInfo->blocks * 512;
    initFileSysInfo->numAllocBlocks = INODELIST_BLOCK_FIRST + INODELIST_BLOCKS + 1;
    initFileSysInfo->numFreeBlocks = 512 - initFileSysInfo->numAllocBlocks;
    initFileSysInfo->numAllocInodes = 1;
    initFileSysInfo->blockBitmapBlock = BLOCK_BITMAP_BLOCK_NUM;
    initFileSysInfo->inodeBitmapBlock = INODE_BITMAP_BLOCK_NUM;
    initFileSysInfo->inodeListBlock = INODELIST_BLOCK_FIRST;
    initFileSysInfo->dataRegionBlock = INODELIST_BLOCK_FIRST + INODELIST_BLOCKS;

    // FileSysInfo* fileSysInfo = initFileSysInfo;
    // printf("<==> %d %d %d %d %d %d %d %d %d %d\n", fileSysInfo->blocks, fileSysInfo->rootInodeNum, fileSysInfo->diskCapacity, fileSysInfo->numAllocBlocks, fileSysInfo->numFreeBlocks, fileSysInfo->numAllocInodes, fileSysInfo->blockBitmapBlock, fileSysInfo->inodeBitmapBlock, fileSysInfo->inodeListBlock, fileSysInfo->dataRegionBlock);


    memcpy(tempBlock, initFileSysInfo, sizeof(FileSysInfo));
    BufWrite(FILESYS_INFO_BLOCK, tempBlock);
    free(initFileSysInfo);

    free(tempBlock);

    BufSync();      // Must do BufSync.

    return;
}


void    OpenFileSystem()
{
    BufInit();
    // DevCloseDisk();
    DevOpenDisk();


    //char * tempBlock = malloc((size_t)BLOCK_SIZE);

    // for(int i = 511; i >= 511 - MAX_BUF_NUM; i--){
    //     BufRead(i, tempBlock);
        
    //     //BufWrite(i, tempBlock);
    // }

    // // BufSync();

    // // BufRead(FILESYS_INFO_BLOCK, tempBlock);

    // // for(int i = 0; i < BLOCK_SIZE; i++){
    // //     printf("%d ", tempBlock[i]);
    // // }
    // // printf("\n");

    // for(int i = 0; i < MAX_BUF_NUM; i++){
    //     BufRead(i, tempBlock);
    //     for(int j = 0; j < BLOCK_SIZE; j++){
    //         printf("%d ", tempBlock[j]);
    //     }
    //     printf("\n");
    //     //BufWrite(i, tempBlock);
    // }

    // BufRead(0, tempBlock);
    
    // FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));

    // memcpy(fileSysInfo, tempBlock, sizeof(FileSysInfo));

    // //FileSysInfo* fileSysInfo = initFileSysInfo;
    // printf("<==> %d %d %d %d %d %d %d %d %d %d\n", fileSysInfo->blocks, fileSysInfo->rootInodeNum, fileSysInfo->diskCapacity, fileSysInfo->numAllocBlocks, fileSysInfo->numFreeBlocks, fileSysInfo->numAllocInodes, fileSysInfo->blockBitmapBlock, fileSysInfo->inodeBitmapBlock, fileSysInfo->inodeListBlock, fileSysInfo->dataRegionBlock);
    
    // free(fileSysInfo);
    // free(tempBlock);

    //BufSync();

    return;
}


void     CloseFileSystem()
{
    BufSync();

    // Buf* tempBuf;

    // for(int i = 0; i < MAX_BUFLIST_NUM; i++){               // Buffer 들 메모리 해제
    //     CIRCLEQ_FOREACH(tempBuf, &(bufList[i]), llist){
    //         free(tempBuf->pMem);
    //     }

    // }

    // CIRCLEQ_FOREACH(tempBuf, &(stateList[BUF_CLEAN_LIST]), llist){
    //     free(tempBuf->pMem);
    // }

    // CIRCLEQ_FOREACH(tempBuf, &(stateList[BUF_DIRTY_LIST]), llist){
    //     free(tempBuf->pMem);
    // }

    // CIRCLEQ_FOREACH(tempBuf, &lruListHead, llist){
    //     free(tempBuf->pMem);
    // }

    // CIRCLEQ_FOREACH(tempBuf, &freeListHead, llist){
    //     free(tempBuf->pMem);
    // }

    DevCloseDisk();

    return;
}


int      GetFileStatus(const char* szPathName, FileStatus* pStatus)
{
    int parentDirInodeNum = SearchDirectoryPath(szPathName);

    int inodeNum;

    if(parentDirInodeNum < 0)
        return -1;

    //char* tempFileSysInfoBlock = malloc((size_t)BLOCK_SIZE);
    Inode* tempParentInode = malloc((size_t)sizeof(Inode));
    Inode* tempInode = malloc((size_t)sizeof(Inode));
    char* tempBlockBlock = malloc((size_t)BLOCK_SIZE);
    DirEntry* tempDirEntry = malloc((size_t)sizeof(DirEntry));

    // FileSysInfo * fileSysInfo = malloc((size_t)sizeof(FileSysInfo));
    // BufRead(FILESYS_INFO_BLOCK, tempFileSysInfoBlock);
    // memcpy(fileSysInfo, tempFileSysInfoBlock, sizeof(FileSysInfo));

    GetInode(parentDirInodeNum, tempParentInode);       // Get the inode of Parent of Lowest Dir.

    Path* tempPath = TAILQ_LAST(&pathListHead, pathList);

    for(int i = 0; i < tempParentInode->allocBlocks; i++){
        BufRead(tempParentInode->dirBlockPtr[i], tempBlockBlock);       // Read Parent Dir Inode's dirBLockPtr[i] and copy to tempBlockBlock.

        for(int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++){
            memcpy(tempDirEntry, tempBlockBlock + j * sizeof(DirEntry), sizeof(DirEntry));  // Get a DirEntry of Parent Dir.

            if(strcmp(tempDirEntry->name, tempPath->pathName) == 0){        // Found the Lowest File or Dir.
                inodeNum = tempDirEntry->inodeNum;

                Inode* dirInode = malloc((size_t)sizeof(Inode));
                //DirEntry* dirEntry = malloc((size_t)sizeof(DirEntry));
                //char* tempBlock = malloc((size_t)BLOCK_SIZE);

                GetInode(inodeNum, dirInode);           // Get Inode of Lowest File or Dir.

                FileStatus* fileStatus = malloc((size_t)sizeof(FileStatus));

                fileStatus->allocBlocks = dirInode->allocBlocks;
                fileStatus->size = dirInode->size;
                fileStatus->type = dirInode->type;
                
                for(int a = 0; a < NUM_OF_DIRECT_BLOCK_PTR; a++){
                    fileStatus->dirBlockPtr[a] = dirInode->dirBlockPtr[a];
                }
                
                memcpy(pStatus, fileStatus, sizeof(FileStatus));

                free(dirInode);
                free(fileStatus);
                
                goto FINAL_GETFILESTATUS;
            }
        }
    }

    FINAL_GETFILESTATUS:

    // free(tempFileSysInfoBlock);
    // free(fileSysInfo);
    free(tempParentInode);
    free(tempInode);
    free(tempBlockBlock);
    free(tempDirEntry);

    FreePathList();

    return inodeNum;
}


void	Sync()
{
    BufSync();

    return;
}
