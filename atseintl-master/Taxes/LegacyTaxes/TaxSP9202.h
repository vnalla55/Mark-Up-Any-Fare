//---------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#ifndef TAXSP9202_H
#define TAXSP9202_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class Loc;

class TaxSP9202 : public Tax
{
  friend class TaxZQTest;

public:
  TaxSP9202();
  virtual ~TaxSP9202();

  void preparePortionOfTravelIndexes(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg) override;
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
  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

protected:
  virtual bool setJourneyBreakEndIndex(PricingTrx& trx,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg,
                                       uint16_t& startIndex,
                                       uint16_t& endIndex);
  virtual uint16_t findJourneyBreak(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t startIndex);

  bool applyPortionOfTravel(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex);
  bool validateLoc1(const Loc*& loc, TaxCodeReg& taxCodeReg);
  bool validateLoc2(const Loc*& loc, TaxCodeReg& taxCodeReg);
  bool isGeoMatch(const Loc& aLoc, LocTypeCode& locType, LocCode& loc, Indicator& locExclInd);
  bool isDomSeg(const TravelSeg* ts) const;
  uint16_t findMirrorImage(PricingTrx& trx, TaxResponse& taxResponse);

private:
  TaxSP9202(const TaxSP9202&);
  TaxSP9202& operator=(const TaxSP9202&);
};

} /* end tse namespace */

#endif
