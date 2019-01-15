#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bplustree.h"

#define N 4
//#define N 5
#define DSIZE 16

void test_leaf_node_insert();
void test_leaf_node_remove();
void test_leaf_node_lookup();

void test_nonleaf_node_insert();
void test_nonleaf_node_remove();
void test_nonleaf_node_lookup();

void test_leaf_node_split();
void test_leaf_node_redis_from_left();
void test_leaf_node_redis_from_right();
void test_leaf_node_merge();

void test_nonleaf_node_split();
void test_nonleaf_node_redis_from_left();
void test_nonleaf_node_redis_from_right();
void test_nonleaf_node_merge();

int main(int argc, char **argv)
{
    printf("Starting the B+tree tests!\n");
    
    test_leaf_node_insert();
    printf("Test 1 ................................ (fininshed)\n");

    test_leaf_node_remove();
    printf("Test 2 ................................ (fininshed)\n");
    
    test_leaf_node_lookup();
    printf("Test 3 ................................ (fininshed)\n");

    test_nonleaf_node_insert();
    test_nonleaf_node_remove();
    test_nonleaf_node_lookup();

    test_leaf_node_split();
    test_leaf_node_redis_from_left();
    test_leaf_node_redis_from_right();
    test_leaf_node_merge();

    test_nonleaf_node_split();
    test_nonleaf_node_redis_from_left();
    test_nonleaf_node_redis_from_right();
    test_nonleaf_node_merge();
    
    printf("Finished the B+tree tests\n");
    return 0;
}

void test_leaf_node_insert()
{
    int rc, i, len;
    bplustree_t *tree;
    bpnode_t *leaf;
    char data[N + 1][16];

    for (i = 0; i <= N; i++) {
        memset(data[i], 'a' + i, DSIZE - 1);
        data[i][DSIZE - 1] = 0;
    }

    tree = (bplustree_t*) malloc(sizeof(bplustree_t));
    rc = bplustree_init(tree, N);
    if (rc != BPLUSTREE_RET_OK) {
        printf("ERROR: initializing the B+tree failed!\n");
        goto error1;
    }

    leaf = (bpnode_t *) malloc(sizeof(bpnode_t));
    rc = bpnode_init(leaf, LEAF, tree);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: initializing the leaf node failed!\n");
        goto error2;
    }

    len = 0;
    for (i = N - 1; i >= 0; i -= 2) {
        rc = bpnode_insert_leaf(leaf, i, (void*) data[i]);
        if (rc != BPNODE_RET_OK) {
            printf("ERROR: insert failed (rc) %d:%s\n", i, data[i]);
            goto error2;
        } else if (leaf->len != len + 1) {
            printf("ERROR: insert failed (len) %d:%s\n", i, data[i]);
            goto error2;
        } else if (leaf->keys[0] != i) {
            printf("ERROR: insert failed (key) %d:%s\n", i, data[i]);
            goto error2;
        } else if (leaf->data[0] != data[i]) {
            printf("ERROR: insert failed (data) %d:%s\n", i, data[i]);    
            goto error2;
        } else {
            printf("INFO: insert successful %d:%s\n", i, data[i]);
        }
        len++;
    }
    
    for (i = 0; i < N; i += 2) {
        rc = bpnode_insert_leaf(leaf, i, (void*) data[i]);
        if (rc != BPNODE_RET_OK) {
            printf("ERROR: insert failed (rc) %d:%s\n", i, data[i]);
            goto error2;
        } else if (leaf->len != len + 1) {
            printf("ERROR: insert failed (len) %d:%s\n", i, data[i]);
            goto error2;
        } else if (leaf->keys[i] != i) {
            printf("ERROR: insert failed (key) %d:%s\n", i, data[i]);
            goto error2;
        } else if (leaf->data[i] != data[i]) {
            printf("ERROR: insert failed (data) %d:%s\n", i, data[i]);    
            goto error2;
        } else {
            printf("INFO: insert successful %d:%s\n", i, data[i]);
        }
        len++;
    }

    i = N;
    rc = bpnode_insert_leaf(leaf, i, (void*) data[i]);
    if (rc != BPNODE_RET_OVERFLOW) {
        printf("ERROR: overflow insert failed %d:%s\n", N, data[i]);
        goto error2;
    } else {
        printf("INFO: overflow insert successful %d:%s\n", N, data[i]);
    }

error2:
    rc = bpnode_destroy(leaf);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: destroying the leaf node failed!\n");
    }
    free(leaf);

error1:
    rc = bplustree_destroy(tree);
    if (rc != BPLUSTREE_RET_OK) {
        printf("ERROR: destroying the B+tree failed!\n");
    }
    free(tree);

    return;
}

