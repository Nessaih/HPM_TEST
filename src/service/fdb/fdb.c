

#include <fdb.h>
#include <fdb_low_lvl.h>
#include <inttypes.h>
#include <string.h>

#ifdef FDB_USING_FILE_POSIX_MODE
#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#endif

#define FDB_LOG_TAG ""

#if !defined(FDB_USING_FAL_MODE) && !defined(FDB_USING_FILE_MODE)
#error "Please defined the FDB_USING_FAL_MODE or FDB_USING_FILE_MODE macro"
#endif

fdb_err_t _fdb_init_ex(fdb_db_t db, const char *name, const char *path, fdb_db_type type)
{
    FDB_ASSERT(db);
    FDB_ASSERT(name);
    FDB_ASSERT(path);

    if(db->init_ok) {
        return FDB_NO_ERR;
    }

    db->name = name;
    db->type = type;
    db->path = path;

    if(db->file_mode) {

#ifdef FDB_USING_FILE_MODE
        memset(db->cur_file_sec, FDB_FAILED_ADDR, FDB_FILE_CACHE_TABLE_SIZE * sizeof(db->cur_file_sec[0]));

        FDB_ASSERT(db->sec_size != 0);
        FDB_ASSERT(db->max_size != 0);
#ifdef FDB_USING_FILE_POSIX_MODE
        memset(db->cur_file, -1, FDB_FILE_CACHE_TABLE_SIZE * sizeof(db->cur_file[0]));
#else
        memset(db->cur_file, 0, FDB_FILE_CACHE_TABLE_SIZE * sizeof(db->cur_file[0]));
#endif
        FDB_ASSERT(strlen(path) != 0)
#endif

    } else {

#ifdef FDB_USING_FAL_MODE
        size_t                 block_size;
        const fal_partition_t *part      = NULL;
        const fal_flash_dev_t *flash_dev = NULL;

        fal_init();
        part = fal_find_partition(path);
        if(part == NULL) {
            FDB_INFO("Error: Partition (%s) not found.\n", path);
            return FDB_PART_NOT_FOUND;
        }

        flash_dev = fal_find_flash(part->flash_name);
        if(NULL == flash_dev) {
            return FDB_INIT_FAILED;
        }

        block_size    = flash_dev->blk_size;
        db->user_data = (void *)part;

        FDB_ASSERT(block_size > 0);

        if(db->sec_size == 0) {
            db->sec_size = block_size;
        } else {
            if(db->sec_size % block_size != 0) {
                FDB_INFO("Error: db sector size (%" PRIu32 ") MUST align with block size (%zu).\n", db->sec_size, block_size);
                return FDB_INIT_FAILED;
            }
        }
        db->max_size = part->len;
#endif
    }

    FDB_ASSERT((db->sec_size & (db->sec_size - 1)) == 0);

    if(db->max_size % db->sec_size != 0) {
        FDB_INFO("Error: db total size (%" PRIu32 ") MUST align with sector size (%" PRIu32 ").\n", db->max_size, db->sec_size);
        return FDB_INIT_FAILED;
    }

    if(db->max_size / db->sec_size < 2) {
        FDB_INFO("Error: db MUST has more than or equal 2 sectors, current has %" PRIu32 " sector(s)\n", db->max_size / db->sec_size);
        return FDB_INIT_FAILED;
    }

    return FDB_NO_ERR;
}

void _fdb_init_finish(fdb_db_t db, fdb_err_t result)
{
    static bool log_is_show = false;
    if(result == FDB_NO_ERR) {
        db->init_ok = true;
        if(!log_is_show) {
            FDB_INFO("FlashDB V%s is initialize success.\n", FDB_SW_VERSION);
            log_is_show = true;
        }
    } else if(!db->not_formatable) {
        FDB_INFO("Error: %s (%s@%s) is initialize fail (%d).\n", db->type == FDB_DB_TYPE_KV ? "KVDB" : "TSDB", db->name, _fdb_db_path(db), (int)result);
    }
}

void _fdb_deinit(fdb_db_t db)
{
    FDB_ASSERT(db);

    if(db->init_ok) {
#ifdef FDB_USING_FILE_MODE
        for(int i = 0; i < FDB_FILE_CACHE_TABLE_SIZE; i++) {
#ifdef FDB_USING_FILE_POSIX_MODE
            if(db->cur_file[i] > 0) {
                close(db->cur_file[i]);
            }
#else
            if(db->cur_file[i] != 0) {
                fclose(db->cur_file[i]);
            }
#endif
        }
#endif
    }

    db->init_ok = false;
}

const char *_fdb_db_path(fdb_db_t db)
{
    if(db->file_mode) {
#ifdef FDB_USING_FILE_MODE
        return db->storage.dir;
#else
        return NULL;
#endif
    } else {
#ifdef FDB_USING_FAL_MODE
        return db->path;
#else
        return NULL;
#endif
    }
}
