//----------------------------------------------------------------------------
//  File:        MarketRoutingSinglesLoader.cpp
//  Created:     2008-07-09
//
//  Description: Market routing singles loader
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

#include "FileLoader/MarketRoutingSinglesLoader.h"

#include "Common/Logger.h"

using namespace tse;

static Logger
logger("atseintl.FileLoader.MarketRoutingSinglesLoader");
const char* MarketRoutingSinglesLoader::_separator = "|: ,";
const size_t MarketRoutingSinglesLoader::_sepLen = 4;

MarketRoutingSinglesLoader::MarketRoutingSinglesLoader(const std::string& dataDir,
                                                       const std::string& singlesFile,
                                                       MarketRoutingSinglesMap& singlesMap)
  : TextFileLoader(dataDir + singlesFile, "MarketRoutingSingles"),
    _dataDir(dataDir),
    _singlesFile(singlesFile),
    _singlesMap(singlesMap)
{
}

MarketRoutingSinglesLoader::~MarketRoutingSinglesLoader() {}

bool
MarketRoutingSinglesLoader::parseLine(const char* pLine, size_t length)
{
  const char* pBuffer = pLine;
  const char* const pBufferEnd = pLine + length;
  const char* pDelim = pBufferEnd;

  MarketRoutingKey key;
  key.initialized = true;
  MarketRoutingSinglesVec* singlesVec = new MarketRoutingSinglesVec();

  LocCode market = "";
  int fieldIndex = 0;
  int singles = 0;

  while ((pBuffer < pBufferEnd) &&
         ((pDelim = find(pBuffer, pBufferEnd, _separator, _sepLen)) != pBufferEnd))
  {
    size_t sz(pDelim - pBuffer);

    switch (fieldIndex)
    {
    case MRS_VENDOR:
      key._a.assign(pBuffer, sz);
      break;

    case MRS_TARIFF:
      key._b = atoi(pBuffer, sz);
      break;

    case MRS_CARRIER:
      key._c.assign(pBuffer, sz);
      break;

    case MRS_ROUTING:
      key._d.assign(pBuffer, sz);
      break;

    case MRS_INDEX:
      singles = atoi(pBuffer, sz);
      break;

    default:
      // Skip space before market string
      if (pBuffer[0] == ' ')
      {
        pBuffer++;
        sz--;
      }

      market.assign(pBuffer, sz);
      singlesVec->push_back(market);
      break;
    }

    ++fieldIndex;
    pBuffer = pDelim + 1;
  }

  if (pBuffer != pBufferEnd)
  {
    return false;
  }

  // Add to map
  MarketRoutingSinglesMap::iterator itr = _singlesMap.find(key);

  if (itr == _singlesMap.end())
  {
    if (singles != 1)
    {
      LOG4CXX_ERROR(logger,
                    "Market Routing Singles Descrepancy Found: "
                        << "Vendor[" << key._a << "] Tariff[" << key._b << "] Carrier[" << key._c
                        << "] Routing[" << key._d << "], Singles Index[" << singles << "]");

      return false;
    }

    _singlesMap[key].push_back(nullptr); // first index will be null
  }

  _singlesMap[key].push_back(singlesVec);

  return true;
}

bool
MarketRoutingSinglesLoader::parseStarting()
{
  LOG4CXX_INFO(logger,
               "MarketRoutingSinglesLoader::parseStarting() - Routing Singles Map Size = "
                   << _singlesMap.size());
  return true;
}

bool
MarketRoutingSinglesLoader::parseFinished()
{
  LOG4CXX_INFO(logger,
               "MarketRoutingSinglesLoader::parseFinished() - Routing Singles Map Size = "
                   << _singlesMap.size());
  return true;
}
