/*
 *  Copyright Sabre 2011
 *
 *  The copyright to the computer program(s) herein
 *  is the property of Sabre.
 *
 *  The program(s) may be used and/or copied only with
 *  the written permission of Sabre or in accordance
 *  with the terms and conditions stipulated in the
 *  agreement/contract under which the program(s)
 *  have been supplied.
 */

#pragma once

#include "DataModel/SOLItinGroups.h"
#include "Xform/XMLShoppingResponse.h"

namespace tse
{

class XMLShoppingSoloCarnivalResponse : public XMLShoppingResponse
{
public:
  explicit XMLShoppingSoloCarnivalResponse(PricingTrx& trx);
  virtual ~XMLShoppingSoloCarnivalResponse() {}

private:
  virtual void generateItineraries() override;
  void generateFlexFareGRI(const SOLItinGroups::ItinGroup& group);
  void generateSubItinFlights(const Itin* itin);
  void generateSumOfLocalITNbody(const Itin* itin, const SOLItinGroups::ItinGroup& group);
};
}

