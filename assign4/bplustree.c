#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bplustree.h"
#include "tables.h"

int bpiterator_next(bpiterator_t *iter, void **data)
{
    if (iter->index >= iter->length) {
        return BPITERATOR_RET_END;
    }

    *data = iter->data[(iter->index)++];

    return BPITERATOR_RET_OK;
}

int bpiterator_clear(bpiterator_t *iter)
{
    iter->length = 0;
    iter->index = 0;
    free(iter->data);

    return BPITERATOR_RET_OK;
}

int bpnode_init(bpnode_t *node, enum bpnode_type type, struct bplustree *tree)
{
    node->type = type;
    node->len = 0;
    node->keys = (int*) malloc(sizeof(int) * (tree->n + 1));
    switch (node->type) {
        case NONLEAF:
            node->nodes = 
                    (bpnode_t**) malloc(sizeof(bpnode_t*) * (tree->n + 2));
            node->data = NULL;
            node->next = NULL;
            break;
        case LEAF:
            node->nodes = NULL;
            node->data = (void**) malloc(sizeof(void*) * (tree->n + 1));
            node->next = NULL;
            break;
        default:
            return BPNODE_RET_ERR;
    }
    node->tree = tree;

    return BPNODE_RET_OK;
}

int bpnode_destroy(bpnode_t *node)
{
    int rc, i;

    switch (node->type) {
        case NONLEAF:
            for (i = 0; i <= node->len; i++) {
                if (node->nodes[i] == NULL) {
                    continue;
                }
                rc = bpnode_destroy(node->nodes[i]);
                if (rc != BPNODE_RET_OK) {
                    return BPNODE_RET_ERR;
                }
            }
            free(node->nodes);
            break;
        case LEAF:
            free(node->data);
            break;
        default:
            return BPNODE_RET_ERR;
    }
    
    free(node->keys);
    node->tree = NULL;
    node->next = NULL;
    node->len = 0;

    return BPNODE_RET_OK;
}

int bpnode_insert_leaf(bpnode_t *node, int key, void *data)
{
    int beg, mid, end, i;

    if (node->type != LEAF) {
        return BPNODE_RET_ERR;
    }

    if (node->len >= (node->tree->n + 1)) {
        return BPNODE_RET_ERR;
    }

    beg = 0;
    end = node->len;
    mid = (end - beg) / 2;
    while (beg < end) {
        mid = beg + ((end - beg) / 2);
        if (key < node->keys[mid]) {
            end = mid;
            continue;
        } else {
            beg = mid + 1;
            continue;
        }
    }

    for (i = node->len - 1; i >= beg; i--) {
        node->keys[i + 1] = node->keys[i];
        node->data[i + 1] = node->data[i];
    }
    node->keys[beg] = key;
    node->data[beg] = data;
    node->len++;

    if (node->len == (node->tree->n + 1)) {
        return BPNODE_RET_OVERFLOW;
    } else {
        return BPNODE_RET_OK;
    }
}

int bpnode_remove_leaf(bpnode_t *node, int key, void **data)
{
    int beg, mid, end, i;

    if (node->type != LEAF) {
        return BPNODE_RET_ERR;
    }

    if (node->len <= 0) {
        return BPNODE_RET_ERR;
    }

    beg = 0;
    end = node->len;
    mid = (end - beg) / 2;
    while (beg < end) {
        mid = beg + ((end - beg) / 2);
        if (key == node->keys[mid]) {
            break;
        }
        if (key < node->keys[mid]) {
            end = mid;
        } else {
            beg = mid + 1;
        }
    }

    if (key != node->keys[mid]) {
        return BPNODE_RET_ERR;
    }

    *data = node->data[mid];
    for (i = mid; i < node->len - 1; i++) {
        node->keys[i] = node->keys[i + 1];
        node->data[i] = node->data[i + 1];
    }
    node->len--;

    if (node->len < (node->tree->n / 2)) {
        return BPNODE_RET_UNDERFLOW;
    } else {
        return BPNODE_RET_OK;
    }
}

