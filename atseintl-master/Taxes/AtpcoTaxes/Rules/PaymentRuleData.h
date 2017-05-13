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

#include "Common/RulesRecordUtil.h"
#include "Common/TaxableUnitTagSet.h"
#include "Common/Timestamp.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/RulesRecord.h"

namespace tax
{
struct PaymentRuleData
{
  PaymentRuleData(const type::SeqNo seqNo,
                  const type::TicketedPointTag& ticketedPointTag,
                  const TaxableUnitTagSet taxableUnits,
                  const type::MoneyAmount& publishedAmount,
                  const type::CurrencyCode& publishedCurrency,
                  const type::TaxAppliesToTagInd& taxAppliesToTagInd)
    : _seqNo(seqNo),
      _ticketedPointTag(ticketedPointTag),
      _taxableUnits(taxableUnits),
      _publishedAmount {publishedAmount, publishedCurrency},
      _taxAppliesToTagInd(taxAppliesToTagInd) {}

  PaymentRuleData(const RulesRecord& rulesRecord,
                  type::ProcessingGroup processingGroup)
    : _discDate(rulesRecord.discDate),
      _effDate(rulesRecord.effDate),
      _expireDate(rulesRecord.expiredDate),
      _histSaleDiscDate(rulesRecord.histSaleDiscDate),
      _histSaleEffDate(rulesRecord.histSaleEffDate),
      _seqNo(rulesRecord.seqNo),
      _ticketedPointTag(rulesRecord.ticketedPointTag),
      _taxableUnits(RulesRecordUtil::narrowDownTuts(
          rulesRecord.applicableTaxableUnits, processingGroup)),
      _publishedAmount {RulesRecordUtil::getTaxAmt(rulesRecord),
                        rulesRecord.taxCurrency},
      _taxAppliesToTagInd(rulesRecord.taxAppliesToTagInd) {}


  type::Date _discDate;
  type::Date _effDate;
  type::Timestamp _expireDate;
  type::Date _histSaleDiscDate;
  type::Date _histSaleEffDate;
  type::SeqNo _seqNo;
  type::TicketedPointTag _ticketedPointTag;
  TaxableUnitTagSet _taxableUnits;
  type::Money _publishedAmount;
  type::TaxAppliesToTagInd _taxAppliesToTagInd;
};
}

