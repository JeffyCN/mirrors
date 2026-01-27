#include "aiq_base.h"
#include "aiq_list.h"
#include "aiq_map.h"
#include "aiq_pool.h"
#include "aiq_queue.h"

typedef struct aiq_test_que_elem_s {
    int a;
    bool b;
} aiq_test_que_elem_t;

static int aiq_test_queue_cb(void* data) {
    aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)data;
    LOGK("cb %d, %d", elem->a, elem->b);
    return 0;
}

static int aiq_test_queue_ptr_cb(void* data) {
    aiq_test_que_elem_t** elem = (aiq_test_que_elem_t**)data;
    LOGK("cb %d, %d", (*elem)->a, (*elem)->b);
    return 0;
}

void aiq_test_cbase_queue() {
    AiqQueueConfig_t cfg;
    AiqQueue_t* queue = NULL;

    cfg._name      = "aiq_queue";
    cfg._item_nums = 10;

    // 1. test PTR type
    LOGK("start queue PTR test: ");
    cfg._item_size     = sizeof(aiq_test_que_elem_t*);
    cfg._cbs.insert_cb = aiq_test_queue_ptr_cb;
    cfg._cbs.erase_cb  = aiq_test_queue_ptr_cb;
    queue              = aiqQueue_init(&cfg);
    if (queue) {
        aiq_test_que_elem_t elem;
        aiq_test_que_elem_t* pelem = &elem;
        aiq_test_que_elem_t* res   = NULL;

        elem.a = 1;
        elem.b = 4;
        aiqQueue_push(queue, &pelem);

        elem.a = 2;
        aiqQueue_push(queue, &pelem);

        elem.a = 3;
        aiqQueue_push(queue, &pelem);

        elem.a = 4;
        aiqQueue_push(queue, &pelem);
        LOGK("queue size:%d", aiqQueue_size(queue));

        aiqQueue_dump(queue);

        aiqQueue_get(queue, &res, -1);
        LOGK("queue get :%d", res->a);
        aiqQueue_get(queue, &res, -1);
        LOGK("queue get :%d", res->a);
        aiqQueue_get(queue, &res, -1);
        LOGK("queue get :%d", res->a);
        LOGK("queue size:%d", aiqQueue_size(queue));

        aiqQueue_reset(queue);
        LOGK("queue size:%d", aiqQueue_size(queue));
    }

    if (queue) {
        aiqQueue_deinit(queue);
    }

    // 2. test struct type
    LOGK("start queue obj test: ");
    cfg._item_size     = sizeof(aiq_test_que_elem_t);
    cfg._cbs.insert_cb = aiq_test_queue_cb;
    cfg._cbs.erase_cb  = aiq_test_queue_cb;
    queue              = aiqQueue_init(&cfg);
    if (queue) {
        aiq_test_que_elem_t elem;
        aiq_test_que_elem_t res;

        elem.a = 1;
        elem.b = 4;
        aiqQueue_push(queue, &elem);

        elem.a = 2;
        aiqQueue_push(queue, &elem);

        elem.a = 3;
        aiqQueue_push(queue, &elem);

        elem.a = 4;
        aiqQueue_push(queue, &elem);
        LOGK("queue size:%d", aiqQueue_size(queue));

        aiqQueue_dump(queue);

        aiqQueue_get(queue, &res, -1);
        LOGK("queue get :%d", res.a);
        aiqQueue_get(queue, &res, -1);
        LOGK("queue get :%d", res.a);
        aiqQueue_get(queue, &res, -1);
        LOGK("queue get :%d", res.a);
        LOGK("queue size:%d", aiqQueue_size(queue));

        aiqQueue_reset(queue);
        LOGK("queue size:%d", aiqQueue_size(queue));
    }

    if (queue) {
        aiqQueue_deinit(queue);
    }
}

