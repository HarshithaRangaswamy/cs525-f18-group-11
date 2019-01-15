#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include<string.h>


FILE *page;

extern void initStorageManager (void){
	page = NULL;
}


extern RC createPageFile (char *fileName) {

page = fopen(fileName,"w+");
	if(fileName == NULL){
		return RC_FILE_NOT_FOUND;
	}	
	if(page==NULL){
		fclose(page);
		printf("Page Creation Failed");
	}
	
	char *pg;
	pg= (char *) calloc (PAGE_SIZE , sizeof(char));
	
	size_t wc= fwrite(pg, sizeof(char) , PAGE_SIZE, page);
		
	if (wc == PAGE_SIZE){
        fclose(page);
        return RC_OK;
    }
	
    else{
        fclose(page);
        free(pg);
        printf("Page Creation Failed");
		}
    return RC_ERROR;

}

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle){
	page = fopen(fileName,"r");
	if(page == NULL){
		return RC_FILE_NOT_FOUND;
	}else{
		fHandle->fileName = fileName;
		fseek(page,0L,SEEK_END);
		int size = ftell(page);
		rewind(page);
		size = size/ PAGE_SIZE;
		fHandle->totalNumPages = size;
		fHandle->mgmtInfo = page;
		fHandle->curPagePos = 0;
		return RC_OK;
	}

}

extern RC closePageFile(SM_FileHandle *fHandle){
	 
	 int value = fclose((*fHandle).mgmtInfo);
	 if(value == EOF)
	 {
		 return RC_ERROR;
	 }
	 return RC_OK;
	 
 }


extern RC destroyPageFile(char *fileName){
	
	FILE *pg = fopen(fileName , "r");

	if(pg==NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	int remfile= 1;
	remfile  = remove(fileName);
	if(remfile == 0){
		return RC_OK;
	}
	else{
		return RC_FILE_REMOVE_ERROR;
	}
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){


	if(fHandle->mgmtInfo == NULL){
		return RC_FILE_NOT_FOUND;
	}
	if(pageNum > fHandle->totalNumPages || pageNum < 0 ){

		return RC_READ_NON_EXISTING_PAGE;
	}

	int value = fseek(fHandle->mgmtInfo, pageNum*PAGE_SIZE,SEEK_SET);
	if(value != 0){
		return RC_ERROR;
	}
	fread(memPage,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);
	fHandle->curPagePos = pageNum;
	return RC_OK;

}

extern int getBlockPos(SM_FileHandle *fHandle)
{
	int currPos = fHandle->curPagePos;

	if(fHandle == NULL)
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else{
		return currPos;
	}
}


extern RC readFirstBlock(SM_FileHandle *fHandle , SM_PageHandle memPage)
{
	
	if(fHandle == NULL)
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
	else{
		return readBlock(0,fHandle,memPage);
	}
  
}

 
extern RC readLastBlock(SM_FileHandle *fHandle , SM_PageHandle memPage)
{

  int tp = fHandle->totalNumPages -1;
 
  if(fHandle == NULL)
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
  else
  {
	  return readBlock(tp, fHandle, memPage);
  }
  
 }

extern RC readPreviousBlock(SM_FileHandle *fHandle , SM_PageHandle memPage)
{
  int prev_block = getBlockPos(fHandle)-1;
 if(fHandle == NULL)
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
  else
  {
	  return readBlock(prev_block,fHandle,memPage);
  }

}


 extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
 {
	
	int curr_block=fHandle->curPagePos;
	if(fHandle == NULL)
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
  else
  {
	  return readBlock(curr_block,fHandle,memPage);
  }
	
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

  int currentPage = getBlockPos(fHandle);
  int nextPage = currentPage+1;
  readBlock(nextPage,fHandle,memPage);
  return RC_OK;
}


extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	if(pageNum <0 )
	{
		return RC_WRITE_FAILED;
	}
	
	page = fopen(fHandle->fileName,"r+");	
	if(page == NULL)
	{
    return RC_FILE_NOT_FOUND;
	}
		
	int value = fseek(page, pageNum*PAGE_SIZE,SEEK_SET);
	if(value != 0){
		return RC_ERROR;
	}
	else
	{
        fwrite(memPage,sizeof(char),strlen(memPage),page);
	fHandle->curPagePos = pageNum;
	fclose(page);
	return RC_OK;
  }
}


extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
		
	int curr_pos = fHandle->curPagePos;
	if(fHandle == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}	
  else if(memPage == NULL) {
	  
    return RC_NO_SUCH_PAGE_IN_BUFF;
	  
  }
  else
  {
	return writeBlock(curr_pos, fHandle, memPage);
	
  }
  
}

extern RC appendEmptyBlock (SM_FileHandle *fHandle){
  page = fopen(fHandle->fileName,"r+");
  int tp = fHandle->totalNumPages;
  fHandle->totalNumPages += 1;
  fseek(page,tp*PAGE_SIZE,SEEK_SET);
  char ch = 0 ;
  int i = 0;
  
  while(i<PAGE_SIZE){
	  fwrite(&ch,sizeof(ch),1,page);
	  i++;
  }
  
  fclose(page);
  return RC_OK;
}

extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle){

  int pagesToAdd = numberOfPages - fHandle->totalNumPages;
  int i;
  if(pagesToAdd > 0){
    for(i = 0; i < pagesToAdd; i++)
        appendEmptyBlock(fHandle);
  }
  if(fHandle->totalNumPages == numberOfPages){
    return RC_OK;
  }
  return RC_ERROR;
}
