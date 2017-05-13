//----------------------------------------------------------------------------
//  File:        MarketRoutingLoader.cpp
//  Created:     2008-07-09
//
//  Description: Market routing loader
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "FileLoader/MarketRoutingLoader.h"

#include "Common/Logger.h"
#include "FileLoader/MarketRoutingDoublesLoader.h"
#include "FileLoader/MarketRoutingSinglesLoader.h"

#include <utility>
#include <vector>

using namespace tse;

static Logger
logger("atseintl.FileLoader.MarketRoutingLoader");

MarketRoutingLoader::MarketRoutingLoader(const std::string& dataDir,
                                         const std::string& marketsFile,
                                         const std::string& singlesFile,
                                         const std::string& doublesFile,
                                         const int32_t& generationNumber,
                                         MarketRoutingCache* cache)
  : TextFileLoader(dataDir + marketsFile, "MarketRoutingMarkets"),
    _dataDir(dataDir),
    _marketsFile(marketsFile),
    _singlesFile(singlesFile),
    _doublesFile(doublesFile),
    _generationNumber(generationNumber),
    _cache(cache),
    _marketRouting(nullptr)
{
}

MarketRoutingLoader::~MarketRoutingLoader()
{
  // Delete remaining/unreferenced singles vector from maps
  MarketRoutingSinglesMap::iterator mrsmIter = _singleRoutingMap.begin();
  MarketRoutingSinglesMap::iterator mrsmIterEnd = _singleRoutingMap.end();

  for (; mrsmIter != mrsmIterEnd; ++mrsmIter)
  {
    std::vector<MarketRoutingSinglesVec*>& svec = mrsmIter->second;
    std::vector<MarketRoutingSinglesVec*>::iterator svitr;
    for (svitr = svec.begin(); svitr != svec.end(); ++svitr)
    {
      delete *svitr;
    }
  }

  // Delete remaining/unreferenced doubles vector from maps
  MarketRoutingDoublesMap::iterator mrdmIter = _doubleRoutingMap.begin();
  MarketRoutingDoublesMap::iterator mrdmIterEnd = _doubleRoutingMap.end();

  for (; mrdmIter != mrdmIterEnd; ++mrdmIter)
  {
    std::vector<MarketRoutingDoublesVec*>& dvec = mrdmIter->second;
    std::vector<MarketRoutingDoublesVec*>::iterator dvitr;
    for (dvitr = dvec.begin(); dvitr != dvec.end(); ++dvitr)
    {
      delete *dvitr;
    }
  }
}

bool
MarketRoutingLoader::parseLine(const char* pLine, size_t length)
{
  const char* pBuffer = pLine;
  const char* const pBufferEnd = pLine + length;
  const char* pDelim = pBufferEnd;

  MarketRoutingKey key;
  MarketRoutingIndexKey indexKey;
  int singles = 0, doubles = -1;

  key.initialized = true;
  indexKey.initialized = true;
  int fieldIndex = 0;

  while ((pBuffer < pBufferEnd) && ((pDelim = std::find(pBuffer, pBufferEnd, ',')) != pBufferEnd))
  {
    size_t sz(pDelim - pBuffer);

    switch (fieldIndex)
    {
    case MR_VENDOR:
      key._a.assign(pBuffer, sz);
      break;

    case MR_TARIFF:
      key._b = atoi(pBuffer, sz);
      break;

    case MR_CARRIER:
      key._c.assign(pBuffer, sz);
      break;

    case MR_ROUTING:
      key._d.assign(pBuffer, sz);
      break;

    case MR_MARKET1:
      indexKey._a.assign(pBuffer, sz);
      break;

    case MR_MARKET2:
      indexKey._b.assign(pBuffer, sz);
      break;

    case MR_SINGLES:
      singles = atoi(pBuffer, sz);
      break;

    case MR_DOUBLES:
      doubles = atoi(pBuffer, sz);
      break;
    }

    ++fieldIndex;
    pBuffer = pDelim + 1;
  }

  if ((doubles == -1) && (pBuffer != pBufferEnd))
  {
    doubles = atoi(pBuffer, (pBufferEnd - pBuffer));
  }

  if (!(_key == key))
  {
    _key = key;
    _marketRouting = new MarketRouting();

    insertRouting(_key);
  }

  _marketRouting->addIndexInfo(indexKey, singles, doubles);

  return true;
}

bool
MarketRoutingLoader::parseStarting()
{
  MarketRoutingSinglesLoader singlesLoader(_dataDir, _singlesFile, _singleRoutingMap);

  if (false == singlesLoader.parse())
  {
    return false;
  }

  MarketRoutingDoublesLoader doublesLoader(_dataDir, _doublesFile, _doubleRoutingMap);

  if (false == doublesLoader.parse())
  {
    return false;
  }

  LOG4CXX_INFO(
      logger,
      "MarketRoutingLoader::parseStarting() - Market Routing Cache Size = " << _cache->size());

  return true;
}

bool
MarketRoutingLoader::parseFinished()
{
  LOG4CXX_INFO(
      logger,
      "MarketRoutingLoader::parseFinished() - Market Routing Cache Size = " << _cache->size());

  std::shared_ptr<std::vector<MarketRoutingKey>> keys = _cache->keys();

  std::vector<MarketRoutingKey>::iterator keysIter = keys->begin();
  std::vector<MarketRoutingKey>::iterator keysIterRem = keysIter;
  std::vector<MarketRoutingKey>::iterator keysIterEnd = keys->end();

  while (keysIter != keysIterEnd)
  {
    keysIterRem = keysIter;
    ++keysIter;

    MarketRoutingKey& key = (*keysIterRem);
    MarketRouting* ret = nullptr;

    MarketRoutingCache::pointer_type ptr = _cache->getIfResident(key);

    if (ptr)
    {
      ret = ptr.get();

      if (ret->generationNumber() != _generationNumber)
      {
        _cache->invalidate(key);
      }
    }
  }

  LOG4CXX_INFO(logger,
               "MarketRoutingLoader::parseFinished() - Market Routing Cache Size after cleanup = "
                   << _cache->size());

  return true;
}

void
MarketRoutingLoader::insertRouting(const MarketRoutingKey& key)
{
  _marketRouting->generationNumber() = _generationNumber;

  MarketRoutingSinglesMap::iterator mrsmIter = _singleRoutingMap.find(key);

  if (mrsmIter != _singleRoutingMap.end())
  {
    _marketRouting->addSingles(mrsmIter->second);
    _singleRoutingMap.erase(mrsmIter);
  }

  MarketRoutingDoublesMap::iterator mrdmIter = _doubleRoutingMap.find(key);

  if (mrdmIter != _doubleRoutingMap.end())
  {
    _marketRouting->addDoubles(mrdmIter->second);
    _doubleRoutingMap.erase(mrdmIter);
  }

  _cache->put(key, _marketRouting, false);
}
