//-------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "DataModel/BaseExchangeTrx.h"
#include "Service/Service.h"

namespace tse
{
class RexBaseTrx;

class ExchangePricingTrx : public BaseExchangeTrx
{
public:
  ExchangePricingTrx();

  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  bool initialize(RexBaseTrx& trx, bool forDiagnostic);

  virtual const DateTime& travelDate() const override;
  virtual const DateTime& adjustedTravelDate(const DateTime& travelDate) const override;

  void applyCurrentBsrRoeDate() override;
  void applyHistoricalBsrRoeDate() override;

private:
  void initializeOverrides(RexBaseTrx& trx);
};
} // tse namespace
