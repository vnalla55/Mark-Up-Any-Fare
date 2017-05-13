//-------------------------------------------------------------------
//
//  File:        FareSectionFilter.h
//  Created:     August 20, 2005
//  Authors:     Mike Carroll
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

#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Group.h"

#include <vector>

namespace tse
{

class FaresSectionFilter
{
  friend class FaresSectionFilterTest;

public:
  FaresSectionFilter() = default;
  FaresSectionFilter(const FaresSectionFilter&) = delete;
  FaresSectionFilter& operator=(const FaresSectionFilter&) = delete;

  void initialize(FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::determineGroupingAndSorting
  //
  // Description: Temporary method to determing what the grouping and sorting
  //              types were applied.
  //
  // @param none
  // @return - void
  //--------------------------------------------------------------------------
  void determineGroupingAndSorting();

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::doFiltering
  //
  // Description: Perform filtering
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @return - true if filtering occurred (a break), false otherwise
  //--------------------------------------------------------------------------
  bool doFiltering(PaxTypeFare& paxTypeFare, bool firstGroup, bool hasMultipleGlobalForYY);
  bool displayYYHdrMsg();
  void displayYYNotPermitted(std::ostringstream& p0, InclusionCode& incCode,
                             std::string& privateString, int fareCount, bool fistcall = true);
  void displayYYNotPublished(std::ostringstream& p0, InclusionCode& incCode,
                             std::string& privateString, int fareCount, bool fistcall = true);

private:
  FareDisplayTrx* _trx = nullptr;
  PaxTypeCode _prevPaxType;
  CarrierCode _prevCarrier;
  BrandCode _prevBrand;
  LocCode _prevOrigDest;
  GlobalDirection _prevGlobalDir = GlobalDirection::NO_DIR;
  bool _isInternational = false;
  Group::GroupType _groupType = Group::GroupType::GROUP_NOT_REQUIRED;
  bool _isMultiTransportHeader = false;
  bool _isBrandHeader = false;
  bool _isS8BrandHeader = false;
  bool _needGlobalAndMPM = false;
  bool _isCabinHeader = false;
  bool _isMultiGlobalForCabin = false;
  bool _isMultiTransportCitiesFogCabin = false;
  bool _brandHeaderAdded = false;
  bool addMultiTransportHeader(const PaxTypeFare& paxTypeFare);
  bool addMultiTransportHeader(const PaxTypeFare& paxTypeFare, bool hasMultipleGlobalForYY,
                               uint8_t inclusionCabinNum, bool singleInclusionCabin);

  std::pair<ProgramCode, BrandCode> _prevS8Brand;
  uint8_t _prevCabin = 0;

  // For Axess
  bool _isAxessGlobalDirHeader = false;
  DateTime _travelDate;
  DateTime _emptyDate;
  DateTime _prevEffDate;
  DateTime _prevDiscDate;

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::addPsgrSectionText
  //
  // Description: Add display information to a passenger grouping
  //
  // @param paxType - a valid PaxTypeCode
  // @param vendor - a valid VendorCode
  // @return - void
  //--------------------------------------------------------------------------
  void addPsgrSectionText(const PaxTypeCode& paxType, const VendorCode& vendor);

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::addGlobalDirSectionText
  //
  // Description: Add display information to a global direction grouping
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @return - void
  //--------------------------------------------------------------------------
  void addGlobalDirSectionText(const PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::addYYFaresText
  //
  // Description: Add display information to a YY fares grouping
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @return - void
  //--------------------------------------------------------------------------
  void addYYFaresText(const PaxTypeFare& paxTypeFare, bool firstGroup, uint8_t inclusionCabinNum);

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::addBrandSectionText
  //
  // Description: Add display information to a brand grouping
  //
  // @param brand code - a brand code
  // @return - void
  //--------------------------------------------------------------------------
  void addBrandSectionText(const BrandCode& brandCode);
  void addS8BrandSectionText(const std::pair<ProgramCode, BrandCode>& newS8Brand);

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::addAxessGlobalDirHeader
  //
  // Description: Add display information to a global direction grouping
  //              for Axess users
  //
  // @param brand code - a valid PaxTypeFare
  // @return - void
  //--------------------------------------------------------------------------
  void addAxessGlobalDirHeader(const PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::checkHeaderType
  //
  // Description: Add display information to a YY fares grouping
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @return - void
  //--------------------------------------------------------------------------
  bool checkHeaderType(Group::GroupType groupType);

  bool isSameMarket(const std::string& currentMarket,
                    const std::string& origDest,
                    const std::string& destOrig);
  bool isGlobalDirectionNeeded();
  void addOriginDestination(const PaxTypeFare& pFare);
  const DateTime getTravelEffDate(const PaxTypeFare& p) const;
  const DateTime& getTravelDiscDate(const PaxTypeFare& p) const;

  //--------------------------------------------------------------------------
  // @function FaresSectionFilter::addCabinSectionText
  //
  // Description: Add display information to a Cabin (Inclusion) grouping
  //
  // @param 
  // @return - void
  //--------------------------------------------------------------------------
  void addCabinSectionText(uint8_t cabinInclusion, bool none=false);
  void addNoneCabinSectionText(uint8_t inclusionCabinNum);
  bool isSingleGlobalOrCityPair(bool multiYY, bool globalDirAdd);
};
} // namespace tse
