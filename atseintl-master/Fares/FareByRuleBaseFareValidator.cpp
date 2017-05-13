#include "Fares/FareByRuleBaseFareValidator.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "Rules/RuleUtil.h"

namespace tse
{
const std::vector<uint16_t> FareByRuleBaseFareValidator::_catListForUniqueFMs =
{
  RuleConst::FLIGHT_APPLICATION_RULE,
  RuleConst::SEASONAL_RULE,
  RuleConst::DAY_TIME_RULE,
  RuleConst::TRAVEL_RESTRICTIONS_RULE,
  RuleConst::BLACKOUTS_RULE
};

FareByRuleBaseFareValidator::FareByRuleBaseFareValidator()
 : FareMarketRuleController(FBRBaseFarePrevalidation, _catListForUniqueFMs)
{
}

void
FareByRuleBaseFareValidator::setCat25R3(const FareByRuleItemInfo* fbrItemInfo)
{
  bool checkFareCategory = false, checkBaseFareCategory = false;

  _categorySequence.clear();

  for (const uint16_t category : _catListForUniqueFMs)
  {
    RuleUtil::determineRuleChecks(category, *fbrItemInfo, checkFareCategory,
                                  checkBaseFareCategory);

    if (checkBaseFareCategory)
      _categorySequence.push_back(category);
  }
  
  RuleUtil::determineRuleChecks(RuleConst::SALE_RESTRICTIONS_RULE, *fbrItemInfo, checkFareCategory,
                                checkBaseFareCategory);
  
  _checkCat15 = checkBaseFareCategory;
}

bool
FareByRuleBaseFareValidator::isFareValidInContext(const PaxTypeFare& paxTypeFare) const
{

  if (_checkCat15 && 
      !paxTypeFare.isCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE) &&
      !paxTypeFare.cat15SecurityFail())
  {
     return false;
  }

  for (const uint16_t category : _categorySequence)
  {
    if (!paxTypeFare.isCategoryValid(category))
      return false; 
  }
  return true;
}

Record3ReturnTypes
FareByRuleBaseFareValidator::getFareStatusInContext(const PaxTypeFare& paxTypeFare) const
{
  for (const uint16_t category : _categorySequence)
  {
    if (!paxTypeFare.isCategoryProcessed(category))
      return NOTPROCESSED; // needs validation

    if (!paxTypeFare.isCategoryValid(category))
      return FAIL;
  }
  return PASS;
}

}