int bpnode_lookup_leaf(bpnode_t *node, int key, void **data)
{
    int beg, mid, end;

    if (node->type != LEAF) {
        return BPNODE_RET_ERR;
    }

    if (node->len <= 0) {
        return BPNODE_RET_ERR;
    }

    beg = 0;
    end = node->len;
    mid = beg + ((end - beg) / 2);
    while (beg < end) {
        mid = beg + ((end - beg) / 2);
        if (key == node->keys[mid]) {
            break;
        }
        if (key < node->keys[mid]) {
            end = mid;
            continue;
        } else {
            beg = mid + 1;
            continue;
        }
    }

    if (key != node->keys[mid]) {
        return BPNODE_RET_ERR;
    }

    *data = node->data[mid];

    return BPNODE_RET_OK;
}

int bpnode_insert_nonleaf(bpnode_t *node, int key, void *data)
{
    int rc, beg, mid, end, i;
    bpnode_t *nodec, *nodel, *noder;

    if (node->type != NONLEAF) {
        return BPNODE_RET_ERR;
    }

    beg = 0;
    end = node->len;
    while (beg < end) {
        mid = beg + ((end - beg) / 2);
        if (key < node->keys[mid]) {
            end = mid;
        } else {
            beg = mid + 1;
        }
    }
    
    nodec = node->nodes[beg];

    switch (nodec->type) {
        case NONLEAF:
            rc = bpnode_insert_nonleaf(nodec, key, data);
            break;
        case LEAF:
            rc = bpnode_insert_leaf(nodec, key, data);
            break;
        default:
            return BPNODE_RET_ERR;
    }
        
    switch (rc) {
        case BPNODE_RET_OK:
            return BPNODE_RET_OK;
        case BPNODE_RET_ERR:
            return BPNODE_RET_ERR;
        case BPNODE_RET_OVERFLOW:
            break;
        default:
            return BPNODE_RET_ERR;
    }

    // OVERFLOW
    nodel = (bpnode_t*) malloc(sizeof(bpnode_t));
    noder = (bpnode_t*) malloc(sizeof(bpnode_t));
    switch (nodec->type) {
        case NONLEAF:
            bpnode_init(nodel, NONLEAF, node->tree);
            bpnode_init(noder, NONLEAF, node->tree);
            break;
        case LEAF:
            bpnode_init(nodel, LEAF, node->tree);
            bpnode_init(noder, LEAF, node->tree);
            break;
        default:
            return BPNODE_RET_ERR;
    }
    rc = bpnode_split(nodec, nodel, noder);
    if (nodec->type == LEAF && beg != 0) {
        node->nodes[beg - 1]->next = nodel;
    }

    if (rc != BPNODE_RET_OK) {
        return BPNODE_RET_ERR;
    }

    node->nodes[node->len + 1] = node->nodes[node->len];
    for (i = node->len - 1; i >= beg; i--) {
        node->keys[i + 1] = node->keys[i];
        node->nodes[i + 1] = node->nodes[i];
    }
    if (nodec->type == LEAF) {
        node->keys[beg] = noder->keys[0];
    } else {
        node->keys[0] = nodec->keys[nodec->len / 2];
    }
    node->nodes[beg] = nodel;
    node->nodes[beg + 1] = noder;
    node->len++;
    node->tree->num_nodes++;
 
    switch (nodec->type) {
        case NONLEAF:
            free(nodec->nodes);
            free(nodec->keys);
            break;
        case LEAF:
            free(nodec->data);
            free(nodec->keys);
            break;
    }
    free(nodec);
    
    if (node->len == (node->tree->n + 1)) {
        return BPNODE_RET_OVERFLOW;
    } else {
        return BPNODE_RET_OK;
    }
}

