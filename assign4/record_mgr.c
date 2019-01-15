#include <stdio.h>
#include "storage_mgr.h"
#include "record_mgr.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "buffer_mgr.h"

dbmanage * dm;
//this variable is used to determine record count
int reccnt = 0;

//initializing record manager
extern RC initRecordManager(void * mgmtData) 
{
  initStorageManager();
  return RC_OK;
}

extern RC shutdownRecordManager() 
{
  dm = NULL;
  return RC_OK;
}

//this function uses storage manager functions to create a table
extern RC createTable(char * name, Schema * schema) 
{
  int a = 0, b = 0, len = 0;
  char content[4096];
  int init = 0;
  char rep[40];
  char arr[40];
  int intsize = sizeof(int);
  dm = (dbmanage * ) malloc(sizeof(dbmanage));
  int check_createfile;
  int check_openfile;
  int check_writefile;
  int check_closefile;

  BM_BufferPool * dbuffer = & (*dm). buffer;

  //allocating 50 pages to buffer manager
  int check_buffer_stat = initBufferPool(dbuffer, name, 50, RS_LRU, NULL);

  if (check_buffer_stat != RC_OK) 
  {
    return RC_ERROR;
  }

  //make all the characters in content as '+'
  while (len < 4096) 
  {
    content[len] = '-';
    len++;
  }

  content[init] = '1';
  init += intsize;
  (*dm). pagefree = 1;
  //stores the count of rows in the table
  (*dm). nrows = 0;
  content[init] = '0';
  init = init + intsize;
  //number of attributes in the table is stored
  sprintf(rep, "%d", ( * schema).numAttr);
  content[init] = * rep;
  init += intsize;
  (*dm). attrno = ( * schema).numAttr;

  //attributes of schema are copied to a table
  while (a < ( * schema).numAttr) 
  {
    while (b < strlen(( * schema).attrNames[a])) 
	{
      content[init] = ( * schema).attrNames[a][b];
      b += 1;
    }
    char * rep = & (*dm). attributename[a];
    memcpy(rep, ( * schema).attrNames[a], 1);
    init += 10;
    sprintf(rep, "%d", ( * schema).dataTypes[a]);
    content[init] = * rep;
    init += intsize;
    (*dm). dtype[a] = ( * schema).dataTypes[a];
    sprintf(arr, "%d", ( * schema).typeLength[a]);
    content[init] = * arr;
    init = init + intsize;
    (*dm). tlength[a] = ( * schema).typeLength[a];
    a++;
  }
  
  //end of string 
  content[init] = '\0';

  SM_FileHandle handle;
  check_createfile = createPageFile(name);
  check_openfile = openPageFile(name, & handle);
  check_writefile = writeBlock(0, & handle, content);
  check_closefile = closePageFile( & handle);

  //create pagefile using file handler
  if (check_createfile != RC_OK)
    return check_createfile;
  if (check_openfile != RC_OK)
    return check_openfile;
  if (check_writefile != RC_OK)
    return check_writefile;
  if (check_closefile != RC_OK)
    return check_closefile;
  return RC_OK;
}

//this function opens table file from the disk
extern RC openTable(RM_TableData * rel, char * name) 
{
  int schsize;
  int intsize;
  int charsize;
  int dtype;
  int cnt;
  intsize = sizeof(int);
  charsize = sizeof(char * );
  schsize = sizeof(Schema);
  dtype = sizeof(DataType);
  int len2 = 0;
  int len3 = 3;

  ( * rel).name = name;
  ( * rel).mgmtData = dm;
  Schema * s = (Schema * ) malloc(schsize);
  (*s).numAttr = (*dm). attrno;
  cnt = (*s).numAttr;
  int cnts = cnt * charsize;
  (*s).attrNames = (char * * ) malloc(cnts);
  (*s).dataTypes = malloc(cnt * dtype);
  (*s).typeLength = malloc(cnt * intsize);

  while (len2 < len3) 
  {
    (*s).typeLength[len2] = (*dm). tlength[len2];
    (*s).attrNames[len2] = (char * ) malloc(10);
    char * temp = & (*dm). attributename[len2];
    memcpy(s->attrNames[len2], temp, 1);
    s->dataTypes[len2] = (*dm). dtype[len2];
    len2++;
  }

  ( * rel).schema = s;
  return RC_OK;
}

