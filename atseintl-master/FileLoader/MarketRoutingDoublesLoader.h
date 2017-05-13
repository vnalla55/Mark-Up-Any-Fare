//----------------------------------------------------------------------------
//  File:        MarketRoutingDoublesLoader.h
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

#pragma once

#include "FileLoader/MarketRoutingMap.h"
#include "FileLoader/TextFileLoader.h"

namespace tse
{

enum MRD_FIELD_INDEX
{
  MRD_VENDOR,
  MRD_TARIFF,
  MRD_CARRIER,
  MRD_ROUTING,
  MRD_INDEX,
  MRD_MARKETS
};

class MarketRoutingDoublesLoader : public TextFileLoader
{
public:
  MarketRoutingDoublesLoader(const std::string& dataDir,
                             const std::string& fileName,
                             MarketRoutingDoublesMap& doublesMap);

  virtual ~MarketRoutingDoublesLoader();

protected:
  virtual bool parseLine(const char* pLine, size_t length) override;

  virtual bool parseStarting() override;

  virtual bool parseFinished() override;

private:
  static const char* _separator;
  static const size_t _sepLen;
  std::string _dataDir;
  std::string _doublesFile;
  MarketRoutingDoublesMap& _doublesMap;
};

} // end namespace tse