int bpnode_remove_nonleaf(bpnode_t *node, int key, void **data)
{
    int rc, beg, mid, end, i;
    bpnode_t *nodec, *nodel, *noder;

    if (node->type != NONLEAF) {
        return BPNODE_RET_ERR;
    }

    if (node->len <= 0) {
        return BPNODE_RET_ERR;
    }

    beg = 0;
    end = node->len;
    mid = (end - beg) / 2;
    while (beg < end) {
        mid = beg + ((end - beg) / 2);
        if (key == node->keys[mid]) {
            break;
        }
        if (key < node->keys[mid]) {
            end = mid;
            continue;
        } else {
            beg = mid + 1;
            continue;
        }
    }

    if (key >= node->keys[mid]) {
        mid = mid + 1;
    }

    switch (node->nodes[mid]->type) {
        case NONLEAF:
            rc = bpnode_remove_nonleaf(node->nodes[mid], key, data);
            if (rc == BPNODE_RET_ERR) {
                return BPNODE_RET_ERR;
            }
            break;
        case LEAF:
            rc = bpnode_remove_leaf(node->nodes[mid], key, data);
            if (rc == BPNODE_RET_ERR) {
                return BPNODE_RET_ERR;
            }
            break;
        default:
            return BPNODE_RET_ERR;
    }

    if (rc == BPNODE_RET_OK) {
        if (mid != 0) {
            node->keys[mid - 1] = node->nodes[mid]->keys[0];
        }
        return BPNODE_RET_OK;
    }

    // UNDERFLOW
    if (mid != 0) {
        rc = bpnode_redis_from_left(node->nodes[mid - 1], node->nodes[mid]);
        if (rc == BPNODE_RET_ERR) {
            return BPNODE_RET_ERR;
        }
        if (rc == BPNODE_RET_OK) {
            node->keys[mid - 1] = node->nodes[mid]->keys[0];
            if (mid != node->len) {
                node->keys[mid] = node->nodes[mid + 1]->keys[0];
            }
            return BPNODE_RET_OK;
        }
    }

    if (mid != node->len) {
        rc = bpnode_redis_from_right(node->nodes[mid], node->nodes[mid + 1]);
        if (rc == BPNODE_RET_ERR) {
            return BPNODE_RET_ERR;
        }
        if (rc == BPNODE_RET_OK) {
            if (mid != 0) {
                node->keys[mid - 1] = node->nodes[mid]->keys[0];
            }
            node->keys[mid] = node->nodes[mid + 1]->keys[0];
            return BPNODE_RET_OK;
        }
    }

    nodec = (bpnode_t*) malloc(sizeof(bpnode_t));
    rc = bpnode_init(nodec, node->nodes[mid]->type, node->tree);
    if (rc != BPNODE_RET_OK) {
        return BPNODE_RET_ERR;
    }

    if (mid == 0) {
        nodel = node->nodes[mid];
        noder = node->nodes[mid + 1];
    } else {
        nodel = node->nodes[mid - 1];
        noder = node->nodes[mid];
    }
    
    rc = bpnode_merge(nodel, noder, nodec);
    if (rc != BPNODE_RET_OK) {
        return BPNODE_RET_ERR;
    }

    if (mid == 0) {
        for (i = 0; i < node->len; i++) {
            node->keys[i] = node->keys[i + 1];
            node->nodes[i] = node->nodes[i + 1];
        }
        node->nodes[0] = nodec;
    } else {
        for (i = mid - 1; i < node->len; i++) {
            node->keys[i] = node->keys[i + 1];
            node->nodes[i] = node->nodes[i + 1];
        }
        node->nodes[mid - 1] = nodec;
        if (mid - 2 >= 0) {
            node->nodes[mid - 2]->next = nodec;
        }
    }
    node->len--;

    if (mid != 0) {
        node->keys[mid - 1] = node->nodes[mid]->keys[0];
    }
    node->tree->num_nodes--;
     
    if (node->len < (node->tree->n / 2)) {
        return BPNODE_RET_UNDERFLOW;
    } else {
        return BPNODE_RET_OK;
    }
}