// tables are closed after flushing all the values to the disk
extern RC closeTable(RM_TableData * rel) 
{
  dbmanage * dbmanger = ( * rel).mgmtData;
  forceFlushPool( & dbmanger->buffer);
  return RC_OK;
}

//this function deletes the table
extern RC deleteTable(char * name) 
{
  destroyPageFile(name);
  return RC_OK;
}

//gets the number of rows retured by the table
extern int getNumTuples(RM_TableData * rel) 
{
  dbmanage * dm = (dbmanage * )( * rel).mgmtData;
  return (*dm). nrows;
}

//this function inserts a record into the table 
extern RC insertRecord(RM_TableData * rel, Record * record) 
{
  char * info, * locat;
  dbmanage * dmanage = ( * rel).mgmtData;
  RID * rid = & record->id;
  BM_BufferPool * buffer1;
  BM_PageHandle * buffer2;
  //fetching the record size using schema
  int rval = (rid->slot * (getRecordSize(( * rel).schema)));
  BM_BufferPool * dbuffer = & (*dm). buffer;
  BM_PageHandle * datpage = & dmanage->page;
  pinPage(dbuffer, datpage, dmanage->pagefree);
  info = dmanage->page.data;
  int loc = 0, fit = 0;
  int x = dmanage->pagefree;
  //the page which is freed is set as pagenumber
  rid->page = dmanage->pagefree;
  rid->slot = dmanage->nrows;
  bool a = (getNumTuples(rel)) > (4096 / getRecordSize(( * rel).schema)) ? TRUE : FALSE;

  if (a == TRUE) 
  {
    x++;
    dmanage->pagefree += 1;
    info = dmanage->page.data;
    rid->page = dmanage->pagefree;
    buffer1 = & dmanage->buffer;
    buffer2 = & dmanage->page;
    pinPage(buffer1, buffer2, rid->page);
    rid->slot = 0;
    unpinPage(buffer1, buffer2);
  }

  markDirty(dbuffer, datpage);
  //below code is used to move the pointer to the slot which is free
  locat = info;
  locat += rval;
  //to get numerical posotion
  loc = rid->slot * (getRecordSize(( * rel).schema));

  while (fit < (getRecordSize(( * rel).schema))) 
  {
    info[loc++] = record->data[fit];
    fit++;
  }
  //unpin the page
  unpinPage(dbuffer, datpage);
  //force page to disk
  forcePage(dbuffer, datpage);
  ( * dmanage).nrows += 1;
  return RC_OK;
}
 
//this function is used to record the record and its corresponding table 
extern RC updateRecord(RM_TableData * rel, Record * record) 
{
  dbmanage * dbmanager = ( * rel).mgmtData;
  int rsize = getRecordSize(( * rel).schema);
  int rslot = (record->id.slot * rsize);
  BM_BufferPool * buff;
  buff = & dbmanager->buffer;
  BM_PageHandle * pag;
  pag = & dbmanager->page;
  //force page back to disk
  forcePage(buff, pag);
  markDirty(buff, pag);
  //pin the page with corresponding RID
  pinPage(buff, pag, record->id.page);
  //unpin page
  unpinPage(buff, pag);
  dbmanager->page.data = (dbmanager->page.data) + rslot;
  memcpy(dbmanager->page.data, record->data, rsize);
  return RC_OK;
}

//this function is used to delete the record with specified RID
extern RC deleteRecord(RM_TableData * rel, RID id) 
{
  dbmanage * dbmanager = ( * rel).mgmtData;
  int lc = 0;
  int pid = id.page;
  int pslot = id.slot;
  char * array = dbmanager->page.data;
  int record_size = getRecordSize(( * rel).schema);
  BM_BufferPool * buff;
  buff = & dbmanager->buffer;
  BM_PageHandle * pag;
  pag = & dbmanager->page;
  //pin the page which has specified RID
  pinPage(buff, pag, pid);
  //delete the record
  while (lc < record_size) {
    array[(pslot * record_size) + lc] = '-';
    lc++;
  }
  markDirty(buff, pag);
  //force page to disk
  forcePage(buff, pag);
  return RC_OK;
}
 
