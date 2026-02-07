// copyright. zmkjh 2026/3/6 - --

#ifndef ZTREAM_CONTAINER_C
#define ZTREAM_CONTAINER_C

#include "container.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static inline ztream_container_t ztream_container_alloc(ztream_size_t single, ztream_size_t reserve) {
    ztream_container_t container;
    container.head    = 0;
    container.tail    = 0;
    container.single  = single;
    container.size    = reserve;
    container.data    = malloc(single * reserve);
    return container;
}

static inline ztream_data_t ztream_container_data(ztream_container_t* container) {
    return (unsigned char*)container->data + container->single * container->head;
}

static inline ztream_data_t ztream_container_get(ztream_container_t* container, ztream_size_t index) {
    return (unsigned char*)ztream_container_data(container) + container->single * (container->head + index);
}

static inline uint64_t ztream_container_size(ztream_container_t* container) {
    return container->tail - container->head;
}

static inline void ztream_container_clear(ztream_container_t* container) {
    container->head = 0;
    container->tail = 0;
}

static inline void ztream_container_resize(ztream_container_t* container, ztream_size_t size) {
    ztream_data_t new_zone = malloc(container->single * size);

    memset(new_zone, 0, container->single * size);
    ztream_size_t new_tail = container->tail - container->head <= size ?
                                container->tail - container->head : size;

    memmove(
        new_zone,
        (unsigned char*)container->data + container->single * container->head,
        container->single * new_tail
    );

    free(container->data);
    container->data = new_zone;
    container->head = 0;
    container->tail = new_tail;
    container->size = size;
}

static inline void ztream_container_revalue(ztream_container_t* container, ztream_size_t size) {
    ztream_container_resize(container, size);
    container->tail = size;
    memset(ztream_container_data(container), 0, container->single * size);
}

static inline void ztream_container_push_back(ztream_container_t* container, ztream_data_t data) {
    if (container->tail - container->head >= container->size) {
        if (container->single < ZTREAM_CONTAINER_BIG_SIZE_STD)
            ztream_container_resize(container, container->size + ZTREAM_CONTAINER_ADD_SIZE_STEP);
        else
            ztream_container_resize(container, container->size + 1);
    }

    if (container->tail == container->size) {
        if (container->head != 0) {
            ztream_size_t new_tail = container->tail - container->head;
            memmove(
                container->data,
                (unsigned char*)container->data + container->single * container->head,
                container->single * new_tail
            );
            container->head = 0;
            container->tail = new_tail;
        }
    }

    memcpy((unsigned char*)container->data + container->single * (container->tail++), data, container->single);
}

static inline void ztream_container_push_front(ztream_container_t* container, ztream_data_t data) {
    if (container->tail - container->head >= container->size) {
        if (container->single < ZTREAM_CONTAINER_BIG_SIZE_STD)
            ztream_container_resize(container, container->size + ZTREAM_CONTAINER_ADD_SIZE_STEP);
        else
            ztream_container_resize(container, container->size + 1);
    }

    if (container->head == 0) {
        memmove(
            (unsigned char*)container->data + container->single * (container->head + 1),
            (unsigned char*)container->data + container->single * container->head,
            container->single * (container->tail - container->head)
        );
        container->head++;
        container->tail++;
    }

    memcpy((unsigned char*)container->data + container->single * (--container->head), data, container->single);
}

static inline void ztream_container_pop_back(ztream_container_t* container, ztream_data_t reciever) {
    if (container->tail - container->head)
        memcpy(reciever, (unsigned char*)container->data + container->single * (--container->tail), container->single);
}

static inline void ztream_container_pop_front(ztream_container_t* container, ztream_data_t reciever) {
    if (container->tail - container->head)
        memcpy(reciever, (unsigned char*)container->data + container->single * (container->head++), container->single);
}

static inline void ztream_container_free(ztream_container_t* container) {
    free(container->data);
}

static inline ztream_list_t ztream_list_alloc(ztream_size_t single) {
    ztream_list_t list;
    list.single     = single;
    list.size       = 0;
    list.head       = (ztream_list_node_t*)malloc(ZTREAM_TYPE_SINGLE(ztream_list_node_t));
    list.tail       = (ztream_list_node_t*)malloc(ZTREAM_TYPE_SINGLE(ztream_list_node_t));
    list.head->last = ZTREAM_DATA_ENCODE(NULL);
    list.head->next = ZTREAM_DATA_ENCODE(list.tail);
    list.head->data = ZTREAM_DATA_ENCODE(NULL);
    list.tail->last = ZTREAM_DATA_ENCODE(list.head);
    list.tail->next = ZTREAM_DATA_ENCODE(NULL);
    list.tail->data = ZTREAM_DATA_ENCODE(NULL);
    return list;
}

