// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "TestServer/Facades/TaxRoundingInfoServiceServer.h"
#include "Rules/RequestLogicError.h"
#include "Rules/MathUtils.h"
#include "Common/RoundingUtil.h"
#include "DataModel/Common/CodeIO.h"

namespace tax {

TaxRoundingInfoServiceServer::TaxRoundingInfoServiceServer()
{
}

void
TaxRoundingInfoServiceServer::initialize(const boost::ptr_vector<TaxRounding>& data)
{
  _dataMap.clear();
  for(const TaxRounding& tr : data)
  {
    std::map<type::Nation, TaxRounding>::iterator it = _dataMap.find(tr.nation());
    if (it == _dataMap.end())
      _dataMap[tr.nation()] = tr;
    else
      throw RequestLogicError() << "Tax rounding data for nation " << tr.nation() << " present twice!";
  }
}

void
TaxRoundingInfoServiceServer::getTrxRoundingInfo(const type::Nation& nation,
                                                 type::MoneyAmount& unit,
                                                 type::TaxRoundingDir& dir) const
{
  if (unit != -1 && dir != type::TaxRoundingDir::Blank)
  {
    return;
  }

  std::map<type::Nation, TaxRounding>::const_iterator it = _dataMap.find(nation);
  if (it == _dataMap.end())
  {
    throw RequestLogicError() << "No default tax rounding for nation " << nation;
  }
  const TaxRounding& result (it->second);
  if (dir == type::TaxRoundingDir::Blank)
  {
    switch (result.taxRoundingDir()) {
      case type::TaxRoundingDefaultDir::RoundUp:
        dir = type::TaxRoundingDir::RoundUp;
        break;
      case type::TaxRoundingDefaultDir::RoundDown:
        dir = type::TaxRoundingDir::RoundDown;
        break;
      case type::TaxRoundingDefaultDir::Nearest:
        dir = type::TaxRoundingDir::Nearest;
        break;
      case type::TaxRoundingDefaultDir::NoRounding:
        dir = type::TaxRoundingDir::NoRounding;
        break;
      case type::TaxRoundingDefaultDir::Blank:
        // do nothing - already blank
        break;
    }
  }
  if (unit == -1)
  {
    if (result.taxRoundingUnit() == 0)
      unit = 0;
    else
      unit = MathUtils::adjustDecimal(result.taxRoundingUnit(), result.taxUnitDecimals());
  }
}

void
TaxRoundingInfoServiceServer::doStandardRound(type::MoneyAmount& amount,
                                              type::MoneyAmount& unit,
                                              type::TaxRoundingDir& dir,
                                              type::MoneyAmount /*currencyUnit*/,
                                              bool /*isOcFee*/) const
{
  if (dir == type::TaxRoundingDir::NoRounding)
  {
    dir = type::TaxRoundingDir::RoundDown;
    unit = type::MoneyAmount(1, 100);
  }

  amount = doRound(amount, unit, dir, true);
}

}
