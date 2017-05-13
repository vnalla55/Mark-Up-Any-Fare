//----------------------------------------------------------------------------
//  File:        MarketRoutingLoader.h
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

#pragma once

#include "DBAccess/Cache.h"
#include "FileLoader/MarketRoutingMap.h"
#include "FileLoader/TextFileLoader.h"


namespace tse
{

enum MR_FIELD_INDEX
{
  MR_VENDOR,
  MR_TARIFF,
  MR_CARRIER,
  MR_ROUTING,
  MR_MARKET1,
  MR_MARKET2,
  MR_SINGLES,
  MR_DOUBLES,
  MR_TOKEN_COUNT
};

class MarketRoutingLoader : public TextFileLoader
{
public:
  MarketRoutingLoader(const std::string& dataDir,
                      const std::string& marketsFile,
                      const std::string& singlesFile,
                      const std::string& doublesFile,
                      const int32_t& generationNumber,
                      MarketRoutingCache* cache);

  virtual ~MarketRoutingLoader();

protected:
  bool parseLine(const char* pLine, size_t length) override;

  bool parseStarting() override;

  bool parseFinished() override;

  void insertRouting(const MarketRoutingKey& key);

private:

  std::string _dataDir;
  std::string _marketsFile;
  std::string _singlesFile;
  std::string _doublesFile;
  int32_t _generationNumber;
  MarketRoutingCache* const _cache;
  MarketRoutingSinglesMap _singleRoutingMap;
  MarketRoutingDoublesMap _doubleRoutingMap;
  MarketRouting* _marketRouting;
  MarketRoutingKey _key;
};

} // end namespace tse