static inline ztream_data_t ztream_list_data(ztream_list_node_t* node) {
    return node->data;
}

static inline ztream_list_node_t* ztream_list_head(ztream_list_t* list) {
    return list->head;
}

static inline ztream_list_node_t* ztream_list_tail(ztream_list_t* list) {
    return (ztream_list_node_t*)list->tail->last;
}

static inline int ztream_list_empty(ztream_list_t* list) {
    return list->size == 0;
}

static inline ztream_list_node_t* ztream_list_last(ztream_list_node_t* node) {
    return ZTREAM_DATA_DECODE(node->last, ztream_list_node_t);
}

static inline ztream_list_node_t* ztream_list_next(ztream_list_node_t* node) {
    return  ZTREAM_DATA_DECODE(node->next, ztream_list_node_t);
}

static inline void ztream_list_erase(ztream_list_t* list, ztream_list_node_t* node) {
    if (!node->last || !node->next)
        return;
    ztream_list_node_t* last = ZTREAM_DATA_DECODE(node->last, ztream_list_node_t);
    ztream_list_node_t* next = ZTREAM_DATA_DECODE(node->next, ztream_list_node_t);
    last->next               = ZTREAM_DATA_ENCODE(next);
    next->last               = ZTREAM_DATA_ENCODE(last);
    free(node->data);
    free(node);
    list->size--;
}

static inline ztream_list_node_t* ztream_list_insert(ztream_list_t* list, ztream_list_node_t* aim, ztream_data_t data) {
    if (!aim->next)
        return NULL;
    ztream_list_node_t* last = ZTREAM_DATA_DECODE(aim, ztream_list_node_t);
    ztream_list_node_t* next = ZTREAM_DATA_DECODE(aim->next, ztream_list_node_t);
    ztream_list_node_t* newn = (ztream_list_node_t*)malloc(ZTREAM_TYPE_SINGLE(ztream_list_node_t));

    newn->data = ZTREAM_DATA_ENCODE(malloc(list->single));
    memcpy(newn->data, data, list->single);

    newn->last = ZTREAM_DATA_ENCODE(last);
    newn->next = ZTREAM_DATA_ENCODE(next);
    last->next = ZTREAM_DATA_ENCODE(newn);
    next->last = ZTREAM_DATA_ENCODE(newn);
    list->size++;
    return newn;
}

static inline void ztream_list_free(ztream_list_t* list) {
    ztream_list_node_t* curr = ZTREAM_DATA_DECODE(list->head->next, ztream_list_node_t);
    ztream_list_node_t* next = ZTREAM_DATA_DECODE(curr->next, ztream_list_node_t);
    for (; next; curr = next, next = ZTREAM_DATA_DECODE(curr->next, ztream_list_node_t)) {
        free(curr->data);
        free(curr);
    }
    free(list->head);
    free(list->tail);
}

static inline ztream_hive_block_t ztream_hive_block_alloc(ztream_size_t single, ztream_size_t size, ztream_size_t id) {
    ztream_hive_block_t block;
    block.id         = id;
    block.data       = ztream_container_alloc(single, 0);
    block.skip_field = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_size_t), 0);
    block.free_nodes = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_list_node_t*), size);
    ztream_container_revalue(&block.data, size);
    ztream_container_revalue(&block.skip_field, size);
    ztream_container_revalue(&block.free_nodes, size);
    block.free_list = ztream_list_alloc(ZTREAM_TYPE_SINGLE(ztream_hive_block_itor_t));

    *ZTREAM_DATA_DECODE(ztream_container_get(&block.skip_field, 0), ztream_size_t)      = size;
    *ZTREAM_DATA_DECODE(ztream_container_get(&block.skip_field, size-1), ztream_size_t) = size;

    ztream_hive_block_itor_t id_head = 0;

    *ZTREAM_DATA_DECODE(ztream_container_get(&block.free_nodes, 0), ztream_list_node_t*) =
        ztream_list_insert(&block.free_list, ztream_list_head(&block.free_list), &id_head);

    return block;
}

static inline int ztream_hive_block_has_free(ztream_hive_block_t* block) {
    return ZTREAM_DATA_DECODE(block->free_list.head->next, ztream_list_node_t)->next != NULL;
}

