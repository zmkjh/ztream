// copyright. zjh 2025/12/26 - --

#ifndef ZTREAM_ECS_C
#define ZTREAM_ECS_C

#include "ecs.h"
#include <stdlib.h>

static inline void ztream_ecs_init() {
    ztream_ecs.entities             = ztream_hive_alloc(ZTREAM_TYPE_SINGLE(ztream_entity_struct_t));
    ztream_ecs.properties           = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_property_struct_t), 8);
    ztream_ecs.traits               = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_trait_struct_t), 4);
    ztream_ecs.systems              = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_system_struct_t), 8);
    ztream_ecs.system_sequence      = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_system_t), 8);
    ztream_ecs.system_sequence_should_reload = 1;
}

static inline void ztream_ecs_system_reload() {
    ztream_container_clear(&ztream_ecs.system_sequence);

    ztream_container_t divided_degree = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_size_t), 0);
    ztream_container_revalue(&divided_degree, ztream_container_size(&ztream_ecs.systems));

    ztream_container_t dequeue = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_system_t), ztream_container_size(&ztream_ecs.systems));

    for (ztream_system_t i = 0; i < ztream_container_size(&ztream_ecs.systems); i++) {
        ztream_system_struct_t* system = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, i), ztream_system_struct_t);
        ztream_size_t*          divide = ZTREAM_DATA_DECODE(ztream_container_get(&divided_degree, i), ztream_size_t);

        if (system->degree - *divide == 0) {
            ztream_container_push_back(&ztream_ecs.system_sequence, &i);
            ztream_container_push_front(&dequeue, ZTREAM_DATA_ENCODE(&i));
        }
    }

    while (ztream_container_size(&dequeue)) {
        ztream_system_t i;
        ztream_container_pop_back(&dequeue, ZTREAM_DATA_ENCODE(&i));
        ztream_system_struct_t* system = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, i), ztream_system_struct_t);

        for (ztream_size_t j = 0; j < ztream_container_size(&system->contribute); j++) {
            ztream_system_t         contribute                  = *ZTREAM_DATA_DECODE(ztream_container_get(&system->contribute, j), ztream_system_t);
            ztream_system_struct_t* contribute_system           = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, contribute), ztream_system_struct_t);
            ztream_size_t*          contribute_divided_degree   = ZTREAM_DATA_DECODE(ztream_container_get(&divided_degree, contribute), ztream_size_t);
            *contribute_divided_degree = *contribute_divided_degree + 1;
            if (contribute_system->degree - *contribute_divided_degree == 0) {
                ztream_container_push_back(&ztream_ecs.system_sequence, &contribute);
                ztream_container_push_front(&dequeue, ZTREAM_DATA_ENCODE(&contribute));
            }
        }
    }

    ztream_ecs.system_sequence_should_reload = 0;
}

static inline void ztream_ecs_run() {
    if (ztream_ecs.system_sequence_should_reload)
        ztream_ecs_system_reload();

    ztream_size_t sequence_size = ztream_container_size(&ztream_ecs.system_sequence);
    for (ztream_size_t i = 0; i < sequence_size; i++) {
        ztream_system_t*            system          = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.system_sequence, i), ztream_system_t);
        ztream_system_struct_t*     system_struct   = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, *system), ztream_system_struct_t);

        if (!system_struct->for_trait) {
            ztream_property_struct_t* property = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, system_struct->subject.property), ztream_property_struct_t);
            ztream_hive_iterate(&property->hive, system_struct->handler.property);
        } else {
            ztream_trait_struct_t* trait = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, system_struct->subject.trait), ztream_trait_struct_t);
            ztream_hive_iterate(&trait->entities, system_struct->handler.entity);
        }
    }
}

static inline ztream_property_t ztream_property(ztream_size_t single, ztream_ctor_t ctor, ztream_dtor_t dtor) {
    ztream_property_struct_t property;
    property.ctor = ctor;
    property.dtor = dtor;
    property.hive = ztream_hive_alloc(single);
    ztream_container_push_back(&ztream_ecs.properties, ZTREAM_DATA_ENCODE(&property));
    return ztream_container_size(&ztream_ecs.properties) - 1;
}