int bpnode_lookup_nonleaf(bpnode_t *node, int key, void **data)
{
    int rc, beg, mid, end;

    if (node->type != NONLEAF) {
        return BPNODE_RET_ERR;
    }

    if (node->len <= 0) {
        return BPNODE_RET_ERR;
    }

    beg = 0;
    end = node->len;
    do {
        mid = beg + ((end - beg) / 2);
        if (key == node->keys[mid]) {
            break;
        }
        if (key < node->keys[mid]) {
            end = mid;
            continue;
        } else {
            beg = mid + 1;
            continue;
        }
    } while (beg < end);
    
    if (key >= node->keys[mid]) {
        mid = mid + 1;
    }

    switch (node->nodes[mid]->type) {
        case NONLEAF:
            rc = bpnode_lookup_nonleaf(node->nodes[mid], key, data);
            break;
        case LEAF:
            rc = bpnode_lookup_leaf(node->nodes[mid], key, data);
            break;
        default:
            return BPNODE_RET_ERR;
    }

    if (rc != BPNODE_RET_OK) {
        return BPNODE_RET_ERR;
    }
    
    return BPNODE_RET_OK;
}

int bpnode_split(bpnode_t *nodec, bpnode_t *nodel, bpnode_t *noder)
{
    int i, il, ir;

    switch (nodel->type) {
        case NONLEAF:
            for (i = 0, il = 0, ir = 0; i < nodec->len; i++) {
                if (i == (nodec->len / 2)) {
                    continue;
                }
                if (i < (nodec->len / 2)) {
                    nodel->keys[il] = nodec->keys[i];
                    nodel->nodes[il] = nodec->nodes[i];
                    il++;
                } else {
                    noder->keys[ir] = nodec->keys[i];
                    noder->nodes[ir] = nodec->nodes[i];
                    ir++;
                }
            }
            nodel->nodes[il] = nodec->nodes[nodec->len / 2];
            noder->nodes[ir] = nodec->nodes[nodec->len];
            nodel->len = il;
            noder->len = ir;
            break;
        case LEAF:
            for (i = 0, il = 0, ir = 0; i < nodec->len; i++) {
                if (i <= (nodec->len / 2)) {
                    nodel->keys[il] = nodec->keys[i];
                    nodel->data[il] = nodec->data[i];
                    il++;
                } else {
                    noder->keys[ir] = nodec->keys[i];
                    noder->data[ir] = nodec->data[i];
                    ir++;
                }
            }
            nodel->next = noder;
            noder->next = nodec->next;
            nodel->len = il;
            noder->len = ir;
            break;
        default:
            return BPNODE_RET_ERR;
    }
    
    return BPNODE_RET_OK;
}

int bpnode_redis_from_left(bpnode_t *nodel, bpnode_t *noder)
{
    int i;

    if (nodel->type != noder->type) {
        return BPNODE_RET_ERR;
    }

    if (nodel->len <= (nodel->tree->n / 2)) {
        return BPNODE_RET_FAILED;
    }

    switch (nodel->type) {
        case NONLEAF:
            for (i = noder->len - 1; i >= 0; i--) {
                noder->keys[i + 1] = noder->keys[i];
                noder->nodes[i + 2] = noder->nodes[i + 1];
            }
            noder->nodes[1] = noder->nodes[0];
            noder->keys[0] = nodel->keys[nodel->len - 1];
            noder->nodes[0] = nodel->nodes[nodel->len];
            break;
        case LEAF:
            for (i = noder->len - 1; i >= 0; i--) {
                noder->keys[i + 1] = noder->keys[i];
                noder->data[i + 1] = noder->data[i];
            }
            noder->keys[0] = nodel->keys[nodel->len - 1];
            noder->data[0] = nodel->data[nodel->len - 1];
            break;
        default:
            return BPNODE_RET_ERR;
    }
    nodel->len--;
    noder->len++;

    return BPNODE_RET_OK;
}

