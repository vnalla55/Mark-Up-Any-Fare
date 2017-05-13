//-------------------------------------------------------------------
//
//  File:        FareDisplayTrx.h
//  Authors:     Abu
//
//  Description: Shopping Transaction object
//
//
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
//-------------------------------------------------------------------

#pragma once

#include "Common/MultiTransportMarkets.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FDAddOnFareInfo.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{
class FareMarket;
class Loc;
class MPData;
class PaxTypeFare;

class FareDisplayTrx : public PricingTrx
{
public:
  FareDisplayTrx();
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  void setRequest(FareDisplayRequest* request) { _request = request; }
  const FareDisplayRequest* getRequest() const
  {
    return static_cast<FareDisplayRequest*>(_request);
  }

  FareDisplayRequest* getRequest() { return static_cast<FareDisplayRequest*>(_request); }

  void setOptions(FareDisplayOptions* options) { _options = options; }
  const FareDisplayOptions* getOptions() const
  {
    return static_cast<FareDisplayOptions*>(_options);
  }

  FareDisplayOptions* getOptions() { return static_cast<FareDisplayOptions*>(_options); }

  const std::map<MultiTransportMarkets::Market, std::set<CarrierCode> >* marketCarrierMap() const
  {
    return _marketCarrierMap;
  }

  std::map<MultiTransportMarkets::Market, std::set<CarrierCode> >*& marketCarrierMap()
  {
    return _marketCarrierMap;
  }

  void
  setMarketCarrierMap(std::map<MultiTransportMarkets::Market, std::set<CarrierCode> >*& mktCxrMap)
  {
    _marketCarrierMap = mktCxrMap;
  }

  const std::set<CarrierCode>& preferredCarriers() const { return _preferredCarriers; }
  std::set<CarrierCode>& preferredCarriers() { return _preferredCarriers; }

  const bool displayShoppingFare() const { return _displayShoppingFare; }
  bool& displayShoppingFare() { return _displayShoppingFare; }

  std::ostringstream& response() { return _response; }
  std::ostringstream& errorResponse() { return _errorResponse; }

  const FareDisplayResponse* fdResponse() const { return _fdResponse; }
  FareDisplayResponse*& fdResponse() { return _fdResponse; }

  const std::vector<PaxTypeFare*>& allPaxTypeFare() const { return _allPaxTypeFare; }
  std::vector<PaxTypeFare*>& allPaxTypeFare() { return _allPaxTypeFare; }

  const std::vector<PaxType*>& paxTypeRec8() const { return _paxTypeRec8; }
  std::vector<PaxType*>& paxTypeRec8() { return _paxTypeRec8; }

  const std::vector<FDAddOnFareInfo*>& allFDAddOnFare() const { return _allFDAddOnFare; }
  std::vector<FDAddOnFareInfo*>& allFDAddOnFare() { return _allFDAddOnFare; }

  const bool multipleCarriersEntered() const { return _multipleCarriersEntered; }
  bool& multipleCarriersEntered() { return _multipleCarriersEntered; }

  const bool isSeasonTemplate() const { return _seasonTemplate; }
  bool isSeasonTemplate() { return _seasonTemplate; }

  const bool isTwoColumnTemplate() const { return _twoColumnTemplate; }
  bool isTwoColumnTemplate() { return _twoColumnTemplate; }

  const std::vector<std::map<FieldColumnElement, std::string> *>& getFareDataMapVec() const
  {
    return _fareDataMapVec;
  }

  void addFareDataMapVec(std::map<FieldColumnElement, std::string> *fareDataMap)
  {
    _fareDataMapVec.push_back(fareDataMap);
  }

  void getAlternateCurrencies(std::set<CurrencyCode>& alternateCurrencies);

  const CarrierCode& requestedCarrier() const { return fareMarket().front()->governingCarrier(); }

  const Loc* origin() const
  {
    if (travelSeg().empty())
      return nullptr;
    else
      return travelSeg().front()->origin();
  }

