#include "mongo_handle.h"

#include <stdio.h>

int mongo_create_conn(char *host, short port, void **conn) {
  mongoc_init();
  char url[260];
  snprintf(url, sizeof(url), "mongodb://%s:%d/", host, port);
  mongoc_client_t *client = mongoc_client_new(url);
  if (!client) {
    return -1;
  }
  *conn = client;
  return 0;
}

int mongo_destory_conn(void *conn) {
  mongoc_client_destroy((mongoc_client_t *)conn);
  mongoc_cleanup();
  return 0;
}

int mongo_find(void *conn, const char *db_name, const char *coll_name,
               MONGO_FIND_CB find_cb, void *arg) {
  mongoc_client_t *client = (mongoc_client_t *)conn;
  mongoc_collection_t *collection =
      mongoc_client_get_collection(client, db_name, coll_name);
  bson_t *query = bson_new();
  mongoc_cursor_t *cursor = mongoc_collection_find(
      collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
  const bson_t *doc;
  while (mongoc_cursor_next(cursor, &doc)) {
    char *str = bson_as_json(doc, NULL);
    if (find_cb && find_cb(arg, str) == -1) {
      bson_free(str);
      break;
    }
    bson_free(str);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  return 0;
}

int mongo_find_one(void *conn, const char *db_name, const char *coll_name,
                   const char *key, const char *value, MONGO_FIND_CB find_cb,
                   void *arg) {
  mongoc_client_t *client = (mongoc_client_t *)conn;
  mongoc_cursor_t *cursor;
  const bson_t *doc;
  mongoc_collection_t *collection =
      mongoc_client_get_collection(client, db_name, coll_name);
  bson_t *query = BCON_NEW(key, BCON_UTF8(value));
  cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query,
                                  NULL, NULL);
  while (mongoc_cursor_next(cursor, &doc)) {
    char *str = bson_as_json(doc, NULL);
    if (find_cb && find_cb(arg, str) == -1) {
      bson_free(str);
      break;
    }
    bson_free(str);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  return 0;
}

int mongo_insert(void *conn, const char *db_name, const char *coll_name) {
  return 0;
}

int mongo_update(void *conn, const char *db_name, const char *coll_name,
                 const char *query_key, const char *query_value, bson_t *data) {
  int ret = 0;
  bson_error_t error;
  mongoc_client_t *client = (mongoc_client_t *)conn;
  mongoc_collection_t *collection =
      mongoc_client_get_collection(client, db_name, coll_name);

  bson_t *query = BCON_NEW(query_key, BCON_UTF8(query_value));
  if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, data,
                                NULL, &error)) {
    ret = -1;
  }
  bson_destroy(query);
  mongoc_collection_destroy(collection);
  return ret;
}

int mongo_delete(void *conn, const char *db_name, const char *coll_name) {
  return 0;
}

int mongo_query(void *conn, const char *db_name, const char *) { return 0; }

int mongo_update_add_set(void *conn, const char *db_name, const char *coll_name,
                         const char *query_key, const char *query_value,
                         const char *update_key, const char *update_value) {
  int ret = 0;
  bson_error_t error;
  mongoc_client_t *client = (mongoc_client_t *)conn;
  mongoc_collection_t *collection =
      mongoc_client_get_collection(client, db_name, coll_name);

  bson_t *query = BCON_NEW(query_key, BCON_UTF8(query_value));
  bson_t *update =
      BCON_NEW("$addToSet", "{", update_key, BCON_UTF8(update_value), "}");
  if (!mongoc_collection_update(collection, MONGOC_UPDATE_UPSERT, query, update,
                                NULL, &error)) {
    ret = -1;
  }
  bson_destroy(query);
  bson_destroy(update);
  mongoc_collection_destroy(collection);
  return ret;
}

int mongo_unset_key(void *conn, const char *db_name, const char *coll_name,
                    const char *query_key, const char *query_value,
                    const char *remove_key) {
  int ret = 0;
  bson_error_t error;
  mongoc_client_t *client = (mongoc_client_t *)conn;
  mongoc_collection_t *collection =
      mongoc_client_get_collection(client, db_name, coll_name);

  bson_t *query = BCON_NEW(query_key, BCON_UTF8(query_value));
  bson_t *update = BCON_NEW("$unset", "{", remove_key, BCON_UTF8("1"), "}");
  if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, update,
                                NULL, &error)) {
    ret = -1;
  }
  bson_destroy(query);
  bson_destroy(update);
  mongoc_collection_destroy(collection);
  return ret;
  return 0;
}

int mongo_update_pull(void *conn, const char *db_name, const char *coll_name,
                      const char *query_key, const char *query_value,
                      const char *pull_key, const char *pull_value) {

  int ret = 0;
  bson_error_t error;
  mongoc_client_t *client = (mongoc_client_t *)conn;
  mongoc_collection_t *collection =
      mongoc_client_get_collection(client, db_name, coll_name);

  bson_t *query = BCON_NEW(query_key, BCON_UTF8(query_value));
  bson_t *update = BCON_NEW("$pull", "{", pull_key, BCON_UTF8(pull_value), "}");
  if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, update,
                                NULL, &error)) {
    ret = -1;
  }
  bson_destroy(query);
  bson_destroy(update);
  mongoc_collection_destroy(collection);
  return ret;
}
