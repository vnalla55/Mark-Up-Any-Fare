//-------------------------------------------------------------------
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/RexBaseRequest.h"

namespace tse
{

class RexPricingRequest : public RexBaseRequest
{
public:
  void setNewAndExcAccCodeCorpId();

  const std::vector<std::string>& newAndExcAccCode() const { return _newAndExcAccCode; }
  const std::vector<std::string>& newAndExcCorpId() const { return _newAndExcCorpId; }

private:
  std::vector<std::string> _newAndExcAccCode;
  std::vector<std::string> _newAndExcCorpId;
};
} // tse namespace

