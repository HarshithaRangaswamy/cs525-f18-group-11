#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"

int buffsize= 0;
int maxbuffsize;
int lineSize;
int isBuffMax= 0;
int currLineSize;
int writeCnt = 0;
int readCnt = 0;

typedef struct Page {
		int pstats;
		PageNumber PageNumber;
        void *lPoint ;
        int stat;
        int crud;
        int fcnt;
        SM_PageHandle data;
} Page; 

typedef struct Line {
		Page *framePointer;
        int pNo;
        int cnt;
        int pos;
        
} Line;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData)
{

        currLineSize = 0;
		lineSize =numPages;
        writeCnt= 0;
		readCnt =0;
        (*bm).pageFile = (char *)pageFileName;
		(*bm).numPages = numPages;
        (*bm).strategy = strategy;
		maxbuffsize = numPages;
        Page *pageFrame = malloc(sizeof(Page) * numPages);
        Line *lineFrame = malloc(sizeof(Line) * numPages);
		SM_PageHandle size=malloc(PAGE_SIZE);
        int frm = 0,tot=-1,i=0,j=0;
       while(i<maxbuffsize) 
	   {		
                pageFrame[i].data = size;
				pageFrame[i].PageNumber = tot;
                pageFrame[i].crud = frm;
				pageFrame[i].fcnt = frm;
                pageFrame[i].pstats = frm;
                pageFrame[i].stat = frm;
            i++;  
		}
		int pnt = 0;
		while(j<maxbuffsize) 
		{

			lineFrame[j].framePointer = NULL;
			lineFrame[j].cnt = pnt;
			lineFrame[j].pos = pnt;
			lineFrame[j].pNo = -1;
			j++;
		}

        pageFrame[0].lPoint = lineFrame;
        (*bm).mgmtData = pageFrame;
        return RC_OK;
}
Line *q;


Page * FIFO_pageFramePointer(BM_BufferPool *const bm,BM_PageHandle *const page)
{
        
		Page *pgo= (*bm).mgmtData;
        Page *FIFO_pframe = pgo;
		int i;
        for(i = 0; i < maxbuffsize; i++)
		{			
                int fifo = ((*page).pageNum == FIFO_pframe[i].PageNumber)?1:0;
				if(fifo==1) 
				{
                        return &FIFO_pframe[i];
                        break;
						
                }
				
        }
        return NULL;
}

Page * LRU_pageFramePointer(BM_BufferPool *const bm,BM_PageHandle *const page)
{
        
		Page *pgo= (*bm).mgmtData;
        Page *LRU_pframe = pgo;
		int i;
        for(i = 0; i < maxbuffsize; i++)
		{			
                int lru =((*page).pageNum == LRU_pframe[i].PageNumber)?1:0;
				if(lru==1) 
				{
                        return &LRU_pframe[i];
                        break;	
                }
				
        }
        return NULL;
}

int maxQueue(Line *q){
        int max = -1;
	int i = 0;
        while(i < currLineSize) 
		{
			max=((q[i].pos > max)?q[i].pos:max);	
        }
 	i++;
        return max;
}

