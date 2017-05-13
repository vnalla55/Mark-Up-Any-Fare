// ----------------------------------------------------------------------------

//  Copyright Sabre 2013

//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.

// ----------------------------------------------------------------------------
#pragma once

#include <vector>

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class FlightUsage;
class Flight;
class TaxPointLoc1TransferTypeRule;

class TaxPointLoc1TransferTypeApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc1TransferTypeApplicator(TaxPointLoc1TransferTypeRule const& parent,
                                     const std::vector<FlightUsage>& flightUsages,
                                     const std::vector<Flight>& flights);

  ~TaxPointLoc1TransferTypeApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const std::vector<FlightUsage>& _flightUsages;
  const std::vector<Flight>& _flights;
  TaxPointLoc1TransferTypeRule const& _taxPointLoc1TransferTypeRule;

  bool failSubjects(PaymentDetail& paymentDetail) const;
  bool applyInterline(const type::Index flightA, const type::Index flightB) const;
  bool
  applyOnlineWithChangeOfFlightNumber(const type::Index flightA, const type::Index flightB) const;
  bool applyOnlineWithChangeOfGauge(const type::Index flightA, const type::Index flightB) const;
  bool applyOnlineWithNoChangeOfGauge(const type::Index flightA, const type::Index flightB) const;

  bool
  isSameCarrierOrBlank(const type::Index& flightUsageIdA, const type::Index& flightUsageIdB) const;
  bool isSameFlightNumberOrBlank(const type::Index& flightUsageIdA,
                                 const type::Index& flightUsageIdB) const;
};

} // namespace tax