//this function fetches record from memory 
extern RC getRecord(RM_TableData * rel, RID id, Record * record) 
{
  dbmanage * dbmanager = (dbmanage * )( * rel).mgmtData;
  char * matrix = dbmanager->page.data;
  int pslot = id.slot;
  int cnt = 0, result;
  BM_BufferPool * buff;
  buff = & dbmanager->buffer;
  BM_PageHandle * pag;
  pag = & dbmanager->page;
  //pin page
  pinPage(buff, pag, id.page);
  while (cnt < (getRecordSize(( * rel).schema))) 
  {
    result = cnt;
    record->data[cnt] = matrix[(pslot * (getRecordSize(( * rel).schema))) + result];
    cnt++;
  }
  //unpin page
  unpinPage(buff, pag);
  return RC_OK;
}

extern RC startScan(RM_TableData * rel, RM_ScanHandle * scan, Expr * cond) 
{
  scan->mgmtData = cond;
  scan->rel = rel;
  return RC_OK;
}

//this function fetches record which matches scan condition
extern RC next(RM_ScanHandle * scan, Record * record) 
{
  Value * valuesize = malloc(sizeof(Value));
  Value * vallength = valuesize;
  Expr * cond = (Expr * ) scan->mgmtData;
  Schema * schema = scan->rel->schema;
  while (reccnt <= ((*dm). nrows)) 
  {
    pinPage( & (*dm). buffer, & (*dm). page, 1);
    char * data = (*dm). page.data;
    data = data + ((getRecordSize(schema)) * reccnt);
    record->id.page = 1;
    record->id.slot = reccnt;
    reccnt++;
    strncpy(record->data, data, getRecordSize(schema));
    evalExpr(record, schema, cond, & vallength);
    int value = (vallength->v.boolV == TRUE) ? 1 : 0;
    if (value == 1) 
	{
      unpinPage( & (*dm). buffer, & (*dm). page);
      return RC_OK;
    }
  }
  reccnt = 0;
  return RC_RM_NO_MORE_TUPLES;
}

//closing scan
extern RC closeScan(RM_ScanHandle * scan) 
{
  return RC_OK;
}

//this function is used to fetch the record size 
extern int getRecordSize(Schema * schema) 
{
  Schema * s = schema;
  int i = 0, result = 0;
  while (i < (s->numAttr)) 
  {
    if (( * schema).dataTypes[i] == DT_INT) 
	{
      result = result + sizeof(int);
    } 
	else if (( * schema).dataTypes[i] == DT_STRING) 
	{
      result = result + s->typeLength[i];
    } 
	else if (( * schema).dataTypes[i] == DT_FLOAT) 
	{
      result = result + sizeof(float);
    }
	else if (( * schema).dataTypes[i] == DT_BOOL) 
	{
      result = result + sizeof(bool);
    }
    i++;
  }
  return result;
}

//this function is used to create the schema using the parameters passed in the function
extern Schema * createSchema(int numAttr, char * * attrNames, DataType * dataTypes, int * typeLength, int keySize, int * keys) 
{
  int a = sizeof(Schema);
  Schema * s = (Schema * ) malloc(a);
  ( * s).attrNames = attrNames;
  ( * s).numAttr = numAttr;
  ( * s).keySize = keySize;
  ( * s).typeLength = typeLength;
  ( * s).keyAttrs = keys;
  ( * s).dataTypes = dataTypes;
  return s;
}

//this function is used to free the schema
extern RC freeSchema(Schema * schema) 
{
  Schema * s = schema;
  int i = 0;
  while (i < ( * s).numAttr) 
  {
    free(( * s).dataTypes);
    free(( * s).typeLength);
    free(( * s).keyAttrs);
    free(s);
    return RC_OK;
    i++;
  }
  return RC_ERROR;
}

//creates a record
extern RC createRecord(Record * * record, Schema * schema) 
{
  int rec = 0;
  Record * tab = (Record * ) malloc(sizeof(Record));
  tab->id.page = -1;
  tab->id.slot = -1;
  tab->data = (char * ) malloc(getRecordSize(schema));
  while (rec < getRecordSize(schema)) 
  {
    tab->data[rec] = '-';
    rec++;
  } * record = tab;
  return RC_OK;
}
 
