#include <stdio.h>          // 2020203011 배주환
#include <stdlib.h>
#include "buf.h"            // gcc testcase.c buf.c disk.c -o main
#include "queue.h"
#include "disk.h"                                                                        // LRU List 에 대해 다시 이해할 필요가 있음.
#include <string.h>                                                                      // 매번 read 또는 write 할때마다 Update 하는 게 아님.

void BufInsertToBufList(Buf* pBuf, int blkno);           // Buf List 에 Insert.
void BufRemoveFromBufList(Buf* pBuf, int blkno);         // Buf List 에서 Remove.

void BufSetToCleanState(Buf* pBuf);             // 해당 Buffer 의 State 를 Clean 으로 설정.환
void BufSetToDirtyState(Buf* pBuf);             // 해당 Buffer 의 State 를 Dirty 으로 설정.
void BufSetToTailOfCleanState(Buf* pBuf);       // Clean 상태의 Buffer 을 Victim 으로 삼아 재사용해야 할 때, Clean State List 의 마지막으로 이동시킴.

Buf* BufGetOldestFromLRUList();          // LRU List 에서 가장 오랫동안 쓰지 않은 Buffer 을 반환.
void BufUpdateLRUList(Buf* pBuf);        // LRU List 를 Update 함.

Buf* BufGetNewBuffer();          // Free List 에서 아직 사용하지 않은 Buffer 을 가져옴.


struct bufList      bufList[MAX_BUFLIST_NUM];
struct stateList    stateList[MAX_BUF_STATE_NUM];
struct freeList     freeListHead;
struct lruList      lruListHead;

int diskSize = 0;

void BufInit(void)
{
    for(int i = 0; i < MAX_BUFLIST_NUM; i++)
        CIRCLEQ_INIT(&bufList[i]);

    for(int i = 0; i < MAX_BUF_STATE_NUM; i++)
        CIRCLEQ_INIT(&stateList[i]);

    CIRCLEQ_INIT(&freeListHead);

    for(int i = 0; i < MAX_BUF_NUM; i++){       // freeListHead 에 MAX_BUF_NUM 만큼의 비어있는 Buffer 들을 생성.
        Buf* tempBuf = malloc(sizeof(Buf));
        tempBuf->pMem = (char*)malloc(BLOCK_SIZE);
        memset(tempBuf->pMem, 0, BLOCK_SIZE);
        CIRCLEQ_INSERT_TAIL(&freeListHead, tempBuf, flist);
    }

    CIRCLEQ_INIT(&lruListHead);

    // char* temp = malloc(BLOCK_SIZE);
    // DevOpenDisk();
    // for(int i = 0; i < 48; i++){
    //     DevWriteBlock(i, temp);
    // }
    // DevCloseDisk();
    // free(temp);
    // DevResetDiskAccessCount();

    DevOpenDisk();

    return;
}


//================================================================================================================================//


void BufRead(int blkno, char* pData)
{
    Buf* pBuf;

    //printf("=== 00\n"); ///

    if((pBuf = BufFind(blkno)) == NULL){
        //printf("=== 01\n"); ///
        if((pBuf = BufGetNewBuffer()) == NULL){     // 현재 Buffer Cache 에 해당 Buffer 가 없고, Free List 에도 남은 Buffer 가 없는 경우.
            //printf("=== 02\n"); ///
            pBuf = BufGetOldestFromLRUList();

            //printf("=== 03\n"); ///

            if(pBuf->state == BUF_STATE_DIRTY){             // Dirty State 인 경우, 해당 Buffer 을 Disk 로 Sync 해주어야 함.
                BufSyncBlock(pBuf->blkno);
            }
            else{
                BufSetToTailOfCleanState(pBuf);
            }

            //printf("=== 04\n"); ///
            
            BufUpdateLRUList(pBuf);

            //printf("=== 05\n"); ///

            BufRemoveFromBufList(pBuf, pBuf->blkno);

            //printf("=== 06\n"); ///

            pBuf->blkno = blkno;

            BufInsertToBufList(pBuf, blkno);

            //printf("=== 07\n"); ///

            if(blkno <= diskSize){
                //DevOpenDisk();
                DevReadBlock(blkno, pBuf->pMem);
                //DevCloseDisk();
            }

            //printf("=== 08\n"); ///
        }
        else{                                       // 현재 Buffer Cache 에 해당 Buffer 가 없지만, Free List 에 남은 Buffer 가 있는 경우.
            BufUpdateLRUList(pBuf);

            //printf("=== 09\n"); ///

            BufSetToCleanState(pBuf);

            //printf("=== 10\n"); ///
            
            pBuf->blkno = blkno;

            BufInsertToBufList(pBuf, blkno);

            DevReadBlock(blkno, pBuf->pMem);        // Read 하도록 추가 ///

            //printf("=== 11\n"); ///
        }
    }
    else{                                           // 현재 Buffer Cache 에 해당 Buffer 가 있는 경우.
        BufUpdateLRUList(pBuf);

        //DevReadBlock(blkno, pBuf->pMem);        // Read 하도록 추가 ///

        //printf("=== 12\n"); ///
    }

    memcpy(pData, (char*)pBuf->pMem, BLOCK_SIZE);       // strcpy 사용시 이상한 값이 들어가는 등 에러 발생.

    //printf("=== 13\n"); ///

    // for(int i = 0; i < 512; i++){
    //     printf("%d ", *((char*)pBuf->pMem + i));
    // }
    // printf("\n");

    // for(int i = 0; i < 512; i++){
    //     printf("%d ", *(pData + i));
    // }
    // printf("\n");


    return;
}