void test_leaf_node_remove()
{
    int rc, i, len;
    bplustree_t *tree;
    bpnode_t *leaf;
    char data[N + 1][16];
    char *pdata;

    for (i = 0; i <= N; i++) {
        memset(data[i], 'a' + i, DSIZE - 1);
        data[i][DSIZE - 1] = 0;
    }

    tree = (bplustree_t*) malloc(sizeof(bplustree_t));
    rc = bplustree_init(tree, N);
    if (rc != BPLUSTREE_RET_OK) {
        printf("ERROR: initializing the B+tree failed!\n");
        return;
    }

    leaf = (bpnode_t *) malloc(sizeof(bpnode_t));
    rc = bpnode_init(leaf, LEAF, tree);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: initializing the leaf node failed!\n");
        return;
    }

    len = 0;
    for (i = 0; i < N; i++) {
        rc = bpnode_insert_leaf(leaf, i, data[i]);
        if (rc != BPNODE_RET_OK) {
            printf("ERROR: insert failed (rc) %d:%s\n", i, data[i]);
            return;
        }
        len++;
    }
    
    for (i = 0; i < N - 1; i += 2) {
        rc = bpnode_remove_leaf(leaf, i, (void**) &pdata);
        if (rc != BPNODE_RET_OK) {
            printf("ERROR: remove failed (rc) %d:%s\n", i, data[i]);
            return;
        } else if (leaf->len != len - 1) {
            printf("ERROR: remove failed (len) %d:%s\n", i, data[i]);
            return;
        } else if (leaf->keys[i / 2] != i + 1) {
            printf("ERROR: remove failed (key) %d:%s\n", i, data[i]);
            return;
        } else if (pdata != data[i]) {
            printf("ERROR: remove failed (data) %d:%s\n", i, data[i]);    
            return;
        } else {
            printf("INFO: remove successful %d:%s\n", i, data[i]);
        }
        len--;
    }

    rc = bpnode_remove_leaf (leaf, N - 1, (void**) &pdata);
    if (rc != BPNODE_RET_UNDERFLOW) {
        printf("ERROR: overflow remove failed %d:%s\n", N - 1, data[N - 1]);
        return;
    } else {
        printf("INFO: overflow remove successful %d:%s\n", N - 1, data[N - 1]);
    }

    rc = bpnode_destroy(leaf);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: destroying the leaf node failed!\n");
        return;
    }
    free(leaf);

    rc = bplustree_destroy(tree);
    if (rc != BPLUSTREE_RET_OK) {
        printf("ERROR: destroying the B+tree failed!\n");
        return;
    }
    free(tree);

    return;
}

void test_leaf_node_lookup()
{
    int rc, i;
    bplustree_t *tree;
    bpnode_t *leaf;
    char data[N + 1][16];
    char *pdata;

    for (i = 0; i <= N; i++) {
        memset(data[i], 'a' + i, DSIZE - 1);
        data[i][DSIZE - 1] = 0;
    }

    tree = (bplustree_t*) malloc(sizeof(bplustree_t));
    rc = bplustree_init(tree, N);
    if (rc != BPLUSTREE_RET_OK) {
        printf("ERROR: initializing the B+tree failed!\n");
        return;
    }

    leaf = (bpnode_t *) malloc(sizeof(bpnode_t));
    rc = bpnode_init(leaf, LEAF, tree);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: initializing the leaf node failed!\n");
        return;
    }

    for (i = 0; i < N; i++) {
        rc = bpnode_insert_leaf(leaf, i, data[i]);
        if (rc != BPNODE_RET_OK) {
            printf("ERROR: insert failed (rc) %d:%s\n", i, data[i]);
            return;
        }
    }

    rc = bpnode_remove_leaf (leaf, N - 1, (void**) &pdata);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: remove failed %d:%s\n", N - 1, data[N - 1]);
        return;
    }
    
    for (i = N / 2; i < N - 1; i++) {
        rc = bpnode_lookup_leaf(leaf, i, (void**) &pdata);
        if (rc != BPNODE_RET_OK) {
            printf("ERROR: lookup failed (rc) %d:%s\n", i, data[i]);
            return;
        } else if (pdata != data[i]) {
            printf("ERROR: lookup failed (data) %d:%s\n", i, data[i]);    
            return;
        } else {
            printf("INFO: lookup successful %d:%s\n", i, data[i]);
        }
    }
    
    rc = bpnode_lookup_leaf (leaf, N - 1, (void**) &pdata);
    if (rc != BPNODE_RET_ERR) {
        printf("ERROR: (not E) lookup failed %d:%s\n", N - 1, data[N - 1]);
        return;
    } else {
        printf("INFO: (not E) lookup successful %d:%s\n", N - 1, data[N - 1]);
    }

    rc = bpnode_destroy(leaf);
    if (rc != BPNODE_RET_OK) {
        printf("ERROR: destroying the leaf node failed!\n");
        return;
    }
    free(leaf);

    rc = bplustree_destroy(tree);
    if (rc != BPLUSTREE_RET_OK) {
        printf("ERROR: destroying the B+tree failed!\n");
        return;
    }
    free(tree);

    return;
}

void test_nonleaf_node_insert()
{
}
void test_nonleaf_node_remove()
{
}
void test_nonleaf_node_lookup()
{
}

void test_leaf_node_split()
{
}
void test_leaf_node_redis_from_left()
{
}
void test_leaf_node_redis_from_right()
{
}
void test_leaf_node_merge()
{
}

void test_nonleaf_node_split()
{
}
void test_nonleaf_node_redis_from_left()
{
}
void test_nonleaf_node_redis_from_right()
{
}
void test_nonleaf_node_merge()
{
}
