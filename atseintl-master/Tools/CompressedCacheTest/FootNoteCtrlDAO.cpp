//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "FootNoteCtrlDAO.h"
#include "FootNoteCtrlInfo.h"
#include "CompressionTestCommon.h"

namespace tse
{
const bool _poolObjects(true);

FootNoteCtrlDAO& FootNoteCtrlDAO::instance()
{
    return *_instance;
}

std::vector<FootNoteCtrlInfo*>* FootNoteCtrlDAO::create(FootNoteCtrlKey key)
{
  size_t numitems(key._a % MAXNUMBERENTRIES);
  std::vector<FootNoteCtrlInfo*>* ret = new std::vector<FootNoteCtrlInfo*>(numitems);
  for (size_t i = 0; i < numitems; ++i)
  {
    FootNoteCtrlInfo *obj(new FootNoteCtrlInfo);
    obj->dummyData();
    (*ret)[i] = obj;
  }
  return ret;
}

void FootNoteCtrlDAO::destroy(FootNoteCtrlKey key, std::vector<FootNoteCtrlInfo*>* recs)
{
  if (!destroyPooledVectorCRI(recs))
  {
    destroyContainer( recs ) ;
  }
}

FootNoteCtrlKey FootNoteCtrlDAO::createKey( FootNoteCtrlInfo * info )
{
  return FootNoteCtrlKey() ;
}

void  FootNoteCtrlDAO::load()
{
}

sfc::CompressedData* FootNoteCtrlDAO::compress(const std::vector<FootNoteCtrlInfo*>* vect) const
{
  return compressVector(vect, _poolObjects);
}

std::vector<FootNoteCtrlInfo*>* FootNoteCtrlDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressEntry<FootNoteCtrlInfo>(compressed);
}

std::string FootNoteCtrlDAO::_name("FootNoteCtrl");
std::string FootNoteCtrlDAO::_cacheClass("Rules");
FootNoteCtrlDAO* FootNoteCtrlDAO::_instance = 0;
} // namespace tse
