//-------------------------------------------------------------------
//
//  File:        OCEmdDataProvider.h
//  Created:     2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
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
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

namespace tse
{

class OCEmdDataProvider : boost::noncopyable
{
public:
  Indicator& emdType() { return _emdType; }
  const Indicator& emdType() const { return _emdType; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  CrsCode& gds() { return _gds; }
  const CrsCode& gds() const { return _gds; }

  CarrierCode& emdValidatingCarrier() { return _emdValidatingCarrier; }
  const CarrierCode& emdValidatingCarrier() const { return _emdValidatingCarrier; }

  void setEmdMostSignificantCarrier(const CarrierCode& msc) { _emdMostSignificantCarrier = msc; }
  const CarrierCode& getEmdMostSignificantCarrier() const { return _emdMostSignificantCarrier; }

  std::set<CarrierCode>& operatingCarriers() { return _operatingCarriers; }
  const std::set<CarrierCode>& operatingCarriers() const { return _operatingCarriers; }

  std::set<CarrierCode>& marketingCarriers() { return _marketingCarriers; }
  const std::set<CarrierCode>& marketingCarriers() const { return _marketingCarriers; }

private:
  Indicator _emdType = ' ';
  NationCode _nation;
  CrsCode _gds;
  CarrierCode _emdValidatingCarrier;
  CarrierCode _emdMostSignificantCarrier;
  std::set<CarrierCode> _operatingCarriers;
  std::set<CarrierCode> _marketingCarriers;
};

} // tse

