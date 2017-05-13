//----------------------------------------------------------------------------
//  File:        TaxShoppingRequestHandlerOld.h
//  Created:     2012-05-28
//
//  Description: Shopping handler for charger tax requests
//
//  Updates:
//
//  Copyright Sabre 2012
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

#include "Common/ShoppingTaxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"
#include "Xform/CommonRequestHandler.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"

#include <boost/unordered_map.hpp>

#include <map>
#include <string>
#include <vector>

#include <tr1/array>

namespace tse
{

class TaxShoppingConfig;

class TaxShoppingRequestHandlerOld : protected CommonRequestHandler
{
public:
  TaxShoppingRequestHandlerOld(Trx*& taxTrx, const TaxShoppingConfig& taxCfg, bool throttled);
  virtual ~TaxShoppingRequestHandlerOld();
  void parse(DataHandle& dataHandle, const std::string& content) override;
  void parse(DataHandle& dataHandle, const char*& content);
  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual void characters(const char* value, size_t length) override;
  virtual bool endElement(int idx) override;
  virtual bool startElement(const IKeyString& name, const IAttributes& attrs) override;
  virtual bool endElement(const IKeyString& name) override;

private:
  struct CAL
  {
    CAL()
      : _farebasisCode(""), _fareType(""), _owrt(ALL_WAYS), _direction(BOTH), _equivalentAmount(0.0)
      , _discountCategory(0), _discountPercent(0.0), _paxType(nullptr), _markupAmount(0.0)
    {
    }

    std::string _farebasisCode;
    FareType _fareType;
    Indicator _owrt;
    Directionality _direction;
    MoneyAmount _equivalentAmount;
    uint8_t _discountCategory;
    MoneyAmount _discountPercent;
    PaxType* _paxType;
    MoneyAmount _markupAmount;
  };

  struct FLI
  {
    FLI() : _origin(nullptr), _destination(nullptr), _flightNumber(-1), _geoTravelType(GeoTravelType::UnknownGeoTravelType) {}

    LocCode _departureAirport;
    LocCode _arrivalAirport;
    LocCode _boardMultiCity;
    LocCode _offMultiCity;
    const Loc* _origin;
    const Loc* _destination;
    CarrierCode _marketingCarrier;
    CarrierCode _operatingCarrier;
    FlightNumber _flightNumber;
    EquipmentType _equipmentType;
    GeoTravelType _geoTravelType;
    boost::posix_time::time_duration _departureTime;
    boost::posix_time::time_duration _arrivalTime;
    std::vector<const Loc*> _hiddenStops;
  };

  struct NationTransitTimes
  {
    std::set<Hours> _transitHours;
    std::set<Minutes> _transitTotalMinutes;

    void insertTime(const Hours h)
    {
      _transitHours.insert(h);
      _transitTotalMinutes.insert(Minutes(h.total_seconds() / 60));
    }
  };
  typedef std::map<NationCode, NationTransitTimes> NationTransitTimesMap;
  typedef std::map<NationCode, bool> NationWithNextStopoverRestrMap;
  typedef std::set<boost::gregorian::date> TaxFirstTravelDates;
  typedef std::map<NationCode, TaxFirstTravelDates> NationTaxFirstTravelDatesMap;
  typedef std::set<NationCode> NationSet;
  typedef std::map<NationSet, TaxFirstTravelDates> NationFirstTvlDatesMap;