static inline ztream_hive_block_itor_t ztream_hive_block_emplace(ztream_hive_block_t* block) {
    ztream_list_node_t* free_id_node = ZTREAM_DATA_DECODE(ztream_list_head(&block->free_list)->next, ztream_list_node_t);
    if (!free_id_node) return 0;

    ztream_hive_block_itor_t* free_id = ZTREAM_DATA_DECODE(ztream_list_data(free_id_node), ztream_hive_block_itor_t);

    if (*free_id >= block->skip_field.size) return 0;

    ztream_size_t* free_id_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, *free_id), ztream_size_t);
    if (*free_id_skip == 0) return 0;

    ztream_hive_block_itor_t free_end = *free_id + *free_id_skip - 1;
    if (free_end >= block->skip_field.size) return 0;

    ztream_size_t* free_end_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, free_end), ztream_size_t);

    ztream_hive_block_itor_t result = *free_id;

    // alloc the first block
    *free_end_skip = *free_end_skip - 1;
    *free_id_skip = 0;

    if (*free_end_skip == 0) {
        ztream_list_erase(&block->free_list, free_id_node);

        // clean up
        if (result < block->free_nodes.size) {
            *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, result), ztream_list_node_t*) = NULL;
        }
    } else {
        ztream_hive_block_itor_t free_next = *free_id + 1;

        if (free_next < block->skip_field.size) {
            ztream_size_t* free_next_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, free_next), ztream_size_t);

            if (free_next < block->free_nodes.size) {
                ztream_list_node_t** free_next_free_node = ZTREAM_DATA_DECODE(
                    ztream_container_get(&block->free_nodes, free_next), ztream_list_node_t*);

                *free_id = free_next;
                *free_next_skip = *free_end_skip;
                *free_next_free_node = free_id_node;

                // clean up
                if (result < block->free_nodes.size) {
                    *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, result), ztream_list_node_t*) = NULL;
                }
            }
        }
    }

    return result + 1;
}


static inline void ztream_hive_block_release(ztream_hive_block_t* block, ztream_hive_block_itor_t id) {
    id--;

    if (id >= block->data.size) return;

    ztream_size_t* skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, id), ztream_size_t);

    // merge the left side
    if (id > 0) {
        ztream_hive_block_itor_t last_id = id - 1;
        ztream_size_t* last_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, last_id), ztream_size_t);

        if (*last_skip != 0) {
            // merge
            ztream_hive_block_itor_t grand_id = last_id - *last_skip + 1;
            ztream_size_t* grand_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, grand_id), ztream_size_t);

            // update size
            *grand_skip = *grand_skip + 1;
            *skip = *grand_skip;  // update the end point
        } else {
            // create a new free block whose size is 1
            *skip = 1;
            ztream_hive_block_itor_t temp_id = id;
            ztream_list_node_t* new_node = ztream_list_insert(&block->free_list,
                ztream_list_head(&block->free_list), &temp_id);
            if (id < block->free_nodes.size) {
                *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, id), ztream_list_node_t*) = new_node;
            }
        }
    } else {
        // create a new free block whose size is 1
        *skip = 1;
        ztream_hive_block_itor_t temp_id = id;
        ztream_list_node_t* new_node = ztream_list_insert(&block->free_list,
            ztream_list_head(&block->free_list), &temp_id);
        if (id < block->free_nodes.size) {
            *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, id), ztream_list_node_t*) = new_node;
        }
    }

    // merge the right side
    if (id + 1 < block->data.size) {
        ztream_hive_block_itor_t next_id = id + 1;
        ztream_size_t* next_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, next_id), ztream_size_t);

        if (*next_skip != 0) {
            // merge
            ztream_hive_block_itor_t grand_id = next_id + *next_skip - 1;
            ztream_size_t* grand_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, grand_id), ztream_size_t);

            // find its start
            ztream_hive_block_itor_t before_id = id;
            ztream_size_t* before_skip = skip;

            // if *skip > 1, the left side had been merged, so it starts from id - (*skip - 1)
            if (*skip > 1) {
                before_id = id - (*skip - 1);
                before_skip = ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, before_id), ztream_size_t);
            }

            // erase the useless node
            if (next_id < block->free_nodes.size) {
                ztream_list_node_t* right_node = *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, next_id), ztream_list_node_t*);
                if (right_node) {
                    ztream_list_erase(&block->free_list, right_node);
                    *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, next_id), ztream_list_node_t*) = NULL;
                }
            }

            // sum the size
            ztream_size_t new_size = *before_skip + *next_skip;
            *before_skip = new_size;
            *grand_skip = new_size;

            // when current block is an inner space which means it had been merged with the left one, clean it.
            if (before_id != id) {
                *skip = 0;  // clean
            }

            // clean up
            if (next_id < block->free_nodes.size) {
                *ZTREAM_DATA_DECODE(ztream_container_get(&block->free_nodes, next_id), ztream_list_node_t*) = NULL;
            }
        }
    }
}



