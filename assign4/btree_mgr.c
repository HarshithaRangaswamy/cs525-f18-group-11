#include <stdlib.h>
#include <string.h>

#include "btree_mgr.h"
#include "tables.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include "bplustree.h"

#define MAX_BTREES 8
#define MAX_IDX_LEN 256

BTreeHandle *trees[MAX_BTREES];

// init and shutdown index manager
RC initIndexManager (void *mgmtData)
{
    int i;

    for (i = 0; i < MAX_BTREES; i++) {
        trees[i] = (BTreeHandle*) malloc(sizeof(BTreeHandle));
        trees[i]->idxId = NULL;
        trees[i]->mgmtData = NULL;
    }

    return RC_OK;
}

RC shutdownIndexManager ()
{
    int i;

    for (i = 0; i < MAX_BTREES; i++) {
        if (trees[i]->idxId != NULL) {
            closeBtree(trees[i]);
            deleteBtree(trees[i]->idxId);
        }
        free(trees[i]);
    }

    return RC_OK;
}

// create, destroy, open, and close an btree index
RC createBtree (char *idxId, DataType keyType, int n)
{
	int i;
    bplustree_t *tree;
    
    // Find a place in memory
    i = 0;
    while (i < MAX_BTREES) {
        if (trees[i]->idxId == NULL) {
            break;
        }
        i++;
    }

    // If no place in memory has been found return error
    if (i >= MAX_BTREES) {
        return RC_ERROR;
    }

    trees[i]->idxId = (char*) malloc(sizeof(char) * MAX_IDX_LEN);
    strcpy(trees[i]->idxId, idxId);
    trees[i]->keyType = keyType;
    tree = (bplustree_t*) malloc(sizeof(bplustree_t));
    if (bplustree_init(tree, n) != BPLUSTREE_RET_OK) {
        return RC_ERROR;
    }
    trees[i]->mgmtData = (void*) tree;
    
    return RC_OK;
}

RC openBtree (BTreeHandle **tree, char *idxId)
{
    int i;

    i = 0;
    while (i < MAX_BTREES) {
        if (trees[i]->idxId != NULL && strcmp(trees[i]->idxId, idxId) == 0) {
            break;
        }
        i++;
    }

    if (i >= MAX_BTREES) {
        return RC_ERROR;
    }

    *tree = trees[i];
    
    /* TODO - When opening the B+tree should probably open page file, init 
     * buffer manager and populate the B+tree with values from the disk */
    
    return RC_OK;
}

RC closeBtree (BTreeHandle *tree)
{
    
    /* TODO - When closing the B+tree should probably flush buffer manager to
     * disk, close buffer manager and close storage manager */
    
    return RC_OK;
}

RC deleteBtree (char *idxId)
{
    int i;
    bplustree_t *tree;

    i = 0;
    while (i < MAX_BTREES) {
        if (trees[i]->idxId != NULL && strcmp(trees[i]->idxId, idxId) == 0) {
            break;
        }
        i++;
    }

    if (i >= MAX_BTREES) {
        return RC_ERROR;
    }

    tree = (bplustree_t*) trees[i]->mgmtData;
    if (bplustree_destroy(tree) != BPLUSTREE_RET_OK) {
        return RC_ERROR;
    }

    free(tree);
    free(trees[i]->idxId);

    trees[i]->idxId = NULL;
    trees[i]->mgmtData = NULL;

    return RC_OK;
}


// access information about a b-tree
RC getNumNodes (BTreeHandle *tree, int *result)
{
    bplustree_t *btree;
    
    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }
    btree = (bplustree_t*) tree->mgmtData;

    *result = btree->num_nodes;

    return RC_OK;
}

RC getNumEntries (BTreeHandle *tree, int *result)
{
    bplustree_t *btree;
    
    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }
    btree = (bplustree_t*) tree->mgmtData;

    *result = btree->num_data;

    return RC_OK;
}

RC getKeyType (BTreeHandle *tree, DataType *result)
{
    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }

    *result = tree->keyType;

    return RC_OK;
}