static inline ztream_trait_t ztream_trait(ztream_property_t* property, ztream_size_t property_num) {
    ztream_trait_struct_t trait;
    trait.properties = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_property_t), property_num);
    for (ztream_size_t i = 0; i < property_num; i++) {
        ztream_container_push_back(&trait.properties, ZTREAM_DATA_ENCODE(&property[i]));
    }
    trait.entities = ztream_hive_alloc(ZTREAM_TYPE_SINGLE(ztream_entity_t));
    ztream_container_push_back(&ztream_ecs.traits, ZTREAM_DATA_ENCODE(&trait));
    return ztream_container_size(&ztream_ecs.traits) - 1;
}

static inline ztream_system_t ztream_property_system(ztream_property_t property, ztream_property_handler_t handler) {
    ztream_system_struct_t system;
    system.for_trait          = 0;
    system.subject.property   = property;
    system.handler.property   = handler;
    system.degree             = 0;
    system.contribute         = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_system_t), 8);
    ztream_container_push_back(&ztream_ecs.systems, ZTREAM_DATA_ENCODE(&system));
    uint32_t zero = 0;
    ztream_ecs.system_sequence_should_reload = 1;
    return ztream_container_size(&ztream_ecs.systems) - 1;
}

static inline ztream_system_t ztream_trait_system(ztream_trait_t trait, ztream_entity_handler_t handler) {
    ztream_system_struct_t system;
    system.for_trait          = 1;
    system.subject.trait      = trait;
    system.handler.entity     = handler;
    system.degree             = 0;
    system.contribute         = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_system_t), 8);
    ztream_container_push_back(&ztream_ecs.systems, ZTREAM_DATA_ENCODE(&system));
    uint32_t zero = 0;
    ztream_ecs.system_sequence_should_reload = 1;
    return ztream_container_size(&ztream_ecs.systems) - 1;
}

static inline void ztream_system_add_dependency(ztream_system_t system, ztream_system_t depend) {
    ztream_system_struct_t* system_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, system), ztream_system_struct_t);
    ztream_system_struct_t* depend_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, depend), ztream_system_struct_t);
    ztream_container_push_back(&depend_struct->contribute, ZTREAM_DATA_ENCODE(&system));
    system_struct->degree++;
    ztream_ecs.system_sequence_should_reload = 1;
}

static inline void ztream_system_lock(ztream_system_t system) {
    ztream_system_struct_t* system_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, system), ztream_system_struct_t);
    if (system_struct->degree < ZTREAM_SYSTEM_NUM_MAX)
        system_struct->degree += ZTREAM_SYSTEM_NUM_MAX;
    ztream_ecs.system_sequence_should_reload = 1;
}

static inline void ztream_system_unlock(ztream_system_t system) {
    ztream_system_struct_t* system_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.systems, system), ztream_system_struct_t);
    if (system_struct->degree >= ZTREAM_SYSTEM_NUM_MAX)
        system_struct->degree -= ZTREAM_SYSTEM_NUM_MAX;
    ztream_ecs.system_sequence_should_reload = 1;
}

static inline ztream_entity_t ztream_entity() {
    ztream_entity_t          entity        = ztream_hive_emplace(&ztream_ecs.entities);
    ztream_entity_struct_t*  entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, &entity), ztream_entity_struct_t);
    entity_struct->properties              = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_hive_itor_t), 1);
    entity_struct->traits                  = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_hive_itor_t), 0);
    return entity;
}

static inline void ztream_entity_add_property(ztream_entity_t* entity, ztream_property_t property, ztream_data_t init) {
    if (!ztream_hive_iterator_valid(entity))
        return;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->properties) <= property) {
        ztream_container_resize(&entity_struct->properties, property + 1);
        (&entity_struct->properties)->tail = property + 1;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
    if (ztream_hive_iterator_valid(itor_data)) {
        return;
    }
    ztream_property_struct_t* property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
    *itor_data = ztream_hive_emplace(&property_struct->hive);
    property_struct->ctor(ztream_hive_data(&property_struct->hive, itor_data), init);
}

