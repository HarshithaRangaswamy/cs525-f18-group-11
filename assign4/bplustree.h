#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#define BPITERATOR_RET_OK 1
#define BPITERATOR_RET_ERR -1
#define BPITERATOR_RET_END -1

typedef struct bpiterator {
    int index;
    int length;
    void **data;
} bpiterator_t;

int bpiterator_next(bpiterator_t *iter, void **data);
int bpiterator_clear(bpiterator_t *iter);

#define BPNODE_RET_OK 0
#define BPNODE_RET_ERR -1
#define BPNODE_RET_OVERFLOW 1
#define BPNODE_RET_UNDERFLOW 2
#define BPNODE_RET_FAILED 3

enum bpnode_type {NONLEAF, LEAF};

typedef struct bpnode {
    enum bpnode_type type;
    int id;
    int len;
    int *keys;
    struct bpnode **nodes;
    void **data;
    struct bpnode *next;
    struct bplustree *tree;
} bpnode_t;

int bpnode_init(bpnode_t *node, enum bpnode_type type, struct bplustree *tree);
int bpnode_destroy(bpnode_t *node);
int bpnode_insert_leaf(bpnode_t *node, int key, void *data);
int bpnode_remove_leaf(bpnode_t *node, int key, void **data);
int bpnode_lookup_leaf(bpnode_t *node, int key, void **data);
int bpnode_insert_nonleaf(bpnode_t *node, int key, void *data);
int bpnode_remove_nonleaf(bpnode_t *node, int key, void **data);
int bpnode_lookup_nonleaf(bpnode_t *node, int key, void **data);
int bpnode_split(bpnode_t *nodec, bpnode_t *nodel, bpnode_t *noder);
int bpnode_redis_from_left(bpnode_t *nodel, bpnode_t *noder);
int bpnode_redis_from_right(bpnode_t *nodel, bpnode_t *noder);
int bpnode_merge(bpnode_t *nodel, bpnode_t *noder, bpnode_t *nodec);
void bpnode_print(bpnode_t *node, int depth);
void bpnode_update_id(bpnode_t *node, int *id);
char* bpnode_printv2(bpnode_t *node);

#define BPLUSTREE_RET_OK 0
#define BPLUSTREE_RET_ERR -1

typedef struct bplustree {
    int n;
    int num_data;
    int num_nodes;
    struct bpnode *root;
} bplustree_t;

int bplustree_init(bplustree_t *tree, int n);
int bplustree_destroy(bplustree_t *tree);
int bplustree_insert(bplustree_t *tree, int key, void *data);
int bplustree_remove(bplustree_t *tree, int key, void **data);
int bplustree_lookup(bplustree_t *tree, int key, void **data);
int bplustree_iterator(bplustree_t *tree, bpiterator_t *iter);
void bplustree_print(bplustree_t *tree);
char* bplustree_printv2(bplustree_t *tree);

#endif
