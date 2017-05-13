//-------------------------------------------------------------------
//
//  File:        DataAggregator.cpp
//  Authors:     Mike Carroll
//  Created:     July 24, 2005
//  Description: Base class for a data aggregator
//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/DataAggregator.h"

#include "Common/FareDisplaySurcharge.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/SurchargeData.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Templates.DataAggregator");

DataAggregator::DataAggregator() {}

DataAggregator::~DataAggregator() {}

//--------------------------------------------------------------------------
// @function DataAggregator::surcharges
//
// Description: Method to aggregate surcharges
//
// @param field - field to be filtered
// @param paxTypeFare - a valid PaxTypeFare
// @param trx - a valid FareDisplayTrx
//--------------------------------------------------------------------------
void
DataAggregator::surcharges(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  MoneyAmount owSurcharge = 0;
  FareDisplaySurcharge::getTotalOWSurcharge(trx, paxTypeFare, owSurcharge);
  field.moneyValue() = owSurcharge;
  if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ||
      (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED && trx.getOptions()->roundTripFare() == 'T'))
  {
    MoneyAmount rtSurcharge = 0;
    FareDisplaySurcharge::getTotalRTSurcharge(trx, paxTypeFare, rtSurcharge);
    field.moneyValue() = rtSurcharge;
  }
}

//--------------------------------------------------------------------------
// @function DataAggregator::taxes
//
// Description: Method to aggregate taxes
//
// @param field - field to be filtered
// @param paxTypeFare - a valid PaxTypeFare
// @param trx - a valid FareDisplayTrx
//--------------------------------------------------------------------------
void
DataAggregator::US1Taxes(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  MoneyAmount owTax, rtTax;

  FareDisplayTax::getOWTax(trx, paxTypeFare, FareDisplayTax::TAX_CODE_US1, owTax);
  FareDisplayTax::getRTTax(trx, paxTypeFare, FareDisplayTax::TAX_CODE_US1, rtTax);

  Indicator owrt = paxTypeFare.owrt();

  if (owrt == ONE_WAY_MAY_BE_DOUBLED || owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    field.moneyValue() = owTax;
  else
    field.moneyValue() = rtTax;
}
}
