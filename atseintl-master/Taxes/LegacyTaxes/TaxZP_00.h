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

#ifndef TAXZP_00_H
#define TAXZP_00_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse

{

class TaxZP_00 : public Tax
{
  friend class TaxZP_00Test;
  friend class TaxCodeValidatorZP;

  bool validateUS1(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg, int16_t segId=-1);

  bool isInLoc(const Loc& loc,
               const LocTypeCode& locTypeCode,
               const LocCode& locCode,
               const DateTime& ticketingDate,
               const Indicator& exclInd) const;

  void
  apply(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg, TravelSeg* travelSeg);

  bool validateTransitTime(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t startIndex,
                           bool bOrgUS);

  uint16_t getSegmentOrder(const PricingTrx& trx,
                           const TaxResponse& taxResponse,
                           TravelSeg* const travelSeg,
                           int16_t dflt = -1) const;

public:
  TaxZP_00();
  virtual ~TaxZP_00();

  //  virtual bool validateLocRestrictions (
  //                                        PricingTrx& trx,
  //                                        TaxResponse& taxResponse,
  //                                        TaxCodeReg& taxCodeReg,
  //                                        uint16_t& startIndex,
  //                                        uint16_t& endIndex);

  virtual bool validateTripTypes(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& startIndex,
                                 uint16_t& endIndex) override
  {
    return true;
  }
  //
  //  virtual bool validateRange (
  //                              PricingTrx& trx,
  //                              TaxResponse& taxResponse,
  //                              TaxCodeReg& taxCodeReg,
  //                              uint16_t& startIndex,
  //                              uint16_t& endIndex);
  //
  //  virtual bool validateTransit (
  //                                PricingTrx& trx,
  //                                TaxResponse& taxResponse,
  //                                TaxCodeReg& taxCodeReg,
  //                                uint16_t travelSegIndex);

  //  virtual bool validateCarrierExemption (
  //                                         PricingTrx& trx,
  //                                         TaxResponse& taxResponse,
  //                                         TaxCodeReg& taxCodeReg,
  //                                         uint16_t travelSegIndex);

  virtual bool validateEquipmentExemption(PricingTrx& trx,
                                          TaxResponse& taxResponse,
                                          TaxCodeReg& taxCodeReg,
                                          uint16_t travelSegIndex) override
  {
    return true;
  }
  //
  //  virtual bool validateFareClass (
  //                                  PricingTrx& trx,
  //                                  TaxResponse& taxResponse,
  //                                  TaxCodeReg& taxCodeReg,
  //                                  uint16_t travelSegIndex);

  //  virtual bool validateCabin (PricingTrx& trx,
  //                              TaxResponse& taxResponse,
  //                              TaxCodeReg& taxCodeReg,
  //                              uint16_t travelSegIndex);

  //  virtual bool validateTicketDesignator (
  //                                         PricingTrx& trx,
  //                                         TaxResponse& taxResponse,
  //                                         TaxCodeReg& taxCodeReg,
  //                                         uint16_t travelSegIndex);
  //  virtual bool validateSequence(
  //                                PricingTrx& trx,
  //                                TaxResponse& taxResponse,
  //                                TaxCodeReg& taxCodeReg,
  //                                uint16_t& travelSegStartIndex,
  //                                uint16_t& travelSegEndIndex,
  //                                bool checkSpn = false);

  virtual bool validateFinalGenericRestrictions(PricingTrx& trx,
                                                TaxResponse& taxResponse,
                                                TaxCodeReg& taxCodeReg,
                                                uint16_t& startIndex,
                                                uint16_t& endIndex) override;

  virtual bool
  validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  virtual void taxCreate(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegStartIndex,
                         uint16_t travelSegEndIndex) override;

  //  virtual void applyTaxOnTax (
  //                              PricingTrx& trx,
  //                              TaxResponse& taxResponse,
  //                              TaxCodeReg& taxCodeReg);

  //  virtual void adjustTax (
  //                          PricingTrx& trx,
  //                          TaxResponse& taxResponse,
  //                          TaxCodeReg& taxCodeReg);

  //  virtual void doTaxRange (
  //                           PricingTrx& trx,
  //                           TaxResponse& taxResponse,
  //                           uint16_t& startIndex,
  //                           uint16_t& endIndex,
  //                           TaxCodeReg& taxCodeReg);

  //  virtual void doTaxRound (PricingTrx& trx, TaxCodeReg& taxCodeReg);

private:
  static log4cxx::LoggerPtr _logger;
};
}
#endif // TAXZP_00_H