static inline void ztream_entity_del_property(ztream_entity_t* entity, ztream_property_t property) {
    if (!ztream_hive_iterator_valid(entity))
        return;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->properties) <= property) {
        return;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
    if (!ztream_hive_iterator_valid(itor_data)) {
        return;
    }
    ztream_property_struct_t* property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
    property_struct->dtor(ztream_hive_data(&property_struct->hive, itor_data));
    ztream_hive_release(&property_struct->hive, itor_data);
}

static inline void ztream_entity_move_property_out(ztream_entity_t* entity, ztream_property_t property, ztream_data_t reg) {
    if (!ztream_hive_iterator_valid(entity))
        return;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->properties) <= property) {
        return;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
    if (!ztream_hive_iterator_valid(itor_data)) {
        return;
    }
    ztream_property_struct_t* property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
    memcpy(reg, ztream_hive_data(&property_struct->hive, itor_data), property_struct->hive.single);
    ztream_hive_release(&property_struct->hive, itor_data);
}

static inline void ztream_entity_move_property_in(ztream_entity_t* entity, ztream_property_t property, ztream_data_t reg) {
    if (!ztream_hive_iterator_valid(entity))
        return;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->properties) <= property) {
        ztream_container_resize(&entity_struct->properties, property + 1);
        (&entity_struct->properties)->tail = property + 1;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
    if (ztream_hive_iterator_valid(itor_data)) {
        return;
    }
    ztream_property_struct_t* property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
    *itor_data = ztream_hive_emplace(&property_struct->hive);
    memcpy(ztream_hive_data(&property_struct->hive, itor_data), reg, property_struct->hive.single);
}

static inline ztream_entity_info_t ztream_entity_clone(ztream_entity_t* entity) {
    ztream_entity_info_t info;
    info.valid = 0;
    if (!ztream_hive_iterator_valid(entity))
        return info;

    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    info.properties = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_entity_property_register_t), 0);
    info.traits     = ztream_container_alloc(ZTREAM_TYPE_SINGLE(int), 0);
    ztream_container_revalue(&info.properties, ztream_container_size(&entity_struct->properties));
    ztream_container_revalue(&info.traits, ztream_container_size(&entity_struct->traits));

    for (int property = 0; property < ztream_container_size(&entity_struct->properties); property++) {
        ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
        ztream_property_struct_t*   property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
        if (ztream_hive_iterator_valid(itor_data)) {
            ztream_entity_property_register_t* reg = ZTREAM_DATA_DECODE(ztream_container_get(&info.properties, property), ztream_entity_property_register_t);
            reg->data = malloc(property_struct->hive.single);
            memcpy(reg->data, ztream_hive_data(&property_struct->hive, itor_data), property_struct->hive.single);
            reg->valid = 1;
        }
    }
    for (int trait = 0; trait < ztream_container_size(&entity_struct->traits); trait++) {
        ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->traits, trait), ztream_hive_itor_t);
        ztream_trait_struct_t*      trait_struct    = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, trait), ztream_trait_struct_t);
        if (ztream_hive_iterator_valid(itor_data)) {
            int* valid = ZTREAM_DATA_DECODE(ztream_container_get(&info.traits, trait), int);
            *valid = 1;
        }
    }

    info.valid = 1;
    return info;
}

static inline ztream_entity_info_t ztream_entity_move_out(ztream_entity_t* entity) {
    ztream_entity_info_t info = ztream_entity_clone(entity);

    // erase the entity without dtor
    if (!ztream_hive_iterator_valid(entity))
        return info;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    for (int property = 0; property < ztream_container_size(&entity_struct->properties); property++) {
        ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
        ztream_property_struct_t*   property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
        if (ztream_hive_iterator_valid(itor_data)) {
            ztream_hive_release(&property_struct->hive, itor_data);
        }
    }
    for (int trait = 0; trait < ztream_container_size(&entity_struct->traits); trait++) {
        ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->traits, trait), ztream_hive_itor_t);
        ztream_trait_struct_t*      trait_struct    = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, trait), ztream_trait_struct_t);
        if (ztream_hive_iterator_valid(itor_data)) {
            ztream_hive_release(&trait_struct->entities, itor_data);
        }
    }
    ztream_container_free(&entity_struct->properties);
    ztream_container_free(&entity_struct->traits);
    ztream_hive_release(&ztream_ecs.entities, entity);

    return info;
}

