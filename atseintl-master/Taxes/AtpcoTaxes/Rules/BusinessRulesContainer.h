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

#include "Common/TaxableUnitTagSet.h"
#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "Rules/GroupId.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/ProcessingGroupTagSet.h"

#include "Rules/RulesGroups.h"

#include <map>
#include <vector>

namespace tax
{
class RulesRecord;
class BusinessRule;

typedef std::map<GroupId, std::vector<BusinessRule*> > GroupedRules;
typedef std::pair<type::Index, type::RuleName> RuleIdName;
typedef std::vector<RuleIdName> RuleIdNames;

class BusinessRulesContainer
{
public:
  BusinessRulesContainer(RulesRecord const&, type::ProcessingGroup);
  ~BusinessRulesContainer();

  static void createRuleNames(RuleIdNames& ruleNames);

  const TaxName& taxName() const { return _taxName; }

  const PaymentRuleData& getPaymentRuleData() const { return _paymentRuleData; }
  type::SeqNo const& seqNo() const { return _paymentRuleData._seqNo; }

  type::Vendor const& vendor() const { return _vendor; }

  type::TicketedPointTag const& ticketedPointTag() const
  {
    return _paymentRuleData._ticketedPointTag;
  }

  const TaxableUnitTagSet& taxableUnits() const { return _paymentRuleData._taxableUnits; }

  const type::Index& getServiceBaggageItemNo() const { return _serviceBaggageItemNo; }
  const type::ServiceBaggageApplTag& getServiceBaggageApplTag() const { return _serviceBaggageApplTag; }

  bool isValid() const { return _valid; }
  const type::Money& getPublishedAmount() const { return _paymentRuleData._publishedAmount; }

  const ValidatorsGroups& getValidatorsGroups() const { return _validatorsGroups; }
  const LimitGroup& getLimitGroup() const { return _limitGroup; }
  const CalculatorsGroups& getCalculatorsGroups() const { return _calculatorsGroups; }

private:
  void splitRecord(RulesRecord const& rulesRecord);
  TaxName _taxName;
  type::Vendor _vendor;

  PaymentRuleData _paymentRuleData;

  type::Index _serviceBaggageItemNo;
  type::ServiceBaggageApplTag _serviceBaggageApplTag;
  bool _valid;

  ValidatorsGroups _validatorsGroups;
  LimitGroup _limitGroup;
  CalculatorsGroups _calculatorsGroups;

  friend class BusinessRulesProcessorTest;
};
}

