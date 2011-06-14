/*-
 * Copyright (c) 2011 Varnish Software AS
 * All rights reserved.
 *
 * Author: Kristian Lyngstol <kristian@bohemians.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <pthread.h>

#include "vrt.h"
#include "bin/varnishd/cache.h"
#include "include/vct.h"

#include "vcc_if.h"

pthread_mutex_t header_mutex;

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	int ret;
	ret = pthread_mutex_init(&header_mutex, NULL);
	assert (ret == 0);
	return (0);
}

/*
 * Stolen bluntly from cache_vrt.c (should be fixed)
 */
static struct http *
header_vrt_selecthttp(const struct sess *sp, enum gethdr_e where)
{
        struct http *hp;

        CHECK_OBJ_NOTNULL(sp, SESS_MAGIC);
        switch (where) {
        case HDR_REQ:
                hp = sp->http;
                break;
        case HDR_BEREQ:
                hp = sp->wrk->bereq;
                break;
        case HDR_BERESP:
                hp = sp->wrk->beresp;
                break;
        case HDR_RESP:
                hp = sp->wrk->resp;
                break;
        case HDR_OBJ:
                CHECK_OBJ_NOTNULL(sp->obj, OBJECT_MAGIC);
                hp = sp->obj->http;
                break;
        default:
                assert("ops");
        }
        CHECK_OBJ_NOTNULL(hp, HTTP_MAGIC);
        return (hp);
}


void __match_proto__()
vmod_append(struct sess *sp, enum gethdr_e e, const char *h, const char *fmt, ...)
{
	va_list ap;
	struct http *hp;
	char *b;
	CHECK_OBJ_NOTNULL(sp, SESS_MAGIC);
	assert(fmt != NULL);
	
	hp = header_vrt_selecthttp(sp, e);
	va_start(ap, fmt);
	b = VRT_String(hp->ws, h + 1, fmt, ap);
	if (b == NULL)
		WSP(sp, SLT_LostHeader, "vmod_header: %s", h+1);
	else
		http_SetHeader(sp->wrk,sp->fd,hp,b);
	va_end(ap);
}

/*
void __match_proto__()
vmod_remove(struct sess *sp, enum gethdr_e e, const char *h, const char *s)
{
	(void)e;
	(void)sp;
	(void)h;
	return (1);
}

*/

static unsigned
header_http_findhdr(const struct http *hp, unsigned l, const char *hdr, void *re)
{
        unsigned u;

        for (u = HTTP_HDR_FIRST; u < hp->nhd; u++) {
                Tcheck(hp->hd[u]);
                if (hp->hd[u].e < hp->hd[u].b + l + 1) 
                        continue;
                if (hp->hd[u].b[l] != ':') 
                        continue;
                if (strncasecmp(hdr, hp->hd[u].b, l))
                        continue;
		if (!VRT_re_match(hp->hd[u].b + l + 1,re))
			continue;
                return (u);
        }
        return (0);
}

/*
 * XXX: Needs to be cleaned up a bit
 */
const char * __match_proto__()
vmod_get(struct sess *sp, struct vmod_priv *priv, enum gethdr_e e, const char *h, const char *s)
{
	struct http *hp;
	unsigned u;
	char *p;
	int ret;

	if (priv->priv == NULL) {
		ret = pthread_mutex_lock(&header_mutex);
		assert (ret == 0);
		/*
		 * Need to re-test to avoid race
		 */
		if (priv->priv == NULL) {
			VRT_re_init(&priv->priv,s);
			priv->free = VRT_re_fini;
		}
		pthread_mutex_unlock(&header_mutex);
	}
	
	hp = header_vrt_selecthttp(sp, e);
	u = header_http_findhdr(hp,h[0] - 1,h+1,priv->priv);
	if (u == 0) {
		return NULL;
	}
	p = hp->hd[u].b + h[0];
	while (vct_issp(*p))
		p++;
	return p;
}

