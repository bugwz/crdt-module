/*
 * Copyright (c) 2009-2012, CTRIP CORP <RDkjdata at ctrip dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
//
// Created by zhuchen(zhuchen at ctrip dot com) on 2019-04-16.
//

#ifndef XREDIS_CRDT_REGISTER_H
#define XREDIS_CRDT_REGISTER_H

#include "include/rmutil/sds.h"
#include "ctrip_crdt_common.h"
#include "include/redismodule.h"
#include "crdt_util.h"

#define CRDT_REGISTER_DATATYPE_NAME "crdt_regr"
#define CRDT_REGISTER_TOMBSTONE_DATATYPE_NAME "crdt_regt"
int registerStartGc();
int registerStopGc();

typedef CrdtObject CRDT_Register ;
int getCrdtRegisterLastGid(CRDT_Register* r);
sds getCrdtRegisterLastValue(CRDT_Register* r);
long long getCrdtRegisterLastTimestamp(CRDT_Register* r);
VectorClock getCrdtRegisterLastVc(void* r);
CrdtMeta* createCrdtRegisterLastMeta(CRDT_Register* reg);

typedef CrdtObject CRDT_RegisterTombstone ;
typedef struct CRDT_Register* (*dupCrdtRegisterFunc)(CRDT_Register*);
typedef int (*delCrdtRegisterFunc)(CRDT_Register*, CrdtMeta*);
typedef sds (*getCrdtRegisterFunc)(CRDT_Register*);
typedef struct crdtRegisterSetValue* (*getValueCrdtRegisterFunc)(CRDT_Register*);
typedef int (*crdtRegisterSetValueFunc)(CRDT_Register* target, CrdtMeta* meta, sds value);
typedef sds (*getInfoCrdtRegisterFunc)(CRDT_Register* target);
typedef struct CRDT_Register* (*filterCrdtRegisterFunc)(CRDT_Register* target,int gid, long long logic_time);
typedef int (*cleanCrdtRegisterFunc)(CRDT_Register* target, CRDT_RegisterTombstone* tombstone);
typedef struct CRDT_Register* (*mergeCrdtRegisterFunc)(CRDT_Register* target, struct CRDT_Register* other);
typedef void (*updateLastVCCrdtRegisterFunc)(CRDT_Register* target, VectorClock vc);
typedef struct CrdtRegisterMethod {
    dupCrdtRegisterFunc dup;
    delCrdtRegisterFunc del;
    getCrdtRegisterFunc get;
    getValueCrdtRegisterFunc getValue;
    crdtRegisterSetValueFunc set;
    getInfoCrdtRegisterFunc getInfo;
    filterCrdtRegisterFunc filter;
    mergeCrdtRegisterFunc merge;
    updateLastVCCrdtRegisterFunc updateLastVC;
} CrdtRegisterMethod;

typedef int (*compareCrdtRegisterTombstoneFunc)(CRDT_RegisterTombstone* target, CrdtMeta* meta);
typedef CrdtMeta* (*addCrdtRegisterTombstoneFunc)(CRDT_RegisterTombstone* target,CrdtMeta* other);
typedef CRDT_RegisterTombstone* (*filterCrdtRegisterTombstoneFunc)(CRDT_RegisterTombstone* target, int gid, long long logic_time);
typedef CRDT_RegisterTombstone* (*dupCrdtRegisterTombstoneFunc)(CRDT_RegisterTombstone* target);
typedef CRDT_RegisterTombstone* (*mergeRegisterTombstoneFunc)(CRDT_RegisterTombstone* target, CRDT_RegisterTombstone* other);
typedef int (*purgeTombstoneFunc)(CrdtTombstone* tombstone, CrdtObject* obj);
typedef struct CrdtRegisterTombstoneMethod {
    compareCrdtRegisterTombstoneFunc isExpire;
    addCrdtRegisterTombstoneFunc add;
    filterCrdtRegisterTombstoneFunc filter;
    dupCrdtRegisterTombstoneFunc dup;
    mergeRegisterTombstoneFunc merge;
    purgeTombstoneFunc purge;
} CrdtRegisterTombstoneMethod;



void *createCrdtRegister(void);
void initRegister(CRDT_Register *crdtRegister);
void freeCrdtRegister(void *crdtRegister);
void crdtRegisterSetValue(CRDT_Register* r, CrdtMeta* meta, sds value) ;
int crdtRegisterTryUpdate(CRDT_Register* r, CrdtMeta* meta, sds value, int compare);
int compareCrdtRegisterAndDelMeta(CRDT_Register* current, CrdtMeta* meta);
int initRegisterModule(RedisModuleCtx *ctx);

//register command methods
CrdtObject *crdtRegisterMerge(CrdtObject *currentVal, CrdtObject *value);
int crdtRegisterDelete(int dbId, void *keyRobj, void *key, void *value);
CrdtObject** crdtRegisterFilter(CrdtObject* common, int gid, long long logic_time, long long maxsize, int* length);
CrdtObject** crdtRegisterFilter2(CrdtObject* common, int gid, VectorClock min_vc, long long maxsize, int* length);

int crdtRegisterTombstonePurge(CrdtTombstone* tombstone, CrdtObject* current);
CRDT_Register* dupCrdtRegister(CRDT_Register *val);

void crdtRegisterUpdateLastVC(void *data, VectorClock vc);
CRDT_Register** filterRegister(CRDT_Register*  common, int gid, long long logic_time, long long maxsize, int* length);
void freeRegisterFilter(CrdtObject** filters, int num);
static CrdtObjectMethod RegisterCommonMethod = {
    .merge = crdtRegisterMerge,
    .filterAndSplit = crdtRegisterFilter,
    .filterAndSplit2 = crdtRegisterFilter2,
    .freefilter = freeRegisterFilter,
};
sds crdtRegisterInfo(void *crdtRegister);
static CrdtDataMethod RegisterDataMethod = {
    .propagateDel = crdtRegisterDelete,
    .getLastVC = getCrdtRegisterLastVc,
    .updateLastVC = crdtRegisterUpdateLastVC,
    .info = crdtRegisterInfo,
};

void *RdbLoadCrdtRegister(RedisModuleIO *rdb, int encver);
void RdbSaveCrdtRegister(RedisModuleIO *rdb, void *value);


sds crdtRegisterInfoFromMetaAndValue(CrdtMeta* meta, sds value);
CRDT_Register* mergeRegister(CRDT_Register* target, CRDT_Register* other, int* conflict);
sds getCrdtRegisterSds(CRDT_Register* r);
CRDT_Register* addRegister(void *tombstone, CrdtMeta* meta, sds value);
CrdtMeta* getCrdtRegisterLastMeta(CRDT_Register* target);
void crdtRegisterTombstoneDigestFunc(RedisModuleDigest *md, void *value);
size_t crdtRegisterTombstoneMemUsageFunc(const void *value);
void freeCrdtRegisterTombstone(void *obj);
void RdbSaveCrdtRegisterTombstone(RedisModuleIO *rdb, void *value);
void *RdbLoadCrdtRegisterTombstone(RedisModuleIO *rdb, int encver) ;
void AofRewriteCrdtRegisterTombstone(RedisModuleIO *aof, RedisModuleString *key, void *value);

CRDT_RegisterTombstone* createCrdtRegisterTombstone();
int isRegisterTombstone(void *data);
int isRegister(void *data);
CRDT_RegisterTombstone* dupCrdtRegisterTombstone(CRDT_RegisterTombstone* target);
CRDT_RegisterTombstone* mergeRegisterTombstone(CRDT_RegisterTombstone* target, CRDT_RegisterTombstone* other, int* comapre);
CRDT_RegisterTombstone** filterRegisterTombstone(CRDT_RegisterTombstone* target, int gid, long long logic_time,long long maxsize, int* length);
int purgeRegisterTombstone(CRDT_RegisterTombstone* tombstone, CRDT_Register* target);
CrdtMeta* addRegisterTombstone(CRDT_RegisterTombstone* target, CrdtMeta* meta, int* compare);
int compareCrdtRegisterTombstone(CRDT_RegisterTombstone* tombstone, CrdtMeta* meta);

static RedisModuleType *CrdtRegister;
static RedisModuleType *CrdtRegisterTombstone;
RedisModuleType* getCrdtRegister();
RedisModuleType* getCrdtRegisterTombstone();
int compareTombstoneAndRegister(CRDT_RegisterTombstone* tombstone, CRDT_Register* target);
size_t crdtRegisterMemUsageFunc(const void *value);
VectorClock getCrdtRegisterTombstoneLastVc(CRDT_RegisterTombstone* t);
CrdtMeta* getCrdtRegisterTombstoneMeta(CRDT_RegisterTombstone* t);
CRDT_Register* addOrUpdateRegister(RedisModuleCtx *ctx, RedisModuleKey* moduleKey, CRDT_RegisterTombstone* tombstone, CRDT_Register* current, CrdtMeta* meta, RedisModuleString* key,sds value);


//register tombstone command methods
CrdtTombstone* crdtRegisterTombstoneMerge(CrdtTombstone* target, CrdtTombstone* other);
CrdtObject** crdtRegisterTombstoneFilter(CrdtTombstone* target, int gid, long long logic_time, long long maxsize,int* length) ;
CrdtObject** crdtRegisterTombstoneFilter2(CrdtTombstone* target, int gid, VectorClock min_vc, long long maxsize,int* length) ;
int crdtRegisterTombstoneGc(CrdtTombstone* target, VectorClock clock);
sds crdtRegisterTombstoneInfo(void *t);
void freeRegisterTombstoneFilter(CrdtObject** filters, int num);
VectorClock clone_rt_vc(void* rt);
static CrdtTombstoneMethod RegisterTombstoneMethod = {
    .merge = crdtRegisterTombstoneMerge,
    .filterAndSplit =  crdtRegisterTombstoneFilter,
    .filterAndSplit2 =  crdtRegisterTombstoneFilter2,
    .freefilter = freeRegisterTombstoneFilter,
    .gc = crdtRegisterTombstoneGc,
    .purge = crdtRegisterTombstonePurge,
    .info = crdtRegisterTombstoneInfo,
    .getVc = clone_rt_vc,
};
#endif //XREDIS_CRDT_CRDT_REGISTER_H
