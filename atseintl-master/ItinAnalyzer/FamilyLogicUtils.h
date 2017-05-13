//----------------------------------------------------------------------------
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

#include "Common/TseCodeTypes.h"

#include <algorithm>
#include <map>
#include <vector>

namespace tse
{
class Cabin;
class ClassOfService;
class Itin;
class ItinAnalyzerService;
class PricingTrx;
class RexExchangeTrx;
class TravelSeg;

class FamilyLogicUtils
{

public:

  static void setFareFamiliesIds(const PricingTrx& trx);
  static void setInfoForSimilarItins(PricingTrx& trx, ItinAnalyzerService* iis);
  static void splitItinFamilies(PricingTrx& trx,
                                bool splitItinsForDomesticOnly,
                                uint16_t requestedNumberOfSeats);

  static void sumUpChildAvailabilityForMotherItins(PricingTrx& trx,
                                                   uint16_t requestedNumberOfSeats);

  static bool diag982DDAllActive(PricingTrx& trx);

  static void printFamily(Itin* itin, std::stringstream& diag);

  static void displayDiag982String(PricingTrx& trx, const char* s);

  static bool
  checkExcItinFamily(RexExchangeTrx* excTrx, Itin* itin, std::vector<Itin*>& splitItins);

  static int segmentOrderWithoutArunk(const Itin* itin, const TravelSeg* segment);

  static const Cabin* getCabin(PricingTrx& trx, const Itin* itin, ClassOfService* classOfService);

  static void fillSimilarItinData(PricingTrx&, Itin& itin, ItinAnalyzerService* ias);

private:
  static uint16_t _requestedNumberOfSeats;
};
}

