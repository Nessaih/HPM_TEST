/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public APIs.
 */

#ifndef _FLASHDB_H_
#define _FLASHDB_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "fal.h"
#include "fdb_cfg.h"
#include "fdb_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FlashDB database API */
fdb_err_t         fdb_kvdb_init(fdb_kvdb_t db, const char *name, const char *path);
void              fdb_kvdb_control(fdb_kvdb_t db, int cmd, void *arg);
fdb_err_t         fdb_kvdb_check(fdb_kvdb_t db);
fdb_err_t         fdb_kvdb_deinit(fdb_kvdb_t db);

/* blob API */
fdb_blob_t        fdb_blob_make(fdb_blob_t blob, const void *value_buf, size_t buf_len);
size_t            fdb_blob_read(fdb_db_t db, fdb_blob_t blob);

/* Key-Value API like a KV DB */
fdb_err_t         fdb_kv_set(fdb_kvdb_t db, const char *key, const char *value);
char             *fdb_kv_get(fdb_kvdb_t db, const char *key);
fdb_err_t         fdb_kv_set_blob(fdb_kvdb_t db, const char *key, fdb_blob_t blob);
size_t            fdb_kv_get_blob(fdb_kvdb_t db, const char *key, fdb_blob_t blob);
fdb_err_t         fdb_kv_del(fdb_kvdb_t db, const char *key);
fdb_kv_t          fdb_kv_get_obj(fdb_kvdb_t db, const char *key, fdb_kv_t kv);
fdb_blob_t        fdb_kv_to_blob(fdb_kv_t kv, fdb_blob_t blob);
fdb_err_t         fdb_kv_set_default(fdb_kvdb_t db);
void              fdb_kv_print(fdb_kvdb_t db);
fdb_kv_iterator_t fdb_kv_iterator_init(fdb_kvdb_t db, fdb_kv_iterator_t itr);
bool              fdb_kv_iterate(fdb_kvdb_t db, fdb_kv_iterator_t itr);

/* fdb_utils.c */
uint32_t          fdb_calc_crc32(uint32_t crc, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _FLASHDB_H_ */
