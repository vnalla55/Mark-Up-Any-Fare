// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Xform/ReissueExchangeModel.h"

class XMLConstruct;

namespace tse
{
class CalcTotals;
class Money;
class PricingTrx;
class RexPricingTrx;

class ReissueExchangeFormatter
{
  PricingTrx& _pricingTrx;
  XMLConstruct& _construct;

  ReissueExchangeFormatter(const ReissueExchangeFormatter&) = delete;
  ReissueExchangeFormatter& operator=(const ReissueExchangeFormatter&) = delete;

public:
  ReissueExchangeFormatter(PricingTrx& pricingTrx, XMLConstruct& construct)
    : _pricingTrx(pricingTrx), _construct(construct)
  {
  }

  void formatReissueExchange(CalcTotals& calcTotals);
  void formatReissueExchange(const ReissueExchangeModel& model);
  void prepareTaxesOnChangeFee(CalcTotals& calcTotals);
  void electronicTicketInd(const Indicator& electronicTicketIndicator);
  void prepareChangeFee(RexPricingTrx& trx, CalcTotals& calcTotals);
  void addChangeFeesForRefund(const RefundPermutation& winnerPerm);
  void formatChangeFee(const AbstractChangeFeeModel& model);
  void formatTaxesOnChangeFee(const ReissueExchangeModel& model);
};

} // end of tse namespace
