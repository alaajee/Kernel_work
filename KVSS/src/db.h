
#ifndef KR_DB_H
#define KR_DB_H

#include "internal.h"
#include "outbuf.h"
#include "../../Client.h"
// A quoi sert KrDevice ? 

struct KrDevice;
struct KrBuf;
struct KrSuperBlock;
struct connection_context;

typedef struct KrDb {
    char path[255];
    u8 id;
    int refcnt;
    struct KrDevice* dev;

    struct KrBuf* sb_buf;
    struct KrSuperBlock* sb; /* address owned by the sb_buf */
} KrDb;

int    kr_db_open     (KrDb ** dp, const char * path);
KrDb*  kr_db_from_id  (u8 id);
int    kr_db_close    (KrDb * db);
int    kr_db_put      (KrDb * db, KrSlice key, KrSlice val);
int    kr_db_get      (KrDb * db, KrSlice key, KrOutbuf* buf, u64* size,struct connection_context* cn);

#endif
