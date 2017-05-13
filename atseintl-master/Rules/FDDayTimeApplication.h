//----------------------------------------------------------------------------
//  File: FDDayTimeApplication.h
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

#include "DataModel/FareDisplayInfo.h"
#include "Rules/DayTimeApplication.h"

namespace tse
{

class FDDayTimeApplication : public DayTimeApplication
{
  friend class FDDayTimeApplicationTest;

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
  //   @param FareMarket           - Fare Market
  //   @param bool                 - isQualifiedCategory
  //
  //   @return Record3ReturnTypes - possible values are:
  //                               NOT_PROCESSED
  //                                FAIL
  //                                PASS
  //                                SKIP
  //                                STOP
  //
  // ----------------------------------------------------------------

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& paxTypeFare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket,
                              bool isQualifiedCategory,
                              bool isInbound);

  // ----------------------------------------------------------------
  //   @method ~FDDayTimeApplication
  //
  //   Description: Desctructor to performs any cleanup
  // ----------------------------------------------------------------

  virtual ~FDDayTimeApplication() {};

private:

  Record3ReturnTypes
  validateDayOfWeekInRange(DateTime& earliestDepartureDate, DateTime& latestDepartureDate);

  Record3ReturnTypes validateDateDayOfWeek(DateTime& vaDate);

  void diagReturnType(DiagManager& diag, const Record3ReturnTypes& retval);

  bool processOWSingleDateFare(FareDisplayTrx* fareDisplayTrx,
                               const PaxTypeFare& paxTypeFare,
                               const TravelSeg* tvSeg,
                               DiagManager& diag,
                               Record3ReturnTypes& retval);

}; // end of class FDDayTimeApplication

}; // end of namespace tse

