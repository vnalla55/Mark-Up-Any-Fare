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

#include "DataModel/Common/Types.h"
#include "DataModel/RequestResponse/BCHOutputItinPaxDetail.h"

namespace tax
{
class BCHOutputItin
{
public:
  BCHOutputItin(const type::Index id) : _id(id) {}

  BCHOutputItin& operator=(const BCHOutputItin&) = delete;

  void setId(type::Index id) { _id = id; }
  const type::Index& getId() const { return _id; }

  BCHOutputItinPaxDetail& mutablePaxDetail() { return _paxDetail; }
  const BCHOutputItinPaxDetail& constPaxDetail() const { return _paxDetail; }

private:
  type::Index _id;
  BCHOutputItinPaxDetail _paxDetail;
};
}
