// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#ifndef LOC_RESTRICTION_VALIDATOR_H
#define LOC_RESTRICTION_VALIDATOR_H

#include "Common/TseCodeTypes.h"

namespace tse

{
class PricingTrx;
class TaxCodeReg;
class TaxResponse;
class TravelSeg;
class Itin;

class LocRestrictionValidator
{

public:
  LocRestrictionValidator();
  virtual ~LocRestrictionValidator();

  bool validateLocation(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& startIndex,
                        uint16_t& endIndex);

  //-------------------------------
  //--- Tax Database definition ---
  //-------------------------------

  static constexpr char TAX_EXCLUDE = 'Y';

  //------------------------
  //--- apply to LocAppl ---
  //------------------------

  static constexpr char TAX_ORIGIN = 'O'; // first board pt
  static constexpr char TAX_DESTINATION = 'D'; // furthest point
  static constexpr char TAX_ENPLANEMENT = 'E'; // any board pt in travelSeg
  static constexpr char TAX_DEPLANEMENT = 'X'; // any off pt in travelSeg
  static constexpr char TAX_TERMINATION = 'T'; // the last off point

  static constexpr char FURTHEST_POINT = 'F'; // furthest point

  void setItin(const Itin* itin) { _itin=itin; };

protected:
  LocRestrictionValidator(const LocRestrictionValidator& map);
  LocRestrictionValidator& operator=(const LocRestrictionValidator& map);

  virtual bool validateOrigin(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex);

  virtual bool validateEnplanement(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex);

  virtual bool validateDeplanement(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex);

  virtual bool validateDestination(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex);

  virtual bool validateTermination(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex);

  const std::vector<TravelSeg*>& getTravelSeg(TaxResponse& taxResponse) const;

private:
  const Itin* _itin = nullptr;
};
}

#endif // LOC_RESTRICTION_VALIDATOR_H
