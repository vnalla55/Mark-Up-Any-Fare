// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/Message.h"
#include "Common/Money.h"
#include "Common/MultiTicketUtil.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/XMLConstruct.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FcMessage.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "Xform/FareCalcModel.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/ResponseFormatter.h"


#include <boost/core/noncopyable.hpp>

namespace tse
{
class PricingTrx;

class XMLFareCalcFormatter : private boost::noncopyable
{
  FareCalcModel& _model;
  XMLConstruct& _construct;
  PricingResponseFormatter& _pricingResponseFormatter;

public:
  XMLFareCalcFormatter(FareCalcModel& model, XMLConstruct& construct,
      PricingResponseFormatter& pricingResponseFormatter)
      : _model(model), _construct(construct), _pricingResponseFormatter(pricingResponseFormatter)
  {
  }

  void
  formatCAL(PricingTrx& pricingTrx,
      CalcTotals& calcTotals,
      const FareUsage& fareUsage,
      const PricingUnit& pricingUnit,
      const CurrencyNoDec& noDecCalc,
      const FarePath& farePath,
      uint16_t& segmentOrder,
      XMLConstruct& construct,
      std::vector<const FareUsage*>& fuPlusUpsShown,
      const uint16_t& fcId);
};

}
