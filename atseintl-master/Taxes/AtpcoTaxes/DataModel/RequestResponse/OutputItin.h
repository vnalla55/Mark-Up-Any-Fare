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

#include "DataModel/Common/Types.h"
#include <boost/optional.hpp>

namespace tax
{
class OutputItin
{
public:
  OutputItin(const type::Index id) : _id(id) {}

  type::Index& id() { return _id; }
  const type::Index& id() const { return _id; }

  boost::optional<type::Index>& taxGroupId() { return _taxGroupId; }
  const boost::optional<type::Index>& taxGroupId() const { return _taxGroupId; }

  boost::optional<type::Index>& taxOnOptionalServiceGroupRefId()
  {
    return _taxOnOptionalServiceGroupRefId;
  }
  const boost::optional<type::Index>& taxOnOptionalServiceGroupRefId() const
  {
    return _taxOnOptionalServiceGroupRefId;
  }

  boost::optional<type::Index>& taxOnChangeFeeGroupRefId() { return _taxOnChangeFeeGroupRefId; }
  const boost::optional<type::Index>& taxOnChangeFeeGroupRefId() const
  {
    return _taxOnChangeFeeGroupRefId;
  }

  void setTaxOnBaggageGroupRefId(const type::Index& id) { _taxOnBaggageGroupRefId = id; }
  const boost::optional<type::Index>& getTaxOnBaggageGroupRefId() const
  {
    return _taxOnBaggageGroupRefId;
  }

private:
  OutputItin& operator=(const OutputItin&);

  type::Index _id;
  boost::optional<type::Index> _taxGroupId;
  boost::optional<type::Index> _taxOnOptionalServiceGroupRefId;
  boost::optional<type::Index> _taxOnChangeFeeGroupRefId;
  boost::optional<type::Index> _taxOnBaggageGroupRefId;
};
}
