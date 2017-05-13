// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"

#include <vector>

namespace tax
{

struct TaxReissue
{
  type::MoneyAmount amount;
  type::CurrencyCode currency;
  type::CurDecimals currencyDec;
  type::LocExceptTag locExceptTag;
  type::LocCode locCode;
  type::ReissueLocTypeTag locType;
  type::ReissueRefundableTag refundableTag;
  type::SeqNo seqNo;
  type::TaxCode taxCode;
  type::TaxType taxType;
  std::vector<type::CarrierCode> ticketingCarriers;
  type::TicketingExceptTag ticketingExceptTag;
};

} // namespace tax
