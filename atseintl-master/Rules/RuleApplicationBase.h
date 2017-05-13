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

#include "Rules/RuleUtil.h"

#include <memory>

namespace tse
{

class PaxTypeFare;
class FareMarket;
class FarePath;
class PricingTrx;
class DateTime;
class TravelSeg;
class PricingUnit;
class FareUsage;
class Itin;
class DiagCollector;
class DateOverrideRuleItem;
class AdditionalValidationResults;
class RuleItemInfo;
class RuleControllerDataAccess;
class Logger;
class RuleValidationChancelor;

/**
 *   @class RuleApplicationBase
 *
 *   Description:
 *   RuleApplicationBase is an Abstract Base Class. Derived Rules application
 *   classes must override the 2 pure virtual validate methods.
 *   For example, SeasonalApplication and Eligibility derive from this
 *   class and override these 2 methods.
 *
 */

class RuleApplicationBase
{
  friend class RuleApplicationBaseTest;

public:
  virtual ~RuleApplicationBase() = default;

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a FareMarket.
   *
   *   @param PricingTrx           - Pricing transaction
   *   @param Itin                 - itinerary
   *   @param PaxTypeFare          - reference to Pax Type Fare
   *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
   *                                 Note: Each derived class must do a dynamic cast
   *                                 to the derived RuleItemInfo class they
   *                                 are interested in. For example:
   *                                 The SeasonalApplication uses a SeasonalAppl rule.
   *                                 Therefore within the validate method of
   *                                 SeasonalApplication a dynamic_cast must be performed:
   *
   *                                 const SeasonalAppl* seasonRule =
   *                                 dynamic_cast<const SeasonalAppl*>(rule);
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
                                      const PaxTypeFare& fare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket) = 0;
  /**
   *   @method validate
   *
   *   Description: Performs rule validations on a PricingUnit.
   *
   *   @param PricingTrx           - Pricing transaction
   *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
   *                                 Note: Each derived class must do a dynamic cast
   *                                 to the derived RuleItemInfo class they
   *                                 are interested in. For example:
   *                                 The SeasonalApplication uses a SeasonalAppl rule.
   *                                 Therefore within the validate method of
   *                                 SeasonalApplication a dynamic_cast must be performed:
   *
   *                                 const SeasonalAppl* seasonRule =
   *                                 dynamic_cast<const SeasonalAppl*>(rule);
   *
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
                                      const FareUsage& fareUsage) = 0;

  /**
   *   @method validateDataUnavailableTag
   *
   *   Description: Checks the data unavailable tag for either X or Y. If
   *                the value is X then FAIL this record. If the value is Y
   *                or text only then SKIP this record.
   *
   *   @param Indicator  - data unavailable tag , values are either 'X' or 'Y'
   *
   *   @return Record3ReturnTypes - possible values are:
   *                                 FAIL          = 2
   *                                 PASS          = 3
   *                                 SKIP          = 4
   */
  virtual Record3ReturnTypes validateUnavailableDataTag(Indicator dataUnavailableTag) const;

  /**
   *   @method removeGeoTravelSegs
   *
   *   Description: Checks whether any of the Geo Travel Segments returned
   *                from validateGeoRuleItem are contained within the FareUsage.
   *                If any of them are they are erased fromt the TravelSegWrapperVector.

   *
   *   @param RuleUtil::TravelSegWrapperVector - applTravelSegment
   *   @param FareUsage                        - current fare usage
   *
   *   @return bool - true - yes, travel segments were erased from the TravelSegWrapperVector,
   *                  else false. User needs to check size of vector after return from this
   *                  method.
   */

  bool removeGeoTravelSegs(RuleUtil::TravelSegWrapperVector& applTravelSegment,
                           const FareUsage& fareUsage,
                           const PricingUnit& pu,
                           const int16_t& tsi,
                           PricingTrx* trx = 0);