// index access
RC findKey (BTreeHandle *tree, Value *key, RID *result)
{
    int rc;
    bplustree_t *btree;
    RID *rid;
    
    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }
    btree = (bplustree_t*) tree->mgmtData;

    if (tree->keyType != DT_INT || key->dt != DT_INT) {
        return RC_ERROR;
    }

    rc = bplustree_lookup(btree, key->v.intV, (void**) &rid);
    if (rc != BPLUSTREE_RET_OK) {
        return RC_IM_KEY_NOT_FOUND;
    }
    *result = *rid;

    return RC_OK;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
    int rc;
    bplustree_t *btree;
    RID *rid_data;
    
    if (tree == NULL) {
        return RC_ERROR;
    }

    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }
    btree = (bplustree_t*) tree->mgmtData;

    if (tree->keyType != DT_INT || key->dt != DT_INT) {
        return RC_ERROR;
    }
    
    rc = bplustree_lookup(btree, key->v.intV, (void**) &rid_data);
    if (rc == BPLUSTREE_RET_OK) {
        return RC_IM_KEY_ALREADY_EXISTS;
    }

    rid_data = (RID*) malloc(sizeof(RID));
    *rid_data = rid;

    rc = bplustree_insert(btree, key->v.intV, (void*) rid_data);
    if (rc != BPLUSTREE_RET_OK) {
        return RC_ERROR;
    }

    return RC_OK;
}

RC deleteKey (BTreeHandle *tree, Value *key)
{
    int rc;
    bplustree_t *btree;
    RID *rid;
    
    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }
    btree = (bplustree_t*) tree->mgmtData;

    if (tree->keyType != DT_INT || key->dt != DT_INT) {
        return RC_ERROR;
    }
    
    rc = bplustree_lookup(btree, key->v.intV, (void**) &rid);
    if (rc != BPLUSTREE_RET_OK) {
        return RC_IM_KEY_NOT_FOUND;
    }

    rc = bplustree_remove(btree, key->v.intV, (void**) &rid);
    if (rc != BPLUSTREE_RET_OK) {
        return RC_ERROR;
    }
    free(rid);

    return RC_OK;
}

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
    int rc;
    bplustree_t *btree;
    bpiterator_t *iter;
    
    if (tree->mgmtData == NULL) {
        return RC_ERROR;
    }
    btree = (bplustree_t*) tree->mgmtData;

    iter = (bpiterator_t*) malloc(sizeof(bpiterator_t));
    rc = bplustree_iterator(btree, iter);
    if (rc != BPITERATOR_RET_OK) {
        return RC_ERROR;
    }
    
    *handle = (BT_ScanHandle*) malloc(sizeof(BT_ScanHandle));
    (*handle)->tree = tree;
    (*handle)->mgmtData = (void*) iter;

    return RC_OK;
}

RC nextEntry (BT_ScanHandle *handle, RID *result)
{
    int rc;
    bpiterator_t *iter;
    RID *rid;

    iter = (bpiterator_t*) handle->mgmtData;
    rc = bpiterator_next(iter, (void**) &rid);
    if (rc == BPITERATOR_RET_END) {
        return RC_IM_NO_MORE_ENTRIES;
    }
    if (rc == BPITERATOR_RET_ERR) {
        return RC_ERROR;
    }
    *result = *rid;

    return RC_OK;
}

RC closeTreeScan (BT_ScanHandle *handle)
{
    int rc;
    bpiterator_t *iter;

    iter = (bpiterator_t*) handle->mgmtData;
    rc = bpiterator_clear(iter);
    if (rc == BPITERATOR_RET_ERR) {
        return RC_ERROR;
    }
    
    free(iter);
    free(handle);

    return RC_OK;
}


// debug and test functions
char *printTree (BTreeHandle *tree)
{
    bplustree_t *btree;
    
    if (tree->mgmtData == NULL) {
        return NULL;
    }
    btree = (bplustree_t*) tree->mgmtData;

    return bplustree_printv2(btree);
}
