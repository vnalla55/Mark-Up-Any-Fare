#include "Common/FilterPredicates.h"

#include "Rules/BusinessRulesContainer.h"
#include "Rules/PaymentRuleData.h"

namespace tax
{
namespace
{
inline type::Date getValidEffDate(const type::Date& effDate,
                                  const type::Date& histSaleEffDate)
{
  if (!histSaleEffDate.is_blank_date() &&
      !effDate.is_blank_date() &&
      histSaleEffDate < effDate)
  {
    return histSaleEffDate;
  }

  return effDate;
}

inline type::Date getValidDiscDate(const type::Date& discDate,
                                   const type::Date& histSaleDiscDate)
{
  bool isCurrDateBlank = discDate.is_blank_date();
  bool isHistDateBlank = histSaleDiscDate.is_blank_date();

  if (isCurrDateBlank && isHistDateBlank)
    return type::Date::pos_infinity();

  if (isCurrDateBlank)
    return histSaleDiscDate;

  if (isHistDateBlank)
    return discDate;

  if (histSaleDiscDate < discDate)
    return histSaleDiscDate;


  return discDate;
}

inline bool validateByDate(const type::Date& discDate,
                           const type::Date& effDate,
                           const type::Timestamp& expireDate,
                           const type::Date& histSaleDiscDate,
                           const type::Date& histSaleEffDate,
                           const type::Timestamp& ticketingDate)
{
  if (getValidEffDate(effDate, histSaleEffDate) > ticketingDate.date())
    return false;
  if (ticketingDate.date() > getValidDiscDate(discDate, histSaleDiscDate))
    return false;
  if (ticketingDate > expireDate)
    return false;

  return true;
}
} // anonymous namespace



bool validBusinessRuleDatePredicate(const std::shared_ptr<tax::BusinessRulesContainer>& rule,
                                    type::Timestamp ticketingDate)
{
  const PaymentRuleData& ruleData = rule->getPaymentRuleData();
  return validateByDate(ruleData._discDate,
                        ruleData._effDate,
                        ruleData._expireDate,
                        ruleData._histSaleDiscDate,
                        ruleData._histSaleEffDate,
                        ticketingDate);
}

bool
validRulesRecordDatePredicate(const RulesRecord& rule,
                              type::Timestamp ticketingDate)
{
  return validateByDate(rule.discDate,
                        rule.effDate,
                        rule.expiredDate,
                        rule.histSaleDiscDate,
                        rule.histSaleEffDate,
                        ticketingDate);
}

bool
validTravelDatePredicate(const RulesRecord& rule,
                         const type::Date travelDate)
{
  if (rule.firstTravelDate.is_correct() && travelDate < rule.firstTravelDate)
    return false;

  if (rule.lastTravelDate.is_correct() && travelDate > rule.lastTravelDate)
    return false;

  return true;
}

bool
validDatePredicate(const type::Date& effectiveDate,
                   const type::Date& discDate,
                   const type::Timestamp& expireDate,
                   const type::Timestamp& ticketingDate)
{
  if (effectiveDate > ticketingDate.date())
    return false;
  if (ticketingDate.date() > discDate)
    return false;
  if (ticketingDate > expireDate)
    return false;

  return true;
}

} // namespace tax
