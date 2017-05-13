//-------------------------------------------------------------------
//
//  File:        GroupingPreferenceSection.h
//  Authors:     Abu Islam
//  Created:     July 07, 2005
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Flight Counts that
//               appear on the Sabre greenscreen.)
//
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
//---------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Group.h"
#include "FareDisplay/Templates/Section.h"

#include <string>

namespace tse
{

class FareDisplayTrx;

class GroupingPreferenceSection : public Section
{

public:
  GroupingPreferenceSection(FareDisplayTrx& trx);

  void buildDisplay() override;

private:
  std::map<Group::GroupType, std::string> _groupTextMap;
  void displayUserInfo(Group& group, FareDisplayTrx& trx) const;
  void displayGroupInfo(Group& group, FareDisplayTrx& trx, uint16_t&) const;
  void displayGroupDetails(Group& group) const;
  void displayGlobals(Group& group) const;
  void displayPaxTypes(Group& group) const {}
  void displayBrandGroupingInfo(Group& group, FareDisplayTrx& trx) const;
  void displayBrandServiceGroupingInfo(Group& group, FareDisplayTrx& trx) const;
};
} // namespace tse
