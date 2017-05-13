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
#pragma once

namespace tse
{
class GatewayPair;

enum eGatewayPairType
{ eAtpcoGatewayPair,
  eSitaGatewayPair,
  eSmfGatewayPair };

struct GatewayPairFactory
{
  static GatewayPair* create(eGatewayPairType type);
};
}

