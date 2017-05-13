//---------------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "Common/TseCodeTypes.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Tax.h"

#include <boost/utility.hpp>

namespace tse
{
class PricingTrx;

class TaxOI_01 : public Tax, boost::noncopyable
{
  friend class TaxOI_01Test;

public:
  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  bool validateCarrierExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex) override;

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

private:
  bool validateSameDayRestrictions(TaxCodeReg& taxCodeReg,
                                   TravelSeg* travelSegFrom,
                                   TravelSeg* travelSegTo) const;

  bool validateTransitSeq(bool isTransit,
                          const TaxCodeReg& taxCodeReg,
                          TravelSeg* travelSegFrom,
                          TravelSeg* travelSegTo) const;

  bool validateStopoverSeq(bool isTransit,
                           const TaxCodeReg& taxCodeReg,
                           TravelSeg* travelSegFrom,
                           TravelSeg* travelSegTo) const;
};
} /* end tse namespace */
