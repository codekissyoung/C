#ifndef RS_MONGO_HANDLE_H
#define RS_MONGO_HANDLE_H

#include "mongo/mongoc.h"
#include "bson/bson.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*MONGO_FIND_CB)(void *arg, char *result);

int mongo_create_conn(char *host, short port, void **conn);
int mongo_destory_conn(void *conn);

int mongo_find_one(void *conn, const char *db_name, const char *coll_name,
                   const char *key, const char *value, MONGO_FIND_CB find_cb,
                   void *arg);

int mongo_find(void *conn, const char *db_name, const char *coll_name,
               MONGO_FIND_CB find_cb, void *arg);

int mongo_insert(void *conn, const char *db_name, const char *coll_name);

int mongo_update(void *conn, const char *db_name, const char *coll_name,
                 const char *query_key, const char *query_value,
                 bson_t *update);

int mongo_update_add_set(void *conn, const char *db_name, const char *coll_name,
                         const char *key, const char *value,
                         const char *set_key, const char *add_value);

int mongo_update_pull(void *conn, const char *db_name, const char *coll_name,
                      const char *key, const char *value, const char *set_key,
                      const char *pull_value);

int mongo_unset_key(void *conn, const char *db_name, const char *coll_name,
                    const char *key, const char *value, const char *unset_key);

int mongo_delete(void *conn, const char *db_name, const char *coll_name);

#ifdef __cplusplus
}
#endif

#endif // RS_MONGO_HANDLE_H