static inline ztream_entity_t ztream_entity_move_in(ztream_entity_info_t* info) {
    ztream_entity_t entity = ztream_entity();
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, &entity), ztream_entity_struct_t);
    
    ztream_container_revalue(&entity_struct->properties, ztream_container_size(&info->properties));
    ztream_container_revalue(&entity_struct->traits, ztream_container_size(&info->traits));

    for (int property = 0; property < ztream_container_size(&info->properties); property++) {
        ztream_entity_property_register_t* reg = ztream_container_get(&info->properties, property);
        if (reg->valid) {
            ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
            ztream_property_struct_t*   property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
            
            *itor_data = ztream_hive_emplace(&property_struct->hive);
            memcpy(ztream_hive_data(&property_struct->hive, itor_data), reg->data, property_struct->hive.single);
        }
    }
    for (int trait = 0; trait < ztream_container_size(&info->traits); trait++) {
        int* valid = ztream_container_get(&info->traits, trait);
        if (*valid) {
            ztream_hive_itor_t*     itor_data    = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->traits, trait), ztream_hive_itor_t);
            ztream_trait_struct_t*  trait_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, trait), ztream_trait_struct_t);

            *itor_data = ztream_hive_emplace(&trait_struct->entities);
            *ZTREAM_DATA_DECODE(ztream_hive_data(&trait_struct->entities, itor_data), ztream_entity_t) = entity;
        }
    }

    return entity;
}

static inline int ztream_entity_check_trait(ztream_entity_t* entity, ztream_trait_t trait) {
    if (!ztream_hive_iterator_valid(entity))
        return 0;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->traits) <= trait) {
        ztream_container_resize(&entity_struct->traits, trait + 1);
        (&entity_struct->traits)->tail = trait + 1;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->traits, trait), ztream_hive_itor_t);
    if (ztream_hive_iterator_valid(itor_data)) {
        return 1;
    }
    ztream_trait_struct_t* trait_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, trait), ztream_trait_struct_t);

    for (int i = 0; i < ztream_container_size(&trait_struct->properties); i++) {
        ztream_property_t property = *ZTREAM_DATA_DECODE(ztream_container_get(&trait_struct->properties, i), ztream_property_t);
        if (ztream_container_size(&entity_struct->properties) <= property)
            return 0;
        if (!ztream_hive_iterator_valid(ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t))) {
            return 0;
        }
    }

    *itor_data = ztream_hive_emplace(&trait_struct->entities);
    *ZTREAM_DATA_DECODE(ztream_hive_data(&trait_struct->entities, itor_data), ztream_entity_t) = *entity;
    return 1;
}

static inline void ztream_entity_throw_trait(ztream_entity_t* entity, ztream_trait_t trait) {
    if (!ztream_hive_iterator_valid(entity))
        return;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->traits) <= trait) {
        return;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->traits, trait), ztream_hive_itor_t);
    if (!ztream_hive_iterator_valid(itor_data)) {
        return;
    }
    ztream_trait_struct_t* trait_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, trait), ztream_trait_struct_t);
    ztream_hive_release(&trait_struct->entities, itor_data);
}

static inline ztream_data_t ztream_entity_get(ztream_entity_t* entity, ztream_property_t property) {
    if (!ztream_hive_iterator_valid(entity)) {
        return NULL;
    }
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    if (ztream_container_size(&entity_struct->properties) <= property) {
        return NULL;
    }
    ztream_hive_itor_t* itor_data = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
    if (!ztream_hive_iterator_valid(itor_data)) {
        return NULL;
    }
    ztream_property_struct_t* property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
    return ztream_hive_data(&property_struct->hive, itor_data);
}

