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
#include "DataModel/RequestResponse/IoTaxName.h"
#include "DataModel/RequestResponse/OutputTaxDetailsRef.h"

#include <tuple>

namespace tax
{

class OutputTax
{
public:
  const std::vector<OutputTaxDetailsRef>& taxDetailsRef() const { return _taxDetailsRef; }
  std::vector<OutputTaxDetailsRef>& taxDetailsRef() { return _taxDetailsRef; }

  boost::optional<IoTaxName>& taxName() { return _taxName; }
  const boost::optional<IoTaxName>& taxName() const { return _taxName; }

  boost::optional<type::MoneyAmount>& totalAmt() { return _totalAmt; }
  const boost::optional<type::MoneyAmount>& totalAmt() const { return _totalAmt; }

  bool operator==(const OutputTax& other) const
  {
    return ((_taxName == other._taxName) && (_totalAmt == other._totalAmt) &&
            (_taxDetailsRef == other._taxDetailsRef));
  }

private:
  std::vector<OutputTaxDetailsRef> _taxDetailsRef;

  boost::optional<IoTaxName> _taxName;
  boost::optional<type::MoneyAmount> _totalAmt;
};
}
