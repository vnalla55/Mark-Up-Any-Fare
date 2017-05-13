//----------------------------------------------------------------------------
//
//  File: FDMaxStayApplication.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Rules/MaximumStayApplication.h"

namespace tse
{
class FareDisplayInfo;

class FDMaxStayApplication : public MaximumStayApplication
{
public:
  // ----------------------------------------------------------------
  //   @method validate
  //
  //   Description: Performs rule validations on a FareMarket.
  //
  //   @param PricingTrx           - Pricing transaction
  //   @param Itin                 - itinerary
  //   @param PaxTypeFare          - reference to Pax Type Fare
  //   @param RuleItemInfo         - Record 2 Rule Item Segment Info
  //   @param FareMarket          -  Fare Market
  //
  //   @return Record3ReturnTypes - possible values are:
  //                                NOT_PROCESSED
  //                                FAIL
  //                                PASS
  //                                SKIP
  //                                STOP
  //
  // ----------------------------------------------------------------
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& paxTypeFare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket,
                                      bool isQualifiedCategory);

  // ----------------------------------------------------------------
  //	@method ~FDMaxStayApplication
  //  Desctiption: Performs all clean up activities.
  // ----------------------------------------------------------------
  virtual ~FDMaxStayApplication() {}

private:
  void fillInfoObject(FareDisplayInfo* fareDisplayInfo,
                      const MaxStayRestriction* maxStayRule,
                      const PaxTypeFare& paxTypeFare);

}; // end of class FDMaxStayApplication

}; // end of namespace tse

