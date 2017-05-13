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

#include "Diagnostic/Diag807Collector.h"

namespace tse
{
void
Diag807Collector::initInfoStrings()
{
  _infoStrings[HEADER] = "*    ATSE PFC AND TAX EXEMPTION PROCESS DIAGNOSTIC 807     *\n";
  _infoStrings[EXEMPTION_MODIFIERS_USED] =
      "PFC/TAX EXEMPT NOT APPL WITH TAX EXEMPT MODIFIERS TE/TN/TX\n";
}

void
Diag807Collector::printHeader()
{
  if (_active)
  {
    *this << "************************************************************\n"
          << _infoStrings[HEADER]
          << "************************************************************\n";
  }
}

void
Diag807Collector::printInfo(const Diag807Collector::Diag807Option option)
{
  if (_active)
  {
    *this << _infoStrings[option];
  }
}

Diag807Collector&
Diag807Collector::operator<<(const PricingTrx& x)
{
  return *this;
}

Diag807Collector&
Diag807Collector::operator<<(const TaxResponse& x)
{
  return *this;
}

} // namespace tse