int bpnode_redis_from_right(bpnode_t *nodel, bpnode_t *noder)
{
    int i;

    if (nodel->type != noder->type) {
        return BPNODE_RET_ERR;
    }

    if (noder->len <= (noder->tree->n / 2)) {
        return BPNODE_RET_FAILED;
    }

    switch (noder->type) {
        case NONLEAF:
            nodel->keys[nodel->len] = noder->keys[0];
            nodel->nodes[nodel->len + 1] = noder->data[0];
            noder->nodes[0] = noder->nodes[1];
            for (i = 0; i < noder->len - 1; i++) {
                noder->keys[i] = noder->keys[i + 1];
                noder->data[i + 1] = noder->data[i + 2];
            }
            break;
        case LEAF:
            nodel->keys[nodel->len] = noder->keys[0];
            nodel->data[nodel->len] = noder->data[0];
            for (i = 0; i < noder->len - 1; i++) {
                noder->keys[i] = noder->keys[i + 1];
                noder->data[i] = noder->data[i + 1];
            }
            break;
        default:
            return BPNODE_RET_ERR;
    }
    nodel->len++;
    noder->len--;
    
    return BPNODE_RET_OK;
}

int bpnode_merge(bpnode_t *nodel, bpnode_t *noder, bpnode_t *nodec)
{
    int i, j;

    switch (nodec->type) {
        case NONLEAF:
            i = 0;
            for (j = 0; j < nodel->len; j++, i++) {
                nodec->keys[i] = nodel->keys[j];
                nodec->nodes[i] = nodel->nodes[j];
            }
            nodec->nodes[i++] = nodel->nodes[nodel->len];
            for (j = 0; j < noder->len; j++, i++) {
                nodec->keys[i] = noder->keys[j];
                nodec->nodes[i] = noder->nodes[j];
            }
            nodec->nodes[i + 1] = noder->nodes[noder->len];
            nodec->len = i;
            break;
        case LEAF:
            i = 0;
            for (j = 0; j < nodel->len; j++, i++) {
                nodec->keys[i] = nodel->keys[j];
                nodec->data[i] = nodel->data[j];
            }
            for (j = 0; j < noder->len; j++, i++) {
                nodec->keys[i] = noder->keys[j];
                nodec->data[i] = noder->data[j];
            }
            nodec->len = i;
            nodec->next = noder->next;
            break;
        default:
            return BPNODE_RET_ERR;
    }

    return BPNODE_RET_OK;
}

