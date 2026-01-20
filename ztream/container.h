// copyright. zjh 2025/11/23 - --

#ifndef ZTREAM_CONTAINER_H
#define ZTREAM_CONTAINER_H

#include <inttypes.h>

typedef void*    ztream_data_t;
typedef uint32_t ztream_size_t;

typedef void(*ztream_handler_t)(ztream_data_t);

#define ZTREAM_TYPE_SINGLE(type)        (sizeof(type))
#define ZTREAM_DATA_ENCODE(data)        ((ztream_data_t) (data))
#define ZTREAM_DATA_DECODE(data, type)  ((type*) (data))

#define ZTREAM_CONTAINER_ADD_SIZE_STEP (8)
#define ZTREAM_CONTAINER_BIG_SIZE_STD  (512/8)

typedef struct {
    ztream_size_t single;
    ztream_data_t data;
    ztream_size_t size;
    ztream_size_t head;
    ztream_size_t tail;
} ztream_container_t;

static inline ztream_container_t  ztream_container_alloc(ztream_size_t single, ztream_size_t reserve);
static inline ztream_data_t       ztream_container_data(ztream_container_t* container);
static inline ztream_data_t       ztream_container_get(ztream_container_t* container, ztream_size_t index);
static inline uint64_t            ztream_container_size(ztream_container_t* container);
static inline void                ztream_container_clear(ztream_container_t* container);
static inline void                ztream_container_resize(ztream_container_t* container, ztream_size_t size);
static inline void                ztream_container_revalue(ztream_container_t* container, ztream_size_t size);
static inline void                ztream_container_push_back(ztream_container_t* container, ztream_data_t data);
static inline void                ztream_container_push_front(ztream_container_t* container, ztream_data_t data);
static inline void                ztream_container_pop_back(ztream_container_t* container, ztream_data_t reciever);
static inline void                ztream_container_pop_front(ztream_container_t* container, ztream_data_t reciever);
static inline void                ztream_container_free(ztream_container_t* container);

typedef struct {
    ztream_data_t last;
    ztream_data_t next;
    ztream_data_t data;
} ztream_list_node_t;

typedef struct {
    ztream_size_t       single;
    ztream_size_t       size;
    ztream_list_node_t* head;
    ztream_list_node_t* tail;
} ztream_list_t;

static inline ztream_list_t        ztream_list_alloc(ztream_size_t single);
static inline ztream_list_node_t*  ztream_list_head(ztream_list_t* list);
static inline ztream_list_node_t*  ztream_list_tail(ztream_list_t* list);
static inline int                  ztream_list_empty(ztream_list_t* list);
static inline ztream_data_t        ztream_list_data(ztream_list_node_t* node);
static inline ztream_list_node_t*  ztream_list_last(ztream_list_node_t* node);
static inline ztream_list_node_t*  ztream_list_next(ztream_list_node_t* node);
static inline ztream_list_node_t*  ztream_list_insert(ztream_list_t* list, ztream_list_node_t* aim, ztream_data_t data);
static inline void                 ztream_list_erase(ztream_list_t* list, ztream_list_node_t* node);
static inline void                 ztream_list_free(ztream_list_t* list);

#define ZTREAM_HIVE_BLOCK_FIRST_SIZE (4)

// start from 1
typedef ztream_size_t ztream_hive_block_itor_t;

typedef struct {
    ztream_size_t       id;
    // @note type: the type indeed
    ztream_container_t  data;
    // @note type: ztream_size_t
    ztream_container_t  skip_field;
    // @note type: ztream_list_node_t*
    ztream_container_t  free_nodes;
    // @note type: ztream_hive_block_itor_t
    ztream_list_t       free_list;
} ztream_hive_block_t;

static inline ztream_hive_block_t       ztream_hive_block_alloc(ztream_size_t single, ztream_size_t size, ztream_size_t id);
static inline int                       ztream_hive_block_has_free(ztream_hive_block_t* block);
static inline ztream_hive_block_itor_t  ztream_hive_block_emplace(ztream_hive_block_t* block);
static inline void                      ztream_hive_block_release(ztream_hive_block_t* block, ztream_hive_block_itor_t itor);
static inline ztream_data_t             ztream_hive_block_data(ztream_hive_block_t* block, ztream_hive_block_itor_t itor);
static inline ztream_hive_block_itor_t  ztream_hive_block_iterator(ztream_hive_block_t* block);
static inline ztream_hive_block_itor_t  ztream_hive_block_next(ztream_hive_block_t* hive, ztream_hive_block_itor_t itor);
static inline void                      ztream_hive_block_iterate(ztream_hive_block_t* block, ztream_handler_t handler);
static inline void                      ztream_hive_block_free(ztream_hive_block_t* block);

#define ZTREAM_HIVE_INCREASE_MULTIPLE 2

typedef struct {
    int                         valid;
    ztream_list_node_t*         block_node;
    ztream_hive_block_itor_t    block_itor;
} ztream_hive_itor_t;

typedef struct {
    ztream_size_t single;
    // @note type: ztream_hive_block_t
    ztream_list_t block_list;
    // @note type: ztream_list_node_t*
    ztream_list_t free_blocks;
} ztream_hive_t;

static inline ztream_hive_t         ztream_hive_alloc(ztream_size_t single);
static inline ztream_hive_itor_t    ztream_hive_emplace(ztream_hive_t* hive);
static inline void                  ztream_hive_release(ztream_hive_t* hive, ztream_hive_itor_t* itor);
static inline ztream_data_t         ztream_hive_data(ztream_hive_t* hive, ztream_hive_itor_t* itor);
static inline ztream_hive_itor_t    ztream_hive_iterator(ztream_hive_t* hive);
static inline int                   ztream_hive_iterator_valid(ztream_hive_itor_t* itor);
static inline ztream_hive_itor_t    ztream_hive_next(ztream_hive_t* hive, ztream_hive_itor_t* itor);
static inline void                  ztream_hive_iterate(ztream_hive_t* hive, ztream_handler_t handler);
static inline void                  ztream_hive_free(ztream_hive_t* hive);

#endif