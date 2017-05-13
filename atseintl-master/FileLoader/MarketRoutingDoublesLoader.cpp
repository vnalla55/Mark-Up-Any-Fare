//----------------------------------------------------------------------------
//  File:        MarketRoutingDoublesLoader.cpp
//  Created:     2008-07-09
//
//  Description: Market routing doubles loader
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

#include "FileLoader/MarketRoutingDoublesLoader.h"

#include "Common/Logger.h"

using namespace tse;

static Logger
logger("atseintl.FileLoader.MarketRoutingDoublesLoader");
const char* MarketRoutingDoublesLoader::_separator = "|: -,";
const size_t MarketRoutingDoublesLoader::_sepLen = 5;

MarketRoutingDoublesLoader::MarketRoutingDoublesLoader(const std::string& dataDir,
                                                       const std::string& doublesFile,
                                                       MarketRoutingDoublesMap& doublesMap)
  : TextFileLoader(dataDir + doublesFile, "MarketRoutingDoubles"),
    _dataDir(dataDir),
    _doublesFile(doublesFile),
    _doublesMap(doublesMap)
{
}

MarketRoutingDoublesLoader::~MarketRoutingDoublesLoader() {}

bool
MarketRoutingDoublesLoader::parseLine(const char* pLine, size_t length)
{
  const char* pBuffer = pLine;
  const char* const pBufferEnd = pLine + length;
  const char* pDelim = pBufferEnd;

  MarketRoutingKey key;
  key.initialized = true;
  MarketRoutingDoublesVec* doublesVec = new MarketRoutingDoublesVec();

  MarketRoutingDoubles marketPair;
  int fieldIndex = 0;
  int pairInd = 0;
  int doubles = 0;

  while ((pBuffer < pBufferEnd) &&
         ((pDelim = find(pBuffer, pBufferEnd, _separator, _sepLen)) != pBufferEnd))
  {
    size_t sz(pDelim - pBuffer);

    switch (fieldIndex)
    {
    case MRD_VENDOR:
      key._a.assign(pBuffer, sz);
      break;

    case MRD_TARIFF:
      key._b = atoi(pBuffer, sz);
      break;

    case MRD_CARRIER:
      key._c.assign(pBuffer, sz);
      break;

    case MRD_ROUTING:
      key._d.assign(pBuffer, sz);
      break;

    case MRD_INDEX:
      doubles = atoi(pBuffer, sz);
      break;

    default:
      // Skip space before market string
      if (pBuffer[0] == ' ')
      {
        pBuffer++;
        sz--;
      }

      if (pairInd == 0)
      {
        marketPair.market1().assign(pBuffer, sz);
        pairInd = 1;
      }
      else
      {
        marketPair.market2().assign(pBuffer, sz);
        doublesVec->push_back(marketPair);
        pairInd = 0;
      }
      break;
    }

    ++fieldIndex;
    pBuffer = pDelim + 1;
  }

  if (pBuffer != pBufferEnd)
  {
    return false;
  }

  // add to map
  MarketRoutingDoublesMap::iterator mrdmIter = _doublesMap.find(key);

  if (mrdmIter == _doublesMap.end())
  {
    if (doubles != 1)
    {
      LOG4CXX_ERROR(logger,
                    "Market Routing Doubles Descrepancy Found: "
                        << "Vendor[" << key._a << "] Tariff[" << key._b << "] Carrier[" << key._c
                        << "] Routing[" << key._d << "], Doubles Index[" << doubles << "]");
      return false;
    }

    _doublesMap[key].push_back(nullptr); // first index will be null
  }

  _doublesMap[key].push_back(doublesVec);

  return true;
}

bool
MarketRoutingDoublesLoader::parseStarting()
{
  LOG4CXX_INFO(logger,
               "MarketRoutingDoublesLoader::parseStarting() - Routing Doubles Map Size = "
                   << _doublesMap.size());
  return true;
}

bool
MarketRoutingDoublesLoader::parseFinished()
{
  LOG4CXX_INFO(logger,
               "MarketRoutingDoublesLoader::parseFinished() - Routing Doubles Map Size = "
                   << _doublesMap.size());
  return true;
}
