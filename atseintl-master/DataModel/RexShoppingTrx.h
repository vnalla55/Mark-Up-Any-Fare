//-------------------------------------------------------------------
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{

class RexShoppingTrx : public RexPricingTrx
{
private:
  struct FareMarketComparator
  {
    bool operator()(const FareMarket* lfm, const FareMarket* rfm) const
    {
      return lfm->travelSeg().front()->segmentOrder() < rfm->travelSeg().front()->segmentOrder();
    }
  };

public:
  //------------------------DATA STRUCTURES--------------------
  // R3 Tab988 - portion - positions of segments in exchange itinerary that cannot be changed
  // NYC - LAX - DFW - KRK
  //    1     2     3
  using PortionMergeTvlVectType = std::vector<int>;
  using ForcedConnectionSet = std::set<LocCode>;
  struct FareByteCxrAppl
  {
    std::set<CarrierCode> restCxr;
    std::set<CarrierCode> applCxr;
    bool govCxrPref = false;
  };
  using FareByteCxrApplVect = std::vector<FareByteCxrAppl>;
  using FlightNumberRestrictionVect = std::vector<bool>;

  struct R3SeqsConstraint
  {
    PortionMergeTvlVectType portionMerge;
    ForcedConnectionSet forcedConnection;
    bool firstBreakStatus = false;
    FareByteCxrAppl fareByteCxrAppl;
    bool flightNumberRestriction = false;
    PortionMergeTvlVectType outboundPortion;
    Indicator changeInd = ' ';
    ExchShopCalendar::DateRange calendarRange;
    ExchShopCalendar::DateApplication calendarAppl = ExchShopCalendar::WHOLE_PERIOD;
  };
  using R3SeqsConstraintMap = std::map<const uint32_t, std::vector<R3SeqsConstraint>>;
  using OADDataMap = std::map<const FareMarket*, R3SeqsConstraintMap>;
  using OADDataPair = std::pair<const FareMarket*, R3SeqsConstraintMap>;
  struct FareByteCxrApplData
  {
    std::set<CarrierCode> cxrList;
    bool excluded = true;
  };
  struct FareByteCxrData
  {
    FareByteCxrApplData govCxr;
    FareByteCxrApplData usrCxr;
  };
  struct OADResponseData
  {
    PortionMergeTvlVectType portion;
    ForcedConnectionSet forcedConnections;
    bool firstBreakRest = false;
    FareByteCxrApplData fareByteCxrAppl;
    bool flightNumberRestriction = false;
    PortionMergeTvlVectType outboundPortion;
    ExchShopCalendar::DateRange calendarRange;
  };
  using OADResponseDataMap =
      std::map<const FareMarket*, std::vector<OADResponseData>, FareMarketComparator>;

  struct SubOriginDestination
  {
    LocCode origAirport;
    LocCode destAirport;
    DateTime travelDate;
    std::set<int> flights;
    std::set<int> bkcCanChange;
    ForcedConnectionSet forcedConnections;
    FareByteCxrData* carriers = nullptr;
    ExchShopCalendar::DateRange calendarRange;

    bool change = false; // T - change of flight and bkg is not restricted
    bool exact_cxr = false; // T - preferred user cxr list/ F - excluded user cxr list
    bool preferred_cxr = false; // T - preferred gov cxr list/ F - excluded gov cxr list
  };

  struct SODConstraint
  {
    std::set<int> restFlightsWithBkg;
    FareByteCxrData* fareByteCxrAppl = nullptr;
    std::set<int> restFlights;
    ExchShopCalendar::DateRange calendarRange;
  };

  struct OADConsolidatedConstrains
  {
    int firstONDInfoSegPos = -1;
    int lastONDInfoSegPos = -1;
    LocCode origAirport;
    LocCode destAirport;
    DateTime travelDate;
    std::set<int> unshoppedFlights;
    bool unflown = false;
    ForcedConnectionSet forcedConnection;
    std::vector<SubOriginDestination*> sod;
    bool cxrRestrictionNeeded = true;

    std::map<std::size_t, SODConstraint> sodConstraints;
  };
  using OADConsolidatedConstrainsVect = std::vector<OADConsolidatedConstrains*>;
  using FMToFirstFMMap = std::map<const FareMarket*, std::vector<const FareMarket*>>;

  OADDataMap& oadData() { return _oadData; }
  const OADDataMap& oadData() const { return _oadData; }
  std::vector<R3SeqsConstraint>& oadDataItem(const FareMarket* fm, uint32_t recNo)
  {
    return _oadData[fm][recNo];
  }

  OADResponseDataMap& oadResponse() { return _oadResponse; }
  const OADResponseDataMap& oadResponse() const { return _oadResponse; }

  FareByteCxrApplData& cxrListFromPSS() { return _cxrListFromPSS; }
  const FareByteCxrApplData& cxrListFromPSS() const { return _cxrListFromPSS; }

  OADConsolidatedConstrainsVect& oadConsRest() { return _oadConsRest; }
  const OADConsolidatedConstrainsVect& oadConsRest() const { return _oadConsRest; }

  FMToFirstFMMap& fMToFirstFMinPU() { return _FMToFirstFMinPU; }
  const FMToFirstFMMap& fMToFirstFMinPU() const { return _FMToFirstFMinPU; }

  void setShopOnlyCurrentItin(bool shopOnlyCurrentItin)
  {
    _shopOnlyCurrentItin = shopOnlyCurrentItin;
  }

  bool isShopOnlyCurrentItin() const { return _shopOnlyCurrentItin; }

  virtual bool process(Service& srv) override { return srv.process(*this); }

  std::vector<ShoppingTrx::Leg>& excLegs() { return _excLegs; }
  const std::vector<ShoppingTrx::Leg>& excLegs() const { return _excLegs; }

private:
  std::vector<ShoppingTrx::Leg> _excLegs;

  OADDataMap _oadData;
  OADResponseDataMap _oadResponse;
  FareByteCxrApplData _cxrListFromPSS;
  OADConsolidatedConstrainsVect _oadConsRest;
  FMToFirstFMMap _FMToFirstFMinPU;
  bool _shopOnlyCurrentItin = false;
};
} // tse namespace
