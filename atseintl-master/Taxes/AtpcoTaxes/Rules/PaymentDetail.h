// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Rules/ItineraryPaymentDetail.h"

namespace tax
{
class BusinessRule;
struct LimitGroup;

class PaymentDetail : public ItineraryPaymentDetail
{
public:
  PaymentDetail(const PaymentRuleData& paymentRuleData,
                const Geo& taxPointBegin,
                const Geo& taxPointEnd,
                const TaxName& taxName,
                const type::CarrierCode& marketingCarrier = BLANK_CARRIER)
  : ItineraryPaymentDetail(paymentRuleData,
                         taxPointBegin,
                         taxPointEnd,
                         taxName,
                         marketingCarrier)
  {
  }

  const bool& isValidated() const { return _validated; }
  void setValidated() { _validated = true; }

  const bool& isCalculated() const { return _calculated; }
  void setCalculated() { _calculated = true; }

  const bool& isExempt() const { return _exempt; }
  void setExempt() { _exempt = true; }

  const bool& isVatTax() const { return _vatTax; }
  void setIsVatTax() { _vatTax = true; }

  bool isCommandExempt() const { return _commandExempt != type::CalcRestriction::Blank; }
  const type::CalcRestriction& getCommandExempt() const { return _commandExempt; }
  void setCommandExempt(type::CalcRestriction exemptType);

  void setLimitGroup(const LimitGroup* limitGroup) { _limitGroup = limitGroup; }
  const LimitGroup* getLimitGroup() const { return _limitGroup; }

  type::TaxApplicationLimit getLimitType() const;
  const BusinessRule* getLimitRule() const;

  const bool& isSkipExempt() const { return _isSkipExempt; }
  void setSkipExempt(bool skipExempt) { _isSkipExempt = skipExempt; }

private:
  bool _validated{false};
  bool _calculated{false};
  bool _exempt{false};
  bool _vatTax{false};
  type::CalcRestriction _commandExempt{type::CalcRestriction::Blank};
  const LimitGroup* _limitGroup{nullptr};
  bool _isSkipExempt{false};
};
}