void BufWrite(int blkno, char* pData)
{
    Buf* pBuf;
    if((pBuf = BufFind(blkno)) == NULL){
        if((pBuf = BufGetNewBuffer()) == NULL){     // 현재 Buffer Cache 에 해당 Buffer 가 없고, Free List 에도 남은 Buffer 가 없는 경우.
            pBuf = BufGetOldestFromLRUList();

            if(pBuf->state == BUF_STATE_DIRTY){             // Dirty State 인 경우, 해당 Buffer 을 Disk 로 Sync 해주어야 함.
                BufSyncBlock(pBuf->blkno);
            }
            else{
                BufSetToTailOfCleanState(pBuf);
            }

            BufUpdateLRUList(pBuf);

            BufRemoveFromBufList(pBuf, pBuf->blkno);

            pBuf->blkno = blkno;

            BufInsertToBufList(pBuf, blkno);

            if(blkno <= diskSize){
                //DevOpenDisk();
                DevReadBlock(blkno, pBuf->pMem);
                //DevCloseDisk();
            }
        }
        else{                                       // 현재 Buffer Cache 에 해당 Buffer 가 없지만, Free List 에 남은 Buffer 가 있는 경우.
            BufUpdateLRUList(pBuf);

            BufSetToCleanState(pBuf);

            pBuf->blkno = blkno;

            BufInsertToBufList(pBuf, blkno);

            DevReadBlock(blkno, pBuf->pMem);        // Read 하도록 추가 ///
        }
    }
    else{                                           // 현재 Buffer Cache 에 해당 Buffer 가 있는 경우.
        BufUpdateLRUList(pBuf);

        //DevReadBlock(blkno, pBuf->pMem);        // Read 하도록 추가 ///
    }

    memcpy(pBuf->pMem, (void*)pData, BLOCK_SIZE);       // strcpy 사용시 이상한 값이 들어가는 등 에러 발생.

    //printf("%s\n", (char*)pBuf->pMem);

    if(pBuf->state == BUF_STATE_CLEAN){
        BufSetToDirtyState(pBuf);
    }

    return;
}


//================================================================================================================================//


void BufSync(void)
{
    Buf* pBuf;

    while(!CIRCLEQ_EMPTY(&stateList[BUF_DIRTY_LIST])){
        pBuf = CIRCLEQ_FIRST(&stateList[BUF_DIRTY_LIST]);

        BufSyncBlock(pBuf->blkno);
    }

    return;
}

void BufSyncBlock(int blkno)
{
    Buf* pBuf = BufFind(blkno);

    //DevOpenDisk();
    DevWriteBlock(blkno, (char*)pBuf->pMem);
    //DevCloseDisk();

    diskSize = (blkno > diskSize) ? blkno : diskSize;   // Disk 의 크기 갱신 (Block 기준)

    BufSetToCleanState(pBuf);
    //BufUpdateLRUList(pBuf);

    return;
}


//================================================================================================================================//


int GetBufInfoInBufferList(int index, Buf* ppBufInfo[], int numBuf)
{
    Buf* pBuf;
    Buf* pBufInfo[numBuf];

    int count = 0;

    CIRCLEQ_FOREACH(pBuf, &bufList[index], blist){
        pBufInfo[count] = pBuf;
        count++;
    }

    for(int i = 0; i < count; i++){
        ppBufInfo[i] = pBufInfo[i];
    }

    return count;
}


int GetBufInfoInStateList(BufStateList listnum, Buf* ppBufInfo[], int numBuf)
{
    Buf* pBuf;
    Buf* pBufInfo[numBuf];

    int count = 0;

    CIRCLEQ_FOREACH(pBuf, &stateList[listnum], slist){
        pBufInfo[count] = pBuf;
        count++;
    }
    
    for(int i = 0; i < count; i++){
        ppBufInfo[i] = pBufInfo[i];
    }

    return count;
}

