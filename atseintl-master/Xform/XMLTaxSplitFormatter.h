// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/XMLConstruct.h"
#include <boost/core/noncopyable.hpp>

namespace tse
{
class AbstractTaxSplitData;
class AbstractTaxSummaryInfo;

class XMLTaxSplitFormatter : private boost::noncopyable
{
  XMLConstruct& _construct;
  PricingTrx& _pricingTrx;

public:
  XMLTaxSplitFormatter(XMLConstruct& construct, PricingTrx& pricingTrx)
      : _construct(construct), _pricingTrx(pricingTrx) {}

  void
  formatTAX(const AbstractTaxSummaryInfo& taxSplitData);

  void
  formatTBD(const AbstractTaxSplitData& taxSplitData);
};

}
