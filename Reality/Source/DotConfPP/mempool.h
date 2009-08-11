/*  Copyright (C) 2003 Aleksey Krivoshey <voodoo@foss.kharkov.ua>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef ASYNC_DNS_MEMPOOL_H
#define ASYNC_DNS_MEMPOOL_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#undef free
#undef calloc

class AsyncDNSMemPool
{
private:
    struct PoolChunk {
        void * pool;
        size_t pos;
        size_t size;

        PoolChunk(size_t _size);
        ~PoolChunk();
    };
    PoolChunk ** chunks;
    size_t chunksCount;
    size_t defaultSize;

    size_t poolUsage;
    size_t poolUsageCounter;

    void addNewChunk(size_t size);

public:
    AsyncDNSMemPool(size_t _defaultSize = 4096);
    virtual ~AsyncDNSMemPool();

    int initialize();
    void free();
    void * alloc(size_t size);
    void * calloc(size_t size);
    char * _strdup(const char *str);
};

#endif