static inline ztream_data_t ztream_hive_block_data(ztream_hive_block_t* block, ztream_hive_block_itor_t id) {
    id--;

    return ztream_container_get(&block->data, id);
}

static inline ztream_hive_block_itor_t ztream_hive_block_next(ztream_hive_block_t* block, ztream_hive_block_itor_t id) {
    if (!block) {
        return 0;
    }

    id--;

    if (id + 1 >= ztream_container_size(&block->skip_field)) {
        return 0;
    }

    ztream_hive_block_itor_t result = id + 1 + *ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, id + 1), ztream_size_t);
    if (result >= ztream_container_size(&block->data)) {
        return 0;
    }

    return result + 1;
}


static inline ztream_hive_block_itor_t ztream_hive_block_iterator(ztream_hive_block_t* block) {
    if (!block) {
        return 0;
    }

    ztream_hive_block_itor_t result = *ZTREAM_DATA_DECODE(ztream_container_get(&block->skip_field, 0), ztream_size_t);

    if (result >= ztream_container_size(&block->data)) {
        return 0;
    }

    return result + 1;
}

static inline void ztream_hive_block_iterate(ztream_hive_block_t* block, ztream_handler_t handler) {
    ztream_hive_block_itor_t itor = ztream_hive_block_iterator(block);
    for (; itor; itor = ztream_hive_block_next(block, itor)) {
        handler(ztream_hive_block_data(block, itor));
    }
}

static inline void ztream_hive_block_free(ztream_hive_block_t* block) {
    ztream_container_free(&block->data);
    ztream_container_free(&block->skip_field);
    ztream_container_free(&block->free_nodes);
    ztream_list_free(&block->free_list);
}

static inline ztream_hive_t ztream_hive_alloc(ztream_size_t single) {
    ztream_hive_t hive;
    hive.single         = single;
    hive.block_list     = ztream_list_alloc(ZTREAM_TYPE_SINGLE(ztream_hive_block_t));
    hive.free_blocks    = ztream_list_alloc(ZTREAM_TYPE_SINGLE(ztream_list_node_t*));

    ztream_hive_block_t first_block = ztream_hive_block_alloc(single, ZTREAM_HIVE_BLOCK_FIRST_SIZE, 0);
    ztream_list_node_t* new_node = ztream_list_insert(&hive.block_list, ztream_list_head(&hive.block_list), ZTREAM_DATA_ENCODE(&first_block));
    ztream_list_insert(&hive.free_blocks, ztream_list_head(&hive.free_blocks), ZTREAM_DATA_ENCODE(&new_node));

    return hive;
}

static inline ztream_hive_itor_t ztream_hive_emplace(ztream_hive_t* hive) {
    if (ztream_list_empty(&hive->free_blocks)) {
        ztream_hive_block_t* last_block  = ZTREAM_DATA_DECODE(ztream_list_data(ztream_list_tail(&hive->block_list)), ztream_hive_block_t);
        ztream_hive_block_t  new_block   = ztream_hive_block_alloc(hive->single, 2 * ztream_container_size(&last_block->data), last_block->id + 1);
        ztream_list_node_t*  new_node    = ztream_list_insert(&hive->block_list, ztream_list_tail(&hive->block_list), ZTREAM_DATA_ENCODE(&new_block));
        ztream_list_insert(&hive->free_blocks, ztream_list_head(&hive->free_blocks), ZTREAM_DATA_ENCODE(&new_node));
    }

    ztream_list_node_t*         node        = ztream_list_next(ztream_list_head(&hive->free_blocks));
    ztream_list_node_t*         block_node  = *ZTREAM_DATA_DECODE(ztream_list_data(node), ztream_list_node_t*);
    ztream_hive_block_t*        block       = ZTREAM_DATA_DECODE(ztream_list_data(block_node), ztream_hive_block_t);
    ztream_hive_block_itor_t    block_itor  = ztream_hive_block_emplace(block);
    if (!ztream_hive_block_has_free(block)) {
        ztream_list_erase(&hive->free_blocks, node);
    }
    return (ztream_hive_itor_t){1, block_node, block_itor};
}

