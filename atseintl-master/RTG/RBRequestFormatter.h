//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "BookingCode/RBData.h"
#include "BookingCode/RBDataItem.h"
#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{

class RBRequestFormatter
{
public:
  RBRequestFormatter(std::string& request, RBData& rbData) : _request(request), _rbData(rbData) {}

  void buildRequest(FareDisplayTrx& trx, bool skipDiag854);

private:
  std::string& _request;
  RBData& _rbData;
  bool
  populateXMLTags(const FareDisplayTrx& trx, std::vector<RBDataItem*>& items, bool skipDiag854);
  void addRBKType(XMLConstruct&, RBDataItem&);
};
}
