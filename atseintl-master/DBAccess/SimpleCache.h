#ifndef SIMPLE_CACHE_H
#define SIMPLE_CACHE_H

#ifdef _USERAWPOINTERS
#include "DBAccess/SimpleCacheRP.h"

#else
#include "DBAccess/SimpleCacheSP.h"

#endif // _USERAWPOINTERS

#endif // SIMPLE_CACHE_H