static inline void ztream_entity_erase(ztream_entity_t* entity) {
    if (!ztream_hive_iterator_valid(entity))
        return;
    ztream_entity_struct_t* entity_struct = ZTREAM_DATA_DECODE(ztream_hive_data(&ztream_ecs.entities, entity), ztream_entity_struct_t);
    for (int property = 0; property < ztream_container_size(&entity_struct->properties); property++) {
        ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->properties, property), ztream_hive_itor_t);
        ztream_property_struct_t*   property_struct = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.properties, property), ztream_property_struct_t);
        if (ztream_hive_iterator_valid(itor_data)) {
            property_struct->dtor(ztream_hive_data(&property_struct->hive, itor_data));
            ztream_hive_release(&property_struct->hive, itor_data);
        }
    }
    for (int trait = 0; trait < ztream_container_size(&entity_struct->traits); trait++) {
        ztream_hive_itor_t*         itor_data       = ZTREAM_DATA_DECODE(ztream_container_get(&entity_struct->traits, trait), ztream_hive_itor_t);
        ztream_trait_struct_t*      trait_struct    = ZTREAM_DATA_DECODE(ztream_container_get(&ztream_ecs.traits, trait), ztream_trait_struct_t);
        if (ztream_hive_iterator_valid(itor_data)) {
            ztream_hive_release(&trait_struct->entities, itor_data);
        }
    }
    ztream_container_free(&entity_struct->properties);
    ztream_container_free(&entity_struct->traits);
    ztream_hive_release(&ztream_ecs.entities, entity);
}

static inline ztream_scene_t ztream_scene() {
    ztream_scene_t scene;
    scene.paused    = 0;
    scene.entities  = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_entity_t), 4);
    scene.storage   = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_entity_info_t), 4);
    scene.systems   = ztream_container_alloc(ZTREAM_TYPE_SINGLE(ztream_system_t), 4);
    return scene;
}

static inline void ztream_scene_relate_entity(ztream_scene_t* scene, ztream_entity_t* entity) {
    ztream_container_push_back(&scene->entities, entity);
}

static inline void ztream_scene_relate_system(ztream_scene_t* scene, ztream_system_t* system) {
    ztream_container_push_back(&scene->systems, system);
}

static inline void ztream_scene_pause(ztream_scene_t* scene) {
    if (scene->paused) return;
    ztream_container_clear(&scene->storage);
    for (int i = 0; i < ztream_container_size(&scene->entities); i++) {
        ztream_entity_t*        entity; ZTREAM_DATA_GET(entity, ztream_container_get(&scene->entities, i));
        if (!ztream_hive_iterator_valid(entity))
            continue;
        ztream_entity_info_t    info    = ztream_entity_move_out(entity);
        ztream_container_push_back(&scene->storage, &info);
    }
    for (int i = 0; i < ztream_container_size(&scene->systems); i++) {
        ztream_system_t*        system; ZTREAM_DATA_GET(system, ztream_container_get(&scene->systems, i));
        ztream_system_lock(*system);
    }
    scene->paused = 1;
}

static inline void ztream_scene_launch(ztream_scene_t* scene) {
    if (!scene->paused) return;
    for (int i = 0; i < ztream_container_size(&scene->storage); i++) {
        ztream_entity_info_t* info; ZTREAM_DATA_GET(info, ztream_container_get(&scene->storage, i));
        ztream_entity_t entity      = ztream_entity_move_in(info);
        *ZTREAM_DATA_DECODE(ztream_container_get(&scene->entities, i), ztream_entity_t) = entity;
    }
    for (int i = 0; i < ztream_container_size(&scene->systems); i++) {
        ztream_system_t*         system; ZTREAM_DATA_GET(system, ztream_container_get(&scene->systems, i));
        ztream_system_unlock(*system);
    }
    scene->paused = 0;
}

static inline void ztream_scene_erase(ztream_scene_t* scene) {
    ztream_scene_pause(scene);
    scene->paused = 0;
    ztream_container_free(&scene->entities);
    ztream_container_free(&scene->storage);
    ztream_container_free(&scene->systems);
}

#endif