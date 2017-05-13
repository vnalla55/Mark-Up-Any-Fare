//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#ifdef _USERAWPOINTERS
#include "DBAccess/LRUCacheRP.h"

#else
#include "DBAccess/LRUCacheSP.h"

#endif // _USERAWPOINTERS

#endif // LRU_CACHE_H
