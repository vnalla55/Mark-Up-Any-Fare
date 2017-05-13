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

#include "DBAccess/MaxStayRestriction.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/SubjectObserved.h"

#include <log4cxx/helpers/objectptr.h>

namespace tse
{

class Fare;
class FareMarket;
class FarePath;
class PricingTrx;
class DateTime;
class TravelSeg;
class PricingUnit;
class MaxStayRestriction;
class PeriodOfStay;
class DiagManager;

template <typename... TypesToNotify>
class IObserver;

class MaximumStayApplication
    : public RuleApplicationBase,
      public SubjectObserved<IObserver<Indicator, const RuleItemInfo*, DateTime, int16_t, LocCode>>
{
  friend class MaximumStayApplicationTest;

public:
  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a FareMarket.
   *
   *   @param PricingTrx           - Pricing transaction
   *   @param Itin                 - itinerary
   *   @param PaxTypeFare          - reference to Pax Type Fare
   *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
   *   @param FareMarket          -  Fare Market
   *
   *   @return Record3ReturnTypes - possible values are:
   *                                NOT_PROCESSED = 1
   *                                FAIL          = 2
   *                                PASS          = 3
   *                                SKIP          = 4
   *                                STOP          = 5
   *
   */
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& paxTypeFare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket) override;

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a FareMarket for Fare Display
   *
   *   @param PricingTrx           - Pricing transaction
   *   @param FareDisplayTrx       - Fare Display Transaction
   *   @param FareMarket           - Fare Market
   *   @param Itin                 - itinerary
   *   @param PaxTypeFare          - reference to Pax Type Fare
   *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
   *
   *   @return Record3ReturnTypes - possible values are:
   *                                NOT_PROCESSED = 1
   *                                FAIL          = 2
   *                                PASS          = 3
   *                                SKIP          = 4
   *                                STOP          = 5
   *
   */
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      FareDisplayTrx* fareDisplayTrx,
                                      const FareMarket& fareMarket,
                                      Itin& itin,
                                      const PaxTypeFare& paxTypeFare,
                                      const RuleItemInfo* rule);

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a PricingUnit.
   *
   *   @param PricingTrx           - Pricing transaction
   *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
   *   @param FarePath             - Fare Path
   *   @param PricingUnit          - Pricing unit
   *   @param FareUsage            - Fare Usage
   *
   *   @return Record3ReturnTypes  - possible values are:
   *                                 NOT_PROCESSED = 1
   *                                 FAIL          = 2
   *                                 PASS          = 3
   *                                 SKIP          = 4
   *                                 STOP          = 5
   */
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      const RuleItemInfo* rule,
                                      const FarePath& farePath,
                                      const PricingUnit& pricingUnit,
                                      const FareUsage& fareUsage) override;

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a PricingUnit.
   *
   *   @param PricingTrx           - Pricing transaction
   *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
   *   @param Itin                 - Itin
   *   @param PricingUnit          - Pricing unit
   *   @param FareUsage            - Fare Usage
   *
   *   @return Record3ReturnTypes  - possible values are:
   *                                 NOT_PROCESSED = 1
   *                                 FAIL          = 2
   *                                 PASS          = 3
   *                                 SKIP          = 4
   *                                 STOP          = 5
   */
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      const RuleItemInfo* rule,
                                      const Itin& itin,
                                      const PricingUnit& pricingUnit,
                                      const FareUsage& fareUsage);

protected:
  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath* farePath,
                              const Itin& itin,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage);

  const MaxStayRestriction* _itemInfo = nullptr;

private:

  bool softpassFareComponentValidation(PricingTrx& trx,
                                       const Itin& itin,
                                       const FareMarket& fm,
                                       const PaxTypeFare& ptf,
                                       DiagManager& dc) const;

  /**
   *   @method formatDiagnostic
   *
   *   Description: Writes the detail information to the diagnostic
   *
   *   @param  DiagCollector       - diagnostic collector
   *   @param  TravelSeg           - travel segment
   *
   *   @return void
   */
  void formatDiagnostic(DiagCollector& diag,
                        const TravelSeg* travelSeg,
                        Record3ReturnTypes retval,
                        const Indicator arrDepInd = RuleConst::RETURN_TRV_COMMENCE_IND);

  /**
   *   @method validateReturnDate
   *
   *   Description: Validates the return date based on the maximum stay period and/or
   *                the maximum stay date.
   *
   *   @param  DateTime            - Travel Seg departure date
   *   @param  DateTime            - calculated earliest date of departure based on maximum
   *                                 stay period rules
   *   @param  int                 - current time of day expressed in maximum
   *
   *   @return Record3ReturnTypes  - possible values are:
   *                                 NOT_PROCESSED = 1
   *                                 FAIL          = 2
   *                                 PASS          = 3
   *                                 SKIP          = 4
   *                                 STOP          = 5
   */
  Record3ReturnTypes
  validateReturnDate(const DateTime& travelSepDepDate, const DateTime& maxStayPeriodReturnDate);

  Record3ReturnTypes getGeoTravelSegs(RuleUtil::TravelSegWrapperVector& travelSegs,
                                      const int& geoTblItemNo,
                                      PricingTrx& trx,
                                      DiagCollector& diag,
                                      const FareUsage& fareUsage,
                                      const FarePath* farePath,
                                      const Itin& itin,
                                      const PricingUnit& pricingUnit,
                                      const bool origCheck,
                                      const bool destCheck);


  int16_t getTsiTo() { return _tsiToGeo; }

  template <typename T>
  void printDiag(T& diag, std::string msg);

  template <typename T>
  Record3ReturnTypes validateMaximumStayRule(const MaxStayRestriction& maxStayRule, T& diag);

  bool nonFlexFareValidationNeeded(PricingTrx& trx) const;

  int16_t _tsiToGeo = 0;
  DateTime determineLatestReturnDateWithTime(const DateTime& latestReturnDate,
                                             const int timeOfDay,
                                             const std::string& maxStayUnit) const;

  DateTime determineLatestReturnDate(const MaxStayRestriction* maxStayRule,
                                     const DateTime& dateToValidate,
                                     DiagManager& diag,
                                     bool showFullDiag = true) const;
};

template <>
inline void
MaximumStayApplication::printDiag<DiagCollector>(DiagCollector& diag, std::string msg)
{
  if (diag.isActive())
  {
    diag << msg << "\n";
    diag.flush();
  }
}

} // namespace tse

