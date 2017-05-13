//-------------------------------------------------------------------
//  File:        FareDisplayErrorResponse.h
//  Created:     May 10, 2005
//  Authors:     Abu
//  Updates:
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

#include "Common/TseEnums.h"
#include "Common/TseStringTypes.h"


#include <set>
#include <vector>

namespace tse
{
class FareDisplayTrx;
class FareMarket;
class FareDisplayRequest;

/**
*   @class FareDisplayErrorResponse
*
*   Description:
*   FareDisplayErrorResponse selects the appropriate Strategy to build the FareDisplayErrorResponse.
*
*/
class FareDisplayErrorResponse
{
  friend class FareDisplayErrorResponseTest;

public:
  FareDisplayErrorResponse(FareDisplayTrx& trx);
  virtual ~FareDisplayErrorResponse();
  void process();

private:
  void getGlobalDirErrorResponse(const FareMarket& fm);
  void appendMessage(std::ostringstream& response,
                     std::set<GlobalDirection>& validGlobals,
                     const GlobalDirection& requestedGD);
  static const uint16_t NO_OVERRIDE = 0;
  FareDisplayTrx& _trx;
  const FareDisplayRequest& _request;
  const FareMarket& _fareMarket;
};
}
