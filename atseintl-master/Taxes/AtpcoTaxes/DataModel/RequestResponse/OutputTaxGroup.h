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
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/RequestResponse/OutputTax.h"

namespace tax
{

class OutputTaxGroup
{
public:
  OutputTaxGroup(type::TaxGroupType const& type)
    : _id(std::numeric_limits<type::Index>::max()), _type(type)
  {
  }

  bool operator==(const OutputTaxGroup& r)
  {
    return (_id && r._id) ? (_id == r._id)
                          : (_taxSeq == r._taxSeq && _totalAmt == r._totalAmt && _type == r._type);
  }

  const boost::ptr_vector<OutputTax>& taxSeq() const { return _taxSeq; }
  boost::ptr_vector<OutputTax>& taxSeq() { return _taxSeq; }

  const type::Index& id() const { return _id; }
  type::Index& id() { return _id; }

  const type::TaxGroupType& type() const { return _type; }
  type::TaxGroupType& type() { return _type; }

  const boost::optional<type::MoneyAmount>& totalAmt() const { return _totalAmt; }
  boost::optional<type::MoneyAmount>& totalAmt() { return _totalAmt; }

private:
  OutputTaxGroup& operator=(const OutputTaxGroup&);
  boost::ptr_vector<OutputTax> _taxSeq;
  type::Index _id;
  type::TaxGroupType _type;
  boost::optional<type::MoneyAmount> _totalAmt;
};

} // namespace tax
