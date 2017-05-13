/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <vector>

namespace tse
{
class SurchargeData;

struct SavedSurcharges
{
  SavedSurcharges(SavedSurcharges&& other)
    : _surchargeAmt(other._surchargeAmt),
      _surchargeDataFareUsage(std::move(other._surchargeDataFareUsage)),
      _surchargeDataPaxTypeFare(std::move(other._surchargeDataPaxTypeFare))
  {
  }

  SavedSurcharges(const MoneyAmount& surchargeAmt,
                  const std::vector<SurchargeData*>& surchargeDataFareUsage,
                  const std::vector<SurchargeData*>& surchargeDataPaxTypeFare)
    : _surchargeAmt(surchargeAmt),
      _surchargeDataFareUsage(surchargeDataFareUsage),
      _surchargeDataPaxTypeFare(surchargeDataPaxTypeFare)
  {
  }
  MoneyAmount _surchargeAmt;
  std::vector<SurchargeData*> _surchargeDataFareUsage;
  std::vector<SurchargeData*> _surchargeDataPaxTypeFare;
};
}

