// copyright. zmkjh 2025/12/1 - --

#ifndef ZTREAM_ECS_H
#define ZTREAM_ECS_H

#include "container.h"
#include <string.h>

#define ZTREAM_SYSTEM_NUM_MAX 0x3f3f3f3f

#define ZTREAM_DATA_GET(dst, data) {dst = ((typeof(dst))data);}

typedef ztream_hive_itor_t  ztream_entity_t;
typedef ztream_size_t       ztream_property_t;
typedef ztream_size_t       ztream_trait_t;
typedef ztream_size_t       ztream_system_t;

typedef void (*ztream_ctor_t)(ztream_data_t, ztream_data_t);
typedef void (*ztream_dtor_t)(ztream_data_t);

typedef ztream_handler_t ztream_entity_handler_t;
typedef ztream_handler_t ztream_property_handler_t;

typedef struct {
    // type: ztream_hive_itor_t
    ztream_container_t properties;
    // type: ztream_hive_itor_t;
    ztream_container_t traits;
} ztream_entity_struct_t;

typedef struct {
    ztream_ctor_t ctor;
    ztream_dtor_t dtor;
    ztream_hive_t hive;
} ztream_property_struct_t;

typedef struct {
    // type: ztream_property_t
    ztream_container_t properties;
    // type: ztream_entity_t
    ztream_hive_t      entities;
} ztream_trait_struct_t;

typedef struct {
    int for_trait;
    union {
        ztream_property_t   property;
        ztream_trait_t      trait;
    } subject;
    union {
        ztream_property_handler_t   property;
        ztream_entity_handler_t     entity;
    } handler;

    ztream_size_t       degree;
    // typedef: ztream_system_t
    ztream_container_t  contribute;
} ztream_system_struct_t;

typedef struct {
    // type: ztream_entity_struct_t
    ztream_hive_t      entities;
    // type: ztream_property_struct_t
    ztream_container_t properties;
    // type: ztream_trait_struct_t
    ztream_container_t traits;
    // type: ztream_system_struct_t
    ztream_container_t systems;
    // type: ztream_system_t
    ztream_container_t system_sequence;
    int                system_sequence_should_reload;
} ztream_ecs_t;

extern ztream_ecs_t ztream_ecs;

static inline void ztream_ecs_init();
static inline void ztream_ecs_system_reload();
static inline void ztream_ecs_run();

static inline ztream_entity_t       ztream_entity();
static inline void                  ztream_entity_add_property(ztream_entity_t* entity, ztream_property_t property, ztream_data_t init);
static inline void                  ztream_entity_del_property(ztream_entity_t* entity, ztream_property_t property);
static inline int                   ztream_entity_check_trait(ztream_entity_t* entity, ztream_trait_t trait);
static inline void                  ztream_entity_throw_trait(ztream_entity_t* entity, ztream_trait_t trait);
static inline ztream_data_t         ztream_entity_get(ztream_entity_t* entity, ztream_property_t property);
static inline void                  ztream_entity_erase(ztream_entity_t* entity);

static inline ztream_property_t     ztream_property(ztream_size_t single, ztream_ctor_t ctor, ztream_dtor_t dtor);
static inline ztream_trait_t        ztream_trait(ztream_property_t* property, ztream_size_t property_num);

static inline ztream_system_t       ztream_property_system(ztream_property_t property, ztream_property_handler_t handler);
static inline ztream_system_t       ztream_trait_system(ztream_trait_t trait, ztream_entity_handler_t handler);
static inline void                  ztream_system_add_dependency(ztream_system_t system, ztream_system_t dependency);
static inline void                  ztream_system_lock(ztream_system_t system);
static inline void                  ztream_system_unlock(ztream_system_t system);

// move a property to a register place
static inline void  ztream_entity_move_property_out(ztream_entity_t* entity, ztream_property_t property, ztream_data_t reg);
// move a property from a register place
static inline void  ztream_entity_move_property_in(ztream_entity_t* entity, ztream_property_t property, ztream_data_t reg);

typedef struct {
    int             valid;
    ztream_data_t   data;
} ztream_entity_property_register_t;

typedef struct {
    int                valid;
    // ztream_entity_property_register_t
    ztream_container_t properties;
    // int for bool
    ztream_container_t traits;
} ztream_entity_info_t;

/* get info of a entity
/* @note sometimes it's safety relies on your dtor, you may use it only when the property don't have a essencial dtor
*/
static inline ztream_entity_info_t  ztream_entity_clone(ztream_entity_t* entity);
// move a entity to a register place
static inline ztream_entity_info_t  ztream_entity_move_out(ztream_entity_t* entity);
// move a entity from a register place
static inline ztream_entity_t       ztream_entity_move_in(ztream_entity_info_t* info);

typedef struct {
    int                paused;
    // type: ztream_entity_t
    ztream_container_t entities;
    // type: ztream_entity_info_t
    ztream_container_t storage;
    // type: ztream_system_t
    ztream_container_t systems;
} ztream_scene_t;

static inline ztream_scene_t    ztream_scene();
static inline void              ztream_scene_relate_entity(ztream_scene_t* scene, ztream_entity_t* entity);
static inline void              ztream_scene_relate_system(ztream_scene_t* scene, ztream_system_t* system);
static inline void              ztream_scene_pause(ztream_scene_t* scene);
static inline void              ztream_scene_launch(ztream_scene_t* scene);
static inline void              ztream_scene_erase(ztream_scene_t* scene);

#endif