//this function is used to fetch the current offset of the attributes to get attributes 
extern int getAttributeOffset(int attrNum, Schema * schema) 
{
  int i = 0, offset = 0;
  while (i < attrNum) 
  {
    if (( * schema).dataTypes[i] == DT_INT) 
	{
      offset = offset + sizeof(int);
    } 
	else if (( * schema).dataTypes[i] == DT_STRING) 
	{
      offset = offset + ( * schema).typeLength[i];
    }
	else if (( * schema).dataTypes[i] == DT_FLOAT) 
	{
      offset = offset + sizeof(float);
    }
	else if (( * schema).dataTypes[i] == DT_BOOL) 
	{
      offset = offset + sizeof(bool);
    }
    i++;
  }
  return offset;
}
//free's the record 
extern RC freeRecord(Record * record) 
{
  free(record);
  return RC_OK;
}

int extern addDatapOffset(int dataP, int offset) 
{
  int numb = 0;
  numb = dataP + offset;
  return numb;
}

//this function is used to set attribute
extern RC setAttr(Record * record, Schema * schema, int attrNum, Value * value) 
{
  int attno = attrNum;
  int add = 0;
  int dp = 0;
  int point = 1;
  int point_new = 2;

  add = getAttributeOffset(attno, schema);
  dp = addDatapOffset(dp, add);
  if (( * schema).dataTypes[attno] == 0) 
  {
    char temp[20];
    sprintf(temp, "%d", value->v.intV);
    int count = 0, t = 0;
    while (value->v.intV != 0) 
	{
      value->v.intV = value->v.intV / 10;
      ++count;
    }
    while (t < count) 
	{
      record->data[dp + t] = temp[t];
      t++;
    }
  } 
  else if (( * schema).dataTypes[attno] == point) 
  {
    int t = dp;
    int o = 0;
    while (o < strlen(value->v.stringV)) 
	{
      record->data[t + o] = value->v.stringV[o];
      o++;
    }
  } 
  else if (( * schema).dataTypes[attno] == point_new) 
  {
    char temp[20];
    sprintf(temp, "%f", value->v.floatV);
    record->data[dp] = * temp;
  } 
  else 
  {
    char temp[20];
    sprintf(temp, "%d", value->v.boolV);
    record->data[dp] = * temp;
  }
  return RC_OK;
}


int extern addDataOffset(int dataP, int offset) 
{
  int numb = 0;
  numb = dataP + offset;
  return numb;
}

//this function is used to get the attribute
extern RC getAttr(Record * record, Schema * schema, int attrNum, Value * * value) 
{
  int size = sizeof(Value);
  Value * size_val = (Value * ) malloc(size);
  Value * new_value = size_val;
  int adds = 0;
  int dp = 0;
  int type_str = 1;
  int type_float = 2;
  int type_bool = 3;
  adds = getAttributeOffset(attrNum, schema);
  dp = addDataOffset(dp, adds);
  char * data = record->data;
  data = data + adds;

  if (( * schema).dataTypes[attrNum] == 0) 
  {
    char temp[20];
    int num1 = 0;
    int i = 0, b = 4, len = 0, vsize = 0;
    while (i < b) 
	{
      if ((data[i] - '0') > 0) 
	  {
		temp[num1] = data[i];
        num1++;  
      } 
	  else 
	  {
        break;
      }
      i++;
    }
    while (len < num1) 
	{
      vsize = (temp[len] - '0') + vsize * 10;
      len++;
    }
    new_value->v.intV = vsize;
    new_value->dt = 0;
  } 
  else if (( * schema).dataTypes[attrNum] == type_bool) 
  {
    ( * new_value).v.boolV = data[dp] - '0';
    ( * new_value).dt = type_bool;
  }
  else if (( * schema).dataTypes[attrNum] == type_float) 
  {
    ( * new_value).v.floatV = data[dp] - '0';
    ( * new_value).dt = type_float;
  }
  else if (( * schema).dataTypes[attrNum] == type_str) 
  {
    new_value->v.stringV = (char * ) malloc(4);
    new_value->dt = type_str;
    memcpy(new_value->v.stringV, data, 4);
    new_value->v.stringV[4] = '\0';
  } 
  * value = new_value;
  return RC_OK;
}

