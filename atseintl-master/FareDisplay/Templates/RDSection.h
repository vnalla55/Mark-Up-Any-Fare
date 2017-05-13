//-------------------------------------------------------------------
//
//  File:        RDSection.h
//  Authors:     Doug Batchelor
//  Created:     May 4, 2005
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize a FB template (the set of labels
//               and fields that appear for a FB diagnostic on the
//               Sabre greenscreen.)
//
//
//  Copyright Sabre 2003
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

#include "FareDisplay/Templates/Section.h"

namespace tse
{

static const int16_t MAX_PAX_ROW_ONE = 5;
static const int16_t MAX_PAX_TOTAL = 15;
static const std::string BLANK_MAX_PAX = "   ";

class RDSection : public Section
{
  friend class RDSection_buildMenuItems;
  friend class RDSectionTest;

public:
  RDSection(FareDisplayTrx& trx);

  void buildDisplay() override;

protected:
  void addRetailerCategoryDetails(const CatNumber& catNumber, PaxTypeFare& paxTypeFare);
  void addIntlConstructionDetails(const CatNumber& catNumber, PaxTypeFare& paxTypeFare);
  void addPassengerTypeLine(PaxTypeFare& paxTypeFare);
  void addOriginDestinationLine(PaxTypeFare& paxTypeFare);
  void addFareBasisLine(PaxTypeFare& paxTypeFare);
  void addFareTypeLine(PaxTypeFare& paxTypeFare);
  void addSITAFareInfoLine(PaxTypeFare& paxTypeFare);
  void addNETFareInfoLine(PaxTypeFare& paxTypeFare);
  void addFootNotesLine(const PaxTypeFare& paxTypeFare);

  void addCatInfo(const CatNumber& catNumber, FareDisplayInfo& fareDisplayInfo, std::ostringstream* oss);
  bool addCatDescription(const CatNumber& catNumber,
                         FareDisplayInfo& fareDisplayInfo,
                         std::ostringstream* oss);

  void addInvalidEntries();

  void buildMenu(FareDisplayInfo& fareDisplayInfo);

  typedef std::pair<CatNumber, const std::string*> MenuItem;
  std::string buildMenuItems(FareDisplayInfo& fareDisplayInfo,
                             bool dutyCode78,
                             const std::vector<MenuItem>& items);
  std::string
  formatMenuItem(FareDisplayInfo& fareDisplayInfo, bool dutyCode78, const MenuItem& item) const;

  DateTime _effDate;
  DateTime _discDate;

private:
  static const std::string _longFNTypeNames[3];
  static const std::string _shortFNTypeNames[3];
  bool _useNewRDHeader = false;

  enum FootNoteType
  { ORIG_ADDON = 0,
    PUBLISHED = 1,
    DEST_ADDON = 2 };

  bool
  buildFootNotesString(const Footnote& footNote1, const Footnote& footNote2, std::string& output);
};
} // namespace tse
