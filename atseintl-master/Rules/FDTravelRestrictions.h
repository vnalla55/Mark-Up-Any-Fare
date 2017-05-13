//----------------------------------------------------------------
//
//  File:	       FDTravelRestrictions.h
//
//  Authors:     Marco Cartolano
//  Created:     March 22, 2005
//  Description: FDTravelRestrictions class for Fare Display
//
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------
#pragma once

#include "Rules/TravelRestrictions.h"

namespace tse
{

class FareDisplayInfo;

class FDTravelRestrictions : public TravelRestrictions
{
  friend class FDTravelRestrictionsTest;

public:
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& fare,
                                      const RuleItemInfo* ruleInfo,
                                      const FareMarket& fareMarket,
                                      bool isQualifiedCategory);

private:
  Record3ReturnTypes validateTvlStartDate(const DateTime& refStartDT,
                                          const DateTime& refEndDT,
                                          const TravelRestriction& travelRestrictionInfo,
                                          const bool& needEarliestStartDTCheck,
                                          DiagManager& diag) const;

  Record3ReturnTypes validateTvlStopDate(const DateTime& rtnRefDT,
                                         const TravelRestriction& travelRestrictionInfo,
                                         DiagManager& diag) const;

  void updateFareDisplayInfo(const TravelRestriction& travelRestrictionInfo,
                             FareDisplayInfo*& fdInfo,
                             const DateTime& ticketingDate) const;

  static Logger _logger;
};

} // namespace tse

