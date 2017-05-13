//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TSIDAO.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/TSIInfo.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>

#include <boost/tokenizer.hpp>

namespace tse
{
log4cxx::LoggerPtr
TSIDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TSIDAO"));

TSIDAO&
TSIDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TSIInfo*
getTSIData(int key, DeleteList& deleteList)
{
  TSIDAO& dao = TSIDAO::instance();
  return dao.get(deleteList, key);
}

const TSIInfo*
TSIDAO::get(DeleteList& del, int key)
{
  if (UNLIKELY(key == 0))
    return nullptr;

  DAOCache::pointer_type ptr = cache().getIfResident(IntKey(key));
  return ptr.get();
}

struct TSIDAO::groupByKey
{
public:
  void operator()(tse::ConfigMan::NameValue rec)
  {
    DAOCache& cache = TSIDAO::instance().cache();
    int tsi = atoi(rec.name.c_str());
    IntKey k(tsi);
    DAOCache::pointer_type tsiInfo = cache.get(k);
    tsiInfo->tsi() = tsi;
    boost::char_separator<char> sep("|");
    boost::tokenizer<boost::char_separator<char> > tok(rec.value, sep);
    boost::tokenizer<boost::char_separator<char> >::iterator i = tok.begin();
    tsiInfo->description() = (*i);
    i++;
    tsiInfo->geoRequired() = (*i)[0];
    i++;
    tsiInfo->geoNotType() = (*i)[0];
    i++;
    tsiInfo->geoOut() = (*i)[0];
    i++;
    tsiInfo->geoItinPart() = (*i)[0];
    i++;
    tsiInfo->geoCheck() = (*i)[0];
    i++;
    tsiInfo->loopDirection() = (*i)[0];
    i++;
    tsiInfo->loopOffset() = atoi((*i).c_str());
    i++;
    tsiInfo->loopToSet() = atoi((*i).c_str());
    i++;
    tsiInfo->loopMatch() = (*i)[0];
    i++;
    tsiInfo->scope() = (*i)[0];
    i++;
    tsiInfo->type() = (*i)[0];
    i++;
    for (; i != tok.end(); i++)
    {
      char c = (*i)[0];
      if (c == 0)
        c = ' ';
      tsiInfo->matchCriteria().push_back((TSIInfo::TSIMatchCriteria)c);
    }
  }
};

IntKey
TSIDAO::createKey(TSIInfo* info)
{
  return IntKey(info->tsi());
}

void
TSIDAO::load()
{
  try
  {
    CacheManager& cm = CacheManager::instance();
    std::string tsiIni;
    if (!cm.config().getValue("IniFile", tsiIni, "TSI"))
    {
      CONFIG_MAN_LOG_KEY_ERROR(_logger, "IniFile", "TSI");
    }

    tse::ConfigMan config;
    config.read(tsiIni);
    std::vector<tse::ConfigMan::NameValue> recs;
    config.getValues(recs);
    std::for_each(recs.begin(), recs.end(), groupByKey());
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TSIDAO::load");
    throw;
  }
}

TSIInfo*
TSIDAO::create(IntKey key)
{
  return new TSIInfo;
}

void
TSIDAO::destroy(IntKey key, TSIInfo* rec)
{
  delete rec;
}

size_t
TSIDAO::clear()
{
  return 0;
}

std::string
TSIDAO::_name("TSI");
std::string
TSIDAO::_cacheClass("Common");
DAOHelper<TSIDAO>
TSIDAO::_helper(_name);
TSIDAO* TSIDAO::_instance = nullptr;
} // namespace tse
