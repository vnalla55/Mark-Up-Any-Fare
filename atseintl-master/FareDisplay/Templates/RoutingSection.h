//-------------------------------------------------------------------
//
//  File:        RoutingSection.h
//  Description: Builds the Routing section for a FareDisplay response
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Templates/Section.h"

namespace tse
{
class FareDisplayTrx;

class RoutingSection : public Section
{
public:
  RoutingSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

private:
  typedef std::vector<RoutingRestriction*> RoutingRestrictions;
  typedef RoutingRestrictions::const_iterator RoutingRestrictionsConstIter;
  typedef std::vector<std::string> RoutingMapStrings;
  typedef std::vector<const Routing*> Routings;
  typedef Routings::const_iterator RoutingsConstIter;

  bool buildFDDisplay(tse::FareDisplayTrx& trx);
  bool buildADDisplay(tse::FareDisplayTrx& trx);
  bool addMPMvsRTG(FareDisplayTrx&, const RoutingNumber&, const RoutingInfo*, bool) const;

  void displayConstrVsPubl(FareDisplayTrx&, const PaxTypeFare&) const;
  bool isMileageRestriction(RoutingInfo*) const;
  bool displayAnyRoutingForFQ(FareDisplayTrx&,
                              const std::string& rtg,
                              const std::string& seq,
                              const GlobalDirection& glb);
};
}