void aiq_test_cbase_pool() {
    AiqPool_t* pool = NULL;
    AiqPoolConfig_t cfg;

    cfg._name      = "aiq_pool";
    cfg._item_size = sizeof(aiq_test_que_elem_t);
    cfg._item_nums = 3;

    pool = aiqPool_init(&cfg);

    if (pool) {
        AiqPoolItem_t* pool_item = NULL;
        pool_item                = aiqPool_getFree(pool);
        LOGK("pool freenum:%d", aiqPool_freeNums(pool));
        aiqPoolItem_ref(pool_item);
        LOGK("pool freenum:%d", aiqPool_freeNums(pool));
        aiqPoolItem_unref(pool_item);
        LOGK("pool freenum:%d", aiqPool_freeNums(pool));
        aiqPoolItem_unref(pool_item);
        LOGK("pool freenum:%d", aiqPool_freeNums(pool));
        aiqPool_dump(pool);

        pool_item = aiqPool_getFree(pool);
        pool_item = aiqPool_getFree(pool);
        pool_item = aiqPool_getFree(pool);
        pool_item = aiqPool_getFree(pool);
        if (!pool_item) LOGK("get free item failed, pool free num:%d", aiqPool_freeNums(pool));

        LOGK("pool freenum:%d", aiqPool_freeNums(pool));
        aiqPool_dump(pool);
        aiqPool_reset(pool);
    }
}

static int _aiqMap_cb(AiqMapItem_t* item, void* args) {
    aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)(item->_pData);

    printf("%s: elem.a:%d\n", __func__, elem->a);
    return 0;
}

static int _aiqMap_erase_cb(AiqMapItem_t* item, void* args) {
    aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)(item->_pData);
    if (elem->a == 12) {
        printf("%s: elem.a:%d\n", __func__, elem->a);
        aiqMap_erase_locked((AiqMap_t*)args, item->_key);
    }
    return 0;
}

void aiq_test_cbase_map() {
    AiqMapConfig_t cfg;
    AiqMap_t* map      = NULL;
    AiqMapItem_t* item = NULL;

    cfg._name      = "aiq_map";
    cfg._key_type  = AIQ_MAP_KEY_TYPE_UINT32;
    cfg._item_nums = 3;
    cfg._item_size = sizeof(aiq_test_que_elem_t);

    map = aiqMap_init(&cfg);
    if (map) {
        aiq_test_que_elem_t elem;
        elem.a = 10;

        aiqMap_insert(map, (void*)1, &elem);

        elem.a = 12;
        aiqMap_insert(map, (void*)12, &elem);

        elem.a = 14;
        aiqMap_insert(map, (void*)3, &elem);

        aiqMap_dump(map);

        elem.a = 15;
        item   = aiqMap_insert(map, (void*)4, &elem);
        if (!item) LOGK("insert map error");

        item = aiqMap_get(map, (void*)3);
        aiqMap_erase(map, (void*)3);

        aiqMap_dump(map);

        elem.a = 15;
        item   = aiqMap_insert(map, (void*)4, &elem);

        AiqMapItem_t* pItem = NULL;
        bool rm             = false;
        AIQ_MAP_FOREACH(map, pItem, rm) {
            aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)(pItem->_pData);
            if (elem->a == 10 || elem->a == 15) {
                pItem = aiqMap_erase(map, pItem->_key);
                rm    = true;
            }
        }
        aiqMap_foreach(map, _aiqMap_cb, NULL);

        elem.a = 11;
        item   = aiqMap_insert(map, (void*)2, &elem);

        aiqMap_foreach(map, _aiqMap_erase_cb, map);

        aiqMap_dump(map);

        item = aiqMap_begin(map);
        LOGK("begine item: %d", ((aiq_test_que_elem_t*)(item->_pData))->a);
    }
}

static int _aiqList_cb(AiqListItem_t* item, void* args) {
    aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)item->_pData;
    LOGK("elem data:%d", elem->a);
    return 0;
}

static int _aiqList_erase_cb(AiqListItem_t* item, void* args) {
    aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)item->_pData;
    if (elem->a == 12) {
        LOGK("erase elem data:%d", elem->a);
        aiqList_erase_item_locked((AiqList_t*)args, item);
    }
    return 0;
}

