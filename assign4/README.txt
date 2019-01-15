**CS525 Advanced Database Organization (Fall 2018)**  
**Homework Assignment 4**  
Team members:  
*  Alexandru Iulian Orhean (aorhean@hawk.iit.edu)
*  Karthik Mahesh (kmahesh@hawk.iit.edu)
*  Harshitha Rangaswamy (hrangaswamy@hawk.iit.edu)

# Description of the index manager

We are using the Homework Assignment 2 and 3 as implementation of the storage manager, buffer
manager and record manager with no modifications.

# Description of data structures and implementation

The *bplustree.h* and *bplustree.c* files contain the implementation of a general purpose B+tree,
that uses integers as keys and void pointer as values, thus being able to easily work with any kind
of data structure as long as it is passed as a pointer and not as a value. The homework assignment
assumes that the RID's are passed as values, but in reality a new RID data structure is allocated on
every insertion and deallocated on every remove in order to work seamlessly with the B+tree
implementation.

The B+tree is organized in two major data structures: *bplustree_t* and *bpnode_t*. The
*bplustree_t* contains the root of the tree and the implementation provides functions to
initialized, destroy, insert, remove, lookup, print and create iterators, replacing the root of the
tree accordingly during overflow or underflow situations. The *bplustree_t* functions make use of
the functions implemented for the *bpnode_t* data structure, which represents the logical
representation of a B+tree node, be that a leaf or a non-leaf node. The *bplustree_t* data structure
also contains statistical data regarding the B+tree, such as: number of entries (num_data) and
number of nodes (num_nodes); all of them being modified while the B+tree is getting modified, not
requiring traversing the tree. The *bpnode_t* data structure contains an array of keys, an array of
children denoted as *nodes*, an array of void pointers denoting the values and represented as
*data*, a pointer to the next node and a pointer to the *bplustree_t* data structure that contains
the value for the maximum number of keys allowed per node. These members are not allocated on every
node at the same time and are determined by the type of the node which can be either a leaf or a
non-leaf. For example a leaf node is not going to have an array of children, but it is going to have
a pointer to the next leaf.

In terms of functionality and performance the insertion, delete and lookup functions execute a
binary search over the key space, and when the appropriate key is discovered the action to proceed
deeper in the tree or execute the action on the leaf is decided. At a node granularity, insertion,
deletion and lookup happen in O(log(n)) time. Practical performance could be further increased by
implementing a memory sub-allocator that would create nodes in advance making the process of
splitting and merging very fast.

The data structure is not currently integrated with the record manager, and can only support
integer keys. Multi key type support can be implemented through the used of void pointers again, 
the array of keys becoming an array of pointers. The *bplustree_t* data structure would have to be
modified such that it contains two pointer functions: less_than and equal; which would take as
parameter two void pointers and return an integer that represent the ordering of the data. This was
not done, due to lack of time.

# How to build and run

The submission contains a Makefile with the following targets:
make (all) -> build the given test applications and the index manager in one binary file
make clean -> removes all binary files (terminated with .elf)

To run the test applications, just run the generated .elf files.
		
# Description of the implemented tests

Only one extra test file is added, that was meant to test the B+tree implementation with blocks of
characters as values.
