//----------------------------------------------------------------------------
//  File:        Diag956Collector.h
//  Created:     2008-02-14
//
//  Description: Diagnostic 956: Pricing ESVPQ display
//
//  Updates:
//
//  Copyright Sabre 2008
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
#include "Common/TseStringTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"

#include <string>
#include <vector>

namespace tse
{
class ESVPQ;
class ESVPQItem;
class Itin;

class Diag956Collector : public DiagCollector
{
public:
  Diag956Collector& displayAmounts(ESVPQItem* pqItem);
  Diag956Collector& displayESVPQ(ESVPQ* esvpq);
  Diag956Collector& displayAddInfo(std::string addInfo);
  Diag956Collector& displayESVPQItem(ESVPQItem* pqItem, std::string, bool, bool);
  Diag956Collector&
  displayDiagLegsContent(const ESVPQ* esvpq,
                         std::vector<std::vector<ShoppingTrx::SchedulingOption*> >& legVec);
  void
  displayDiagLegsContent(const std::vector<std::vector<ShoppingTrx::SchedulingOption*> >& legVec);
  void displayDiagSOPVec(const std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                         int legIndex,
                         bool fuulDisp = true);
  bool supportedPQType(CarrierCode carrier, std::string diversityOption);

private:
  void displayHeader(const ESVPQ*);
  void displayFarePathInfo(const ESVPQItem* pqItem, const int ledIdx);
  void displayCombinationType(const SOPFarePath::CombinationType ct);
  void displayFBC(const PaxTypeFare& paxTypeFare);
  void displayPQInfo(const ESVPQ* pq);
  void displayCarriers(const Itin* itin);
};
}

