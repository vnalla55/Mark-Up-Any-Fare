//-------------------------------------------------------------------
//
//  File:        FareDisplayResponse.h
//  Description: A class to provide a response object for
//               a FareDisplayTrx.
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
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/Response.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "FareDisplay/FDHeaderMsgText.h"
#include "FareDisplay/Group.h"
#include "Routing/RoutingInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/TravelRoute.h"

#include <map>
#include <tuple>

namespace tse
{

class FlightCount;
class FareDisplayInclCd;
class FDAddOnFareInfo;

typedef std::string RtgSeq;

class RtgSeqCmp
{
public:
  typedef std::map<std::string, uint16_t> GlobalOrder;
  typedef GlobalOrder::const_iterator GlobalOrderConstIter;
  typedef std::string GlobalStr;
  RtgSeqCmp();

  bool operator()(const RtgSeq& lhs, const RtgSeq& rhs) const;

private:
  GlobalOrder globalOrder_;
};

typedef std::map<RtgSeq, const FDAddOnFareInfo*, RtgSeqCmp> AddOnFareInfos;
typedef AddOnFareInfos::const_iterator AddOnFareInfosConstIter;
typedef std::map<RtgSeq, RoutingInfo*> RoutingInfoMap;
typedef RoutingInfoMap::const_iterator RoutingInfoMapConstIter;
typedef std::map<RtgSeq, PaxTypeFare*, RtgSeqCmp> RtgSeq2PaxTypeFareMap;
typedef RtgSeq2PaxTypeFareMap::const_iterator RtgSeq2PaxTypeFareConstIter;

typedef std::string BrandName;
typedef std::string BrandText;
// typedef std::pair<BrandName, BrandText> BrandNameTextPair;
// typedef std::map<BrandCode, BrandNameTextPair> BrandNameTextMap;
typedef std::map<BrandCode, std::pair<BrandName, BrandText> > BrandNameTextMap;
// std::map<std::pair<LocCode,LocCode>, uint16_t> _priorityMap;
// typedef std::map<BrandCode, BrandName>  BrandNameMap;
// typedef std::map<BrandCode, BrandText>  BrandTextMap;

// Brander Fares project 2013
typedef std::map<std::pair<ProgramCode, BrandCode>,
                 std::tuple<ProgramName, BrandNameS8, SystemCode> > ProgramBrandNameMap;

// -------------------------------------------------------------------
//
// @class FareDisplayResponse
//
// A class to provide a response object for the FareDisplayTrx.
//
// -------------------------------------------------------------------------
class FareDisplayResponse : public Response
{
public:
  FareDisplayResponse();
  virtual ~FareDisplayResponse();

  const std::vector<FlightCount*>& scheduleCounts() const
  {
    return _scheduleCounts;
  };
  std::vector<FlightCount*>& scheduleCounts()
  {
    return _scheduleCounts;
  };

  // provide access to the _badAlphaCodes vector.
  std::vector<AlphaCode>& badAlphaCodes() { return _badAlphaCodes; }
  const std::vector<AlphaCode>& badAlphaCodes() const { return _badAlphaCodes; }

  // provide access to the _badCategoryNumbers vector.
  std::vector<CatNumber>& badCategoryNumbers() { return _badCategoryNumbers; }
  const std::vector<CatNumber>& badCategoryNumbers() const { return _badCategoryNumbers; }

  // provide access to the _subCategoryNumbers vector.
  std::vector<CatNumber>& subCategoryNumbers() { return _subCategoryNumbers; }
  const std::vector<CatNumber>& subCategoryNumbers() const { return _subCategoryNumbers; }

  std::vector<Group::GroupType>& groupHeaders() { return _groupHeaders; }
  const std::vector<Group::GroupType>& groupHeaders() const { return _groupHeaders; }

  // std::set<CurrencyCode>& validCurrencies() { return _validCurrencySet;}
  // const std::set<CurrencyCode>& validCurrencies() const { return _validCurrencySet;}

  RoutingInfoMap& uniqueRoutingMap() { return _uniqueRoutingMap; }
  const RoutingInfoMap& uniqueRoutingMap() const { return _uniqueRoutingMap; }

