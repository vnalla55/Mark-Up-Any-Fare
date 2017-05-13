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

#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/RequestResponse/OutputItin.h"
#include "DataModel/RequestResponse/OutputTaxDetails.h"
#include "DataModel/RequestResponse/OutputTaxGroup.h"

#include <memory>

namespace tax
{

class OutputItins
{
public:
  const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetailsSeq() const
  {
    return _taxDetailsSeq;
  }
  std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetailsSeq() { return _taxDetailsSeq; }

  const std::vector<std::shared_ptr<OutputTaxGroup>>& taxGroupSeq() const { return _taxGroupSeq; }
  std::vector<std::shared_ptr<OutputTaxGroup>>& taxGroupSeq() { return _taxGroupSeq; }

  const boost::ptr_vector<OutputItin>& itinSeq() const { return _itinSeq; }
  boost::ptr_vector<OutputItin>& itinSeq() { return _itinSeq; }

  const type::CurrencyCode& paymentCur() const { return _paymentCur; }
  type::CurrencyCode& paymentCur() { return _paymentCur; }

  const type::CurDecimals& paymentCurNoDec() const { return _paymentCurNoDec; }
  type::CurDecimals& paymentCurNoDec() { return _paymentCurNoDec; }

private:
  std::vector<std::shared_ptr<OutputTaxDetails>> _taxDetailsSeq;
  std::vector<std::shared_ptr<OutputTaxGroup>> _taxGroupSeq;
  boost::ptr_vector<OutputItin> _itinSeq;
  type::CurrencyCode _paymentCur;
  type::CurDecimals _paymentCurNoDec;
};
} // namespace tax
