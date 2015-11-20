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

#include "codec.h"
#include "codecbase.h"
#include "base64.h"

CODEC codec_init(CODECProtocol protocol, CODECMethod method) {
    CODECBase *p = 0;
    switch (protocol) {
        case CODECBase64:
            p = init(method, sizeof(struct base64), base64_init);
            break;
            
        default:
            break;
    }
    
    if (p && !p->work) {
        cleanup(p);
        p = 0;
    }
    
    return p;
}

void codec_cleanup(CODEC codec) {
    CODECBase *p = codec;
    cleanup(p);
}

CODECode codec_setup(CODEC codec, CODECOption opt, ...) {
    if (!codec) {
        return CODECNullPtr;
    }
    
    CODECBase *p = codec;
    va_list args;
    
    if (p->setup) {
        va_start(args, opt);
        p->code = p->setup(p, opt, args);
        va_end(args);
    }
    
    return p->code;
}

const CODECData * codec_work(CODEC codec, const CODECData *data) {
    if (!codec) {
        return 0;
    }
    
    CODECBase *p = codec;
    if (!data || !data->data || !data->length) {
        p->code = CODECEmptyInput;
        return 0;
    }
    
    if (p->work) {
        p->code = p->work(p, data);
    }
    
    return p->code == CODECOk ? p->result : 0;
}

void codec_reset(CODEC codec) {
    if (!codec) {
        return;
    }
    
    CODECBase *p = codec;
    if (p->reset) {
        p->reset(p);
    }
}

CODECode codec_lasterror(CODEC codec) {
    if (!codec) {
        return CODECNullPtr;
    }
    
    CODECBase *p = codec;
    return p->code;
}