void bpnode_print(bpnode_t *node, int depth)
{
    int i, j;

    for (i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("####\n");

    switch (node->type) {
        case NONLEAF:
            bpnode_print(node->nodes[0], depth + 1);
            for (i = 0; i < node->len; i++) {
                for (j = 0; j <= depth; j++) {
                    printf("    ");
                }
                printf("<> %d\n", node->keys[i]);
                bpnode_print(node->nodes[i + 1], depth + 1);
            }
            break;
        case LEAF:
            for (i = 0; i < node->len; i++) {
                for (j = 0; j <= depth; j++) {
                    printf("    ");
                }
                printf("* %d : %s\n", node->keys[i], (char*) node->data[i]);
            }
            break;
        default:
            return;
    }

    for (i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("----\n");

    return;
}

void bpnode_update_id(bpnode_t *node, int *id)
{
    int i;

    node->id = *id;
    (*id)++;
    if (node->type == NONLEAF) {
        for (i = 0; i <= node->len; i++) {
            bpnode_update_id(node->nodes[i], id);
        }
    }

    return;
}

char* bpnode_printv2(bpnode_t *node)
{
    int i;
    char *res, *aux, *tmp;
    RID *rid;

    res = (char*) calloc(sizeof(char), 4096);
    aux = (char*) calloc(sizeof(char), 4096);
    strcpy(res, "");
    switch (node->type) {
        case NONLEAF:
            sprintf(aux, "(%d) [", node->id);
            strcat(res, aux);
            for (i = 0; i < node->len; i++) {
                sprintf(aux, "%d,%d,", node->nodes[i]->id, node->keys[i]);
                strcat(res, aux);
            }
            sprintf(aux, "%d]\n", node->nodes[node->len]->id);
            strcat(res, aux);
            for (i = 0; i <= node->len; i++) {
                tmp = bpnode_printv2(node->nodes[i]);
                strcat(res, tmp);
                free(tmp);
            }
            break;
        case LEAF:
            sprintf(aux, "(%d) [", node->id);
            strcat(res, aux);
            for (i = 0; i < node->len; i++) {
                rid = (RID*) node->data[i];
                sprintf(aux, "%d.%d,%d,", rid->page, rid->slot, node->keys[i]);
                strcat(res, aux);
            }
            if (node->next != NULL) {
                sprintf(aux, "%d]\n", node->next->id);
                strcat(res, aux);
            } else {
                res[strlen(res) - 1] = ']';
                strcat(res, "\n");
            }
            break;
    }
    free(aux);

    return res;
}

int bplustree_init(bplustree_t *tree, int n)
{
    int rc;

    tree->n = n;
    tree->num_data = 0;
    tree->num_nodes = 1;
    tree->root = (bpnode_t*) malloc(sizeof(bpnode_t));
    rc = bpnode_init(tree->root, LEAF, tree);
    
    return rc != BPNODE_RET_OK ? BPLUSTREE_RET_ERR : BPLUSTREE_RET_OK;
}

int bplustree_destroy(bplustree_t *tree)
{
    int rc;

    rc = bpnode_destroy(tree->root);
    if (rc != BPNODE_RET_OK) {
        return BPLUSTREE_RET_ERR;
    }
    free(tree->root);
    tree->num_data = 0;
    tree->n = 0;

    return BPLUSTREE_RET_OK;
}

int bplustree_insert(bplustree_t *tree, int key, void *data)
{
    int rc;
    bpnode_t *nodec, *nodel, *noder;

    switch (tree->root->type) {
        case NONLEAF:
            rc = bpnode_insert_nonleaf(tree->root, key, data);
            if (rc == BPNODE_RET_ERR) {
                return BPLUSTREE_RET_ERR;
            }
            break;
        case LEAF:
            rc = bpnode_insert_leaf(tree->root, key, data);
            if (rc == BPNODE_RET_ERR) {
                return BPLUSTREE_RET_ERR;
            }
            break;
        default:
            return BPLUSTREE_RET_ERR;
    }

    if (rc == BPNODE_RET_OK) {
        tree->num_data++;
        return BPLUSTREE_RET_OK;
    }

    // OVERFLOW
    nodec = tree->root;
    nodel = (bpnode_t*) malloc(sizeof(bpnode_t));
    noder = (bpnode_t*) malloc(sizeof(bpnode_t));

    switch (nodec->type) {
        case NONLEAF:
            rc = bpnode_init(nodel, NONLEAF, tree);
            if (rc != BPNODE_RET_OK) {
                return BPLUSTREE_RET_ERR;
            }
            rc = bpnode_init(noder, NONLEAF, tree);
            if (rc != BPNODE_RET_OK) {
                return BPLUSTREE_RET_ERR;
            }
            break;
        case LEAF:
            rc = bpnode_init(nodel, LEAF, tree);
            if (rc != BPNODE_RET_OK) {
                return BPLUSTREE_RET_ERR;
            }
            rc = bpnode_init(noder, LEAF, tree);
            if (rc != BPNODE_RET_OK) {
                return BPLUSTREE_RET_ERR;
            }
            tree->num_nodes++;
            break;
        default:
            return BPLUSTREE_RET_ERR;
    }
    
    rc = bpnode_split(nodec, nodel, noder);
    if (rc != BPNODE_RET_OK) {
        return BPLUSTREE_RET_ERR;
    }

    tree->root = (bpnode_t*) malloc(sizeof(bpnode_t));
    rc = bpnode_init(tree->root, NONLEAF, tree);
    if (rc != BPNODE_RET_OK) {
        return BPLUSTREE_RET_ERR;
    }
    if (nodec->type == LEAF) {
        tree->root->keys[0] = noder->keys[0];
    } else {
        tree->root->keys[0] = nodec->keys[nodec->len / 2];
    }
    tree->root->nodes[0] = nodel;
    tree->root->nodes[1] = noder;
    tree->root->len = 1;
    tree->num_data++;
    tree->num_nodes++;
    
    switch (nodec->type) {
        case NONLEAF:
            free(nodec->nodes);
            free(nodec->keys);
            break;
        case LEAF:
            free(nodec->data);
            free(nodec->keys);
            break;
    }
    free(nodec);

    return BPLUSTREE_RET_OK;
}

int bplustree_remove(bplustree_t *tree, int key, void **data)
{
    int rc;
    bpnode_t *nodec;

    switch (tree->root->type) {
        case NONLEAF:
            rc = bpnode_remove_nonleaf(tree->root, key, data);
            if (rc == BPNODE_RET_ERR) {
                return BPLUSTREE_RET_ERR;
            }
            break;
        case LEAF:
            rc = bpnode_remove_leaf(tree->root, key, data);
            if (rc == BPNODE_RET_ERR) {
                return BPLUSTREE_RET_ERR;
            }
            return BPLUSTREE_RET_OK;
        default:
            return BPLUSTREE_RET_ERR;
    }

    if (rc == BPNODE_RET_OK || tree->root->len >= 2) {
        tree->num_data--;
        return BPLUSTREE_RET_OK;
    }

    // UNDERFLOW
    nodec = tree->root;
    tree->root = tree->root->nodes[0];

    nodec->nodes[0] = NULL;
    rc = bpnode_destroy(nodec);
    if (rc != BPNODE_RET_OK) {
        return BPLUSTREE_RET_ERR;
    }
    free(nodec);
    tree->num_data--;
    tree->num_nodes--;

    return BPLUSTREE_RET_OK;
}

int bplustree_lookup(bplustree_t *tree, int key, void **data)
{
    int rc;

    switch (tree->root->type) {
        case NONLEAF:
            rc = bpnode_lookup_nonleaf(tree->root, key, data);
            break;
        case LEAF:
            rc = bpnode_lookup_leaf(tree->root, key, data);
            break;
        default:
            return BPLUSTREE_RET_ERR;
    }

    return rc;
}

int bplustree_iterator(bplustree_t *tree, bpiterator_t *iter)
{
    int i, j;
    bpnode_t *node;

    iter->length = tree->num_data;
    iter->index = 0;
    iter->data = (void**) malloc(sizeof(void*) * iter->length);

    node = tree->root;
    while (node->type != LEAF) {
        node = node->nodes[0];
    }

    i = 0;
    while (node != NULL) {
        if (i >= iter->length) {
            return BPITERATOR_RET_ERR;
        }
        for (j = 0; j < node->len; j++, i++) {
            iter->data[i] = node->data[j];
        }
        node = node->next;
    }

    return BPITERATOR_RET_OK;
}

void bplustree_print(bplustree_t *tree)
{
    bpnode_print(tree->root, 0);
    return;
}

char* bplustree_printv2(bplustree_t *tree)
{
    int id;
    char *res;

    id = 0;
    bpnode_update_id(tree->root, &id);
    if (tree->root->len == 0) {
        res = (char*) calloc(sizeof(char), 4096);
        strcpy(res, "(0) []\n");
        return res;
    }

    return bpnode_printv2(tree->root);
}