void LRU(BM_BufferPool *const bm, BM_PageHandle *const page,int pageNum)
{
	int var=pageNum;
	Page *pgo= (*bm).mgmtData;
	Page *pf = pgo;
	Line *point=pf[0].lPoint;
	Line *ln = point;
	int i=0;
	while(i <currLineSize) 
	{
			bool o = (pf[i].PageNumber == var)?TRUE:FALSE;
			if(o == TRUE) {
					int loc=1;
					ln[i].pos = loc;

					pf[i].fcnt++;
					int j=0;
					while(j<currLineSize) 
					{
							if(j != i)
									ln[j].pos++;
								j+=1;
					}
					(*page).data  = pf[i].data;
					(*page).pageNum = pf[i].PageNumber;
					return;
			}
			i+=1;
	 }


	if(currLineSize < (*bm).numPages) 
	{        			
		int i =0;
		int locale=1;
		switch (currLineSize){
				case '0' : 
				currLineSize+=1;
						   ln[0].pos = locale;
						   ln[0].framePointer = &pf[0]; 
						   return;
						   break;
				default: 
						 while( i<maxbuffsize) 
						 {
							if(ln[i].framePointer == NULL) 
							{
									ln[i].pos = 1;
									int j=0;
									while(j<maxbuffsize) 
									{
											if(j != i)
												ln[j].pos +=1;
												j++;
									}
									ln[i].framePointer = LRU_pageFramePointer(bm,page);
									currLineSize+=1;
									return;
							}
							i++;
					}
					
			}	
			
	}
	else if(currLineSize == lineSize ) 
	{
        int i;
		for(i =0; i<currLineSize; i++) 
		{
				
			bool g=ln[i].pos == maxQueue(ln)?TRUE:FALSE;
			if(g==TRUE)
			{									
				bool v=pf[i].fcnt == 0?TRUE:FALSE;
				if(v==TRUE) {
						ln[i].pos = 1;
						int j=0;
						while(j<currLineSize) 
						{
							if(j != i)
							ln[j].pos++;
							j++;
						}

						SM_FileHandle fh;
						if(pf[i].crud ==1) {
								openPageFile((*bm).pageFile, &fh);
								ensureCapacity(pf[i].PageNumber,&fh);
								writeBlock(pf[i].PageNumber, &fh, pf[i].data);
								writeCnt++;
						}

						int num =1,num2=0;
			
						pf[i].pstats = num;
						pf[i].PageNumber = var;
						pf[i].stat = num;
						pf[i].crud = num2;
						pf[i].fcnt = num2;
						openPageFile((*bm).pageFile,&fh);
						ensureCapacity(pf[i].PageNumber, &fh);
						readBlock(pf[i].PageNumber,&fh,pf[i].data);
						readCnt+=1;
						(*page).data  = pf[i].data;
						(*page).pageNum = pf[i].PageNumber;
						return;
				}
			}
		}
		
		for(i =0; i<currLineSize; i++) 
		{
			int temp;
			temp = maxQueue(ln)-1;
			if(ln[i].pos == temp && pf[i].fcnt == 0) 
			{
				ln[i].pos = 1;
				int j = 0;
				while(j<currLineSize)
				{
					if(j != i)
					ln[j].pos++;
					j++;
				}
				SM_FileHandle fh;
				bool p=pf[i].crud ==1?TRUE:FALSE;
				if(p==TRUE) {
						openPageFile((*bm).pageFile, &fh);
						ensureCapacity(pf[i].PageNumber,&fh);
						writeBlock(pf[i].PageNumber, &fh, pf[i].data);
						writeCnt+=1;
				}

				int val =1,val2=0;
				pf[i].pstats = val;
				pf[i].PageNumber = var;
				pf[i].stat = val;
				pf[i].crud = val2;
				pf[i].fcnt = val2;

				openPageFile((*bm).pageFile,&fh);
				ensureCapacity(pf[i].PageNumber, &fh);
				readBlock(pf[i].PageNumber,&fh,pf[i].data);
				readCnt+=1;
				(*page).data  = pf[i].data;
				(*page).pageNum = pf[i].PageNumber;
				return;
			}
		}
	}
}

