/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2013 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#ifndef NGX_HTTP_COUCHBASE_MODULE_H
#define NGX_HTTP_COUCHBASE_MODULE_H

#include <ngx_core.h>
#include <ngx_http.h>
#include <libcouchbase/couchbase.h>

#include "ngx_lcb_callbacks.h"
#include "ngx_lcb_plugin.h"

typedef struct ngx_lcb_loc_conf_s {
    ngx_str_t name;
    ngx_msec_t connect_timeout;
    ngx_msec_t timeout;
    struct lcb_create_st options;
    ngx_http_upstream_conf_t upstream;
} ngx_lcb_loc_conf_t;

typedef struct {
    ngx_array_t connection_confs; /* ngx_lcb_loc_conf_t */
} ngx_lcb_main_conf_t;

extern ngx_module_t ngx_http_couchbase_module;

extern ngx_int_t ngx_lcb_cmd_idx;
extern ngx_int_t ngx_lcb_key_idx;
extern ngx_int_t ngx_lcb_val_idx;
extern ngx_int_t ngx_lcb_cas_idx;
extern ngx_int_t ngx_lcb_flags_idx;
extern ngx_int_t ngx_lcb_exptime_idx;

ngx_int_t ngx_lcb_process(ngx_http_request_t *r);

typedef struct ngx_lcb_connection_s {
    ngx_str_t name;
    lcb_t lcb;
    ngx_log_t *log;
    ngx_array_t backlog;    /* the list of postponed (ngx_http_request_t *) */
    unsigned connected:1;
    unsigned connect_scheduled:1;
} ngx_lcb_connection_t;

#endif /* NGX_HTTP_COUCHBASE_MODULE_H */