  virtual void createTransaction(DataHandle& dataHandle, const std::string& content) override;
  void onStartAGI(const IAttributes& attrs);
  void onStartBIL(const IAttributes& attrs);
  void onStartPRO(const IAttributes& attrs);
  void onStartCAL(const IAttributes& attrs);
  void onStartFLI(const IAttributes& attrs);
  void onStartDAT(const IAttributes& attrs);
  void onStartDIA(const IAttributes& attrs);
  void onStartARG(const IAttributes& attrs);
  void onEndFLI();
  void onStartHSP(const IAttributes& attrs);
  void onStartCOM(const IAttributes& attrs);
  void onStartTAX(const IAttributes& attrs);
  void onEndCOM();
  void onStartPXI(const IAttributes& attrs);
  void onEndPXI();
  void onStartCID(const IAttributes& attrs);
  void onStartFID(const IAttributes& attrs);
  void onStartDynamicConfig(const IAttributes& attrs);
  void onStartTRQ(const IAttributes& attrs);
  void onEndTRQ();
  void addFlightsToFareComponents();
  void setGeoTraveltype(FLI& fli);
  void generateUSCAKey(std::string& key);
  void generateIntlKey(std::string& key);

  bool isNextStopoverRestr(const std::vector<TravelSeg*>& segments);
  std::string getFirstTvlDateApplicableTaxes(const NationCode& nation,
                                             const std::vector<TravelSeg*>& segments,
                                             const DateTime& tktDT);

  char getSameDayRuleChar(const DateTime& d1,
                          const DateTime& d2,
                          const NationCodesVec& nations,
                          const NationCode& nation) const;

  char getTransitHoursChar(const NationCode& nation,
                           const DateTime& arrivalDT,
                           const DateTime& departureDT,
                           const DateTime& tktDT);

  bool getRestrWithNextStopover(const NationCode& nation, const DateTime& tktDT);
  const TaxFirstTravelDates& getTaxFirstTravelDates(const NationCode& nation,
                                                    const DateTime& tktDate);
  const NationTransitTimes& getNationTransitTimes(const NationCode& nation, const DateTime& tktDT);
  virtual void collectNationTransitTimes(NationTransitTimes& transitTime,
                                         const NationCode& nation,
                                         const DateTime& tktDate) const;
  void collectTaxFirstTravelDates(TaxFirstTravelDates& firstTravelDates,
                                  const NationCode& nation,
                                  const DateTime& tktDate) const;

  std::string getFirstTvlDateRangeString(const std::vector<TravelSeg*>& segments,
                                         const DateTime& tktDT);

  PaxType* getPaxType(const PaxTypeCode& paxTypeCode, uint16_t paxNumber, uint16_t age = 0);

  void clearNationsFirstTvlDates() { _nationsFirstTvlDates.clear(); }
  TaxTrx* _taxTrx;
  boost::unordered_map<int32_t, CAL*> _fareComponentsMap;
  boost::unordered_map<int32_t, FLI*> _flightsMap;
  boost::unordered_map<int32_t, boost::gregorian::date> _datesMap;
  boost::gregorian::date _firstTvlDate;
  boost::gregorian::date _lastTvlDate;
  Itin* _currentItin;
  FarePath* _currentFarePath;
  FLI* _currentFli;
  FareMarket _fareMarket;
  std::string _currentFPKey;
  int _taxClassId;
  typedef boost::unordered_map<std::string, int> KeyToTaxClassMap;
  KeyToTaxClassMap _keyToTaxClassId;
  int _currentItinId;
  typedef boost::unordered_map<std::string, bool> AYApplicationCache;
  AYApplicationCache _ayCache;
  bool _applyUSCAgrouping;
  typedef std::tr1::array<char, 16> StringBuf;
  std::map<NationCode, ShoppingTaxUtil::DateSegmentation*> _dateSegmenters;
  std::map<NationCode, ShoppingTaxUtil::FlightRanges*> _fltNoSegmenters;
  std::map<PaxTypeCode, PaxType*> _paxTypes;
  NationTransitTimesMap _nationTransitTimes;
  NationWithNextStopoverRestrMap _nationWithNextStopoverRestr;
  NationTaxFirstTravelDatesMap _nationTaxFirstTravelDatesMap;
  NationFirstTvlDatesMap _nationsFirstTvlDates;
  typedef std::map<std::string, MoneyAmount> TaxMap;
  TaxMap _taxMap;
  bool _useFlightRanges;
  const TaxShoppingConfig& _taxShoppingCfg;
  bool _throttled;
};

} // end namespace tse