void aiq_test_cbase_list() {
    AiqListConfig_t cfg;
    AiqList_t* list     = NULL;
    AiqListItem_t* item = NULL;
    int ret             = 0;

    cfg._name      = "aiq_list";
    cfg._item_nums = 4;
    cfg._item_size = sizeof(aiq_test_que_elem_t);

    list = aiqList_init(&cfg);
    if (list) {
        aiqList_dump(list);

        aiq_test_que_elem_t elem;
        elem.a = 10;
        aiqList_push(list, &elem);

        elem.a = 11;
        aiqList_push(list, &elem);

        elem.a = 12;
        aiqList_push(list, &elem);

        elem.a = 13;
        aiqList_push(list, &elem);

        elem.a = 14;
        aiqList_push(list, &elem);

        aiqList_dump(list);
        // 10, 11, 12, 13
        aiqList_foreach(list, _aiqList_cb, NULL);

        AiqListItem_t* pItem = NULL;
        bool rm              = false;
        AIQ_LIST_FOREACH(list, pItem, rm) {
            aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)(pItem->_pData);
            if (elem->a == 10 || elem->a == 13) {
                pItem = aiqList_erase_item_locked(list, pItem);
                rm    = true;
            }
        }

        // 11,12
        aiqList_foreach(list, _aiqList_cb, NULL);
        aiqList_dump(list);

        aiqList_get(list, &elem);
        printf("elem.a: %d \n", elem.a);
        // 12
        aiqList_foreach(list, _aiqList_cb, NULL);
        aiqList_dump(list);

        elem.a = 13;
        aiqList_erase(list, &elem);
        aiqList_dump(list);
        // 12
        aiqList_foreach(list, _aiqList_cb, NULL);

        elem.a = 15;
        aiqList_push(list, &elem);
        aiqList_dump(list);
        // 12, 15
        aiqList_foreach(list, _aiqList_cb, NULL);

        aiqList_foreach(list, _aiqList_erase_cb, list);

        printf("after erase cb\n");
        aiqList_foreach(list, _aiqList_cb, NULL);

        elem.a = 16;
        aiqList_insert(list, &elem, aiqList_get_item(list, NULL));
        aiqList_dump(list);
        // 15, 16
        aiqList_foreach(list, _aiqList_cb, NULL);

        aiqList_reset(list);
        aiqList_dump(list);
        aiqList_deinit(list);
    }
}

void aiq_test_cbase_list_ptr() {
    AiqListConfig_t cfg;
    AiqList_t* list     = NULL;
    AiqListItem_t* item = NULL;
    int ret             = 0;

    cfg._name      = "aiq_list";
    cfg._item_nums = 4;
    cfg._item_size = sizeof(aiq_test_que_elem_t*);

    list = aiqList_init(&cfg);
    if (list) {
        aiq_test_que_elem_t* pelem = (aiq_test_que_elem_t*)aiq_mallocz(sizeof(aiq_test_que_elem_t));
        aiq_test_que_elem_t* res   = NULL;
        printf("res_ptr:%p, pelem:%p\n", res, pelem);
        pelem->a = 10;
        aiqList_push(list, &pelem);
        printf("res_ptr:%p, pelem:%p\n", res, pelem);
        aiqList_get(list, &res);
        printf("res_ptr:%p, pelem:%p\n", res, pelem);
        aiqList_deinit(list);
    }
}

void aiq_test_cbase_autoptr() {
    int ret                   = 0;
    aiq_test_que_elem_t* elem = (aiq_test_que_elem_t*)aiq_mallocz(sizeof(aiq_test_que_elem_t));
    aiq_autoptr_t* autoptr    = aiq_autoptr_init(elem);
    ret                       = aiq_autoptr_ref(autoptr);
    LOGK("refcnts:%d", ret);
    ret = aiq_autoptr_unref(autoptr);
    LOGK("refcnts:%d", ret);
    ret = aiq_autoptr_unref(autoptr);
    LOGK("refcnts:%d", ret);
}

typedef struct aiq_test_elem_s {
    aiq_ref_base_t _ref_base;
    int a;
    bool b;
} aiq_test_elem_t;

void aiq_test_cbase_autoptr_2() {
    printf("%s:%d\n", __func__, __LINE__);
    int ret                  = 0;
    aiq_test_elem_t* test    = (aiq_test_elem_t*)aiq_mallocz(sizeof(aiq_test_elem_t));
    aiq_autoptr_t* test_auto = NULL;
    test_auto                = aiq_autoptr_init(test);

    // test->_ref_base._owner = test_auto;
    // test->_ref_base.ref = aiq_autoptr_ref;
    // test->_ref_base.unref = aiq_autoptr_unref;

    AIQ_REF_BASE_INIT(&test->_ref_base, test_auto, aiq_autoptr_ref, aiq_autoptr_unref);

    ret = AIQ_REF_BASE_REF(&test->_ref_base);
    printf("%s:%d ret :%d\n", __func__, __LINE__, ret);
    ret = AIQ_REF_BASE_UNREF(&test->_ref_base);
    printf("%s:%d ret :%d\n", __func__, __LINE__, ret);
    ret = AIQ_REF_BASE_UNREF(&test->_ref_base);
    printf("%s:%d ret :%d\n", __func__, __LINE__, ret);
    // aiq_autoptr_ref	(test_auto);
    // aiq_autoptr_unref(test_auto);
}