static inline void ztream_hive_release(ztream_hive_t* hive, ztream_hive_itor_t* itor) {
    if(!itor->valid)
        return;

    ztream_hive_block_t* block = ZTREAM_DATA_DECODE(ztream_list_data(itor->block_node), ztream_hive_block_t);
    if (!ztream_hive_block_has_free(block))
        ztream_list_insert(&hive->free_blocks, ztream_list_head(&hive->free_blocks), ZTREAM_DATA_ENCODE(&itor->block_node));
    ztream_hive_block_release(block, itor->block_itor);
    itor->valid = 0;
    itor->block_node = NULL;
    itor->block_itor = 0;
}

static inline ztream_data_t ztream_hive_data(ztream_hive_t* hive, ztream_hive_itor_t* itor) {
    if (!itor->valid || !itor->block_node)
        return NULL;

    ztream_hive_block_t* block = ZTREAM_DATA_DECODE(ztream_list_data(itor->block_node), ztream_hive_block_t);
    if (!block)
        return NULL;

    return ztream_hive_block_data(block, itor->block_itor);
}

static inline ztream_hive_itor_t ztream_hive_iterator(ztream_hive_t* hive) {
    ztream_list_node_t* block_node = ztream_list_next(ztream_list_head(&hive->block_list));

    while (block_node) {
        ztream_hive_block_t* block = ZTREAM_DATA_DECODE(ztream_list_data(block_node), ztream_hive_block_t);
        if (!block) {
            // it's the end
            break;
        }

        ztream_hive_block_itor_t block_itor = ztream_hive_block_iterator(block);
        if (block_itor) {
            // found it out
            return (ztream_hive_itor_t){1, block_node, block_itor};
        }

        // move to the next
        block_node = ztream_list_next(block_node);
    }

    // failed
    return (ztream_hive_itor_t){0, NULL, 0};
}

static inline int ztream_hive_iterator_valid(ztream_hive_itor_t* itor) {
    return itor->valid && itor->block_node;
}

static inline ztream_hive_itor_t ztream_hive_next(ztream_hive_t* hive, ztream_hive_itor_t* itor) {
    if (!ztream_hive_iterator_valid(itor))
        return (ztream_hive_itor_t){0, NULL, 0};

    ztream_hive_block_t* block = ZTREAM_DATA_DECODE(ztream_list_data(itor->block_node), ztream_hive_block_t);
    if (!block) {
        return (ztream_hive_itor_t){0, NULL, 0};
    }

    ztream_hive_block_itor_t next_block_itor = ztream_hive_block_next(block, itor->block_itor);

    if (!next_block_itor) {
        // when current block has no free elements, try to find another.
        ztream_list_node_t* block_node = ztream_list_next(itor->block_node);

        // iterate all
        while (block_node) {
            block = ZTREAM_DATA_DECODE(ztream_list_data(block_node), ztream_hive_block_t);
            if (!block) {
                // it's the end
                break;
            }

            ztream_hive_block_itor_t block_itor = ztream_hive_block_iterator(block);
            if (block_itor) {
                // found it out
                return (ztream_hive_itor_t){1, block_node, block_itor};
            }

            // next turn
            block_node = ztream_list_next(block_node);
        }

        // failed
        return (ztream_hive_itor_t){0, NULL, 0};
    }

    // done
    return (ztream_hive_itor_t){1, itor->block_node, next_block_itor};
}

static inline void ztream_hive_iterate(ztream_hive_t* hive, ztream_handler_t handler) {
    ztream_hive_itor_t itor = ztream_hive_iterator(hive);
    ztream_hive_itor_t next = ztream_hive_next(hive, &itor);
    for (; ztream_hive_iterator_valid(&itor); itor = next, next = ztream_hive_next(hive, &itor)) {
        handler(ztream_hive_data(hive, &itor));
    }
}

static inline void ztream_hive_free(ztream_hive_t* hive) {
    ztream_list_node_t*      block_node = ztream_list_next(ztream_list_head(&hive->block_list));
    ztream_hive_block_t*     block      = ZTREAM_DATA_DECODE(ztream_list_data(block_node), ztream_hive_block_t);
    while (ztream_list_next(block_node)) {
        ztream_hive_block_free(block);
        block_node = ztream_list_next(block_node);
        block      = ZTREAM_DATA_DECODE(ztream_list_data(block_node), ztream_hive_block_t);
    }
    ztream_list_free(&hive->block_list);
    ztream_list_free(&hive->free_blocks);
}

#endif
