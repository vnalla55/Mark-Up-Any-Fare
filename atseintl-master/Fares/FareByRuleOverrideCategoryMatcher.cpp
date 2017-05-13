#include "Fares/FareByRuleOverrideCategoryMatcher.h"

#include "Common/LocUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

namespace tse
{

const Indicator FareByRuleOverrideCategoryMatcher::BLANK = ' ';
const Indicator FareByRuleOverrideCategoryMatcher::ASTERISK = '*';
const Indicator FareByRuleOverrideCategoryMatcher::HYPHEN = '-';
const std::string FareByRuleOverrideCategoryMatcher::NULL_CATEGORY_NAME = ".*";
const std::size_t FareByRuleOverrideCategoryMatcher::MAX_FARE_CLASS_NAME_SIZE = 8;

FareByRuleOverrideCategoryMatcher::FareByRuleOverrideCategoryMatcher(
    PricingTrx& trx, const FareByRuleItemInfo& fbrItemInfo, const PaxTypeFare& dummyPtFare)
  : _trx(trx),
    _fbrItemInfo(fbrItemInfo),
    _dummyPtFare(dummyPtFare),
    _isMultiAirport(LocUtil::isMultiAirport(dummyPtFare.market1(), &trx) ||
                     LocUtil::isMultiAirport(dummyPtFare.market2(), &trx))
{
}

Rec2Wrapper*
FareByRuleOverrideCategoryMatcher::tryMatchRec2(const GeneralFareRuleInfo& rec2)
{
  if (UNLIKELY(rec2.inhibit() == RuleConst::INHIBIT_IGNORE))
    return nullptr;

  // match OWRT
  if (UNLIKELY(_fbrItemInfo.resultowrt() != BLANK &&
      !RuleUtil::matchOneWayRoundTrip(rec2.owrt(), _fbrItemInfo.resultowrt())))
    return nullptr;

  // match FARE TYPE
  if (!_fbrItemInfo.resultFareType1().empty() &&
      !RuleUtil::matchFareType(rec2.fareType(), _fbrItemInfo.resultFareType1()))
    return nullptr;

  // match SEASON
  if (UNLIKELY(_fbrItemInfo.resultseasonType() != BLANK &&
      !RuleUtil::matchSeasons(rec2.seasonType(), _fbrItemInfo.resultseasonType())))
    return nullptr;

  // match DOW
  if (UNLIKELY(_fbrItemInfo.resultdowType() != BLANK &&
      !RuleUtil::matchDayOfWeek(rec2.dowType(), _fbrItemInfo.resultdowType())))
    return nullptr;

  // match LOC
  bool isLocationSwapped = false;

  if (!_isMultiAirport ||
      (rec2.loc1().locType() != LOCTYPE_CITY && rec2.loc1().locType() != LOCTYPE_AIRPORT &&
       rec2.loc2().locType() != LOCTYPE_CITY && rec2.loc2().locType() != LOCTYPE_AIRPORT))
  {
    if (!RuleUtil::matchLoc_R1_2_6(
            _trx, rec2.loc1(), rec2.loc2(), _dummyPtFare, isLocationSwapped))
      return nullptr;
  }

  if (!_fbrItemInfo.resultRouting().empty() &&
      !RuleUtil::matchFareRouteNumber(
          rec2.routingAppl(), rec2.routing(), _fbrItemInfo.resultRouting()))
    return nullptr;

  // match FARE CLASS
  if (!matchFareClass(_fbrItemInfo.resultFareClass1(), rec2.fareClass()))
    return nullptr;

  return createRec2Wrapper(rec2, isLocationSwapped);
}

/*      all possible combinations with case number
 *      referenced in comments below
 *
 *         |             resultFareClass
 *---------------------------------------------------
 * rec2    |  *XXX  | -XXX  |   X-  | XXXX  | blank
 * --------------------------------------------------
 * -XXX    |   1    |   2   |   3   |   4   |   5
 * --------------------------------------------------
 * X-XX    |   6    |   7   |   8   |   9   |   10
 * --------------------------------------------------
 * XXXX    |   11   |   12  |   13  |   14  |   15
 * --------------------------------------------------
 * blank   |   16   |   17  |   18  |   19  |   20
 *
 */
bool
FareByRuleOverrideCategoryMatcher::matchFareClass(const FareClassCode& fbrResultFareClassName,
                                                  const FareClassCode& rec2FareClassName) const
{
  if (fbrResultFareClassName.empty() || rec2FareClassName.empty() ||
      rec2FareClassName == NULL_CATEGORY_NAME)
    return true; // cases 5, 10, 15, 16, 17, 18, 19, 20

  std::size_t fbrFclAsteriskPos = fbrResultFareClassName.find(ASTERISK);
  std::size_t fbrFclHyphenPos = fbrResultFareClassName.find(HYPHEN);
  std::size_t rec2HyphenPos = rec2FareClassName.find(HYPHEN);

  if (rec2HyphenPos == 0)
  {
    if ((fbrFclAsteriskPos == 0) || // case 1
        (fbrFclAsteriskPos == std::string::npos && fbrFclHyphenPos == std::string::npos)) // case 4
      return (fbrResultFareClassName.find(rec2FareClassName.substr(1), 1) != std::string::npos);
    if (LIKELY((fbrFclHyphenPos == 0) || // case 2
        (fbrFclHyphenPos == 1))) // case 3
      return true;
  }
  else if (rec2HyphenPos != std::string::npos)
  {
    if (fbrFclAsteriskPos == 0) // case 6
    {
      std::string firstPart = rec2FareClassName.substr(1, rec2HyphenPos - 1).c_str();
      FareClassCode secondPart = rec2FareClassName.substr(rec2HyphenPos + 1).c_str();
      if (!firstPart.empty())
      {
        std::size_t firstPartPos = fbrResultFareClassName.find(firstPart.c_str(), 1);
        if (firstPartPos != std::string::npos)
          return fbrResultFareClassName.find(secondPart.c_str(), firstPartPos + firstPart.size()) !=
                 std::string::npos;

        return false;
      }
      return fbrResultFareClassName.find(secondPart, 1) != std::string::npos;
    }
    if (fbrFclHyphenPos == 0) // case 7
      return true;
    if (UNLIKELY(fbrFclHyphenPos == 1)) // case 8
      return fbrResultFareClassName[0] == rec2FareClassName[0];
    if (LIKELY(fbrFclAsteriskPos == std::string::npos && fbrFclHyphenPos == std::string::npos)) // case 9
    {
      std::string firstPart = rec2FareClassName.substr(0, rec2HyphenPos).c_str();
      std::string secondPart = rec2FareClassName.substr(rec2HyphenPos + 1).c_str();
      if (LIKELY(!firstPart.empty()))
      {
        std::size_t firstPartPos = fbrResultFareClassName.find(firstPart.c_str());
        if (firstPartPos == 0)
          return fbrResultFareClassName.find(secondPart.c_str(), firstPartPos + firstPart.size()) !=
                 std::string::npos;

        return false;
      }
      return false;
    }
  }
  else // rec2HyphenPos == std::string::npos
  {
    if (fbrFclAsteriskPos == 0) // case 11
      return (fbrResultFareClassName.find(rec2FareClassName.substr(1), 1) != std::string::npos) &&
             fbrResultFareClassName.size() == rec2FareClassName.size();
    if (fbrFclHyphenPos == 0) // case 12
    {
      return rec2FareClassName.size() == MAX_FARE_CLASS_NAME_SIZE ||
             ((fbrResultFareClassName.size() <= rec2FareClassName.size()) &&
              rec2FareClassName.substr(rec2FareClassName.size() - fbrResultFareClassName.size() +
                                       1) == fbrResultFareClassName.substr(1));
    }
    if (UNLIKELY(fbrFclHyphenPos == 1)) // case 13
      return rec2FareClassName[0] == fbrResultFareClassName[0];
    if (LIKELY(fbrFclAsteriskPos == std::string::npos && fbrFclHyphenPos == std::string::npos)) // case 14
      return fbrResultFareClassName == rec2FareClassName;
  }

  return true;
}

bool
FareByRuleOverrideCategoryMatcher::isFieldConditional(const Indicator fbrField,
                                                      const Indicator rec2Field) const
{
  if (UNLIKELY(fbrField == BLANK && rec2Field != BLANK))
    return true;

  return false;
}

bool
FareByRuleOverrideCategoryMatcher::isFieldConditional(const FareType& fbrField,
                                                      const FareType& rec2Field) const
{
  if (fbrField.empty() && !rec2Field.empty())
    return true;

  return false;
}

bool
FareByRuleOverrideCategoryMatcher::isFieldConditional(const RoutingNumber& fbrField,
                                                      const RoutingNumber& rec2Field) const
{
  if (fbrField.empty() && !rec2Field.empty())
    return true;

  return false;
}

bool
FareByRuleOverrideCategoryMatcher::isFareClassNameConditional(
    const FareClassCode& fbrFareClassName, const FareClassCode& rec2FareClassName) const
{
  if (rec2FareClassName.empty() || rec2FareClassName == NULL_CATEGORY_NAME)
    return false;

  if (fbrFareClassName.empty())
    return true;

  std::size_t fbrFclAsteriskPos = fbrFareClassName.find(ASTERISK);
  std::size_t fbrFclHyphenPos = fbrFareClassName.find(HYPHEN);

  if (fbrFclAsteriskPos == std::string::npos && fbrFclHyphenPos == std::string::npos)
    return false;

  std::size_t rec2HyphenPos = rec2FareClassName.find(HYPHEN);

  if (rec2HyphenPos == std::string::npos)
    return true;

  if (fbrFareClassName[0] == ASTERISK)
  {
    if (rec2FareClassName[0] == HYPHEN && fbrFareClassName.size() >= rec2FareClassName.size())
      return false;

    return true;
  }

  if (UNLIKELY(fbrFareClassName[fbrFareClassName.size() - 1] == ASTERISK ||
               fbrFareClassName[fbrFareClassName.size() - 1] == HYPHEN))
  {
    if (rec2FareClassName[rec2FareClassName.size() - 1] == HYPHEN &&
        fbrFareClassName.size() >= rec2FareClassName.size())
      return false;

    return true;
  }

  return true;
}

bool
FareByRuleOverrideCategoryMatcher::isRec2Conditional(const GeneralFareRuleInfo& rec2) const
{
  return ((_isMultiAirport &&
           (rec2.loc1().locType() == LOCTYPE_CITY || rec2.loc1().locType() == LOCTYPE_AIRPORT ||
            rec2.loc2().locType() == LOCTYPE_CITY || rec2.loc2().locType() == LOCTYPE_AIRPORT)) ||
          !rec2.footNote1().empty() || !rec2.footNote2().empty() ||
          isFieldConditional(_fbrItemInfo.resultowrt(), rec2.owrt()) ||
          isFieldConditional(_fbrItemInfo.resultFareType1(), rec2.fareType()) ||
          isFieldConditional(_fbrItemInfo.resultseasonType(), rec2.seasonType()) ||
          isFieldConditional(_fbrItemInfo.resultdowType(), rec2.dowType()) ||
          isFieldConditional(_fbrItemInfo.resultRouting(), rec2.routing()) ||
          isFareClassNameConditional(_fbrItemInfo.resultFareClass1(), rec2.fareClass()));
}

Rec2Wrapper*
FareByRuleOverrideCategoryMatcher::createRec2Wrapper(const GeneralFareRuleInfo& rec2,
                                                     const bool isLocationSwapped)
{
  const GeneralFareRuleInfo* const rec2Ptr(&rec2);
  bool isConditional = isRec2Conditional(rec2);

  return &_trx.dataHandle().safe_create<Rec2Wrapper>(rec2Ptr, isLocationSwapped, isConditional);
}
}