  static constexpr char subJourneyBased = 'S';
  static constexpr char sameDay = 'X';
  static constexpr char dataUnavailable = 'X';
  static constexpr char textOnly = 'Y';
  static constexpr char range = 'R';
  static constexpr char negative = 'X';
  static constexpr char nonNegative = ' ';
  static constexpr char mileageGreaterThan = 'G';
  static constexpr char mileageLessThan = 'L';
  static constexpr char mileageGreaterOrLessThan = 'B';

  static const int16_t LOWEND_TIMEOFDAY = 1;
  static const int16_t HIGHEND_TIMEOFDAY = 1440;

  RuleControllerDataAccess* ruleDataAccess() const { return _ruleDataAccess; }
  void setRuleDataAccess(RuleControllerDataAccess* ruleDataAccess)
  {
    _ruleDataAccess = ruleDataAccess;
  }

  void setChancelor(std::shared_ptr<RuleValidationChancelor> chancelor) { _chancelor = chancelor; }
  bool hasChancelor() const { return bool(_chancelor); }

protected:
  /**
   *   @method isValidTOD
   *
   *   Description: check if a TOD (TimeOfDay) field is valid;
   *                1 ~ 2400 should be what we use.
   *                In normal case, caller would ignore a TOD data field if
   *                it is not valid.
   *
   *   @param int16_t  - timeOfDay to be checked
   *
   *   @return bool - possible values are:
   *            true     valid
   *            false    invalid
   */
  static inline bool isValidTOD(int16_t timeOfDay)
  {
    return (timeOfDay < LOWEND_TIMEOFDAY || timeOfDay > HIGHEND_TIMEOFDAY) ? false : true;
  }

  /**
   *   @method isValidATSEDate
   *
   *   Description: check if a DateTime is valid value;
   *                System assumes that 1980-01-01 means invalid date.
   *                In normal case, caller would skip rule validation
   *                for a datetime if it is not valid.
   *
   *   @param const DateTime&    DateTime to be checked
   *
   *   @return bool - possible values are:
   *            true     valid
   *            false    invalid
   */
  bool isValidATSEDate(const DateTime& dt) const
  {
    return (dt.isInfinity() || (dt.year() < MINIMUM_VALID_YEAR)) ? false : true;
  }

  static const uint16_t MINIMUM_VALID_YEAR = 1981; // 1980.1.1 as invalid date

  /**
   *   @method isDomesticUSCAOrTransborder
   *
   *   Description: Check if Itin is domestic US/CA or Transborder.
   *
   *   @param const Itin&    itin
   *   @param const PricingTrx&    trx
   *
   *   @return bool - possible values are:
   *            true     valid
   *            false    invalid
   */
  bool isDomesticUSCAOrTransborder(const tse::Itin& itin, PricingTrx& trx) const;

  bool validateDateOverrideRuleItem(std::vector<Record3ReturnTypes>& r3ReturnTypes,
                                    const std::vector<DateTime>& uniqueTvlDates,
                                    PricingTrx& trx,
                                    const VendorCode& vendorCode,
                                    const uint32_t& overrideDateTblItemNo,
                                    DiagCollector* diag,
                                    const DiagnosticTypes& callerDiag);

  virtual const DateOverrideRuleItem*
  getDateOverrideRuleItem(PricingTrx& trx,
                          const VendorCode& vendorCode,
                          const uint32_t& overrideDateTblItemNo) const;

  static void
  setResultToAllDates(std::vector<Record3ReturnTypes>& r3ReturnTypes, Record3ReturnTypes res);

  static void setResultToAllDates(std::vector<Record3ReturnTypes>& r3ReturnTypes,
                                  std::vector<AdditionalValidationResults*>& additonalValResults,
                                  Record3ReturnTypes res,
                                  AdditionalValidationResults* extraResults);

  bool isValidationNeeded(const uint16_t category, PricingTrx& trx) const;
  void updateStatus(const uint16_t category, const Record3ReturnTypes& result);
  bool shouldReturn(const uint16_t category) const;

private:
  RuleControllerDataAccess* _ruleDataAccess = nullptr;
  static Logger _logger;

protected:
  std::shared_ptr<RuleValidationChancelor> _chancelor;
};

} // namespace tse