  RtgSeq2PaxTypeFareMap& uniqueRoutings() { return _uniqueRoutings; }
  const RtgSeq2PaxTypeFareMap& uniqueRoutings() const { return _uniqueRoutings; }

  AddOnFareInfos& uniqueAddOnRoutings() { return _uniqueAddOnRoutings; }
  const AddOnFareInfos& uniqueAddOnRoutings() const { return _uniqueAddOnRoutings; }

  // Accessors for Iclusion code Diagnostic
  FareDisplayInclCd*& fareDisplayInclCd() { return _fareDisplayInclCd; }
  const FareDisplayInclCd* fareDisplayInclCd() const { return _fareDisplayInclCd; }

  std::string& rtgText() { return _rtgText; }
  const std::string& rtgText() const { return _rtgText; }

  std::string& rtgDiagInfo() { return _rtgDiagInfo; }
  const std::string& rtgDiagInfo() const { return _rtgDiagInfo; }

  std::string& marketTypeCode() { return _marketTypeCode; }
  const std::string& marketTypeCode() const { return _marketTypeCode; }

  bool& isGroupedByTravelDiscDate() { return _groupedByTravelDiscDate; }
  const bool& isGroupedByTravelDiscDate() const { return _groupedByTravelDiscDate; }
  void setGroupedByTravelDiscDate() { _groupedByTravelDiscDate = true; }

  // Add map for Brand name and text

  BrandNameTextMap& brandNameTextMap() { return _brandNameTextMap; }
  const BrandNameTextMap& brandNameTextMap() const { return _brandNameTextMap; }

  std::vector<tse::FDHeaderMsgText*>& headerMsgs() { return _headerMsgs; }
  const std::vector<tse::FDHeaderMsgText*>& headerMsgs() const { return _headerMsgs; }

  // Brander Fares project 2013
  ProgramBrandNameMap& programBrandNameMap() { return _programBrandNameMap; }
  const ProgramBrandNameMap& programBrandNameMap() const { return _programBrandNameMap; }

  //Price by Cabin FQ phase 1
  typedef std::map<uint8_t, FareDisplayInclCd* > InclCdPtrPerInclCodeMap;
  const InclCdPtrPerInclCodeMap& fdInclCdPerInclCode() const { return _inclCdPerInclusionCode; }
  InclCdPtrPerInclCodeMap& fdInclCdPerInclCode() { return _inclCdPerInclusionCode; }

  std::vector<uint8_t>& multiInclusionCabins() { return _multiInclusionCabins; }
  const std::vector<uint8_t>& multiInclusionCabins() const { return _multiInclusionCabins; }

  const bool& isDisplayHeaderCabin() const { return _displayHeaderCabin; }
  void setDoNotDisplayHeaderCabin() { _displayHeaderCabin = false; }

protected:
  FareDisplayResponse(const FareDisplayResponse&);
  FareDisplayResponse& operator=(const FareDisplayResponse&);

  std::vector<AlphaCode> _badAlphaCodes;
  std::vector<CatNumber> _badCategoryNumbers;
  std::vector<CatNumber> _subCategoryNumbers;

  FareDisplayInclCd* _fareDisplayInclCd;

private:

  std::vector<FlightCount*> _scheduleCounts;

  // Basis for routing data
  RoutingInfoMap _uniqueRoutingMap;
  RtgSeq2PaxTypeFareMap _uniqueRoutings;
  AddOnFareInfos _uniqueAddOnRoutings;

  std::set<CurrencyCode> _validCurrencySet;
  std::vector<Group::GroupType> _groupHeaders;
  std::string _rtgText;
  std::string _rtgDiagInfo;

  // For the brand grouping header
  std::string _marketTypeCode;

  BrandNameTextMap _brandNameTextMap;

  bool _groupedByTravelDiscDate;
  std::vector<tse::FDHeaderMsgText*> _headerMsgs;

  // Brander Fares project 2013
  ProgramBrandNameMap _programBrandNameMap;

  //Price by Cabin FQ phase 1
  InclCdPtrPerInclCodeMap _inclCdPerInclusionCode;
  std::vector<uint8_t> _multiInclusionCabins;
  bool _displayHeaderCabin;
};

} // tse namespace

