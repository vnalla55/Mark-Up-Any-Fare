//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Rules/RuleApplicationBase.h"
#include "Rules/SubjectObserved.h"

#include <vector>

namespace tse
{
class MinStayRestriction;
class ShoppingTrx;
class Diag306Collector;
template <typename... TypesToNotify>
class IObserver;

class MinimumStayApplication
    : public RuleApplicationBase,
      public SubjectObserved<IObserver<const RuleItemInfo*, DateTime, int16_t, LocCode>>
{
  friend class MinimumStayApplicationTest;

public:
  static const std::string HOURS;
  static const std::string MINUTES;
  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a FareMarket.
   *
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& paxTypeFare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override final;

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a PricingUnit.
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override final;

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a PricingUnit.
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const Itin& itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage);

  void printDataUnavailableFailed(Diag306Collector* diag,
                                  Record3ReturnTypes retVal,
                                  const MinStayRestriction* minStayRule);

private:
  Record3ReturnTypes validateEsv(ShoppingTrx& trx,
                                 const PaxTypeFare& paxTypeFare,
                                 const RuleItemInfo* rule,
                                 const FareMarket& fareMarket);

  static const std::string NO_MINIMUM_STAY;

protected:
  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a FareMarket for
   *                Fare Display
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              FareDisplayTrx* fareDisplayTrx,
                              const FareMarket& fareMarket,
                              Itin& itin,
                              const PaxTypeFare& paxTypeFare,
                              const RuleItemInfo* rule);
  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a PricingUnit.
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath* farePath,
                              const Itin& itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage);

  bool softpassFareComponentPhase(PricingTrx& trx,
                                  const Itin& itin,
                                  const FareMarket& fm,
                                  const PaxTypeFare& ptf,
                                  Diag306Collector* diag) const;

  /**
   *   @method validateOriginDOW
   *
   *   Description:  Description: Validates the origin day of week to see if the
   *                 the minimum stay is conditional upon the origin
   *                 day of week.
   */
  Record3ReturnTypes validateOriginDOW(const MinStayRestriction* minStayRule,
                                       const uint32_t& travelDOW,
                                       uint32_t& ruleDOW) const;

  Record3ReturnTypes validateOriginDOW(const MinStayRestriction* minStayRule,
                                       DateTime& travelStartDate,
                                       DateTime& travelEndDate,
                                       uint32_t& ruleDOW) const;
  /**
   *   @method validateReturnDate
   *
   *   Description: Validates the return date using the date selected from the determineReturnDate
   *                method.
   */
  Record3ReturnTypes
  validateReturnDate(const DateTime& travelSepDepDate, const DateTime& earliestReturnDate) const;

  /**
   *   @method validateReturnDate
   *
   *   Description: Validates the return date using the date selected from the
   *                determineReturnDate method for Fare Display
   */
  Record3ReturnTypes validateReturnDateFareDisplay(const DateTime& travelSepDepDate,
                                                   const DateTime& earliestReturnDate) const;

  DateTime determineEarliestReturnDate(const DateTime minStayPeriodReturnDate,
                                       const MinStayRestriction* minStayRule,
                                       const PeriodOfStay& minStayPeriod,
                                       Diag306Collector* diag = nullptr);

  DateTime determineEarliestReturnDateWithTime(const DateTime& earliestReturnDate,
                                               const int tod,
                                               const std::string& minStayUnit) const;

  /**
   *   @method getGeoTravelSegs
   *
   *   Description: Retrieves the Geo Travel segments from the Table 995
   *                or if there are none that apply returns the travel
   *                segments in the Pricing Unit.
   *
   */
  Record3ReturnTypes getGeoTravelSegs(RuleUtil::TravelSegWrapperVector& travelSegs,
                                      const int& geoItemTblNo,
                                      PricingTrx& trx,
                                      const VendorCode& vendor,
                                      const FareUsage* fareUsage,
                                      const FarePath* farePath,
                                      const Itin& itin,
                                      const PricingUnit& pricingUnit,
                                      const bool origCheck,
                                      const bool destCheck);
  int16_t getTsiTo() { return _tsiToGeo; }

  bool nonFlexFareValidationNeeded(PricingTrx& trx) const;

  int16_t _tsiToGeo = 0;
};

} // namespace tse