  const Loc* destination() const
  {
    if (travelSeg().empty())
      return nullptr;
    else
      return travelSeg().front()->destination();
  }

  const LocCode& boardMultiCity() const
  {

    if (!fareMarket().empty())
      return fareMarket().front()->boardMultiCity();
    else
      return travelSeg().front()->boardMultiCity();
  }

  const LocCode& offMultiCity() const
  {
    if (!fareMarket().empty())
      return fareMarket().front()->offMultiCity();
    else
      return travelSeg().front()->offMultiCity();
  }

  // Build Inbound FareMarket for Fare Display
  FareMarket*& inboundFareMarket() { return _inboundFareMarket; }
  const FareMarket* inboundFareMarket() const { return _inboundFareMarket; }
  /** Determines if it is a Circle Trip or Round The world Request.*/

  bool isSameCityPairRqst() const;

  bool isDomestic() const;
  bool isForeignDomestic() const;
  bool isInternational() const;

  MPData* mpData() const { return _mpData; }
  MPData*& mpData() { return _mpData; }

  virtual const std::vector<FareDispTemplate*>&
  getFareDispTemplate(const int& templateID, const Indicator& templateType)
  {
    return dataHandle().getFareDispTemplate(templateID, templateType);
  }

  virtual const std::vector<FareDispTemplateSeg*>&
  getFareDispTemplateSeg(const int& templateID, const Indicator& templateType)
  {
    return dataHandle().getFareDispTemplateSeg(templateID, templateType);
  }

  void setupFootNotePrevalidation() override { _footNotePrevalidationAllowed =  false; }

protected:
  std::ostringstream _response;
  FareMarket* _inboundFareMarket = nullptr;

private:
  std::set<CarrierCode> _preferredCarriers;
  std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>* _marketCarrierMap = nullptr;
  bool _displayShoppingFare = false;
  FareDisplayResponse* _fdResponse = nullptr;
  std::ostringstream _errorResponse;
  std::vector<PaxTypeFare*> _allPaxTypeFare;
  std::vector<PaxType*> _paxTypeRec8;
  // NOTE: _allFDAddOnFare vector holds all addon fares.
  // _allPaxTypeFare and _allFDAddOnFare are mutualy exclusive
  // We can't have values in both at the same time.
  std::vector<FDAddOnFareInfo*> _allFDAddOnFare;

  bool _multipleCarriersEntered = false;
  MPData* _mpData = nullptr;
  bool _seasonTemplate = false;
  bool _twoColumnTemplate = false;
  bool _s8ServiceCalled = false;
  std::vector<std::map<FieldColumnElement, std::string> *> _fareDataMapVec;

  static constexpr Indicator DISPLAY_SCHEDULES = 'S';

public:
  bool isShortRD();
  bool isLongRD();
  bool isRD();
  bool isERD() const;
  bool isERDFromSWS() const;
  bool isFQ();
  bool isFT();
  bool isTravelAgent();
  virtual bool isShopperRequest() const;
  bool multipleCurrencies();
  bool isRecordScopeDomestic();
  void errResponse();
  const InclusionCode getRequestedInclusionCode() const;

  bool isShortRequest(); // RD#, RB# or FT#
  bool isDiagnosticRequest();
  bool isFDDiagnosticRequest();

  bool isSDSOutputType();

  bool isShortRequestForPublished();

  bool needFbrFareCtrl();
  bool needDiscFareCtrl();
  bool needNegFareCtrl();

  /**
   * checks to see if user has requested schedule count information.
   * If user doesnt have a user preference, schedules will be collected
   * only for shopper request.
   */
  bool isScheduleCountRequested() const;
  /**
   * checks for shchedule counts.
   */
  bool hasScheduleCountInfo() const;
  void initializeTemplate();
  void setS8ServiceCalled() { _s8ServiceCalled = true; }
  bool isS8ServiceCalled() { return _s8ServiceCalled; }
};
} // tse namespace
