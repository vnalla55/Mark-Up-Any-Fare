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

#include "Diagnostic/DiagCollector.h"

#include <boost/array.hpp>

namespace tse
{
class Diag807Collector : public DiagCollector
{
  friend class Diag807CollectorTest;

public:
  enum Diag807Option
  {
    HEADER,
    EXEMPTION_MODIFIERS_USED,
    LAST_OPTION = EXEMPTION_MODIFIERS_USED
  };

  Diag807Collector() { initInfoStrings(); }

  Diag807Collector& operator<<(const PricingTrx& x) override;
  Diag807Collector& operator<<(const TaxResponse& x) override;
  void printHeader() override;

  void printInfo(const Diag807Collector::Diag807Option option);

private:
  void initInfoStrings();
  boost::array<const char*, LAST_OPTION + 1> _infoStrings;
};

} // namespace tse