int GetBufInfoInLruList(Buf* ppBufInfo[], int numBuf)
{
    Buf* pBuf;
    Buf* pBufInfo[numBuf];

    int count = 0;

    CIRCLEQ_FOREACH(pBuf, &lruListHead, llist){
        pBufInfo[count] = pBuf;
        count++;
    }
    
    for(int i = 0; i < count; i++){
        ppBufInfo[i] = pBufInfo[i];
    }

    return count;
}


//================================================================================================================================//

//================================================================================================================================//

//================================================================================================================================//
//  Buf List 함수들

void BufInsertToBufList(Buf* pBuf, int blkno)           // Buf List 에 Insert.
{
    int bufListNum = blkno % MAX_BUFLIST_NUM;

    CIRCLEQ_INSERT_HEAD(&bufList[bufListNum], pBuf, blist);

    return;
}

void BufRemoveFromBufList(Buf* pBuf, int blkno)         // Buf List 에서 Remove.
{
    int bufListNum = blkno % MAX_BUFLIST_NUM;

    CIRCLEQ_REMOVE(&bufList[bufListNum], pBuf, blist);

    return;
}

Buf* BufFind(int blkno)         // 해당 blkno 의 Buffer 를 찾음.
{
    Buf* lit;
    int bufListNum = blkno % MAX_BUFLIST_NUM;

    CIRCLEQ_FOREACH(lit, &bufList[bufListNum], blist){  // bufList[blkno % MAX_BUFLIST_NUM] 에서 blkno 의 Buffer 찾기.
        if(lit->blkno == blkno){
            return lit;
        }
    }

    return NULL;
}


//================================================================================================================================//

//================================================================================================================================//
//  State List 함수들

void BufSetToCleanState(Buf* pBuf)      // 해당 Buffer 의 State 를 Clean 으로 설정.
{
    if(CIRCLEQ_PREV(pBuf, slist) != NULL || CIRCLEQ_NEXT(pBuf, slist) != NULL){
        CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], pBuf, slist);
    }

    CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], pBuf, slist);
    pBuf->state = BUF_STATE_CLEAN;

    return;
}

void BufSetToDirtyState(Buf* pBuf)      // 해당 Buffer 의 State 를 Dirty 으로 설정.
{
    if(CIRCLEQ_PREV(pBuf, slist) != NULL || CIRCLEQ_NEXT(pBuf, slist) != NULL){
        CIRCLEQ_REMOVE(&stateList[BUF_CLEAN_LIST], pBuf, slist);
    }

    CIRCLEQ_INSERT_TAIL(&stateList[BUF_DIRTY_LIST], pBuf, slist);
    pBuf->state = BUF_STATE_DIRTY;

    return;
}

void BufSetToTailOfCleanState(Buf* pBuf)
{
    CIRCLEQ_REMOVE(&stateList[BUF_CLEAN_LIST], pBuf, slist);

    CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], pBuf, slist);
    pBuf->state = BUF_STATE_CLEAN;

    return;
}


//================================================================================================================================//

//================================================================================================================================//
//  LRU List 함수들


Buf* BufGetOldestFromLRUList()          // LRU List 에서 가장 오랫동안 쓰지 않은 Buffer 을 반환.
{
    Buf* pBuf;

    if(CIRCLEQ_EMPTY(&lruListHead)){
        return NULL;
    }

    pBuf = CIRCLEQ_FIRST(&lruListHead);
    
    return pBuf;
}


void BufUpdateLRUList(Buf* pBuf)        // LRU List 를 Update 함.
{
    if(CIRCLEQ_PREV(pBuf, llist) == NULL && CIRCLEQ_NEXT(pBuf, llist) == NULL){     // LRU List 에 들어있지 않았던 Buffer 인 경우.
        CIRCLEQ_INSERT_TAIL(&lruListHead, pBuf, llist);
        return;
    }
    
    CIRCLEQ_REMOVE(&lruListHead, pBuf, llist);

    CIRCLEQ_INSERT_TAIL(&lruListHead, pBuf, llist);

    return;
}


//================================================================================================================================//

//================================================================================================================================//
//  Free List 함수들

Buf* BufGetNewBuffer()          // Free List 에서 아직 사용하지 않은 Buffer 을 가져옴.
{
    if(CIRCLEQ_EMPTY(&freeListHead)){
        return NULL;
    }
    else{
        Buf* lastBufOfFreeList;
        lastBufOfFreeList = CIRCLEQ_LAST(&freeListHead);
        CIRCLEQ_REMOVE(&freeListHead, lastBufOfFreeList, flist);

        return lastBufOfFreeList;
    }
}


//================================================================================================================================//
