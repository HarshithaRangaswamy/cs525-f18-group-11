**CS525 Advanced Database Organization (Fall 2018)**  
**Homework Assignment 3**  
Team members:  
*  Alexandru Iulian Orhean (aorhean@hawk.iit.edu)
*  Karthik Mahesh (kmahesh@hawk.iit.edu)
*  Harshitha Rangaswamy (hrangaswamy@hawk.iit.edu)

# How to build and run

The submission contains a Makefile with the following targets:
  make -> to compile all the files
  make run -> runs the given test application that uses the implemented record manager
  make clean -> removes all old compiled files.

		
#Functions used in Record Manager.

initRecordManager 
  It Initializes the record manager by initializing the storage manager.

shutdownRecordManager
  It Shuts down the record manager and also the Buffer Manager and frees the memory that is allocated to the data structure.

createTable  
  It creates a table using the storage manager functions and a LRU Queue with size 50 and Creates a buffer manager with 50 pages and a LRU queue.
  
openTable 
  It opens the table file from the disk and set the data in the RM_TableData and then allocate the space for the attribute names,datatypes and typeLengths. It also stores the schema to the Table Handler. 
  
closeTable
  It closes the table by flushing all the changes to the Disk.
  
deleteTable
  It deletes the table.
  
getNumTuples
  It returns the number of tuples in the table.
  
insertRecord
  It inserts record into the table taking data from the record structure. 
  If the number of rows exceeds the page capacity, move to the next page. 
  Each page can handle PAGE_SIZE number of records.
  On moving to a new page, increase the page count. 
  Unpin the current page to pin the next Page. Pin the new page into the buffer for reading the data.
  Mark the pages dirty because we are inserting a record. 

updateRecord
  It updates the record in the table uing the RID find the location of the record in the table.

deleteRecord  
  It deletes the record from the table.
  It finds the location of the record and mark the record as deleted using the token and
  mark the page as dirty, unpin the page and write the page onto the disk.
   
getRecord
  It retreives the record from the memory.
  It fetches the record and copy it into the data structure using the RID find the location of the record in the table, .
  
startScan
  It Initializes the scan handler with the condition for scan and the table associated with the scan. 
  
next
  It gets the next record that matches the scan condition and runs a loop until all the records are scanned. 
  It checks if the record satisfies the condition and return the same. 

closeScan  
  It closes the scan handler.
  
getRecordSize
  It gets the size of the record using the schema. 

createSchema
  It creates the schema.

freeSchema
  It deletes the schema and all the iterative mallocs. 
  
createRecord
  It creates the Record.

freeRecord
  It deallocates the memory allocated for the record.

getAttr
  It gets the current offset for the attribute to get the attribute.

setAttr
  It sets the attribute for the record.
