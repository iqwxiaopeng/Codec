/**
 Copyright © 2015 2coding. All Rights Reserved.
 
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 3. The name of the author may not be used to endorse or promote products derived
 from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */
#include "base16.h"

static CODECode _base16_setopt(CODECBase *p, CODECOption opt, va_list args);
static CODECode _base16_work(CODECBase *p, const CDCStream *st);

static void _base16_encoding(const struct base16 *b16, const byte *data, size_t datalen, CDCStream *buf);
static BOOL _base16_decoding(const struct base16 *b16, const byte *data, size_t datalen, CDCStream *buf);

void *base16_init(CODECBase *p) {
    struct base16 *b16 = (struct base16 *)p;
    b16->chunkled = TRUE;
    b16->ignorecase = FALSE;
    
    b16->setup = _base16_setopt;
    b16->work = _base16_work;
    return p;
}

CODECode _base16_setopt(CODECBase *p, CODECOption opt, va_list args) {
    CODECode code = CODECOk;
    struct base16 *b16 = (struct base16 *)p;
    switch (opt) {
        case CODECBase16IgnoreCase:
            b16->ignorecase = va_arg(args, long);
            break;
            
        default:
            code = CODECIgnoredOption;
            break;
    }
    return code;
}

CODECode _base16_work(CODECBase *p, const CDCStream *st) {
    if (p->method == CODECEncoding) {
        _base16_encoding((const struct base16 *)p, stream_data(st), stream_size(st), p->result);
    }
    else {
        if (!_base16_decoding((const struct base16 *)p, stream_data(st), stream_size(st), p->result)) {
            return CODECInvalidInput;
        }
    }
    
    return CODECOk;
}

void _base16_encoding(const struct base16 *b16, const byte *data, size_t datalen, CDCStream *buf) {
    static const byte table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    static const int chunklen = 76;
    
    size_t i = 0, idx = 0;
    byte arr[2] = {0};
    for (i = 0; i < datalen; ++i) {
        if (b16->chunkled
            && idx > 0
            && idx % chunklen == 0) {
            idx = stream_write_bytes(buf, (const byte *)"\r\n", 2);
        }
        
        arr[0] = table[(data[i] >> 4) & 0x0f];
        arr[1] = table[data[i] & 0x0f];
        idx = stream_write_bytes(buf, arr, 2);
    }
}

BOOL _base16_decoding(const struct base16 *b16, const byte *data, size_t datalen, CDCStream *buf) {
    if (datalen % 2 != 0) {
        return FALSE;
    }
    
    const static byte table[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 00-0f
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 10-1f
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 20-2f
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 30-3f 0-9
        0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,                                                       // 40-46 A-F
        
    };
    size_t i = 0, k = 0;
    byte c = 0, t = 0;
    for (i = 0; i < datalen; ++i) {
        c = data[i];
        if (c == '\r' || c == '\n') {
            continue;
        }
        
        if (b16->ignorecase && c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        
        if (c > 'F') {
            return FALSE;
        }
        
        c = table[c];
        if (c == 0xff) {
            return FALSE;
        }
        
        t |= (c << (4 - 4 * k)) & 0xff;
        ++k;
        if (k == 2) {
            stream_write_b(buf, t);
            k = 0;
            t = 0;
        }
    }
    
    if (k || t) {
        return FALSE;
    }
    
    return TRUE;
}