#ifndef FIFO_CACHE_H
#define FIFO_CACHE_H

#ifdef _USERAWPOINTERS
#include "DBAccess/FIFOCacheRP.h"

#else
#include "DBAccess/FIFOCacheSP.h"

#endif // _USERAWPOINTERS

#endif // FIFO_CACHE_H
