//-------------------------------------------------------------------
//
//  File:        MarketResponse.h
//  Created:     March 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class MarketCriteria;
class BrandProgram;
class BSDiagnostics;

class MarketResponse : boost::noncopyable
{
public:
  using ProgramIDList = std::vector<ProgramID>;
  using CarrierBrandData = std::vector<BrandProgram*>;

  int& setMarketID() { return _marketID; }
  int getMarketID() const { return _marketID; }

  ProgramIDList& programIDList() { return _programIDList; }
  const ProgramIDList& programIDList() const { return _programIDList; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  MarketCriteria*& marketCriteria() { return _marketCriteria; }
  const MarketCriteria* marketCriteria() const { return _marketCriteria; }

  CarrierBrandData& brandPrograms() { return _brandPrograms; }
  const CarrierBrandData& brandPrograms() const { return _brandPrograms; }

  std::string& errMessage() { return _errMessage; }
  const std::string& errMessage() const { return _errMessage; }

  BSDiagnostics*& bsDiagnostics() { return _bsDiagnostics; }
  const BSDiagnostics* bsDiagnostics() const { return _bsDiagnostics; }

  BrandSource& dataSource() { return _dataSource; }
  const BrandSource& dataSource() const { return _dataSource; }

private:
  int _marketID = 0;
  ProgramIDList _programIDList;
  CarrierCode _carrier;
  MarketCriteria* _marketCriteria = nullptr;
  CarrierBrandData _brandPrograms;
  std::string _errMessage;
  BSDiagnostics* _bsDiagnostics = nullptr;
  BrandSource _dataSource = BrandSource::BRAND_SOURCE_S8;
};
} // tse

