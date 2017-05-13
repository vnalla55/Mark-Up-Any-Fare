//----------------------------------------------------------------------------
//  File:        MarketRoutingSinglesLoader.h
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

#pragma once

#include "FileLoader/MarketRoutingMap.h"
#include "FileLoader/TextFileLoader.h"

namespace tse
{

enum MRS_FIELD_INDEX
{
  MRS_VENDOR,
  MRS_TARIFF,
  MRS_CARRIER,
  MRS_ROUTING,
  MRS_INDEX,
  MRS_MARKETS
};

class MarketRoutingSinglesLoader : public TextFileLoader
{
public:
  MarketRoutingSinglesLoader(const std::string& dataDir,
                             const std::string& singlesFile,
                             MarketRoutingSinglesMap& singlesMap);

  virtual ~MarketRoutingSinglesLoader();

protected:
  virtual bool parseLine(const char* pLine, size_t length) override;

  virtual bool parseStarting() override;

  virtual bool parseFinished() override;

private:
  static const char* _separator;
  static const size_t _sepLen;

  std::string _dataDir;
  std::string _singlesFile;
  MarketRoutingSinglesMap& _singlesMap;
};

} // end namespace tse

