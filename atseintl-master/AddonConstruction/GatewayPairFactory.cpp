//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "AddonConstruction/GatewayPairFactory.h"

#include "AddonConstruction/AtpcoGatewayPair.h"
#include "AddonConstruction/SitaGatewayPair.h"
#include "AddonConstruction/SmfGatewayPair.h"

namespace tse
{
GatewayPair*
GatewayPairFactory::create(eGatewayPairType type)
{
  GatewayPair* pair(nullptr);

  switch (type)
  {
  case eAtpcoGatewayPair:
    pair = new AtpcoGatewayPair;
    break;
  case eSitaGatewayPair:
    pair = new SitaGatewayPair;
    break;
  case eSmfGatewayPair:
    pair = new SmfGatewayPair;
    break;
  default:
    throw std::out_of_range("eGatewayPair type invalid.");
    break;
  }

  return pair;
}
}