void FIFO(BM_BufferPool *const bm, BM_PageHandle *const page,int pageNum)
{
	int pno=pageNum;
	Page *pgo= (*bm).mgmtData;
	Page *pFrms = pgo;
	Line *point=pFrms[0].lPoint;
	Line *queue = point;

	if(currLineSize < (*bm).numPages) 
	{
		int i =0;
		switch (currLineSize)
		{
			case '0' : queue[0].pos = 1;
					   queue[0].framePointer = &pFrms[0];
					   queue[0].pNo = pno;
					   currLineSize++;
					   return;
					   break;
			
			default: 
					 while( i<maxbuffsize) 
					 {
						if(queue[i].framePointer == NULL) 
						{
							queue[i].pos = 1;
							queue[i].pNo = pno;
							int j=0;
							while(j<maxbuffsize) 
							{
									if(j != i)
											queue[j].pos +=1;
										j++;
							}
								queue[i].framePointer = FIFO_pageFramePointer(bm,page);
								currLineSize+=1;
								return;
						}
						i++;
					}
					
			}
			
	}
	else if(currLineSize == lineSize ) 
	{
		int i = 0, j = 0;
		while(i<currLineSize) 
		{
			bool w = (pFrms[i].PageNumber == pageNum)?TRUE:FALSE;	
			if(w == TRUE) 
			{
				pFrms[i].fcnt += 1;
				return;
			}
			bool c = (queue[i].pos == currLineSize)?TRUE:FALSE;
			if(c == TRUE) 
			{
				if(pFrms[i].fcnt == 0) 
				{
				   queue[i].pos = 1;
					
				   while(j<currLineSize) 
				   {
						if(j != i)
						queue[j].pos++;
						j+=1;
					}
					SM_FileHandle fh;
					int x=1;
					if(pFrms[i].crud ==x) {
							openPageFile((*bm).pageFile, &fh);
							ensureCapacity(pFrms[i].PageNumber,&fh);
							writeBlock(pFrms[i].PageNumber, &fh, pFrms[i].data);
							writeCnt+=1;
					}
					int intit=1,inits=0;
					pFrms[i].pstats = intit;
					pFrms[i].PageNumber = pno;
					pFrms[i].stat = intit;
					pFrms[i].crud = inits;
					pFrms[i].fcnt = inits;
					openPageFile((*bm).pageFile,&fh);
					ensureCapacity(pFrms[i].PageNumber, &fh);
					readBlock(pFrms[i].PageNumber,&fh,pFrms[i].data);
					readCnt+=1;
					(*page).data  = pFrms[i].data;
					(*page).pageNum = pFrms[i].PageNumber;
					return;
					}
				}
			i++;
		}

		for(i =0; i<currLineSize; i++) 
		{
			int temp = currLineSize-1;
			int val=1,val2=0;
			if(queue[i].pos == temp ) 
			{
				if(pFrms[i].fcnt == val2)
				{
					queue[i].pos = val;
					for(j=0; j<currLineSize; j++) 
					{
						if(j != i)
						   queue[j].pos++;

					}

					SM_FileHandle fh;
					if(pFrms[i].crud ==1) 
					{
						openPageFile((*bm).pageFile, &fh);
						ensureCapacity(pFrms[i].PageNumber,&fh);
						writeBlock(pFrms[i].PageNumber, &fh, pFrms[i].data);
						writeCnt++;
					}

					int val=1,val2=0;
					pFrms[i].pstats = val;
					pFrms[i].PageNumber = pno;
					pFrms[i].stat = val;
					pFrms[i].crud = val2;
					pFrms[i].fcnt = val2;
					openPageFile((*bm).pageFile,&fh);
					ensureCapacity(pFrms[i].PageNumber, &fh);
					readBlock(pFrms[i].PageNumber,&fh,pFrms[i].data);
					readCnt+=1;
					(*page).data  = pFrms[i].data;
					(*page).pageNum = pFrms[i].PageNumber;
				}   
				return;
			}
		}
			
	}
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pagenum)
{
		int pno=pagenum;
        int size = (*bm).numPages;
        Page *pgo= (*bm).mgmtData;
        Page *pf = pgo;
        int i;
        if(buffsize == maxbuffsize) 
		{
			isBuffMax = 1;
			(*bm).strategy == RS_FIFO? FIFO(bm,page,pno):LRU(bm,page,pno);
			return RC_OK;
        }
        else if(isBuffMax == 0) 
		{	
				for(i = 0; i < buffsize; i++)
				{
                  if(pf[i].PageNumber == pno)
				  {
                    (*page).data  = pf[i].data;
                    (*page).pageNum= pno;
                    return RC_OK;
                  }
                }
				int p=0;
                if(buffsize == p) {
					
				(*bm).strategy == RS_FIFO?FIFO(bm,page,pno):LRU(bm,page,pno);	
                      
                        SM_FileHandle fh;
						int test=1;
                        pf[0].pstats = test;
                        pf[0].PageNumber = pno;
                        pf[0].stat = test;
                        pf[0].fcnt++;
                        openPageFile((*bm).pageFile,&fh);
                        ensureCapacity(pno, &fh);
                        readBlock(pno,&fh,pf[0].data);
                        readCnt++;
                        (*page).data  = pf[0].data;
                        (*page).pageNum = pf[0].PageNumber;
                        buffsize++;
                        return RC_OK;
                }
                else
				{
					int i=1;
					int statstatus=0;
                        while(i<size) 
						{		
                                if(pf[i].stat == statstatus) 
								{
                                        SM_FileHandle fh;
                                        if((*bm).strategy == RS_FIFO)
                                                FIFO(bm,page,pno);
                                        else
                                                LRU(bm,page,pno);
										pf[i].stat = 1;
                                        pf[i].pstats = 1;
                                        pf[i].PageNumber = pno;
                                        pf[i].fcnt++;
                                        openPageFile((*bm).pageFile,&fh);
                                        ensureCapacity(pno, &fh);
                                        readBlock(pno,&fh,pf[i].data);
                                        readCnt++;
                                        (*page).data  = pf[i].data;
                                        (*page).pageNum = pno;
                                        buffsize++;
                                        return RC_OK;
                                }
								
						i++;
                        }
                        return RC_OK;
                }
        } 
        return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){

       Page *pgo= (*bm).mgmtData;
        Page *pageFrame = pgo;
        int i,val=0,tot=1;
        for(i=0; i<buffsize; i++) 
		{	
			bool a= (pageFrame[i].PageNumber == (*page).pageNum)?TRUE:FALSE;
                if(a==TRUE) 
				{
                    pageFrame[i].pstats = val;
      
					if(pageFrame[i].fcnt> val)
					{
						pageFrame[i].fcnt= val;
					}
					else
						pageFrame[i].fcnt=val;

						pageFrame[tot].pstats = val;
						pageFrame[tot].fcnt = val;
						return RC_OK;
                }
        }
		return RC_ERROR;	
}

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	Page *pgo= (*bm).mgmtData;
	Page *pageFrame = pgo;
	int i=0;
	while(i < buffsize) {
		bool a= (pageFrame[i].PageNumber == (*page).pageNum)?TRUE:FALSE;
			if(a==TRUE) 
			{
				pageFrame[i].crud = 1;
				return RC_OK;
			}
		i+=1;
	}
	return RC_ERROR;
}

extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	Page *pgo= (*bm).mgmtData;
	Page *pageFrame = pgo;
	int init=0;
	Line *queue = q;
	forceFlushPool(bm);
	int i=0;
	while(i<buffsize)
	{
		if(pageFrame[i].fcnt != 0)
		{
			return RC_ERROR;
		}
		i+=1;
	}
	
	while(i<maxbuffsize)
	{
	  free(pageFrame[i].data);
	  i+=1;
	}
	free(queue);
	free(pageFrame);
	buffsize = init;
	isBuffMax = init;
	maxbuffsize = init;
	currLineSize=init;
	(*bm).mgmtData = NULL;
	return RC_OK;
}


extern RC forceFlushPool(BM_BufferPool *const bm)
{
	Page *pgo= (*bm).mgmtData;
	Page *pf = pgo;
	int i=0;
	while(i < buffsize)
	{
		int a= (pf[i].crud == 1)?1:0;
		if(a == 1)
		{
			int pagenum=pf[i].PageNumber;
			SM_FileHandle fh;
			openPageFile((*bm).pageFile, &fh);
			ensureCapacity(pagenum,&fh);
			writeBlock(pagenum, &fh, pf[i].data);
			pf[i].crud = 0;
			writeCnt++;
		}
		i+=1;
	}
	return RC_OK;
}
extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    Page *pgo= (*bm).mgmtData;
	Page *pageFrame = pgo;
	int i = 0;
	while(i < buffsize)
	{
		bool r=pageFrame[i].PageNumber == (*page).pageNum?TRUE:FALSE;
		if(r==TRUE)
		{
			SM_FileHandle fh;
			openPageFile((*bm).pageFile, &fh);
			writeBlock(pageFrame[i].PageNumber, &fh, pageFrame[i].data);
			writeCnt+=1;
			pageFrame[i].crud= 0;
		}
		i++;
		}
		
	return RC_OK;  	
}

extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
		PageNumber pg=sizeof(PageNumber) * buffsize;
        PageNumber *pnumb = malloc(pg);
		Page *pgo= (*bm).mgmtData;
        Page *pageFrame = pgo;
        int i;
		for(i = 0; i < maxbuffsize; i++)
		{
		if(pageFrame[i].PageNumber != -1)
			{
				pnumb[i]=pageFrame[i].PageNumber;
				
			}
			else{
				pnumb[i]=NO_PAGE;
			}
				
		}
		      
        return pnumb;
        free(pnumb);
}

extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	bool dirt=sizeof(bool) * buffsize;
	bool *dirtBits = malloc(dirt);
	Page *buff=(Page *)(*bm).mgmtData;
	Page *pageFrame= buff;

	int i;
	for(i=0;i<maxbuffsize;i++)
	{
		if((dirtBits[i]=pageFrame[i].crud) != 1)
		{
			dirtBits[i]=false;
			
		}
		else{
			dirtBits[i]=true;
		}
   }
	
   return dirtBits;
   free(dirtBits);
}

extern int *getFixCounts (BM_BufferPool *const bm)
{
		int a=sizeof(int) * buffsize;
        int *fcnts = malloc(a);
		Page *buff=(Page *)(*bm).mgmtData;
        Page *pageFrame= buff;
        int i;
       	for(i = 0; i < maxbuffsize; i++)
        {
			fcnts[i] = pageFrame[i].fcnt != -1 ? pageFrame[i].fcnt : 0;
            fcnts[i] = pageFrame[i].fcnt; 
			if(fcnts[i] != 1)
			{
				fcnts[i]=pageFrame[i].fcnt;
				
			}
			else{
				fcnts[i]=0;
			}                
        }
        return fcnts;
          free(fcnts);
}

extern int getNumReadIO (BM_BufferPool *const bm)
{
        return readCnt;
}

extern int getNumWriteIO (BM_BufferPool *const bm)
{
        return writeCnt;
}
