//-------------------------------------------------------------------
//
//  File:        FaresSection.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//  Description: Used for display of fares and related fare information
//
//  Updates:
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

#include "DBAccess/FareDispTemplateSeg.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/FaresHeaderSection.h"
#include "FareDisplay/Templates/FaresSectionFilter.h"
#include "FareDisplay/Templates/Section.h"

#include <vector>

namespace tse
{
class FareDisplayTrx;

class FaresSection : public Section
{
public:
  FaresSection(FareDisplayTrx& trx, std::vector<FareDispTemplateSeg*>* templateSegRecs)
    : Section(trx), _templateSegRecs(templateSegRecs)
  {
  }

  FaresSection(FareDisplayTrx& trx,
               FaresHeaderSection& headerSection,
               FaresSectionFilter& sectionFilter,
               std::vector<FareDispTemplateSeg*>* templateSegRecs)
    : Section(trx),
      _headerSection(&headerSection),
      _sectionFilter(&sectionFilter),
      _templateSegRecs(templateSegRecs)
  {
  }

  void buildDisplay() override;

  // Accessors
  std::vector<ElementField*>& columnFields() { return _columnFields; }
  const std::vector<ElementField*>& columnFields() const { return _columnFields; }

  const std::vector<FareDispTemplateSeg*>& templateSegRecs() const { return *_templateSegRecs; }

private:
  // Column headings
  FaresHeaderSection* _headerSection = nullptr;

  // Associative with Grouping and Sorting
  FaresSectionFilter* _sectionFilter = nullptr;

  std::vector<FareDispTemplateSeg*>* _templateSegRecs = nullptr;

  std::vector<ElementField*> _columnFields;

  Indicator getDateFilter(const FieldColumnElement& fieldElement);

  void displayMoreFaresExistMessage(FareDisplayTrx& trx) const;

  bool hasMultipleGlobal(const CarrierCode& carrierCode);

  bool displayAccountCode(const PaxTypeFare& paxTypeFare) const;

  bool displayRetailerCode(const PaxTypeFare& paxTypeFare) const;

  void addCabinSectionText(uint8_t cabinInclusionNum, bool top=false);

  void allowDisplayCabinHeader();

};

} // namespace tse
