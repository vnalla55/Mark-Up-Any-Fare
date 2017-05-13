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

#include "Factories/MakeSabreCode.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PreviousTicketApplicator.h"
#include "Rules/RawPayments.h"
#include "ServiceInterfaces/PreviousTicketService.h"
#include "ServiceInterfaces/Services.h"
#include "DataModel/Services/PreviousTicketTaxInfo.h"

#include <algorithm>
#include <regex>

namespace tax
{
namespace
{
class MatchSabreTaxCodeAndPercent
{
  const type::Percent& _percent;

  bool
  matchSabreTaxCode(const std::string& parentTax, const std::string& taxFromPreviousTicket) const
  {
    if (parentTax.length() == 3 && parentTax[2] == '*')
    {
      return parentTax[0] == taxFromPreviousTicket[0] && parentTax[1] == taxFromPreviousTicket[1];
    }
    else
    {
      return parentTax == taxFromPreviousTicket;
    }
  }

  bool matchPercent(const type::Percent& left, const type::Percent& right) const
  {
    return left == right;
  }

public:
  MatchSabreTaxCodeAndPercent(const type::Percent& percent) : _percent(percent) {}

  bool
  operator()(const std::string& parentTax, const PreviousTicketTaxInfo& previousTicketTaxInfo) const
  {
    return previousTicketTaxInfo.isCanadaExch() &&
           matchSabreTaxCode(parentTax, previousTicketTaxInfo.getSabreTaxCode()) &&
           matchPercent(_percent, tax::doubleToPercent(previousTicketTaxInfo.getPercentage()));
  }
};
}

std::set<std::string>
PreviousTicketApplicator::getParentTaxes(const TaxName& taxName) const
{
  const std::string& sabreTaxCode =
      makeItinSabreCode(taxName.taxCode(), taxName.taxType(), taxName.percentFlatTag());

  return _services.previousTicketService().getParentTaxes(sabreTaxCode);
}

bool
PreviousTicketApplicator::isInTaxesForPreviousTicket(const std::set<std::string>& parentTaxes) const
{
  const std::set<PreviousTicketTaxInfo>& taxesForPreviousTicket =
      _services.previousTicketService().getTaxesForPreviousTicket();

  MatchSabreTaxCodeAndPercent predicate(_percent);

  return std::find_first_of(parentTaxes.begin(),
                            parentTaxes.end(),
                            taxesForPreviousTicket.begin(),
                            taxesForPreviousTicket.end(),
                            predicate) != parentTaxes.end();
}

bool
PreviousTicketApplicator::apply(PaymentDetail& paymentDetail) const
{
  std::set<std::string> parentTaxes = getParentTaxes(paymentDetail.taxName());

  return parentTaxes.empty() || isInTaxesForPreviousTicket(parentTaxes);
}
}
