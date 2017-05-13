#include <stack>

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigMan.h"
#include "Common/CustomerActivationUtil.h"
#include "Common/Global.h"
#include "Common/IntlJourneyUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DST.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "ItinAnalyzer/FamilyLogicSplitter.h"
#include "ItinAnalyzer/FamilyLogicUtils.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);
FALLBACKVALUE_DECL(fallbackLatamJourneyActivation);

using boost::assign::operator+=;
using boost::assign::list_of;

const std::string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
const std::string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const std::string LOC_RIO = "/vobs/atseintl/test/testdata/data/LocRIO.xml";
const std::string LOC_SAO = "/vobs/atseintl/test/testdata/data/LocSAO.xml";

class TrxUtilMock : public TrxUtil
{
public:
  TrxUtilMock()
  {
    std::vector<CarrierCode> interV1{"4M", "LP", "XL"};
    std::vector<CarrierCode> interV2{"**"};
    std::vector<CarrierCode> interV3{"4M", "LA", "LP"};
    std::vector<CarrierCode> interV4{"4T", "HG", "LH"};

    interlineAvailabilityCxrs["OK"] = interV2;
    interlineAvailabilityCxrs["LR"] = interV2;
    interlineAvailabilityCxrs["AB"] = interV4;
    interlineAvailabilityCxrs["TK"] = interV2;
    interlineAvailabilityCxrs["LH"] = interV2;
    interlineAvailabilityCxrs["TA"] = interV2;
    interlineAvailabilityCxrs["SK"] = interV2;
    interlineAvailabilityCxrs["CX"] = interV2;
    interlineAvailabilityCxrs["KA"] = interV2;
    interlineAvailabilityCxrs["LX"] = interV2;
    interlineAvailabilityCxrs["AV"] = interV2;
    interlineAvailabilityCxrs["EK"] = interV2;
    interlineAvailabilityCxrs["LA"] = interV1;
    interlineAvailabilityCxrs["XL"] = interV3;
  };
};

class MockItinAnalyzerService : public ItinAnalyzerService
{
public:
  MockItinAnalyzerService(const std::string& name, TseServer& srv)
    : ItinAnalyzerService(name, srv), _carrierPrefType(JOURNEY_ACTIVATED)
  {
  }

  bool setGeoTravelTypeAndTktCxr(Itin& itin) { return true; };
  void setRetransits(Itin* itin) {};
  void setOpenSegFlag(Itin* itin) {};
  void setTripCharacteristics(Itin itin, bool) {};

  enum CarrierPrefType
  { ZERO_CXR_PREF = 1,
    JOURNEY_ACTIVATED,
    JOURNEY_NOT_ACTIVATED };

  const CarrierPreference*
  getCarrierPref(PricingTrx& trx, const CarrierCode& carrier, const DateTime& date)
  {
    if (_carrierPrefType == ZERO_CXR_PREF)
      return 0;
    CarrierPreference* cxrPref = _memHandle.create<CarrierPreference>();
    if (_carrierPrefType == JOURNEY_ACTIVATED)
    {
      cxrPref->activateJourneyShopping() = YES;
      cxrPref->activateJourneyPricing() = YES;
    }
    else
    {
      cxrPref->activateJourneyShopping() = NO;
      cxrPref->activateJourneyPricing() = NO;
    }
    return cxrPref;
  }

  CarrierPrefType _carrierPrefType;
  TestMemHandle _memHandle;
};

class ExcItinAnalyzerServiceWrapperMock : public ExcItinAnalyzerServiceWrapper
{
public:
  ExcItinAnalyzerServiceWrapperMock(ItinAnalyzerService& itinAnalyzer)
    : ExcItinAnalyzerServiceWrapper(itinAnalyzer)
  {
  }

  enum VISITED
  { PROCESS_EXCHANGE_ITIN_VISITED,
    DETERMINE_CHANGES_VISITED,
    SELECT_TICKETING_CARRIER_VISITED,
    SET_RETRANSITS_VISITED,
    SET_OPEN_SEG_FLAG_VISITED,
    SET_TRIP_CHARACTERISTICS_VISITED,
    CHECK_JOURNEY_ACTIVATION_VISITED,
    CHECK_SOLO_ACTIVATION_VISITED,
    SET_ATAE_CONTENT_VISITED,
    SET_ITIN_ROUNDING_VISITED,
    SET_INFO_FOR_SIMILAR_ITINS_VISITED,
    BUILD_FARE_MARKET_VISITED };

  void processExchangeItin(ExchangePricingTrx& trx) { setVisited(PROCESS_EXCHANGE_ITIN_VISITED); }
  void determineChanges(ExchangePricingTrx& trx) { setVisited(DETERMINE_CHANGES_VISITED); }
  bool selectTicketingCarrier(PricingTrx& trx)
  {
    setVisited(SELECT_TICKETING_CARRIER_VISITED);
    return true;
  }
  void setRetransits(PricingTrx& trx) { setVisited(SET_RETRANSITS_VISITED); }
  void setOpenSegFlag(PricingTrx& trx) { setVisited(SET_OPEN_SEG_FLAG_VISITED); }
  void setTripCharacteristics(PricingTrx& trx) { setVisited(SET_TRIP_CHARACTERISTICS_VISITED); }
  void checkJourneyActivation(PricingTrx& trx) { setVisited(CHECK_JOURNEY_ACTIVATION_VISITED); }
  void checkSoloActivation(PricingTrx& trx) { setVisited(CHECK_SOLO_ACTIVATION_VISITED); }
  void setATAEContent(PricingTrx& trx) { setVisited(SET_ATAE_CONTENT_VISITED); }
  void setATAESchedContent(PricingTrx& trx) { setVisited(SET_ATAE_CONTENT_VISITED); }
  void setATAEAvailContent(PricingTrx& trx) { setVisited(SET_ATAE_CONTENT_VISITED); }
  void setItinRounding(PricingTrx& trx) { setVisited(SET_ITIN_ROUNDING_VISITED); }
  void setInfoForSimilarItins(PricingTrx& trx) { setVisited(SET_INFO_FOR_SIMILAR_ITINS_VISITED); }
  bool buildFareMarket(PricingTrx& trx, Itin& itin)
  {
    setVisited(BUILD_FARE_MARKET_VISITED);
    return true;
  }

  void setVisited(VISITED visited) { _visited.insert(visited); }
  bool isVisited(VISITED visited) { return _visited.find(visited) != _visited.end(); }

private:
  std::set<VISITED> _visited;
};

class ItinAnalyzerServiceMock : public ItinAnalyzerService
{
public:
  enum VISITED
  { SET_FARE_FAMILIES_IDS };
  void setVisited(VISITED visited) { _visited.insert(visited); }
  bool isVisited(VISITED visited) { return _visited.count(visited); }

  ItinAnalyzerServiceMock(const std::string& name, TseServer& srv)
    : ItinAnalyzerService(name, srv),
      _throwSetAtaeContent(false),
      _excItinAnalyzerWrapperMock(*this)
  {
  }

  virtual void checkFirstDatedSegBeforeDT(PricingTrx& trx) {}
  virtual void checkRestrictedCurrencyNation(PricingTrx& trx) {}
  virtual const Loc* getTicketLoc(PricingTrx& trx) { return 0; }
  virtual const Loc* getSalesLoc(PricingTrx& trx) { return 0; }
  virtual void setIntlSalesIndicator(PricingTrx& trx) {}
  virtual void checkJourneyActivation(PricingTrx& trx) {}
  virtual void checkSoloActivation(PricingTrx& trx) {}
  virtual void prepareForJourney(PricingTrx& trx) {}
  virtual void setATAEContent(PricingTrx& trx)
  {
    if (_throwSetAtaeContent)
      throw std::string("ATAE content set");
  }
  virtual void setATAESchedContent(PricingTrx& trx)
  {
    if (_throwSetAtaeContent)
      throw std::string("ATAE content set");
  }
  virtual void setATAEAvailContent(PricingTrx& trx)
  {
    if (_throwSetAtaeContent)
      throw std::string("ATAE content set");
  }
  virtual void setCurrencyOverride(PricingTrx& trx) {}
  virtual void setSortTaxByOrigCity(PricingTrx& trx) {}
  virtual void setValidatingCarrier(PricingTrx& trx) {}
  virtual void setValidatingCarrier(PricingTrx& trx, Itin& itin) {}
  virtual void setItinCurrencies(PricingTrx& trx) {}
  virtual void setItinCurrencies(RexBaseTrx& trx) {}
  virtual void setGeoTravelTypeAndValidatingCarrier(PricingTrx& trx) {}
  virtual void setAgentCommissions(PricingTrx& trx) {}
  virtual void setFurthestPoint(PricingTrx& trx, Itin* itin) {}
  virtual void setFareFamiliesIds(PricingTrx& trx)
  {
    _visited.insert(SET_FARE_FAMILIES_IDS);
    return FamilyLogicUtils::setFareFamiliesIds(trx);
  }

  bool setGeoTravelTypeAndTktCxr(Itin& itin) { return true; }
  void setRetransits(Itin& itin) {}
  void setOpenSegFlag(Itin& itin) {}
  void setTripCharacteristics(const PricingTrx& trx, Itin* itin) {}
  void setTripCharacteristics(Itin*, bool) {}
  void setIntlSalesIndicator(Itin& itin, const Loc& ticketingLoc, const Loc& saleLoc) {}
  virtual ItinAnalyzerServiceWrapper* selectProcesing(PricingTrx& trx)
  {
    ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = ItinAnalyzerService::selectProcesing(trx);
    return baseItinAnalyzerWrapper == &_excItinAnalyzerWrapper ? &_excItinAnalyzerWrapperMock
                                                               : baseItinAnalyzerWrapper;
  }

  virtual void
  getCabins(PricingTrx& trx, AirSeg* const fakeTS, std::vector<ClassOfService*>& cosList) const
  {
    for (char bkg = 'A'; bkg <= 'Z'; ++bkg)
    {
      ClassOfService cos;
      if (bkg == 'A' || bkg == 'F' || bkg == 'P')
        cos.cabin().setFirstClass();
      else if (bkg == 'C' || bkg == 'D' || bkg == 'I' || bkg == 'J')
        cos.cabin().setBusinessClass();
      else
        cos.cabin().setEconomyClass();

      cos.bookingCode() = bkg;
      cos.numSeats() = 9;
      cosList.push_back(&cos);
    }
  }

  virtual bool selectGcmFareMarkets(PricingTrx& trx,
                                    std::vector<FareMarket*>& gcmFareMarkets,
                                    Diag185Collector* diag185 = 0)
  {
    _fm = _memHandle.create<FareMarket>();
    gcmFareMarkets.emplace_back(_fm);
    return true;
  }

  bool _throwSetAtaeContent;
  ExcItinAnalyzerServiceWrapperMock _excItinAnalyzerWrapperMock;
  FareMarket* _fm;
  TestMemHandle _memHandle;

private:
  std::set<VISITED> _visited;
};

class ItinAnalyzerServiceMockForRefund : public ItinAnalyzerService
{
public:
  ItinAnalyzerServiceMockForRefund(const std::string& name, TseServer& srv, TestMemHandle& memory)
    : ItinAnalyzerService(name, srv), _mockWrapper(*this, memory)
  {
  }

private:
  class ItinAnalyzerServiceMockForRefundWrapper : public ItinAnalyzerServiceWrapper
  {
  public:
    ItinAnalyzerServiceMockForRefundWrapper(ItinAnalyzerService& itinAnalyzer,
                                            TestMemHandle& memory)
      : ItinAnalyzerServiceWrapper(itinAnalyzer), _memory(memory)
    {
    }

  protected:
    friend class ItinAnalyzerServiceTest;

    virtual bool buildFareMarket(PricingTrx& trx, Itin& itin)
    {
      trx.fareMarket() += _memory.insert(new FareMarket), _memory.insert(new FareMarket);

      return true;
    }

    TestMemHandle& _memory;
  };

  //  virtual void buildFareMarket(PricingTrx& trx, Itin& itin)
  //  {
  //    trx.fareMarket() += _memory.insert(new FareMarket),
  //      _memory.insert(new FareMarket);
  //  }

  virtual ItinAnalyzerServiceWrapper* selectProcesing(PricingTrx& trx) { return &_mockWrapper; }

  ItinAnalyzerServiceMockForRefundWrapper _mockWrapper;
};

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  MultiAirportCity* getMT(LocCode city, LocCode airport)
  {
    MultiAirportCity* ret = _memHandle.create<MultiAirportCity>();
    ret->airportCode() = airport;
    ret->city() = city;
    return ret;
  }

public:
  const std::vector<LimitationJrny*>& getJLimitation(const DateTime& date)
  {
    std::vector<LimitationJrny*>* ret = _memHandle.create<std::vector<LimitationJrny*>>();
    LimitationJrny* lim = _memHandle.create<LimitationJrny>();
    lim->separateTktInd() = 'Y';
    lim->intlDepartMaxNo() = 4;
    lim->intlArrivalMaxNo() = 4;
    lim->retransitLoc().locType() = 'Z';
    lim->retransitLoc().loc() = "1954";
    ret->push_back(lim);
    return *ret;
  }
  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date)
  {
    if (carrier == "**" && classOfService == "Y")
    {
      Cabin* ret = _memHandle.create<Cabin>();
      ret->cabin().setEconomyClass();
      return ret;
    }
    return DataHandleMock::getCabin(carrier, classOfService, date);
  }
  const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
  {
    std::vector<MultiAirportCity*>* ret = _memHandle.create<std::vector<MultiAirportCity*>>();
    if (city == "CCS")
    {
      return *ret;
    }
    else if (city == "MIA")
    {
      ret->push_back(getMT(city, "MIA"));
      ret->push_back(getMT(city, "MPB"));
      return *ret;
    }
    else if (city == "SCL")
    {
      ret->push_back(getMT(city, "SCL"));
      ret->push_back(getMT(city, "ULC"));
      return *ret;
    }
    else if (city == "SDQ")
    {
      ret->push_back(getMT(city, "SDQ"));
      ret->push_back(getMT(city, "HEX"));
      return *ret;
    }
    return DataHandleMock::getMultiAirportCity(city);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "CCS")
      return "CCS";
    else if (locCode == "SCL")
      return "SCL";
    else if (locCode == "SDQ")
      return "SDQ";
    else if (locCode == "SIN")
      return "SIN";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    if (origin == "SCL" && dest == "CCS")
    {
      ret->mileage() = 3050;
      return ret;
    }
    else if (origin == "SCL" && dest == "MIA")
    {
      ret->mileage() = 4138;
      return ret;
    }
    else if (origin == "SCL" && dest == "SDQ")
    {
      return 0;
    }
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }

  const std::vector<AirlineAllianceCarrierInfo*>&
  getAirlineAllianceCarrier(const CarrierCode& carrier)
  {
    if (carrier == "AA")
    {
      std::vector<AirlineAllianceCarrierInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceCarrierInfo*>>();
      AirlineAllianceCarrierInfo* airlineAllianceCarrierInfo =
          _memHandle.create<AirlineAllianceCarrierInfo>();

      airlineAllianceCarrierInfo->carrier() = "AA";
      airlineAllianceCarrierInfo->genericAllianceCode() = "*O";
      airlineAllianceCarrierInfo->genericName() = "ONEWORLD";
      ret->push_back(airlineAllianceCarrierInfo);

      return *ret;
    }
    else if (carrier == "UA")
    {
      std::vector<AirlineAllianceCarrierInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceCarrierInfo*>>();
      AirlineAllianceCarrierInfo* data1 = _memHandle.create<AirlineAllianceCarrierInfo>();

      data1->carrier() = "AA";
      data1->genericAllianceCode() = "*O";
      data1->genericName() = "ONEWORLD";
      ret->push_back(data1);

      AirlineAllianceCarrierInfo* data2 = _memHandle.create<AirlineAllianceCarrierInfo>();

      data2->carrier() = "AA";
      data2->genericAllianceCode() = "*A";
      data2->genericName() = "STAR ALLIANCE";
      ret->push_back(data2);

      return *ret;
    }
    else if (carrier == "LH")
    {
      std::vector<AirlineAllianceCarrierInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceCarrierInfo*>>();
      return *ret;
    }

    return DataHandleMock::getAirlineAllianceCarrier(carrier);
  }
};
}

class ItinAnalyzerServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinAnalyzerServiceTest);
  CPPUNIT_TEST(testBuildFareMarketForPricingTrx);
  CPPUNIT_TEST(testBuildFareMarketForRtwPricingTrxWrongNumberOfCarrierAllianceCodes1);
  CPPUNIT_TEST(testBuildFareMarketForRtwPricingTrxWrongNumberOfCarrierAllianceCodes2);
  CPPUNIT_TEST(testBuildFareMarketForRtwPricingTrxAunkAtEnd);
  CPPUNIT_TEST(testBuildFareMarketForRexPricingTrxThroughProcess);
  CPPUNIT_SKIP_TEST(testItinAnalyzer);
  CPPUNIT_SKIP_TEST(testScheduleGrouping);
  CPPUNIT_TEST(testNonUSCADomesticTransborderForTag10Exchange);
  CPPUNIT_TEST(testUSCADomesticForTag10Exchange);
  CPPUNIT_TEST(testUSCATransborderForTag10Exchange);
  CPPUNIT_TEST(testRemoveFMPartButNotWholeDummy);

  CPPUNIT_TEST(testGetExistingFareMarketWithEmptyMarketVector);
  CPPUNIT_TEST(testGetExistingFareMarketWithNotEmptyMarketVectorWhichDoesntContainNewMarket);
  CPPUNIT_TEST(testGetExistingFareMarketWithNotEmptyMarketVectorWhichContainsNewMarket);

  CPPUNIT_TEST(testAddFareMarketWithAllMarketVectorsEmpty);
  CPPUNIT_TEST(testAddFareMarketWithAllMarketVectorsNotEmptyAndWithoutNewFareMarket);
  CPPUNIT_TEST(testAddFareMarketWithTrxVectorWhichContainsNewFareMarket);
  CPPUNIT_TEST(testAddFareMarketWithTrxAndCarrierVectorsWhichContainsNewFareMarket);
  CPPUNIT_TEST(testAddFareMarketWithAllVectorsWhichContainsNewFareMarket);

  CPPUNIT_TEST(testbuildMarketsForItineraryWithEmptySegments);
  CPPUNIT_TEST(testbuildMarketsForItineraryWithOneSegmentItinerary);
  CPPUNIT_TEST(testbuildMarketsForItineraryWithMultipleSegmentsItinerary);

  CPPUNIT_TEST(testBuildLocalMarketsWithEmptyLegsVector);
  CPPUNIT_TEST(testBuildLocalMarketsWithEmptySopInOneLeg);
  CPPUNIT_TEST(testBuildLocalMarketsWithEmptyCarrierIndex);
  CPPUNIT_TEST(testBuildLocalMarketsWithNotEmptyCarrierIndex);
  CPPUNIT_TEST(testPremiumCabin);

  CPPUNIT_TEST(testInterlineAvlFareMarketCOS_SKSK);
  CPPUNIT_TEST(testInterlineAvlFareMarketCOS_SKUA);
  CPPUNIT_TEST(testInterlineAvlFareMarketCOS_3SK);
  CPPUNIT_TEST(testInterlineAvlFareMarketCOS_2SK1UA);

  CPPUNIT_TEST(testRexTrxRedirected2ExcTrxWhenPass);
  CPPUNIT_TEST(testRexTrxRedirected2ExcTrxWhenFail);
  CPPUNIT_TEST(testRexTrxRedirected2MultiExcTrxWhenFail);
  CPPUNIT_TEST(testSelectProcesingForPortExc);
  CPPUNIT_TEST(testSelectProcesingForRedirectedToPortExc);
  CPPUNIT_TEST(testFlowOfProcessForPortExc);
  CPPUNIT_TEST(testFlowOfProcessForRedirectedToPortExc);

  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenJpsNoInMip);
  CPPUNIT_TEST(testCheckJourneyActivationActivateWhenJpsYesInMip);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenCxrPrefZeroInMip);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenCxrPrefSaysSoInMip);
  CPPUNIT_TEST(testCheckJourneyActivationActivateWhenCxrPrefSaysSoInMip);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenCustomerTableSaysSoInMip);
  CPPUNIT_TEST(testCheckJourneyActivationActivateWhenCustomerTableSaysSoInMip);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenNotWpnc);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenShoppingTrx);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenJpsNo);
  CPPUNIT_TEST(testCheckJourneyActivationActivateWhenJpsYes);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenCxrPrefZero);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenCxrPrefSaysSo);
  CPPUNIT_TEST(testCheckJourneyActivationActivateWhenCxrPrefSaysSo);
  CPPUNIT_TEST(testCheckJourneyActivationDontActivateWhenCustomerTableSaysSo);
  CPPUNIT_TEST(testCheckJourneyActivationActivateWhenCustomerTableSaysSo);
  CPPUNIT_TEST(testPrepareForJourney);

  CPPUNIT_TEST(testProcessRexCommonCallSetAtaeContentForExchangeItin);

  CPPUNIT_TEST(testDetermineExchangeReissueStatusItinNonCat31Reissue);
  CPPUNIT_TEST(testDetermineExchangeReissueStatusItinNonCat31Exchange);
  CPPUNIT_TEST(testDetermineExchangeReissueStatusItinCat31Reissue);
  CPPUNIT_TEST(testDetermineExchangeReissueStatusItinCat31Exchange);
  CPPUNIT_TEST(testDetermineExchangeReissueStatusItinCat31UnchangedReissue);
  CPPUNIT_TEST(testAddRegularFareMarketsForPlusUpCalculation);
  CPPUNIT_TEST(testRemoveDuplicatedFMs);

  CPPUNIT_TEST(testPreviousExhDateFareIndicatorIsOFFforCat31WhenNOapplyReissueExchangeSet);
  CPPUNIT_TEST(testPreviousExhDateFareIndicatorIsOFFforCat31WhenApplyReissueExchangeSet);
  CPPUNIT_TEST(testPreviousExhDateFareIndicatorIsONforCat31WhenPreviousExchangeDTSet);

  CPPUNIT_TEST(testSplitItinsSingleTheSame);
  CPPUNIT_TEST(testSplitItinsMotherTheSameWith2NewFamilies);
  CPPUNIT_TEST(testSplitItinsChildTheSame);
  CPPUNIT_TEST(testSplitItinsChildWithTheSameStatus);

  CPPUNIT_TEST(testPlusUpSumVsFCSum_zero);
  CPPUNIT_TEST(testPlusUpSumVsFCSum_notHipExc);
  CPPUNIT_TEST(testPlusUpSumVsFCSum_HipExc);
  CPPUNIT_TEST(testPlusUpSumVsFCSum_HipNotExc);
  CPPUNIT_TEST(testPlusUpSumVsFCSum_DiffEqual);
  CPPUNIT_TEST(testPlusUpSumVsFCSum_DiffNotEqual);

  CPPUNIT_TEST(testDiscoverThroughFarePrecedenceEmpty);
  CPPUNIT_TEST(testDiscoverThroughFarePrecedenceUnrelated);
  CPPUNIT_TEST(testDiscoverThroughFarePrecedenceWrongCountry);
  CPPUNIT_TEST(testDiscoverThroughFarePrecedenceWrongCarrier);
  CPPUNIT_TEST(testDiscoverThroughFarePrecedenceOk);

  CPPUNIT_TEST(testFailIfThroughFarePrecedenceImpossibleNoTFP_NoForce);
  CPPUNIT_TEST(testFailIfThroughFarePrecedenceImpossibleNoTFP_Force);
  CPPUNIT_TEST(testFailIfThroughFarePrecedenceImpossibleTFP_NoForce);
  CPPUNIT_TEST(testFailIfThroughFarePrecedenceImpossibleTFP_Force);

  CPPUNIT_TEST(testBuildFareMarketForAncillaryPricingTrx);

  CPPUNIT_TEST(testIsJointUSCAFareMarket_Domestic);
  CPPUNIT_TEST(testIsJointUSCAFareMarket_Transborder);
  CPPUNIT_TEST(testIsJointUSCAFareMarket_NotUSCA);
  CPPUNIT_TEST(testIsJointUSCAFareMarket_SkipARNK);
  CPPUNIT_TEST(testIsJointUSCAFareMarket_FallBack);

  CPPUNIT_TEST(testSetInfoForSimilarItinsCallsSetFareFamilyIds);
  CPPUNIT_TEST(testSetFareFamilyIds);

  CPPUNIT_TEST(testCloneFareMarket);

  CPPUNIT_TEST(testIsValidSideTripCombinationTrueWhenValid);
  CPPUNIT_TEST(testIsValidSideTripCombinationTrueWhenValid1);
  CPPUNIT_TEST(testIsValidSideTripCombinationTrueWhenValid2);
  CPPUNIT_TEST(testIsValidSideTripCombinationFalseWhenNotValid);
  CPPUNIT_TEST(testIsValidSideTripCombinationFalseWhenNotValid1);
  CPPUNIT_TEST(testIsValidSideTripCombinationFalseWhenNotValid2);

  CPPUNIT_TEST(testCopyFromInvalidSideTripWhenEmptyNewSideTrip);
  CPPUNIT_TEST(testCopyFromInvalidSideTripWhenInvalidNewSideTrip);

  CPPUNIT_TEST(testSetValidatingCarrierWorksOnAltDatesItins);

  CPPUNIT_TEST(testGetInboundOutbound_StartFromOriginPoint);
  CPPUNIT_TEST(testGetInboundOutbound_InternationalJourney_ReturnToOrigin_Domestic);
  CPPUNIT_TEST(testGetInboundOutbound_InternationalJourney_ReturnToOrigin_ForeignDomestic);
  CPPUNIT_TEST(testGetInboundOutbound_InternationalJourney_ReturnToOrigin_Transborder);
  CPPUNIT_TEST(testGetInboundOutbound_InternationalJourney_ReturnToOrigin_International);

  CPPUNIT_TEST(testMarkFlownSegments);

  CPPUNIT_TEST(testProcessFMsForValidatingCarriers);

  CPPUNIT_TEST(testAddFakeTravelSeg_emptyDepartureDates);
  CPPUNIT_TEST(testAddFakeTravelSeg_bothDepartureDates);
  CPPUNIT_TEST(testAddFakeTravelSeg_emptyItin);
  CPPUNIT_TEST(testAddFakeTravelSeg_fakeOnOutbound_timeFromDepartureDate);
  CPPUNIT_TEST(testAddFakeTravelSeg_fakeOnOutbound_timeDependsOnAdjacentSegment);
  CPPUNIT_TEST(testAddFakeTravelSeg_fakeOnInbound_timeFromDepartureDate);
  CPPUNIT_TEST(testAddFakeTravelSeg_fakeOnInbound_timeDependsOnAdjacentSegment);
  CPPUNIT_TEST(testZeroOutIntlFlightSecondaryRBD_IntlFlightWithPrimaryBC);
  CPPUNIT_TEST(testDoNotZeroOutIntlFlightSecondaryRBD_IntlFlightWithoutPrimaryBC);
  CPPUNIT_TEST(testDoNotZeroOutDomesticFlightSecondaryRBD_IntlFlightWithPrimaryBC);
  CPPUNIT_TEST(testDoNotZeroOutDummyFlightSecondaryRBD_IntlFlightWithPrimaryRBD);
  CPPUNIT_TEST(testZeroOutSecondaryRBD_GoodItinWithIntlFlightPrimaryRBD);
  CPPUNIT_TEST(testVerifyFMsSharedAcrossItins);
  CPPUNIT_TEST(testVerifyBadItinContainsUniqueFMs);
  CPPUNIT_TEST(testVerifyInvalidItin_IntlDummyFlightWithPrimaryRBD);
  CPPUNIT_TEST(testVerifyInvalidItins_IntlFlightsWithNoPrimaryBC);
  CPPUNIT_TEST(testZeroOutSecondaryRBD_ItinWithDomesticFlights);
  CPPUNIT_TEST(testDoNotZeroOutSecondaryRBD_DummyDomesticFlightWithPrimaryBC);
  CPPUNIT_TEST(testZeroOutSecondaryRBD_DummyDomesticFlightWithPrimaryBC);
  CPPUNIT_TEST(testZeroOutSecondaryRBD_ItinWithDomesticFlights_NoPrimaryBC);
  CPPUNIT_TEST(testsetAirSegWithReqSpecificCabin);
  CPPUNIT_TEST(testsetAirSegWithReqSpecificCabinThrowException);
  CPPUNIT_TEST(testisItinOutsideNetherlandAntilles);
  CPPUNIT_TEST(testisItinOutsideEurope);
  CPPUNIT_TEST(testisInternationalItin);

  // CPPUNIT_TEST(testIsPopulateFareMarkets_True);
  // CPPUNIT_TEST(testIsPopulateFareMarkets_False);
  // CPPUNIT_TEST(testIsItinHasValidatingCxrData_True);
  // CPPUNIT_TEST(testIsItinHasValidatingCxrData_False);

  CPPUNIT_TEST_SUITE_END();

public:
  ItinAnalyzerServiceTest() {}

  typedef std::vector<TravelSeg*> TravelSegs;

  void setUp()
  {
    _tseServer = _memHandle.create<MockTseServer>();
    fallback::value::fallbackLatamJourneyActivation.set(true);
    _itinAnalyzer = _memHandle(new ItinAnalyzerService("ITIN_SVC", *_tseServer));
    _itinAnalyzerMock = _memHandle(new ItinAnalyzerServiceMock("ITIN_SVC", *_tseServer));
    _pricingTrx = _memHandle.create<PricingTrx>();
    _rexTrx = _memHandle.create<RexPricingTrx>();
    _excTrx = _memHandle.create<ExchangePricingTrx>();
    _excItin = _memHandle.create<ExcItin>();
    _newItin = _memHandle.create<Itin>();
    _request = _memHandle.create<PricingRequest>();
    _option = _memHandle.create<PricingOptions>();
    _memHandle.create<MyDataHandle>();

    _trx = _memHandle.create<PricingTrx>();
    ActivationResult* acResult = _memHandle.create<ActivationResult>();
    acResult->isActivationFlag() = true;
    _trx->projCACMapData().insert(std::make_pair("RTW", acResult));

    _trxUtilMock = new TrxUtilMock();
  }

  void tearDown()
  {
    _memHandle.clear();
    _excTrx->exchangeItin().clear();
    _excTrx->newItin().clear();
  }

  void testItinAnalyzer()
  {
    try
    {
      preparePricingTrx();
      // A simple round trip
      AirSeg airSeg1;
      airSeg1.origAirport() = "DFW";
      airSeg1.departureDT() = DateTime(2005, 7, 20);
      airSeg1.origin() =
          _pricingTrx->dataHandle().getLoc(airSeg1.origAirport(), airSeg1.departureDT());
      airSeg1.destAirport() = "LAX";
      airSeg1.arrivalDT() = DateTime(2005, 7, 20);
      airSeg1.destination() =
          _pricingTrx->dataHandle().getLoc(airSeg1.destAirport(), airSeg1.arrivalDT());
      airSeg1.carrier() = "AA";

      AirSeg airSeg2;
      airSeg2.origAirport() = "LAX";
      airSeg2.departureDT() = DateTime(2005, 7, 21);
      airSeg2.origin() =
          _pricingTrx->dataHandle().getLoc(airSeg2.origAirport(), airSeg2.departureDT());
      airSeg2.destAirport() = "DFW";
      airSeg2.arrivalDT() = DateTime(2005, 7, 21);
      airSeg2.destination() =
          _pricingTrx->dataHandle().getLoc(airSeg2.destAirport(), airSeg2.arrivalDT());
      airSeg2.carrier() = "AA";

      // Build itinerary
      Itin itn, shitn;
      itn.travelSeg() += &airSeg1, &airSeg2;
      shitn.travelSeg() += &airSeg1, &airSeg2;

      _pricingTrx->itin().push_back(&itn);
      _pricingTrx->travelSeg().push_back(itn.travelSeg().front());

      PaxType paxType;
      paxType.paxType() = "ADT";
      paxType.number() = 1;
      PaxTypeInfo paxTypeInfo;
      paxTypeInfo.numberSeatsReq() = 1;
      paxType.paxTypeInfo() = &paxTypeInfo;
      _pricingTrx->paxType().push_back(&paxType);

      ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

      PricingRequest* pricingRequest;
      _pricingTrx->dataHandle().get(pricingRequest);

      pricingRequest->ticketingAgent() = _pricingTrx->getRequest()->ticketingAgent();
      shoppingTrx->setRequest(pricingRequest);
      shoppingTrx->setOptions(_pricingTrx->getOptions());
      shoppingTrx->billing() = _pricingTrx->billing();
      shoppingTrx->status() = _pricingTrx->status();
      shoppingTrx->itin().push_back(&shitn);
      shoppingTrx->legs().push_back(ShoppingTrx::Leg());
      shoppingTrx->legs().front().sop().push_back(ShoppingTrx::SchedulingOption(&shitn, 1, true));
      shoppingTrx->travelSeg().push_back(itn.travelSeg().front());
      Global::config().read("../../Server/tseserver.cfg");
      ItinAnalyzerService itin("ITIN_SVC", *_tseServer);
      char** dummy = 0;
      itin.initialize(1, dummy);
      // itin.selectTicketingCarrier(_pricingTrx);
      // CPPUNIT_ASSERT(itn.ticketingCarrier()== "AA");

      CPPUNIT_ASSERT_NO_THROW(itin.process(*_pricingTrx));
      itin.process(*shoppingTrx);

      ShoppingTrx::Leg& curLeg = shoppingTrx->legs().front();
      ShoppingTrx::SchedulingOption& curSop = curLeg.sop().front();
      Itin*& curItin = curSop.itin();

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Transaction itin size is not 1", int(_pricingTrx->itin().size()), 1);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "ShoppingTrx leg sop size is not 1", int(curLeg.sop().size()), 1);

      // Check for one fare market in the shopping trx
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Itinerary contains no fare markets", bool(curItin->fareMarket().empty()), false);
    }
    catch (std::exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE(e.what(), false);
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("General catch: caught exception!", false);
    }
  }

  void testPremiumCabin()
  {
    ShoppingTrx trx;
    BookingCode bc = "Y";

    trx.setOptions(_memHandle.create<PricingOptions>());
    trx.legs().push_back(ShoppingTrx::Leg());
    trx.legs().push_back(ShoppingTrx::Leg());

    ClassOfService oneEconomyClass;
    oneEconomyClass.numSeats() = 1;
    oneEconomyClass.cabin().setEconomyClass();
    oneEconomyClass.bookingCode() = bc;

    ClassOfService twoEconomyClasses;
    twoEconomyClasses.numSeats() = 2;
    twoEconomyClasses.cabin().setEconomyClass();
    twoEconomyClasses.bookingCode() = bc;

    ClassOfService onePremiumEconomyClass;
    onePremiumEconomyClass.numSeats() = 1;
    onePremiumEconomyClass.cabin().setPremiumEconomyClass();
    onePremiumEconomyClass.bookingCode() = bc;

    ClassOfService twoPremiumEconomyClasses;
    twoPremiumEconomyClasses.numSeats() = 2;
    twoPremiumEconomyClasses.cabin().setPremiumEconomyClass();
    twoPremiumEconomyClasses.bookingCode() = bc;

    ClassOfService oneBusinessClass;
    oneBusinessClass.numSeats() = 1;
    oneBusinessClass.cabin().setBusinessClass();
    oneBusinessClass.bookingCode() = bc;

    ClassOfService twoBusinessClasses;
    twoBusinessClasses.numSeats() = 2;
    twoBusinessClasses.cabin().setBusinessClass();
    twoBusinessClasses.bookingCode() = bc;

    ClassOfService oneFirstClass;
    oneFirstClass.numSeats() = 1;
    oneFirstClass.cabin().setFirstClass();
    oneFirstClass.bookingCode() = bc;

    ClassOfService twoFirstClasses;
    twoFirstClasses.numSeats() = 2;
    twoFirstClasses.cabin().setFirstClass();
    twoFirstClasses.bookingCode() = bc;

    ClassOfService onePremiumFirstClass;
    onePremiumFirstClass.numSeats() = 1;
    onePremiumFirstClass.cabin().setPremiumFirstClass();
    onePremiumFirstClass.bookingCode() = bc;

    ClassOfService twoPremiumFirstClasses;
    twoPremiumFirstClasses.numSeats() = 2;
    twoPremiumFirstClasses.cabin().setPremiumFirstClass();
    twoPremiumFirstClasses.bookingCode() = bc;

    // A multi segment trip
    AirSeg airsegs[5];
    airsegs[0].origAirport() = "DFW";
    airsegs[0].departureDT() = DateTime(2005, 8, 1);
    airsegs[0].origin() =
        trx.dataHandle().getLoc(airsegs[0].origAirport(), airsegs[0].departureDT());
    airsegs[0].destAirport() = "MIA";
    airsegs[0].arrivalDT() = DateTime(2005, 8, 1);
    airsegs[0].destination() =
        trx.dataHandle().getLoc(airsegs[0].destAirport(), airsegs[0].arrivalDT());
    airsegs[0].carrier() = "DL";

    airsegs[1].origAirport() = "MIA";
    airsegs[1].departureDT() = DateTime(2005, 8, 1);
    airsegs[1].origin() =
        trx.dataHandle().getLoc(airsegs[1].origAirport(), airsegs[1].departureDT());
    airsegs[1].destAirport() = "LAX";
    airsegs[1].arrivalDT() = DateTime(2005, 8, 1);
    airsegs[1].destination() =
        trx.dataHandle().getLoc(airsegs[1].destAirport(), airsegs[1].arrivalDT());
    airsegs[1].carrier() = "DL";

    airsegs[2].origAirport() = "LAX";
    airsegs[2].departureDT() = DateTime(2005, 8, 1);
    airsegs[2].origin() =
        trx.dataHandle().getLoc(airsegs[2].origAirport(), airsegs[2].departureDT());
    airsegs[2].destAirport() = "SFO";
    airsegs[2].arrivalDT() = DateTime(2005, 8, 1);
    airsegs[2].destination() =
        trx.dataHandle().getLoc(airsegs[2].destAirport(), airsegs[2].arrivalDT());
    airsegs[2].carrier() = "DL";

    airsegs[4].origAirport() = "DFW";
    airsegs[4].departureDT() = DateTime(2005, 8, 1);
    airsegs[4].origin() =
        trx.dataHandle().getLoc(airsegs[2].origAirport(), airsegs[2].departureDT());
    airsegs[4].destAirport() = "LAX";
    airsegs[4].arrivalDT() = DateTime(2005, 8, 1);
    airsegs[4].destination() =
        trx.dataHandle().getLoc(airsegs[2].destAirport(), airsegs[2].arrivalDT());
    airsegs[4].carrier() = "DL";

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[4]);
    sh_itn_1.travelSeg().push_back(&airsegs[2]);

    Itin sh_itn_2;
    sh_itn_2.travelSeg().push_back(&airsegs[0]);
    sh_itn_2.travelSeg().push_back(&airsegs[1]);
    sh_itn_2.travelSeg().push_back(&airsegs[2]);

    ShoppingTrx::SchedulingOption sop1(&sh_itn_1, 1);
    ShoppingTrx::SchedulingOption sop2(&sh_itn_2, 2);

    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);

    CabinType economyCabinClass;
    economyCabinClass.setEconomyClass();
    CabinType businessCabinClass;
    businessCabinClass.setBusinessClass();
    CabinType firstCabinClass;
    firstCabinClass.setFirstClass();

    //-----------------------------------------------------------------------
    //      Requested Economy class - two Economy Classes present
    //-----------------------------------------------------------------------

    std::map<CabinType, bool> cabinsOffered;
    std::map<CabinType, bool> cabinsAvailable;

    bool lastLeg = true;
    bool requestedCabinSopFound = false;
    bool nonRequestedCabinSopFound = false;

    airsegs[2].classOfService().clear();
    airsegs[2].classOfService().push_back(&twoEconomyClasses);

    airsegs[4].classOfService().clear();
    airsegs[4].classOfService().push_back(&twoEconomyClasses);

    itinAnalyzer.validateSOPPremiumTravSegVector(trx,
                                                 sop1,
                                                 economyCabinClass,
                                                 2,
                                                 lastLeg,
                                                 requestedCabinSopFound,
                                                 nonRequestedCabinSopFound,
                                                 cabinsOffered,
                                                 cabinsAvailable);

    CPPUNIT_ASSERT(requestedCabinSopFound == true);

    requestedCabinSopFound = false;
    nonRequestedCabinSopFound = false;

    CPPUNIT_ASSERT(airsegs[2].classOfService().size() == 1);
    CPPUNIT_ASSERT(airsegs[4].classOfService().size() == 1);
    CPPUNIT_ASSERT(sop1.cabinClassValid() == true);

    //-----------------------------------------------------------------------
    //      Requested Economy class - two Business Classes present
    //-----------------------------------------------------------------------
    requestedCabinSopFound = false;
    nonRequestedCabinSopFound = false;

    airsegs[2].classOfService().clear();
    airsegs[2].classOfService().push_back(&twoBusinessClasses);

    airsegs[4].classOfService().clear();
    airsegs[4].classOfService().push_back(&twoBusinessClasses);

    itinAnalyzer.validateSOPPremiumTravSegVector(trx,
                                                 sop1,
                                                 economyCabinClass,
                                                 2,
                                                 lastLeg,
                                                 requestedCabinSopFound,
                                                 nonRequestedCabinSopFound,
                                                 cabinsOffered,
                                                 cabinsAvailable);

    CPPUNIT_ASSERT(requestedCabinSopFound == false);
    CPPUNIT_ASSERT(nonRequestedCabinSopFound == true);

    CPPUNIT_ASSERT(airsegs[2].classOfService().size() == 1);
    CPPUNIT_ASSERT(airsegs[4].classOfService().size() == 1);
    CPPUNIT_ASSERT(sop1.cabinClassValid() == true);

    //-----------------------------------------------------------------------
    //      Requested Business class - all classes available
    //-----------------------------------------------------------------------

    requestedCabinSopFound = false;
    nonRequestedCabinSopFound = false;

    airsegs[0].classOfService().clear();
    airsegs[0].classOfService().push_back(&twoBusinessClasses);
    airsegs[0].classOfService().push_back(&twoEconomyClasses);
    airsegs[0].classOfService().push_back(&twoFirstClasses);

    airsegs[1].classOfService().clear();
    airsegs[1].classOfService().push_back(&twoBusinessClasses);
    airsegs[1].classOfService().push_back(&twoEconomyClasses);
    airsegs[1].classOfService().push_back(&twoFirstClasses);

    airsegs[2].classOfService().clear();
    airsegs[2].classOfService().push_back(&twoBusinessClasses);
    airsegs[2].classOfService().push_back(&twoEconomyClasses);
    airsegs[2].classOfService().push_back(&twoFirstClasses);

    itinAnalyzer.validateSOPPremiumTravSegVector(trx,
                                                 sop2,
                                                 businessCabinClass,
                                                 2,
                                                 lastLeg,
                                                 requestedCabinSopFound,
                                                 nonRequestedCabinSopFound,
                                                 cabinsOffered,
                                                 cabinsAvailable);

    CPPUNIT_ASSERT(requestedCabinSopFound == true);
    CPPUNIT_ASSERT(nonRequestedCabinSopFound == false);

    CPPUNIT_ASSERT(airsegs[0].classOfService().size() == 2);
    CPPUNIT_ASSERT(airsegs[1].classOfService().size() == 2);
    CPPUNIT_ASSERT(airsegs[2].classOfService().size() == 2);
    CPPUNIT_ASSERT(sop2.cabinClassValid() == true);

    //-----------------------------------------------------------------------
    //      Requested Business class - mixed classes available
    //-----------------------------------------------------------------------

    requestedCabinSopFound = false;
    nonRequestedCabinSopFound = false;

    airsegs[0].classOfService().clear();
    airsegs[0].classOfService().push_back(&twoFirstClasses);
    airsegs[0].classOfService().push_back(&twoBusinessClasses);
    airsegs[0].classOfService().push_back(&twoEconomyClasses);

    airsegs[1].classOfService().clear();
    airsegs[1].classOfService().push_back(&twoFirstClasses);

    airsegs[2].classOfService().clear();
    airsegs[2].classOfService().push_back(&twoEconomyClasses);
    airsegs[2].classOfService().push_back(&twoFirstClasses);

    itinAnalyzer.validateSOPPremiumTravSegVector(trx,
                                                 sop2,
                                                 businessCabinClass,
                                                 2,
                                                 lastLeg,
                                                 requestedCabinSopFound,
                                                 nonRequestedCabinSopFound,
                                                 cabinsOffered,
                                                 cabinsAvailable);

    CPPUNIT_ASSERT(requestedCabinSopFound == true);
    CPPUNIT_ASSERT(nonRequestedCabinSopFound == true);

    CPPUNIT_ASSERT(airsegs[0].classOfService().size() == 2);
    CPPUNIT_ASSERT(airsegs[1].classOfService().size() == 1);
    CPPUNIT_ASSERT(airsegs[2].classOfService().size() == 2);
    CPPUNIT_ASSERT(sop2.cabinClassValid() == true);

    //-----------------------------------------------------------------------
    //      Requested Business class - mixed classes available (less than requested seats)
    //-----------------------------------------------------------------------

    requestedCabinSopFound = false;
    nonRequestedCabinSopFound = false;

    airsegs[0].classOfService().clear();
    airsegs[0].classOfService().push_back(&oneBusinessClass);
    airsegs[0].classOfService().push_back(&twoEconomyClasses);

    airsegs[1].classOfService().clear();
    airsegs[1].classOfService().push_back(&twoFirstClasses);

    airsegs[2].classOfService().clear();
    airsegs[2].classOfService().push_back(&twoEconomyClasses);
    airsegs[2].classOfService().push_back(&twoFirstClasses);

    itinAnalyzer.validateSOPPremiumTravSegVector(trx,
                                                 sop2,
                                                 businessCabinClass,
                                                 2,
                                                 lastLeg,
                                                 requestedCabinSopFound,
                                                 nonRequestedCabinSopFound,
                                                 cabinsOffered,
                                                 cabinsAvailable);

    CPPUNIT_ASSERT(sop2.cabinClassValid() == false);
    CPPUNIT_ASSERT(requestedCabinSopFound == true);

    CPPUNIT_ASSERT(airsegs[0].classOfService().size() == 2);
    CPPUNIT_ASSERT(airsegs[1].classOfService().size() == 1);
    CPPUNIT_ASSERT(airsegs[2].classOfService().size() == 2);

    //-----------------------------------------------------------------------
    //      Requested First class - one Business Class present
    //-----------------------------------------------------------------------
    requestedCabinSopFound = false;
    nonRequestedCabinSopFound = false;

    airsegs[2].classOfService().clear();
    airsegs[2].classOfService().push_back(&oneBusinessClass);

    airsegs[4].classOfService().clear();
    airsegs[4].classOfService().push_back(&oneBusinessClass);

    itinAnalyzer.validateSOPPremiumTravSegVector(trx,
                                                 sop1,
                                                 firstCabinClass,
                                                 2,
                                                 lastLeg,
                                                 requestedCabinSopFound,
                                                 nonRequestedCabinSopFound,
                                                 cabinsOffered,
                                                 cabinsAvailable);

    CPPUNIT_ASSERT(requestedCabinSopFound == false);
    CPPUNIT_ASSERT(nonRequestedCabinSopFound == true);

    CPPUNIT_ASSERT(airsegs[2].classOfService().size() == 1);
    CPPUNIT_ASSERT(airsegs[4].classOfService().size() == 1);
    CPPUNIT_ASSERT(sop1.cabinClassValid() == false);
  }

  void testInterlineAvlFareMarketCOS_SKSK()
  {
    TestConfigInitializer::setValue("INTERLINE_AVAILABILITY_CARRIERS", "SK", "SHOPPING_OPT");

    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());
    trx.setOptions(_option);

    ClassOfService classOfService[9];
    for (int i = 0; i < 9; ++i)
    {
      classOfService[i].numSeats() = (i % 2) ? 4 : 7;
      classOfService[i].cabin().setEconomyClass();
    }
    classOfService[0].bookingCode() = BookingCode("Y");
    classOfService[1].bookingCode() = BookingCode("M");
    classOfService[2].bookingCode() = BookingCode("Z");
    classOfService[3].bookingCode() = BookingCode("F");
    classOfService[4].bookingCode() = BookingCode("R");
    classOfService[5].bookingCode() = BookingCode("M");
    classOfService[6].bookingCode() = BookingCode("K");
    classOfService[7].bookingCode() = BookingCode("L");
    classOfService[8].bookingCode() = BookingCode("O");

    ClassOfServiceList cosList1;
    ClassOfServiceList cosList2;
    ClassOfServiceList cosList3;
    ClassOfServiceList cosList4;

    // thru 1+2
    cosList1.push_back(&classOfService[0]);
    cosList1.push_back(&classOfService[1]);
    cosList2.push_back(&classOfService[0]);
    cosList2.push_back(&classOfService[3]);
    // solo 1
    cosList3.push_back(&classOfService[0]);
    cosList3.push_back(&classOfService[5]);
    // solo 2
    cosList4.push_back(&classOfService[5]);
    cosList4.push_back(&classOfService[6]);

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;
    PricingTrx::ClassOfServiceKey cosKey2;
    PricingTrx::ClassOfServiceKey cosKey3;

    AirSeg airsegs[2];
    airsegs[0].setMarketingCarrierCode("SK");
    airsegs[0].originalId() = 0;
    airsegs[1].setMarketingCarrierCode("SK");
    airsegs[1].originalId() = 1;

    airsegs[0].classOfService() = cosList3; // solo 1
    airsegs[1].classOfService() = cosList4; // solo 2

    cosKey1.push_back(&airsegs[0]);
    cosKey1.push_back(&airsegs[1]);
    cosKey2.push_back(&airsegs[0]);
    cosKey3.push_back(&airsegs[1]);

    std::vector<ClassOfServiceList>* cosListVec1 =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();
    std::vector<ClassOfServiceList>* cosListVec2 =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();
    std::vector<ClassOfServiceList>* cosListVec3 =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();

    // thru
    cosListVec1->push_back(cosList1);
    cosListVec1->push_back(cosList2);
    // solo
    cosListVec2->push_back(cosList3);
    cosListVec3->push_back(cosList4);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec1; // thru
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey2)] = cosListVec2; // solo 1
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey3)] = cosListVec3; // solo 2

    FareMarket fm;
    fm.availBreaks().push_back(false);
    fm.availBreaks().push_back(true);
    fm.travelSeg().push_back(&airsegs[0]);
    fm.travelSeg().push_back(&airsegs[1]);

    Itin itin;
    itin.travelSeg().push_back(&airsegs[0]);
    itin.travelSeg().push_back(&airsegs[1]);
    itin.fareMarket().push_back(&fm);

    IntlJourneyUtil::constructJourneyInfo(trx, itin);
    ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, &itin, &fm);

    CPPUNIT_ASSERT_EQUAL(size_t(2), fm.classOfServiceVec().size());
    CPPUNIT_ASSERT_EQUAL(&(*cosListVec1)[0], fm.classOfServiceVec()[0]);
    CPPUNIT_ASSERT_EQUAL(&(*cosListVec1)[1], fm.classOfServiceVec()[1]);
  }

  void testInterlineAvlFareMarketCOS_SKUA()
  {
    TestConfigInitializer::setValue("INTERLINE_AVAILABILITY_CARRIERS", "SK", "SHOPPING_OPT");

    PricingTrx trx;
    trx.setOptions(_option);
    trx.setRequest(trx.dataHandle().create<PricingRequest>());
    trx.setTrxType(PricingTrx::MIP_TRX);

    ClassOfService classOfService[9];
    for (int i = 0; i < 9; ++i)
    {
      classOfService[i].numSeats() = (i % 2) ? 4 : 7;
      classOfService[i].cabin().setEconomyClass();
    }
    classOfService[0].bookingCode() = BookingCode("Y");
    classOfService[1].bookingCode() = BookingCode("M");
    classOfService[2].bookingCode() = BookingCode("Z");
    classOfService[3].bookingCode() = BookingCode("F");
    classOfService[4].bookingCode() = BookingCode("R");
    classOfService[5].bookingCode() = BookingCode("M");
    classOfService[6].bookingCode() = BookingCode("K");
    classOfService[7].bookingCode() = BookingCode("L");
    classOfService[8].bookingCode() = BookingCode("O");

    ClassOfServiceList cosList1;
    ClassOfServiceList cosList2;
    ClassOfServiceList cosList3;
    ClassOfServiceList cosList4;

    // thru 1+2
    cosList1.push_back(&classOfService[0]);
    cosList1.push_back(&classOfService[1]);
    cosList2.push_back(&classOfService[0]);
    cosList2.push_back(&classOfService[3]);
    // solo 1
    cosList3.push_back(&classOfService[0]);
    cosList3.push_back(&classOfService[5]);
    // solo 2
    cosList4.push_back(&classOfService[5]);
    cosList4.push_back(&classOfService[6]);

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;
    PricingTrx::ClassOfServiceKey cosKey2;
    PricingTrx::ClassOfServiceKey cosKey3;

    AirSeg airsegs[2];
    airsegs[0].setMarketingCarrierCode("SK");
    airsegs[0].originalId() = 0;
    airsegs[1].setMarketingCarrierCode("UA");
    airsegs[1].originalId() = 1;

    airsegs[0].classOfService() = cosList3; // solo 1
    airsegs[1].classOfService() = cosList4; // solo 2

    cosKey1.push_back(&airsegs[0]);
    cosKey1.push_back(&airsegs[1]);
    cosKey2.push_back(&airsegs[0]);
    cosKey3.push_back(&airsegs[1]);

    std::vector<ClassOfServiceList>* cosListVec1 =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();
    std::vector<ClassOfServiceList>* cosListVec2 =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();
    std::vector<ClassOfServiceList>* cosListVec3 =
        trx.dataHandle().create<std::vector<ClassOfServiceList>>();

    // thru
    cosListVec1->push_back(cosList1);
    cosListVec1->push_back(cosList2);
    // solo
    cosListVec2->push_back(cosList3);
    cosListVec3->push_back(cosList4);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec1; // thru
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey2)] = cosListVec2; // solo 1
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey3)] = cosListVec3; // solo 2

    FareMarket fm;
    fm.availBreaks().push_back(true);
    fm.availBreaks().push_back(true);
    fm.travelSeg().push_back(&airsegs[0]);
    fm.travelSeg().push_back(&airsegs[1]);

    Itin itin;
    itin.travelSeg().push_back(&airsegs[0]);
    itin.travelSeg().push_back(&airsegs[1]);
    itin.fareMarket().push_back(&fm);

    IntlJourneyUtil::constructJourneyInfo(trx, itin);
    ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, &itin, &fm);

    CPPUNIT_ASSERT_EQUAL(size_t(2), fm.classOfServiceVec().size());
    // CPPUNIT_ASSERT_EQUAL(&(*cosListVec1)[0], fm.classOfServiceVec()[0]);
    // TODO: CPPUNIT_ASSERT_EQUAL(&airsegs[1].classOfService(), fm.classOfServiceVec()[1]);
  }

  void setupInterlineAvlFM3(PricingTrx& trx,
                            ClassOfService& classOfService,
                            ClassOfServiceList cosList[],
                            PricingTrx::ClassOfServiceKey cosKey[],
                            AirSeg airsegs[],
                            std::vector<ClassOfServiceList>* cosListVec[])
  {
    TestConfigInitializer::setValue("INTERLINE_AVAILABILITY_CARRIERS", "SK", "SHOPPING_OPT");

    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    classOfService.numSeats() = 4;
    classOfService.cabin().setEconomyClass();
    classOfService.bookingCode() = BookingCode("M");

    // thru 1+2+3
    cosList[0].push_back(&classOfService);
    cosList[1].push_back(&classOfService);
    cosList[2].push_back(&classOfService);
    // thru 1+2
    cosList[3].push_back(&classOfService);
    cosList[4].push_back(&classOfService);
    // thru 2+3
    cosList[5].push_back(&classOfService);
    cosList[6].push_back(&classOfService);
    // solo
    cosList[7].push_back(&classOfService);
    cosList[8].push_back(&classOfService);
    cosList[9].push_back(&classOfService);

    airsegs[0].setMarketingCarrierCode("SK");
    airsegs[0].originalId() = 0;
    airsegs[1].setMarketingCarrierCode("SK");
    airsegs[1].originalId() = 1;
    airsegs[2].setMarketingCarrierCode("SK");
    airsegs[2].originalId() = 2;

    airsegs[0].classOfService() = cosList[7]; // solo 1
    airsegs[1].classOfService() = cosList[8]; // solo 2
    airsegs[2].classOfService() = cosList[9]; // solo 3

    cosKey[0].push_back(&airsegs[0]);
    cosKey[0].push_back(&airsegs[1]);
    cosKey[0].push_back(&airsegs[2]);

    cosKey[1].push_back(&airsegs[0]);
    cosKey[1].push_back(&airsegs[1]);

    cosKey[2].push_back(&airsegs[1]);
    cosKey[2].push_back(&airsegs[2]);

    cosKey[3].push_back(&airsegs[0]);
    cosKey[4].push_back(&airsegs[1]);
    cosKey[5].push_back(&airsegs[2]);

    for (int i = 0; i < 6; ++i)
      cosListVec[i] = trx.dataHandle().create<std::vector<ClassOfServiceList>>();

    // thru 1+2+3
    cosListVec[0]->push_back(cosList[0]);
    cosListVec[0]->push_back(cosList[1]);
    cosListVec[0]->push_back(cosList[2]);
    // thru 1+2
    cosListVec[1]->push_back(cosList[3]);
    cosListVec[1]->push_back(cosList[4]);
    // thru 2+3
    cosListVec[2]->push_back(cosList[5]);
    cosListVec[2]->push_back(cosList[6]);
    // solo
    cosListVec[3]->push_back(cosList[7]);
    cosListVec[4]->push_back(cosList[8]);
    cosListVec[5]->push_back(cosList[9]);

    // Build Availability map
    for (int i = 0; i < 6; ++i)
      trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey[i])] = cosListVec[i];
  }

  void testInterlineAvlFareMarketCOS_3SK()
  {
    PricingTrx trx;
    trx.setOptions(_option);
    trx.setTrxType(PricingTrx::MIP_TRX);
    ClassOfService classOfService;
    ClassOfServiceList cosList[10];
    PricingTrx::ClassOfServiceKey cosKey[6];
    AirSeg airsegs[3];
    std::vector<ClassOfServiceList>* cosListVec[6];

    setupInterlineAvlFM3(trx, classOfService, cosList, cosKey, airsegs, cosListVec);

    FareMarket fm;
    fm.availBreaks().push_back(false);
    fm.availBreaks().push_back(true);
    fm.availBreaks().push_back(true);
    fm.travelSeg().push_back(&airsegs[0]);
    fm.travelSeg().push_back(&airsegs[1]);
    fm.travelSeg().push_back(&airsegs[2]);

    Itin itin;
    itin.travelSeg().push_back(&airsegs[0]);
    itin.travelSeg().push_back(&airsegs[1]);
    itin.travelSeg().push_back(&airsegs[2]);
    itin.fareMarket().push_back(&fm);

    IntlJourneyUtil::constructJourneyInfo(trx, itin);
    ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, &itin, &fm);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fm.classOfServiceVec().size());
    // CPPUNIT_ASSERT_EQUAL(&(*cosListVec[0])[0], fm.classOfServiceVec()[0]);
    // CPPUNIT_ASSERT_EQUAL(&(*cosListVec[0])[1], fm.classOfServiceVec()[1]);
    // CPPUNIT_ASSERT_EQUAL(&(*cosListVec[0])[2], fm.classOfServiceVec()[2]);
  }

  void testInterlineAvlFareMarketCOS_2SK1UA()
  {
    PricingTrx trx;
    trx.setOptions(_option);
    trx.setTrxType(PricingTrx::MIP_TRX);
    ClassOfService classOfService;
    ClassOfServiceList cosList[10];
    PricingTrx::ClassOfServiceKey cosKey[6];
    AirSeg airsegs[3];
    std::vector<ClassOfServiceList>* cosListVec[6];

    setupInterlineAvlFM3(trx, classOfService, cosList, cosKey, airsegs, cosListVec);
    airsegs[2].setMarketingCarrierCode("UA");

    FareMarket fm;
    fm.availBreaks().push_back(false);
    fm.availBreaks().push_back(true);
    fm.availBreaks().push_back(true);
    fm.travelSeg().push_back(&airsegs[0]);
    fm.travelSeg().push_back(&airsegs[1]);
    fm.travelSeg().push_back(&airsegs[2]);

    Itin itin;
    itin.travelSeg().push_back(&airsegs[0]);
    itin.travelSeg().push_back(&airsegs[1]);
    itin.travelSeg().push_back(&airsegs[2]);
    itin.fareMarket().push_back(&fm);

    IntlJourneyUtil::constructJourneyInfo(trx, itin);
    ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, &itin, &fm);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fm.classOfServiceVec().size());
    // CPPUNIT_ASSERT_EQUAL(&(*cosListVec[0])[0], fm.classOfServiceVec()[0]);
    // CPPUNIT_ASSERT_EQUAL(&(*cosListVec[0])[1], fm.classOfServiceVec()[1]);
    CPPUNIT_ASSERT_EQUAL(&airsegs[2].classOfService(), fm.classOfServiceVec()[2]);
  }

  void testScheduleGrouping()
  {
    try
    {
      // Generate trips to be grouped
      ShoppingTrx trx;

      trx.setOptions(_memHandle.create<PricingOptions>());

      PricingRequest* request;
      trx.dataHandle().get(request);
      Agent* agent;
      trx.dataHandle().get(agent);
      request->ticketingAgent() = agent;
      agent->agentFunctions() = "YFH";
      agent->agentCity() = "HDQ";
      agent->agentDuty() = "8";
      agent->airlineDept() = "HDQ";
      agent->cxrCode() = "1S";
      agent->currencyCodeAgent() = "USD";
      agent->agentLocation() = trx.dataHandle().getLoc(agent->agentCity(), time(NULL));
      trx.setRequest(request);

      // Need billing record
      Billing* billing;
      trx.dataHandle().get(billing);
      billing->userPseudoCityCode() = "HDQ";
      billing->userStation() = "925";
      billing->userBranch() = "3470";
      billing->partitionID() = "AA";
      billing->userSetAddress() = "02BD09";
      billing->aaaCity() = "HDQ";
      billing->aaaSine() = "YFH";
      billing->serviceName() = "ITPRICE1";
      billing->actionCode() = "WPBET";
      trx.billing() = billing;

      // A simple round trip
      AirSeg airSeg1_1;
      airSeg1_1.origAirport() = "DFW";
      airSeg1_1.departureDT() = DateTime(2005, 7, 20);
      airSeg1_1.origin() =
          trx.dataHandle().getLoc(airSeg1_1.origAirport(), airSeg1_1.departureDT());
      airSeg1_1.destAirport() = "LAX";
      airSeg1_1.arrivalDT() = DateTime(2005, 7, 20);
      airSeg1_1.destination() =
          trx.dataHandle().getLoc(airSeg1_1.destAirport(), airSeg1_1.arrivalDT());
      airSeg1_1.carrier() = "AA";

      AirSeg airSeg1_2;
      airSeg1_2.origAirport() = "LAX";
      airSeg1_2.departureDT() = DateTime(2005, 7, 21);
      airSeg1_2.origin() =
          trx.dataHandle().getLoc(airSeg1_2.origAirport(), airSeg1_2.departureDT());
      airSeg1_2.destAirport() = "DFW";
      airSeg1_2.arrivalDT() = DateTime(2005, 7, 21);
      airSeg1_2.destination() =
          trx.dataHandle().getLoc(airSeg1_2.destAirport(), airSeg1_2.arrivalDT());
      airSeg1_2.carrier() = "AA";

      // Another simple trip
      AirSeg airSeg2_1;
      airSeg2_1.origAirport() = "DFW";
      airSeg2_1.departureDT() = DateTime(2005, 7, 20);
      airSeg2_1.origin() =
          trx.dataHandle().getLoc(airSeg2_1.origAirport(), airSeg2_1.departureDT());
      airSeg2_1.destAirport() = "LAX";
      airSeg2_1.arrivalDT() = DateTime(2005, 7, 20);
      airSeg2_1.destination() =
          trx.dataHandle().getLoc(airSeg2_1.destAirport(), airSeg2_1.arrivalDT());
      airSeg2_1.carrier() = "AA";

      AirSeg airSeg2_2;
      airSeg2_2.origAirport() = "LAX";
      airSeg2_2.departureDT() = DateTime(2005, 7, 21);
      airSeg2_2.origin() =
          trx.dataHandle().getLoc(airSeg2_2.origAirport(), airSeg2_2.departureDT());
      airSeg2_2.destAirport() = "SFO";
      airSeg2_2.arrivalDT() = DateTime(2005, 7, 21);
      airSeg2_2.destination() =
          trx.dataHandle().getLoc(airSeg2_2.destAirport(), airSeg2_2.arrivalDT());
      airSeg2_2.carrier() = "AA";

      // A multi segment trip
      AirSeg airsegs[3];
      airsegs[0].origAirport() = "DFW";
      airsegs[0].departureDT() = DateTime(2005, 8, 1);
      airsegs[0].origin() =
          trx.dataHandle().getLoc(airsegs[0].origAirport(), airsegs[0].departureDT());
      airsegs[0].destAirport() = "MIA";
      airsegs[0].arrivalDT() = DateTime(2005, 8, 1);
      airsegs[0].destination() =
          trx.dataHandle().getLoc(airsegs[0].destAirport(), airsegs[0].arrivalDT());
      airsegs[0].carrier() = "DL";

      airsegs[1].origAirport() = "MIA";
      airsegs[1].departureDT() = DateTime(2005, 8, 1);
      airsegs[1].origin() =
          trx.dataHandle().getLoc(airsegs[1].origAirport(), airsegs[1].departureDT());
      airsegs[1].destAirport() = "LAX";
      airsegs[1].arrivalDT() = DateTime(2005, 8, 1);
      airsegs[1].destination() =
          trx.dataHandle().getLoc(airsegs[1].destAirport(), airsegs[1].arrivalDT());
      airsegs[1].carrier() = "DL";

      airsegs[2].origAirport() = "LAX";
      airsegs[2].departureDT() = DateTime(2005, 8, 1);
      airsegs[2].origin() =
          trx.dataHandle().getLoc(airsegs[2].origAirport(), airsegs[2].departureDT());
      airsegs[2].destAirport() = "SFO";
      airsegs[2].arrivalDT() = DateTime(2005, 8, 1);
      airsegs[2].destination() =
          trx.dataHandle().getLoc(airsegs[2].destAirport(), airsegs[2].arrivalDT());
      airsegs[2].carrier() = "DL";

      // Build itinerary
      Itin sh_itn_1;
      sh_itn_1.travelSeg() += &airSeg1_1, &airSeg1_2;

      Itin sh_itn_2;
      sh_itn_2.travelSeg() += &airsegs[0], &airsegs[1], &airsegs[2];

      Itin sh_itn_3;
      sh_itn_3.travelSeg() += &airSeg2_1, &airSeg2_2;

      ShoppingTrx shoppingTrx;
      shoppingTrx.setRequest(trx.getRequest());
      shoppingTrx.setOptions(trx.getOptions());
      shoppingTrx.billing() = trx.billing();
      shoppingTrx.status() = trx.status();
      shoppingTrx.legs().push_back(ShoppingTrx::Leg());
      ShoppingTrx::Leg& leg = shoppingTrx.legs().back();
      leg.requestSops() = 3;
      leg.sop() += ShoppingTrx::SchedulingOption(&sh_itn_1, 1),
          ShoppingTrx::SchedulingOption(&sh_itn_2, 2), ShoppingTrx::SchedulingOption(&sh_itn_3, 3);

      // Process the transaction
      ItinAnalyzerService itin("ITIN_SVC", *_tseServer);
      itin.process(shoppingTrx);

      // Verify that the code worked
      ItinIndex& itinIndex = shoppingTrx.legs().front().carrierIndex();

      // Itin index should be empty because no pax types, booking codes,
      // or classes of service were properly setup.  This test needs
      // to be revamped to utilize the shopping xml request parser which will
      // create a transaction object, that will then be ready for acceptance
      // by the shopping processing function within the ItinAnalyzerService
      CPPUNIT_ASSERT_EQUAL_MESSAGE("ItinIndex is not empty", bool(itinIndex.root().empty()), true);
    }
    catch (std::exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE(e.what(), false);
    }
    catch (...)
    {
      CPPUNIT_ASSERT_MESSAGE("General catch: caught exception!", false);
    }
  }

  void initAirSeg(AirSeg& travelSeg,
                  GeoTravelType geoTravelType,
                  const Loc* orig,
                  const Loc* dest,
                  CarrierCode cxr)
  {
    travelSeg.geoTravelType() = geoTravelType;
    travelSeg.origin() = orig;
    travelSeg.destination() = dest;
    travelSeg.origAirport() = orig->loc();
    travelSeg.destAirport() = dest->loc();
    travelSeg.boardMultiCity() = orig->loc();
    travelSeg.offMultiCity() = dest->loc();
    travelSeg.carrier() = cxr;
    travelSeg.stopOver() = true;
  }

  void testBuildFareMarketForPricingTrx()
  {
    PricingTrx trx;
    testBuildFareMarket(trx);
  }

  void testBuildFareMarketForRexPricingTrxThroughProcess()
  {
    RexPricingTrx trx;
    testBuildFareMarket(trx, true);
  }

  void setUpItinForSideTrip(Trx& trx, Itin& itin)
  {
    AirSeg tSeg1, tSeg2, tSeg3, tSeg4, tSeg5, tSeg6, tSeg7, tSeg8;
    tSeg1.boardMultiCity() = "AAA";
    tSeg1.offMultiCity() = "BBB";
    tSeg2.boardMultiCity() = "BBB";
    tSeg2.offMultiCity() = "CCC";
    tSeg3.boardMultiCity() = "CCC";
    tSeg3.offMultiCity() = "DDD";
    tSeg4.boardMultiCity() = "DDD";
    tSeg4.offMultiCity() = "BBB";
    tSeg5.boardMultiCity() = "BBB";
    tSeg5.offMultiCity() = "EEE";
    tSeg6.boardMultiCity() = "EEE";
    tSeg6.offMultiCity() = "FFF";
    tSeg7.boardMultiCity() = "FFF";
    tSeg7.offMultiCity() = "EEE";
    tSeg8.boardMultiCity() = "EEE";
    tSeg8.offMultiCity() = "GGG";
    itin.travelSeg().push_back(&tSeg1);
    itin.travelSeg().push_back(&tSeg2);
    itin.travelSeg().push_back(&tSeg3);
    itin.travelSeg().push_back(&tSeg4);
    itin.travelSeg().push_back(&tSeg5);
    itin.travelSeg().push_back(&tSeg6);
    itin.travelSeg().push_back(&tSeg7);
    itin.travelSeg().push_back(&tSeg8);
  }

  void setUpItinForSideTrip2(Trx& trx, Itin& itin)
  {
    AirSeg tSeg1, tSeg2, tSeg3, tSeg4, tSeg5, tSeg6;
    tSeg1.boardMultiCity() = "AAA";
    tSeg1.offMultiCity() = "BBB";
    tSeg2.boardMultiCity() = "BBB";
    tSeg2.offMultiCity() = "CCC";
    tSeg3.boardMultiCity() = "CCC";
    tSeg3.offMultiCity() = "DDD";
    tSeg4.boardMultiCity() = "DDD";
    tSeg4.offMultiCity() = "CCC";
    tSeg5.boardMultiCity() = "CCC";
    tSeg5.offMultiCity() = "BBB";
    tSeg6.boardMultiCity() = "BBB";
    tSeg6.offMultiCity() = "FFF";

    itin.travelSeg().push_back(&tSeg1);
    itin.travelSeg().push_back(&tSeg2);
    itin.travelSeg().push_back(&tSeg3);
    itin.travelSeg().push_back(&tSeg4);
    itin.travelSeg().push_back(&tSeg5);
    itin.travelSeg().push_back(&tSeg6);
  }

  void setUpItinForSideTrip3(Trx& trx, Itin& itin)
  {
    AirSeg tSeg1, tSeg2, tSeg3, tSeg4, tSeg5;
    tSeg1.boardMultiCity() = "AAA";
    tSeg1.offMultiCity() = "BBB";
    tSeg2.boardMultiCity() = "BBB";
    tSeg2.offMultiCity() = "CCC";
    tSeg3.boardMultiCity() = "CCC";
    tSeg3.offMultiCity() = "BBB";
    tSeg4.boardMultiCity() = "BBB";
    tSeg4.offMultiCity() = "CCC";
    tSeg5.boardMultiCity() = "CCC";
    tSeg5.offMultiCity() = "FFF";

    itin.travelSeg().push_back(&tSeg1);
    itin.travelSeg().push_back(&tSeg2);
    itin.travelSeg().push_back(&tSeg3);
    itin.travelSeg().push_back(&tSeg4);
    itin.travelSeg().push_back(&tSeg5);
  }
  void setUpItinForSideTrip4(Trx& trx, Itin& itin)
  {
    AirSeg tSeg1, tSeg2, tSeg3, tSeg4, tSeg5, tSeg6;
    tSeg1.boardMultiCity() = "AAA";
    tSeg1.offMultiCity() = "BBB";
    tSeg2.boardMultiCity() = "BBB";
    tSeg2.offMultiCity() = "CCC";
    tSeg3.boardMultiCity() = "CCC";
    tSeg3.offMultiCity() = "BBB";
    tSeg4.boardMultiCity() = "BBB";
    tSeg4.offMultiCity() = "DDD";
    tSeg5.boardMultiCity() = "DDD";
    tSeg5.offMultiCity() = "CCC";
    tSeg6.boardMultiCity() = "CCC";
    tSeg6.offMultiCity() = "FFF";

    itin.travelSeg().push_back(&tSeg1);
    itin.travelSeg().push_back(&tSeg2);
    itin.travelSeg().push_back(&tSeg3);
    itin.travelSeg().push_back(&tSeg4);
    itin.travelSeg().push_back(&tSeg5);
    itin.travelSeg().push_back(&tSeg6);
  }

  void setUpItin(Trx& trx, Itin& itin)
  {
    // Initialize Location
    const Loc* loc1 = trx.dataHandle().getLoc("SCL", DateTime::localTime());
    const Loc* loc2 = trx.dataHandle().getLoc("MIA", DateTime::localTime());
    const Loc* loc3 = trx.dataHandle().getLoc("SDQ", DateTime::localTime());
    const Loc* loc4 = trx.dataHandle().getLoc("CCS", DateTime::localTime());

    AirSeg* travelSeg1, *travelSeg2, *travelSeg3, *travelSeg4;

    trx.dataHandle().get(travelSeg1);
    trx.dataHandle().get(travelSeg2);
    trx.dataHandle().get(travelSeg3);
    trx.dataHandle().get(travelSeg4);
    // Build Travel Segments SCL-AA-MIA-AA-SDQ-AA-MIA-CCS

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 31);
    initAirSeg(*travelSeg2, GeoTravelType::International, loc2, loc3, "AA");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg3, GeoTravelType::International, loc3, loc2, "AA");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg4, GeoTravelType::International, loc2, loc4, "AA");

    itin.travelSeg() += travelSeg1, travelSeg2, travelSeg3, travelSeg4;

    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(travelSeg1->departureDT());
  }

  void setUpItinForRtw(Trx& trx, Itin& itin)
  {
    // Initialize Location
    const Loc* loc1 = trx.dataHandle().getLoc("SIN", DateTime::localTime());
    const Loc* loc2 = trx.dataHandle().getLoc("DFW", DateTime::localTime());
    const Loc* loc3 = trx.dataHandle().getLoc("MIA", DateTime::localTime());
    const Loc* loc4 = trx.dataHandle().getLoc("LON", DateTime::localTime());
    const Loc* loc5 = trx.dataHandle().getLoc("SIN", DateTime::localTime());

    AirSeg* travelSeg1, *travelSeg2, *travelSeg3, *travelSeg4;

    trx.dataHandle().get(travelSeg1);
    trx.dataHandle().get(travelSeg2);
    trx.dataHandle().get(travelSeg3);
    trx.dataHandle().get(travelSeg4);

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 31);
    initAirSeg(*travelSeg2, GeoTravelType::International, loc2, loc3, "AA");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg3, GeoTravelType::International, loc3, loc4, "AA");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg4, GeoTravelType::International, loc4, loc5, "AA");

    itin.travelSeg() += travelSeg1, travelSeg2, travelSeg3, travelSeg4;

    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(travelSeg1->departureDT());
  }

  void setupDataForProcessFunction(Trx* parentTrx)
  {
    _excTrx->setRequest(_request);
    _excTrx->setOptions(_option);
    _excTrx->setParentTrx(parentTrx);
    _excTrx->exchangeItin().push_back(_excItin);
    setUpItin(*_excTrx, *_excItin);
    _excTrx->itin().push_back(_newItin);
    setUpItin(*_excTrx, *_newItin);
    _excTrx->travelSeg() += _newItin->travelSeg()[0], _newItin->travelSeg()[1],
        _newItin->travelSeg()[2], _newItin->travelSeg()[3];

    _paxType.paxType() = "ADT";
    _paxType.number() = 1;
    _paxTypeInfo.numberSeatsReq() = 1;
    _paxType.paxTypeInfo() = &_paxTypeInfo;
    _excTrx->paxType().push_back(&_paxType);
  }

  void testBuildFareMarket(PricingTrx& trx, bool throughProcess = false)
  {
    PricingRequest request;
    trx.setRequest(&request);
    PricingOptions* option;
    trx.dataHandle().get(option);
    trx.setOptions(option);

    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    paxType.paxTypeInfo() = &paxTypeInfo;
    trx.paxType().push_back(&paxType);

    // Build Itin
    Itin itin;
    setUpItin(trx, itin);

    // Add Itin to Trx
    trx.itin().push_back(&itin);
    trx.travelSeg() += itin.travelSeg()[0], itin.travelSeg()[1], itin.travelSeg()[2],
        itin.travelSeg()[3];

    if (throughProcess)
      _itinAnalyzerMock->process(trx);
    else
      _itinAnalyzerMock->buildFareMarket(trx);

    std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

    /*std::vector<FareMarket*>::iterator iter = fareMarkets.begin();

    int marketIndex = 1;
    while(iter != fareMarkets.end())
    {
    FareMarket *fm = (*iter);

    std::cout << "**************** FARE MARKET "<< marketIndex++ << std::endl;
    std::cout << "Board: "<<fm->boardMultiCity()<<"  Off: "<< fm->offMultiCity() << std::endl;
    std::cout << "  governing carrier: "<< fm->governingCarrier() << std::endl;
    std::cout << "  includes: "<< fm->travelSeg().size()<<" travel segments: "
    << _itinAnalyzerMock->getTravelSegOrderStr(itin, fm->travelSeg()) << std::endl;
    std::cout << "  includes: "<< fm->sideTripTravelSeg().size()<<" side trips: "
    << _itinAnalyzerMock->getTravelSegOrderStr(itin, fm->sideTripTravelSeg()) << std::endl;
    std::cout << "  breakIndicator: "<< fm->breakIndicator() << std::endl;

    iter++;
    }*/

    CPPUNIT_ASSERT(itin.fareMarket().size() == 8);

    // Test cloneFareMarket
    FareMarket newFareMarket;

    _itinAnalyzerMock->cloneFareMarket(*fareMarkets[2], newFareMarket, "UA");
    CPPUNIT_ASSERT(fareMarkets[2]->sideTripTravelSeg().size() ==
                   newFareMarket.sideTripTravelSeg().size());
    CPPUNIT_ASSERT(newFareMarket.governingCarrier() == "UA");
  }

  void testBuildFareMarketForRtwPricingTrxWrongNumberOfCarrierAllianceCodes1()
  {
    PricingRequest request;
    _trx->setRequest(&request);
    PricingOptions* option;
    _trx->dataHandle().get(option);
    _trx->setOptions(option);
    _trx->getOptions()->setRtw(true);

    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    paxType.paxTypeInfo() = &paxTypeInfo;
    _trx->paxType().push_back(&paxType);

    // Build Itin
    Itin itin;
    setUpItinForRtw(*_trx, itin);

    ((AirSeg*)itin.travelSeg()[0])->carrier() = "UA";
    ((AirSeg*)itin.travelSeg()[1])->carrier() = "UA";
    ((AirSeg*)itin.travelSeg()[2])->carrier() = "UA";
    ((AirSeg*)itin.travelSeg()[3])->carrier() = "UA";

    // Add Itin to Trx
    _trx->itin().push_back(&itin);
    _trx->travelSeg() += itin.travelSeg()[0], itin.travelSeg()[1], itin.travelSeg()[2],
        itin.travelSeg()[3];

    try
    {
      _itinAnalyzerMock->buildFareMarket(*_trx, false);
    }
    catch (const ErrorResponseException& e)
    {
      if (e.code() ==
          ErrorResponseException::RW_CT_FARE_NOT_APPLICABLE_USE_ALTERNATE_PRICING_COMMAND)
      {
        CPPUNIT_ASSERT(1);
        return;
      }
    }

    CPPUNIT_ASSERT(0);
  }

  void testBuildFareMarketForRtwPricingTrxWrongNumberOfCarrierAllianceCodes2()
  {
    PricingRequest request;
    _trx->setRequest(&request);
    PricingOptions* option;
    _trx->dataHandle().get(option);
    _trx->setOptions(option);
    _trx->getOptions()->setRtw(true);

    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    paxType.paxTypeInfo() = &paxTypeInfo;
    _trx->paxType().push_back(&paxType);

    // Build Itin
    Itin itin;
    setUpItinForRtw(*_trx, itin);

    ((AirSeg*)itin.travelSeg()[0])->carrier() = "LH";
    ((AirSeg*)itin.travelSeg()[1])->carrier() = "LH";
    ((AirSeg*)itin.travelSeg()[2])->carrier() = "LH";
    ((AirSeg*)itin.travelSeg()[3])->carrier() = "LH";

    // Add Itin to Trx
    _trx->itin().push_back(&itin);
    _trx->travelSeg() += itin.travelSeg()[0], itin.travelSeg()[1], itin.travelSeg()[2],
        itin.travelSeg()[3];

    try
    {
      _itinAnalyzerMock->buildFareMarket(*_trx, false);
    }
    catch (const ErrorResponseException& e)
    {
      if (e.code() ==
          ErrorResponseException::RW_CT_FARE_NOT_APPLICABLE_USE_ALTERNATE_PRICING_COMMAND)
      {
        CPPUNIT_ASSERT(0);
        return;
      }
    }

    CPPUNIT_ASSERT(1);
  }

  void testBuildFareMarketForRtwPricingTrxAunkAtEnd()
  {
    PricingRequest request;
    _trx->setRequest(&request);
    PricingOptions* option;
    _trx->dataHandle().get(option);
    _trx->setOptions(option);
    _trx->getOptions()->setRtw(true);

    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    paxType.paxTypeInfo() = &paxTypeInfo;
    _trx->paxType().push_back(&paxType);

    // Build Itin
    Itin itin;
    setUpItinForRtw(*_trx, itin);
    itin.travelSeg().erase(itin.travelSeg().end() - 1);

    // Add Itin to Trx
    _trx->itin().push_back(&itin);
    _trx->travelSeg() += itin.travelSeg()[0], itin.travelSeg()[1], itin.travelSeg()[2];

    try
    {
      _itinAnalyzerMock->buildFareMarket(*_trx, false);
    }
    catch (const ErrorResponseException& e)
    {
      if (e.code() ==
          ErrorResponseException::RW_CT_FARE_NOT_APPLICABLE_USE_ALTERNATE_PRICING_COMMAND)
      {
        CPPUNIT_ASSERT(0);
        return;
      }
    }

    itin.travelSeg().pop_back(); // We need to remove arunk
    itin.travelSeg().pop_back(); // and last airseg
    _trx->travelSeg().pop_back(); // Remove airseg from trx (arunk was not added here)

    try
    {
      _itinAnalyzerMock->buildFareMarket(*_trx, false);
    }
    catch (const ErrorResponseException& e)
    {
      if (e.code() ==
          ErrorResponseException::RW_CT_FARE_NOT_APPLICABLE_USE_ALTERNATE_PRICING_COMMAND)
      {
        CPPUNIT_ASSERT(0);
        return;
      }
    }

    CPPUNIT_ASSERT(1);
  }

  void testNonUSCADomesticTransborderForTag10Exchange()
  {
    MockItinAnalyzerService itinAnalyzerService("ITIN_SVC", *_tseServer);

    ExchangePricingTrx exchangeTrx;
    exchangeTrx.reqType() = "CE";

    Itin itin;
    ExcItin excItin;
    TravelSeg* airSeg1 = buildSegment(exchangeTrx, "AAA", "BBB", "AA");
    itin.travelSeg().push_back(airSeg1);
    excItin.travelSeg().push_back(airSeg1);
    itin.geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    exchangeTrx.itin().push_back(&itin);
    exchangeTrx.exchangeItin().push_back(&excItin);

    bool isException = false;

    try
    {
      itinAnalyzerService.exchangePricingTrxException(exchangeTrx);
    }

    catch (ErrorResponseException)
    {
      isException = true;
    }

    CPPUNIT_ASSERT(isException);
  }

  void testUSCADomesticForTag10Exchange()
  {
    MockItinAnalyzerService itinAnalyzerService("ITIN_SVC", *_tseServer);

    ExchangePricingTrx exchangeTrx;
    exchangeTrx.reqType() = "CE";

    Itin itin;
    ExcItin excItin;
    TravelSeg* airSeg1 = buildSegment(exchangeTrx, "AAA", "BBB", "AA");
    itin.travelSeg().push_back(airSeg1);
    excItin.travelSeg().push_back(airSeg1);
    itin.geoTravelType() = GeoTravelType::Domestic;
    exchangeTrx.itin().push_back(&itin);
    exchangeTrx.exchangeItin().push_back(&excItin);

    bool isException = false;

    try
    {
      itinAnalyzerService.exchangePricingTrxException(exchangeTrx);
    }

    catch (ErrorResponseException)
    {
      isException = true;
    }

    CPPUNIT_ASSERT(!isException);
  }

  void testUSCATransborderForTag10Exchange()
  {
    MockItinAnalyzerService itinAnalyzerService("ITIN_SVC", *_tseServer);

    ExchangePricingTrx exchangeTrx;
    exchangeTrx.reqType() = "CE";
    TravelSeg* airSeg1 = buildSegment(exchangeTrx, "AAA", "BBB", "AA");
    Itin itin;
    ExcItin excItin;
    itin.travelSeg().push_back(airSeg1);
    excItin.travelSeg().push_back(airSeg1);
    itin.geoTravelType() = GeoTravelType::Transborder;
    exchangeTrx.itin().push_back(&itin);
    exchangeTrx.exchangeItin().push_back(&excItin);

    bool isException = false;

    try
    {
      itinAnalyzerService.exchangePricingTrxException(exchangeTrx);
    }

    catch (ErrorResponseException)
    {
      isException = true;
    }

    CPPUNIT_ASSERT(!isException);
  }

  void testRemoveFMPartButNotWholeDummy()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3;

    FareMarket fm1, fm12, fm13, fm2, fm23, fm3;

    fm1.travelSeg().push_back(&tvlSeg1);
    fm12.travelSeg() += &tvlSeg1, &tvlSeg2;
    fm13.travelSeg() += &tvlSeg1, &tvlSeg2, &tvlSeg3;
    fm2.travelSeg().push_back(&tvlSeg2);
    fm23.travelSeg() += &tvlSeg2, &tvlSeg3;
    fm3.travelSeg().push_back(&tvlSeg3);

    ExchangePricingTrx excTrx;
    Itin itin;

    itin.fareMarket() += &fm1, &fm12, &fm13, &fm2, &fm23, &fm3;

    uint16_t dummyFCNum = 1;
    excTrx.exchangeOverrides().dummyFCSegs()[&tvlSeg2] = dummyFCNum;
    fm2.fareBasisCode() = "DUMMY";
    fm2.fareCalcFareAmt() = "123.45";

    ItinAnalyzerService::IsPartButNotWholeDummyFare isPartButNotWholeDummyFare(
        excTrx.exchangeOverrides().dummyFCSegs());

    itin.fareMarket().erase(std::remove_if(itin.fareMarket().begin(),
                                           itin.fareMarket().end(),
                                           isPartButNotWholeDummyFare),
                            itin.fareMarket().end());

    CPPUNIT_ASSERT(itin.fareMarket().size() == 3);
    CPPUNIT_ASSERT_EQUAL(itin.fareMarket()[0], &fm1);
    CPPUNIT_ASSERT_EQUAL(itin.fareMarket()[1], &fm2);
    CPPUNIT_ASSERT_EQUAL(itin.fareMarket()[2], &fm3);
  }

  FareMarket* buildFareMarket(Trx& trx, std::string origin, std::string destination)
  {
    const Loc* locOrigin = trx.dataHandle().getLoc(origin, DateTime::localTime());
    const Loc* locDestination = trx.dataHandle().getLoc(destination, DateTime::localTime());

    FareMarket* fareMarket;
    trx.dataHandle().get(fareMarket);

    fareMarket->origin() = locOrigin;
    fareMarket->destination() = locDestination;
    fareMarket->boardMultiCity() = origin;
    fareMarket->offMultiCity() = destination;
    fareMarket->travelDate() = DateTime::localTime();

    return fareMarket;
  }

  void buildRandomFareMarketsVector(Trx& trx, std::vector<FareMarket*>& fareMarkets)
  {
    fareMarkets += buildFareMarket(trx, "KRK", "DFW"), buildFareMarket(trx, "DFW", "ORD"),
        buildFareMarket(trx, "ORD", "NYC"), buildFareMarket(trx, "NYC", "HTR");
  }

  AirSeg* buildSegment(Trx& trx, std::string origin, std::string destination, std::string carrier)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();

    airSeg->origAirport() = origin;
    airSeg->departureDT() = DateTime::localTime();
    Loc* locOrig = _memHandle.create<Loc>();
    locOrig->loc() = airSeg->origAirport();
    airSeg->origin() = locOrig;

    airSeg->destAirport() = destination;
    airSeg->arrivalDT() = DateTime::localTime();
    Loc* locDest = _memHandle.create<Loc>();
    locDest->loc() = airSeg->destAirport();
    airSeg->destination() = locDest;

    airSeg->carrier() = carrier;

    return airSeg;
  }

  void testGetExistingFareMarketWithEmptyMarketVector()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "KRK", "DFW");

    std::vector<FareMarket*> fareMarkets;

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    FareMarket* resultFareMarket =
        itin.getExistingFareMarket(*shoppingTrx, *fareMarket, fareMarkets);

    // Check if fare market wasn't found in empty fare markets vector
    CPPUNIT_ASSERT(resultFareMarket == NULL);

    // Check if fare markets vector wasn't modified
    CPPUNIT_ASSERT(fareMarkets.size() == 0);
  }

  void testGetExistingFareMarketWithNotEmptyMarketVectorWhichDoesntContainNewMarket()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "AAA", "BBB");

    std::vector<FareMarket*> fareMarkets;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarkets);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    size_t size = fareMarkets.size();

    FareMarket* resultFareMarket =
        itin.getExistingFareMarket(*shoppingTrx, *fareMarket, fareMarkets);

    // Check if fare market wasn't found in fare markets vector
    CPPUNIT_ASSERT(resultFareMarket == NULL);

    // Check if fare markets vector wasn't modified
    CPPUNIT_ASSERT(fareMarkets.size() == size);
  }

  void testGetExistingFareMarketWithNotEmptyMarketVectorWhichContainsNewMarket()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "DFW", "ORD");

    std::vector<FareMarket*> fareMarkets;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarkets);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    size_t size = fareMarkets.size();

    FareMarket* resultFareMarket =
        itin.getExistingFareMarket(*shoppingTrx, *fareMarket, fareMarkets);

    // Check if fare market was found in fare markets vector
    CPPUNIT_ASSERT(resultFareMarket != NULL);

    // Check if it existing fare market wasn't replace by new fare market
    CPPUNIT_ASSERT(resultFareMarket != fareMarket);

    // Check if fare markets vector wasn't modified
    CPPUNIT_ASSERT(fareMarkets.size() == size);
  }

  void testAddFareMarketWithAllMarketVectorsEmpty()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "DFW", "ORD");

    std::vector<FareMarket*> fareMarketsTrx;
    std::vector<FareMarket*> fareMarketsCarrier;
    std::vector<FareMarket*> fareMarketsItinerary;

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    itin.addFareMarket(
        *shoppingTrx, fareMarket, fareMarketsTrx, fareMarketsCarrier, fareMarketsItinerary);

    // Check if new fare market was added to all vectors
    CPPUNIT_ASSERT(fareMarketsTrx.size() == 1);
    CPPUNIT_ASSERT(fareMarketsCarrier.size() == 1);
    CPPUNIT_ASSERT(fareMarketsItinerary.size() == 1);

    // Check if in each fare markets vector we have the same pointer to new fare
    // market
    CPPUNIT_ASSERT(fareMarketsTrx.end() !=
                   find(fareMarketsTrx.begin(), fareMarketsTrx.end(), fareMarket));
    CPPUNIT_ASSERT(fareMarketsCarrier.end() !=
                   find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), fareMarket));
    CPPUNIT_ASSERT(fareMarketsItinerary.end() !=
                   find(fareMarketsItinerary.begin(), fareMarketsItinerary.end(), fareMarket));
  }

  void testAddFareMarketWithAllMarketVectorsNotEmptyAndWithoutNewFareMarket()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "AAA", "BBB");

    std::vector<FareMarket*> fareMarketsTrx;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsTrx);

    std::vector<FareMarket*> fareMarketsCarrier;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsCarrier);

    std::vector<FareMarket*> fareMarketsItinerary;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsItinerary);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    size_t sizeTrx = fareMarketsTrx.size();
    size_t sizeCarrier = fareMarketsCarrier.size();
    size_t sizeItinerary = fareMarketsItinerary.size();

    itin.addFareMarket(
        *shoppingTrx, fareMarket, fareMarketsTrx, fareMarketsCarrier, fareMarketsItinerary);

    // Check if new fare market was added to all vectors
    CPPUNIT_ASSERT(fareMarketsTrx.size() == sizeTrx + 1);
    CPPUNIT_ASSERT(fareMarketsCarrier.size() == sizeCarrier + 1);
    CPPUNIT_ASSERT(fareMarketsItinerary.size() == sizeItinerary + 1);

    // Check if in each fare markets vector we have the same pointer to new fare
    // market
    CPPUNIT_ASSERT(fareMarketsTrx.end() !=
                   find(fareMarketsTrx.begin(), fareMarketsTrx.end(), fareMarket));
    CPPUNIT_ASSERT(fareMarketsCarrier.end() !=
                   find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), fareMarket));
    CPPUNIT_ASSERT(fareMarketsItinerary.end() !=
                   find(fareMarketsItinerary.begin(), fareMarketsItinerary.end(), fareMarket));
  }

  void testAddFareMarketWithTrxVectorWhichContainsNewFareMarket()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "AAA", "BBB");
    FareMarket* fareMarketExisting = buildFareMarket(*shoppingTrx, "AAA", "BBB");

    std::vector<FareMarket*> fareMarketsTrx;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsTrx);
    fareMarketsTrx.push_back(fareMarketExisting);

    std::vector<FareMarket*> fareMarketsCarrier;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsCarrier);

    std::vector<FareMarket*> fareMarketsItinerary;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsItinerary);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    size_t sizeTrx = fareMarketsTrx.size();
    size_t sizeCarrier = fareMarketsCarrier.size();
    size_t sizeItinerary = fareMarketsItinerary.size();

    itin.addFareMarket(
        *shoppingTrx, fareMarket, fareMarketsTrx, fareMarketsCarrier, fareMarketsItinerary);

    // Check if new fare market was added to all vectors exept trx vector
    CPPUNIT_ASSERT(fareMarketsTrx.size() == sizeTrx);
    CPPUNIT_ASSERT(fareMarketsCarrier.size() == sizeCarrier + 1);
    CPPUNIT_ASSERT(fareMarketsItinerary.size() == sizeItinerary + 1);

    // Check if apropriate fare market could be found in fare markets trx
    FareMarket* resultFareMarket =
        itin.getExistingFareMarket(*shoppingTrx, *fareMarket, fareMarketsTrx);

    CPPUNIT_ASSERT(NULL != resultFareMarket);

    if (NULL != resultFareMarket)
    {
      // Check if new fare market pointer wasn't add to any fare markets
      // vector
      CPPUNIT_ASSERT(fareMarketsTrx.end() ==
                     find(fareMarketsTrx.begin(), fareMarketsTrx.end(), fareMarket));
      CPPUNIT_ASSERT(fareMarketsCarrier.end() ==
                     find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), fareMarket));
      CPPUNIT_ASSERT(fareMarketsItinerary.end() ==
                     find(fareMarketsItinerary.begin(), fareMarketsItinerary.end(), fareMarket));

      // Check if existing fare market pointer could be found in all fare
      // markets vector
      CPPUNIT_ASSERT(fareMarketsTrx.end() !=
                     find(fareMarketsTrx.begin(), fareMarketsTrx.end(), resultFareMarket));
      CPPUNIT_ASSERT(fareMarketsCarrier.end() !=
                     find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), resultFareMarket));
      CPPUNIT_ASSERT(fareMarketsItinerary.end() != find(fareMarketsItinerary.begin(),
                                                        fareMarketsItinerary.end(),
                                                        resultFareMarket));
    }
  }

  void testAddFareMarketWithTrxAndCarrierVectorsWhichContainsNewFareMarket()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "AAA", "BBB");
    FareMarket* fareMarketExisting = buildFareMarket(*shoppingTrx, "AAA", "BBB");

    std::vector<FareMarket*> fareMarketsTrx;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsTrx);
    fareMarketsTrx.push_back(fareMarketExisting);

    std::vector<FareMarket*> fareMarketsCarrier;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsCarrier);
    fareMarketsCarrier.push_back(fareMarketExisting);

    std::vector<FareMarket*> fareMarketsItinerary;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsItinerary);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    size_t sizeTrx = fareMarketsTrx.size();
    size_t sizeCarrier = fareMarketsCarrier.size();
    size_t sizeItinerary = fareMarketsItinerary.size();

    itin.addFareMarket(
        *shoppingTrx, fareMarket, fareMarketsTrx, fareMarketsCarrier, fareMarketsItinerary);

    // Check if new fare market was added to all vectors exept trx and carrier
    // vector
    CPPUNIT_ASSERT(fareMarketsTrx.size() == sizeTrx);
    CPPUNIT_ASSERT(fareMarketsCarrier.size() == sizeCarrier);
    CPPUNIT_ASSERT(fareMarketsItinerary.size() == sizeItinerary + 1);

    // Check if apropriate fare market could be found in fare markets trx
    FareMarket* resultFareMarket =
        itin.getExistingFareMarket(*shoppingTrx, *fareMarket, fareMarketsTrx);

    CPPUNIT_ASSERT(NULL != resultFareMarket);

    if (NULL != resultFareMarket)
    {
      // Check if new fare market pointer wasn't add to any fare markets
      // vector
      CPPUNIT_ASSERT(fareMarketsTrx.end() ==
                     find(fareMarketsTrx.begin(), fareMarketsTrx.end(), fareMarket));
      CPPUNIT_ASSERT(fareMarketsCarrier.end() ==
                     find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), fareMarket));
      CPPUNIT_ASSERT(fareMarketsItinerary.end() ==
                     find(fareMarketsItinerary.begin(), fareMarketsItinerary.end(), fareMarket));

      // Check if existing fare market pointer could be found in all fare
      // markets vector
      CPPUNIT_ASSERT(fareMarketsTrx.end() !=
                     find(fareMarketsTrx.begin(), fareMarketsTrx.end(), resultFareMarket));
      CPPUNIT_ASSERT(fareMarketsCarrier.end() !=
                     find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), resultFareMarket));
      CPPUNIT_ASSERT(fareMarketsItinerary.end() != find(fareMarketsItinerary.begin(),
                                                        fareMarketsItinerary.end(),
                                                        resultFareMarket));
    }
  }

  void testAddFareMarketWithAllVectorsWhichContainsNewFareMarket()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    FareMarket* fareMarket = buildFareMarket(*shoppingTrx, "AAA", "BBB");
    FareMarket* fareMarketExisting = buildFareMarket(*shoppingTrx, "AAA", "BBB");

    std::vector<FareMarket*> fareMarketsTrx;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsTrx);
    fareMarketsTrx.push_back(fareMarketExisting);

    std::vector<FareMarket*> fareMarketsCarrier;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsCarrier);
    fareMarketsCarrier.push_back(fareMarketExisting);

    std::vector<FareMarket*> fareMarketsItinerary;
    buildRandomFareMarketsVector(*shoppingTrx, fareMarketsItinerary);
    fareMarketsItinerary.push_back(fareMarketExisting);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    size_t sizeTrx = fareMarketsTrx.size();
    size_t sizeCarrier = fareMarketsCarrier.size();
    size_t sizeItinerary = fareMarketsItinerary.size();

    itin.addFareMarket(
        *shoppingTrx, fareMarket, fareMarketsTrx, fareMarketsCarrier, fareMarketsItinerary);

    // Check if new fare market wasn't added to any fare market vector
    CPPUNIT_ASSERT(fareMarketsTrx.size() == sizeTrx);
    CPPUNIT_ASSERT(fareMarketsCarrier.size() == sizeCarrier);
    CPPUNIT_ASSERT(fareMarketsItinerary.size() == sizeItinerary);

    // Check if apropriate fare market could be found in fare markets trx
    FareMarket* resultFareMarket =
        itin.getExistingFareMarket(*shoppingTrx, *fareMarket, fareMarketsTrx);

    CPPUNIT_ASSERT(NULL != resultFareMarket);

    if (NULL != resultFareMarket)
    {
      // Check if new fare market pointer wasn't add to any fare markets
      // vector
      CPPUNIT_ASSERT(fareMarketsTrx.end() ==
                     find(fareMarketsTrx.begin(), fareMarketsTrx.end(), fareMarket));
      CPPUNIT_ASSERT(fareMarketsCarrier.end() ==
                     find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), fareMarket));
      CPPUNIT_ASSERT(fareMarketsItinerary.end() ==
                     find(fareMarketsItinerary.begin(), fareMarketsItinerary.end(), fareMarket));

      // Check if existing fare market pointer could be found in all fare
      // markets vector
      CPPUNIT_ASSERT(fareMarketsTrx.end() !=
                     find(fareMarketsTrx.begin(), fareMarketsTrx.end(), resultFareMarket));
      CPPUNIT_ASSERT(fareMarketsCarrier.end() !=
                     find(fareMarketsCarrier.begin(), fareMarketsCarrier.end(), resultFareMarket));
      CPPUNIT_ASSERT(fareMarketsItinerary.end() != find(fareMarketsItinerary.begin(),
                                                        fareMarketsItinerary.end(),
                                                        resultFareMarket));
    }
  }

  void testbuildMarketsForItineraryWithEmptySegments()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    Itin* itinerary = _memHandle.create<Itin>();
    Itin* dummyItinerary = _memHandle.create<Itin>();

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    // Check if server wouldn't throw exception
    try
    {
      itin.buildMarketsForItinerary(*shoppingTrx, itinerary, dummyItinerary, 0);
    }
    catch (...)
    {
      CPPUNIT_ASSERT(false);
      return;
    }

    // Check if fare market vectors are empty
    CPPUNIT_ASSERT(itinerary->fareMarket().empty() == true);
    CPPUNIT_ASSERT(dummyItinerary->fareMarket().empty() == true);
  }

  void testbuildMarketsForItineraryWithOneSegmentItinerary()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();
    shoppingTrx->setOptions(_option);

    TravelSeg* airSeg1 = buildSegment(*shoppingTrx, "AAA", "BBB", "AA");
    TravelSeg* airSeg2 = buildSegment(*shoppingTrx, "CCC", "DDD", "AA");

    Itin* itinerary = _memHandle.create<Itin>();
    Itin* dummyItinerary = _memHandle.create<Itin>();

    itinerary->travelSeg().push_back(airSeg1);
    dummyItinerary->travelSeg().push_back(airSeg2);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    itin.buildMarketsForItinerary(*shoppingTrx, itinerary, dummyItinerary, 0);

    CPPUNIT_ASSERT(itinerary->fareMarket().size() == 1);
    CPPUNIT_ASSERT(dummyItinerary->fareMarket().size() == 1);
  }

  void testbuildMarketsForItineraryWithMultipleSegmentsItinerary()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();
    shoppingTrx->setOptions(_option);

    AirSeg* airSeg1 = buildSegment(*shoppingTrx, "AAA", "BBB", "AA");
    AirSeg* airSeg2 = buildSegment(*shoppingTrx, "BBB", "CCC", "AA");
    AirSeg* airSeg3 = buildSegment(*shoppingTrx, "CCC", "DDD", "AA");
    AirSeg* airSeg4 = buildSegment(*shoppingTrx, "DDD", "EEE", "AA");

    TravelSeg* tvlSeg = buildSegment(*shoppingTrx, "FFF", "GGG", "AA");

    Itin* itinerary = _memHandle.create<Itin>();
    Itin* dummyItinerary = _memHandle.create<Itin>();

    itinerary->travelSeg() += airSeg1, airSeg2, airSeg3, airSeg4;

    dummyItinerary->travelSeg().push_back(tvlSeg);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    itin.buildMarketsForItinerary(*shoppingTrx, itinerary, dummyItinerary, 0);

    // Following markets should be generated ( 9 fare markets ):
    // AAA - BBB
    // AAA - CCC
    // AAA - DDD
    // BBB - CCC
    // BBB - DDD
    // BBB - EEE
    // CCC - DDD
    // CCC - EEE
    // DDD - EEE
    //
    // So we'll check if there are 10 fare markets in our vectors
    CPPUNIT_ASSERT(itinerary->fareMarket().size() == 9);
    CPPUNIT_ASSERT(dummyItinerary->fareMarket().size() == 9);
  }

  void testBuildLocalMarketsWithEmptyLegsVector()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    // Check if server wouldn't throw exception
    try
    {
      itin.buildLocalMarkets(*shoppingTrx);
    }
    catch (...)
    {
      CPPUNIT_ASSERT(false);
    }

    // Check if fare market vector is empty
    CPPUNIT_ASSERT(shoppingTrx->fareMarket().empty() == true);
  }

  void prepareDataForBuildLocalMarketsWithEmptySopInOneLeg(ShoppingTrx* shoppingTrx)
  {
    AirSeg* airSeg1 = buildSegment(*shoppingTrx, "AAA", "BBB", "AA");

    Itin* itinerary = _memHandle.create<Itin>();

    itinerary->travelSeg().push_back(airSeg1);

    shoppingTrx->legs() += ShoppingTrx::Leg(), ShoppingTrx::Leg();
    shoppingTrx->legs().front().sop().clear();
    shoppingTrx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itinerary, 1, true));
  }

  void testBuildLocalMarketsWithEmptySopInOneLeg()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    prepareDataForBuildLocalMarketsWithEmptySopInOneLeg(shoppingTrx);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    // Check if server wouldn't throw exception
    try
    {
      itin.buildLocalMarkets(*shoppingTrx);
    }
    catch (...)
    {
      CPPUNIT_ASSERT(true);
    }

    // Check if fare market vector is empty
    CPPUNIT_ASSERT(shoppingTrx->fareMarket().empty() == true);
  }

  void prepareDataForBuildLocalMarketsWithEmptyCarrierIndex(ShoppingTrx* shoppingTrx)
  {
    AirSeg* airSeg1 = buildSegment(*shoppingTrx, "AAA", "BBB", "AA");
    AirSeg* airSeg2 = buildSegment(*shoppingTrx, "BBB", "CCC", "AA");
    AirSeg* airSeg3 = buildSegment(*shoppingTrx, "CCC", "BBB", "AA");
    AirSeg* airSeg4 = buildSegment(*shoppingTrx, "BBB", "AAA", "AA");

    Itin* itineraryOb = _memHandle.create<Itin>();
    Itin* itineraryIb = _memHandle.create<Itin>();

    itineraryOb->travelSeg() += airSeg1, airSeg2, airSeg3, airSeg4;

    shoppingTrx->legs() += ShoppingTrx::Leg(), ShoppingTrx::Leg();
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb, 1, true));
    shoppingTrx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryIb, 1, true));
  }

  void testBuildLocalMarketsWithEmptyCarrierIndex()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();

    prepareDataForBuildLocalMarketsWithEmptyCarrierIndex(shoppingTrx);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    // Check if server wouldn't throw exception
    try
    {
      itin.buildLocalMarkets(*shoppingTrx);
    }
    catch (...)
    {
      CPPUNIT_ASSERT(false);
    }

    // Check if fare market vector is empty
    CPPUNIT_ASSERT(shoppingTrx->fareMarket().empty() == true);
  }

  void prepareDataForBuildLocalMarketsWithNotEmptyCarrierIndex(ShoppingTrx* shoppingTrx)
  {
    AirSeg* airSeg1 = buildSegment(*shoppingTrx, "AAA", "BBB", "AA");
    AirSeg* airSeg2 = buildSegment(*shoppingTrx, "BBB", "CCC", "AA");

    Itin* itineraryOb = _memHandle.create<Itin>();

    itineraryOb->travelSeg() += airSeg1, airSeg2;

    shoppingTrx->legs().push_back(ShoppingTrx::Leg());
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb, 1, true));
    shoppingTrx->legs().front().sop().front().governingCarrier() = "AA";

    AirSeg* airSeg3 = buildSegment(*shoppingTrx, "AAA", "CCC", "AA");

    Itin* dummyItinerary = _memHandle.create<Itin>();

    dummyItinerary->travelSeg().push_back(airSeg3);

    ItinIndex::ItinCellInfo* itinCellInfo = _memHandle.create<ItinIndex::ItinCellInfo>();

    itinCellInfo->flags() |= ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT;

    ItinIndex::Key cxrKey;
    ShoppingUtil::createCxrKey("AA", cxrKey);

    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(scheduleKey);

    shoppingTrx->legs().front().carrierIndex().addItinCell(
        dummyItinerary, *itinCellInfo, cxrKey, scheduleKey);

    shoppingTrx->legs().front().addSop(ShoppingTrx::SchedulingOption(dummyItinerary, true));
    ShoppingTrx::SchedulingOption& newSop = shoppingTrx->legs().front().sop().back();
    newSop.setDummy(true);
  }

  void testBuildLocalMarketsWithNotEmptyCarrierIndex()
  {
    ShoppingTrx* shoppingTrx = _memHandle.create<ShoppingTrx>();
    shoppingTrx->setOptions(_option);

    prepareDataForBuildLocalMarketsWithNotEmptyCarrierIndex(shoppingTrx);

    ItinAnalyzerService itin("ITIN_SVC", *_tseServer);

    // Check if server wouldn't throw exception
    try
    {
      itin.buildLocalMarkets(*shoppingTrx);
    }
    catch (...)
    {
      CPPUNIT_ASSERT(false);
    }

    // Check if fare market vector isn't empty
    CPPUNIT_ASSERT(shoppingTrx->fareMarket().empty() == false);
    // Check if trx fare market vector size is equal to size of fare market
    // vector in carrier index
    CPPUNIT_ASSERT(shoppingTrx->fareMarket().size() ==
                   shoppingTrx->legs().front().sop().front().itin()->fareMarket().size());
  }

  void testRexTrxRedirected2ExcTrxWhenPass()
  {
    ExchangePricingTrx excTrx;
    RexPricingTrx rexTrx;
    excTrx.setParentTrx(&rexTrx);

    CPPUNIT_ASSERT(_itinAnalyzer->isRexTrxRedirected2ExcTrx(excTrx));
  }

  void testRexTrxRedirected2ExcTrxWhenFail()
  {
    ExchangePricingTrx excTrx;

    CPPUNIT_ASSERT(!_itinAnalyzer->isRexTrxRedirected2ExcTrx(excTrx));
  }

  void testRexTrxRedirected2MultiExcTrxWhenFail()
  {
    ExchangePricingTrx excTrx;
    MultiExchangeTrx parentTrx;
    parentTrx.excPricingTrx1() = &excTrx;
    excTrx.setParentTrx(&parentTrx);

    CPPUNIT_ASSERT(!_itinAnalyzer->isRexTrxRedirected2ExcTrx(excTrx));
  }

  void testSelectProcesingForPortExc()
  {
    ExchangePricingTrx excTrx;
    excTrx.setOptions(_option);

    CPPUNIT_ASSERT(_itinAnalyzer->selectProcesing(excTrx) == &_itinAnalyzer->_itinAnalyzerWrapper);
  }

  void testSelectProcesingForRedirectedToPortExc()
  {
    ExchangePricingTrx excTrx;
    RexPricingTrx rexTrx;
    excTrx.setParentTrx(&rexTrx);
    excTrx.setOptions(_option);

    CPPUNIT_ASSERT(_itinAnalyzer->selectProcesing(excTrx) ==
                   &_itinAnalyzer->_excItinAnalyzerWrapper);
  }

  void testFlowOfProcessForPortExc()
  {
    setupDataForProcessFunction(0);

    _itinAnalyzerMock->process(*_excTrx);

    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::PROCESS_EXCHANGE_ITIN_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::DETERMINE_CHANGES_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SELECT_TICKETING_CARRIER_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_RETRANSITS_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_OPEN_SEG_FLAG_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_TRIP_CHARACTERISTICS_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::CHECK_JOURNEY_ACTIVATION_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::CHECK_SOLO_ACTIVATION_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_ATAE_CONTENT_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_ITIN_ROUNDING_VISITED));
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_INFO_FOR_SIMILAR_ITINS_VISITED));
  }

  void testFlowOfProcessForRedirectedToPortExc()
  {
    setupDataForProcessFunction(_rexTrx);

    _itinAnalyzerMock->process(*_excTrx);

    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::PROCESS_EXCHANGE_ITIN_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::DETERMINE_CHANGES_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SELECT_TICKETING_CARRIER_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_RETRANSITS_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_OPEN_SEG_FLAG_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_TRIP_CHARACTERISTICS_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::CHECK_JOURNEY_ACTIVATION_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::CHECK_SOLO_ACTIVATION_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_ATAE_CONTENT_VISITED));
    CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(
        ExcItinAnalyzerServiceWrapperMock::SET_ITIN_ROUNDING_VISITED));
    // CPPUNIT_ASSERT(_itinAnalyzerMock->_excItinAnalyzerWrapperMock.isVisited(ExcItinAnalyzerServiceWrapperMock::SET_INFO_FOR_SIMILAR_ITINS_VISITED));
  }

  void testCheckJourneyActivationDontActivateWhenJpsNoInMip()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    _pricingTrx->getOptions()->jpsEntered() = NO;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationActivateWhenJpsYesInMip()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    _pricingTrx->getOptions()->jpsEntered() = YES;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationDontActivateWhenCxrPrefZeroInMip()
  {
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    itinAnalyzer._carrierPrefType = MockItinAnalyzerService::ZERO_CXR_PREF;
    itinAnalyzer.checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationDontActivateWhenCxrPrefSaysSoInMip()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    itinAnalyzer._carrierPrefType = MockItinAnalyzerService::JOURNEY_NOT_ACTIVATED;
    itinAnalyzer.checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationActivateWhenCxrPrefSaysSoInMip()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    itinAnalyzer._carrierPrefType = MockItinAnalyzerService::JOURNEY_ACTIVATED;
    itinAnalyzer.checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationDontActivateWhenCustomerTableSaysSoInMip()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    Customer* customer = _memHandle.create<Customer>();
    customer->activateJourneyShopping() = NO;
    _pricingTrx->getRequest()->ticketingAgent()->agentTJR() = customer;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationActivateWhenCustomerTableSaysSoInMip()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    Customer* customer = _memHandle.create<Customer>();
    customer->activateJourneyShopping() = YES;
    _pricingTrx->getRequest()->ticketingAgent()->agentTJR() = customer;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->journeyActivatedForShopping());
  }

  void testCheckJourneyActivationDontActivateWhenNotWpnc()
  {
    preparePricingTrx();
    _pricingTrx->getRequest()->lowFareRequested() = NO;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationDontActivateWhenShoppingTrx()
  {
    preparePricingTrx();
    ShoppingTrx shoppingTrx;
    shoppingTrx.setTrxType(PricingTrx::ESV_TRX);
    shoppingTrx.setRequest(_pricingTrx->getRequest());
    shoppingTrx.setOptions(_pricingTrx->getOptions());
    _itinAnalyzer->checkJourneyActivation(shoppingTrx);
    CPPUNIT_ASSERT(!shoppingTrx.getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationDontActivateWhenJpsNo()
  {
    preparePricingTrx();
    _pricingTrx->getOptions()->jpsEntered() = NO;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationActivateWhenJpsYes()
  {
    preparePricingTrx();
    _pricingTrx->getOptions()->jpsEntered() = YES;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationDontActivateWhenCxrPrefZero()
  {
    preparePricingTrx();
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    itinAnalyzer._carrierPrefType = MockItinAnalyzerService::ZERO_CXR_PREF;
    itinAnalyzer.checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationDontActivateWhenCxrPrefSaysSo()
  {
    preparePricingTrx();
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    itinAnalyzer._carrierPrefType = MockItinAnalyzerService::JOURNEY_NOT_ACTIVATED;
    itinAnalyzer.checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationActivateWhenCxrPrefSaysSo()
  {
    preparePricingTrx();
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    itinAnalyzer._carrierPrefType = MockItinAnalyzerService::JOURNEY_ACTIVATED;
    itinAnalyzer.checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationDontActivateWhenCustomerTableSaysSo()
  {
    preparePricingTrx();
    Customer* customer = _memHandle.create<Customer>();
    customer->activateJourneyPricing() = NO;
    _pricingTrx->getRequest()->ticketingAgent()->agentTJR() = customer;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testCheckJourneyActivationActivateWhenCustomerTableSaysSo()
  {
    preparePricingTrx();
    Customer* customer = _memHandle.create<Customer>();
    customer->activateJourneyPricing() = YES;
    _pricingTrx->getRequest()->ticketingAgent()->agentTJR() = customer;
    _itinAnalyzer->checkJourneyActivation(*_pricingTrx);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->journeyActivatedForPricing());
  }

  void testPrepareForJourney()
  {
    preparePricingTrx();
    _pricingTrx->getOptions()->journeyActivatedForPricing() = true;
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->itin() += itin;
    MockItinAnalyzerService itinAnalyzer("ITIN_SVC", *_tseServer);
    CPPUNIT_ASSERT_NO_THROW(itinAnalyzer.prepareForJourney(*_pricingTrx));
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    _pricingTrx->getOptions()->journeyActivatedForShopping() = true;
    CPPUNIT_ASSERT_NO_THROW(itinAnalyzer.prepareForJourney(*_pricingTrx));
  }

  void testProcessRexCommonCallSetAtaeContentForExchangeItin()
  {
    AirSeg as;
    as.destination() = TestLocFactory::create(LOC_LON);
    as.origin() = TestLocFactory::create(LOC_DFW);
    _rexTrx->travelSeg() += &as;
    _excItin->travelSeg() += &as;
    _newItin->travelSeg() += &as;
    _rexTrx->exchangeItin() += _excItin;
    _request->validatingCarrier() = "AA";
    _rexTrx->setRequest(_request);
    _rexTrx->setOptions(_option);
    _itinAnalyzerMock->_throwSetAtaeContent = true;

    try
    {
      _itinAnalyzerMock->processRexCommon(*_rexTrx);
    }
    catch (std::string s)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("ATAE content set"), s);
    }
    catch (...)
    {
      CPPUNIT_FAIL("SetAtaeContent() not called");
    }
  }

  void testDetermineExchangeReissueStatusItinNonCat31Reissue()
  {
    setupDataForProcessFunction(0);
    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_excTrx);
    excTrx->reqType() = PARTIAL_EXCHANGE;
    excTrx->newItin().front()->travelSeg().front()->changeStatus() = TravelSeg::UNCHANGED;

    excTrx->exchangeItin().front()->travelSeg().front()->changeStatus() = TravelSeg::UNCHANGED;

    _itinAnalyzer->determineExchangeReissueStatus(*excTrx);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() != EXCHANGE);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() == REISSUE);
  }

  void testDetermineExchangeReissueStatusItinNonCat31Exchange()
  {
    setupDataForProcessFunction(0);
    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_excTrx);
    excTrx->reqType() = FULL_EXCHANGE;
    excTrx->newItin().front()->travelSeg().front()->changeStatus() = TravelSeg::CHANGED;

    excTrx->exchangeItin().front()->travelSeg().front()->changeStatus() = TravelSeg::CHANGED;
    _itinAnalyzer->determineExchangeReissueStatus(*excTrx);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() == EXCHANGE);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() != REISSUE);
  }

  void testDetermineExchangeReissueStatusItinCat31Reissue()
  {
    _rexTrx->itin().push_back(_newItin);
    setUpItin(*_rexTrx, *_newItin);
    _rexTrx->exchangeItin().push_back(_excItin);
    setUpItin(*_excTrx, *_excItin);

    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_rexTrx);
    excTrx->newItin().front()->travelSeg().front()->unflown() = false;
    _itinAnalyzer->determineExchangeReissueStatus(*excTrx);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() != EXCHANGE);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() == REISSUE);
  }

  void testDetermineExchangeReissueStatusItinCat31Exchange()
  {
    _rexTrx->itin().push_back(_newItin);
    setUpItin(*_rexTrx, *_newItin);
    _rexTrx->exchangeItin().push_back(_excItin);
    setUpItin(*_excTrx, *_excItin);

    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_rexTrx);
    _itinAnalyzer->determineExchangeReissueStatus(*excTrx);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() == EXCHANGE);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() != REISSUE);
  }

  void testDetermineExchangeReissueStatusItinCat31UnchangedReissue()
  {
    _rexTrx->itin().push_back(_newItin);
    setUpItin(*_rexTrx, *_newItin);
    _rexTrx->exchangeItin().push_back(_excItin);
    setUpItin(*_excTrx, *_excItin);

    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(_rexTrx);
    excTrx->exchangeItin().front()->travelSeg().front()->changeStatus() = TravelSeg::UNCHANGED;
    excTrx->newItin().front()->travelSeg().front()->changeStatus() = TravelSeg::UNCHANGED;
    _itinAnalyzer->determineExchangeReissueStatus(*excTrx);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() != EXCHANGE);
    CPPUNIT_ASSERT(excTrx->newItin().front()->exchangeReissue() == REISSUE);
  }

  void testPreviousExhDateFareIndicatorIsOFFforCat31WhenNOapplyReissueExchangeSet()
  {
    CPPUNIT_ASSERT_EQUAL(false, _rexTrx->previousExchangeDateFare());
  }

  void testPreviousExhDateFareIndicatorIsOFFforCat31WhenApplyReissueExchangeSet()
  {
    _rexTrx->setRexPrimaryProcessType('A');
    _itinAnalyzer->applyReissueExchange(*_rexTrx);
    CPPUNIT_ASSERT_EQUAL(false, _rexTrx->previousExchangeDateFare());
  }

  void testPreviousExhDateFareIndicatorIsONforCat31WhenPreviousExchangeDTSet()
  {
    _rexTrx->previousExchangeDT() = DateTime(2010, 1, 10);
    _rexTrx->setRexPrimaryProcessType('A');
    _itinAnalyzer->applyReissueExchange(*_rexTrx);
    CPPUNIT_ASSERT_EQUAL(true, _rexTrx->previousExchangeDateFare());
  }

  class UniqueFMGenerator
  {
    std::stack<LocCode>& _cities;
    TestMemHandle& _mem;

  public:
    UniqueFMGenerator(std::stack<LocCode>& cities, TestMemHandle& mem) : _cities(cities), _mem(mem)
    {
    }
    FareMarket* operator()()
    {
      FareMarket* fm = _mem.insert(new FareMarket);
      fm->boardMultiCity() = _cities.top();
      _cities.pop();
      return fm;
    }
  };

  void setUpForRefund()
  {
    _itinAnalyzer =
        _memHandle(new ItinAnalyzerServiceMockForRefund("ITIN_SVC", *_tseServer, _memHandle));
    _refundTrx = _memHandle(new RefundPricingTrx);

    std::stack<LocCode> cities = list_of("AAD")("AAC")("AAB")("AAA").to_adapter();

    UniqueFMGenerator fMGenerator(cities, _memHandle);

    std::generate_n(std::back_inserter(_excTrx->fareMarket()), 2, fMGenerator);

    std::generate_n(std::back_inserter(_refundTrx->fareMarket()), 2, fMGenerator);
  }

  void testAddRegularFareMarketsForPlusUpCalculation()
  {
    setUpForRefund();

    std::vector<FareMarket*>::size_type two = 2;

    CPPUNIT_ASSERT_EQUAL(two, _excTrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(two, _refundTrx->fareMarket().size());

    _itinAnalyzer->addRegularFareMarketsForPlusUpCalculation(*_refundTrx, *_excItin);

    CPPUNIT_ASSERT_EQUAL(two, _excTrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(two + two, _refundTrx->fareMarket().size());
    CPPUNIT_ASSERT(!_refundTrx->fareMarket()[0]->breakIndicator());
    CPPUNIT_ASSERT(!_refundTrx->fareMarket()[1]->breakIndicator());
    CPPUNIT_ASSERT(_refundTrx->fareMarket()[2]->breakIndicator());
    CPPUNIT_ASSERT(_refundTrx->fareMarket()[3]->breakIndicator());
  }

  void testRemoveDuplicatedFMs()
  {
    std::vector<FareMarket*> fareMarkets;
    std::stack<LocCode> cities =
        list_of("AAD")("AAE")("AAB")("AAD")("AAC")("AAB")("AAA").to_adapter();

    UniqueFMGenerator fMGenerator(cities, _memHandle);

    std::generate_n(std::back_inserter(fareMarkets), cities.size(), fMGenerator);

    bool equal = *fareMarkets[1] == *fareMarkets[4] && *fareMarkets[3] == *fareMarkets[6];

    CPPUNIT_ASSERT(equal);

    FareMarket* shouldBeRemoved1 = fareMarkets[4];
    FareMarket* shouldBeRemoved2 = fareMarkets[6];

    std::vector<FareMarket*>::size_type whole = 7;
    CPPUNIT_ASSERT_EQUAL(whole, fareMarkets.size());

    _itinAnalyzer->removeDuplicatedFMs(fareMarkets, 4);

    CPPUNIT_ASSERT_EQUAL(whole - 2, fareMarkets.size());

    CPPUNIT_ASSERT(fareMarkets.end() ==
                   std::find(fareMarkets.begin(), fareMarkets.end(), shouldBeRemoved1));
    CPPUNIT_ASSERT(fareMarkets.end() ==
                   std::find(fareMarkets.begin(), fareMarkets.end(), shouldBeRemoved2));
  }

  void setUpForSplitItins(RexExchangeTrx& trx)
  {
    trx.exchangeItin().push_back(_excItin);
    setUpItin(trx, *_excItin);
    trx.itin().push_back(_newItin);
    setUpItin(trx, *_newItin);
  }

  void setUpSplitItin(RexExchangeTrx& trx, Itin& itin, int index)
  {
    setUpItin(trx, itin);
    AirSeg* travelSeg;
    trx.dataHandle().get(travelSeg);
    const Loc* loc1 = trx.dataHandle().getLoc("XXX", DateTime::localTime());
    const Loc* loc2 = trx.dataHandle().getLoc("YYY", DateTime::localTime());

    initAirSeg(*travelSeg, GeoTravelType::International, loc1, loc2, "AA");
    itin.travelSeg()[index] = travelSeg;
  }

  void testSplitItinsSingleTheSame()
  {
    RexExchangeTrx trx;
    setUpForSplitItins(trx);

    FamilyLogicSplitter splitter(trx, 1);
    splitter.splitItinFamilies(false);

    CPPUNIT_ASSERT(trx.itin().size() == 1);
  }

  void testSplitItinsMotherTheSameWith2NewFamilies()
  {
    RexExchangeTrx trx;
    setUpForSplitItins(trx);

    Itin childItin1;
    setUpSplitItin(trx, childItin1, 1);
    Itin childItin2;
    setUpSplitItin(trx, childItin2, 1);
    Itin childItin3;
    setUpSplitItin(trx, childItin3, 2);

    trx.itin()[0]->addSimilarItin(&childItin1);
    trx.itin()[0]->addSimilarItin(&childItin2);
    trx.itin()[0]->addSimilarItin(&childItin3);

    Itin itin;
    setUpSplitItin(trx, itin, 1);

    trx.itin().push_back(&itin);

    FamilyLogicSplitter splitter(trx, 1);
    splitter.splitItinFamilies(false);

    CPPUNIT_ASSERT(trx.itin().size() == 3);
    CPPUNIT_ASSERT(trx.itin()[0]->getSimilarItins().size() == 0);
    CPPUNIT_ASSERT(trx.itin()[1]->getSimilarItins().size() == 1);
    CPPUNIT_ASSERT(trx.itin()[1] == &childItin1);
    CPPUNIT_ASSERT(trx.itin()[1]->getSimilarItins()[0].itin == &childItin2);
    CPPUNIT_ASSERT(trx.itin()[2] == &childItin3);
    CPPUNIT_ASSERT(trx.itin()[2]->getSimilarItins().size() == 0);
  }

  void testSplitItinsChildTheSame()
  {
    RexExchangeTrx trx;
    setUpForSplitItins(trx);

    Itin childItin1;
    setUpItin(trx, childItin1);

    Itin itin;
    setUpSplitItin(trx, itin, 1);

    itin.addSimilarItin(&childItin1);
    trx.itin().push_back(&itin);

    FamilyLogicSplitter splitter(trx, 1);
    splitter.splitItinFamilies(false);

    CPPUNIT_ASSERT(trx.itin().size() == 1);
    CPPUNIT_ASSERT(trx.itin()[0]->getSimilarItins().size() == 0);

    CPPUNIT_ASSERT(trx.itin()[0] == &itin);
  }

  void testSplitItinsChildWithTheSameStatus()
  {
    RexExchangeTrx trx;
    setUpForSplitItins(trx);

    Itin childItin1;
    setUpSplitItin(trx, childItin1, 1);
    Itin childItin2;
    setUpSplitItin(trx, childItin2, 1);
    Itin childItin3;
    setUpSplitItin(trx, childItin3, 2);
    Itin childItin4;
    setUpSplitItin(trx, childItin4, 0);
    childItin4.travelSeg()[1] = childItin4.travelSeg()[0];
    childItin4.travelSeg()[2] = childItin4.travelSeg()[0];
    childItin4.travelSeg()[3] = childItin4.travelSeg()[0];

    Itin itin;
    setUpSplitItin(trx, itin, 1);

    itin.addSimilarItin(&childItin1);
    itin.addSimilarItin(&childItin2);
    itin.addSimilarItin(&childItin3);
    itin.addSimilarItin(&childItin4);
    trx.itin().push_back(&itin);

    FamilyLogicSplitter splitter(trx, 1);
    splitter.splitItinFamilies(false);

    CPPUNIT_ASSERT(trx.itin().size() == 3);
    CPPUNIT_ASSERT(trx.itin()[0]->getSimilarItins().size() == 2);
    CPPUNIT_ASSERT(trx.itin()[0] == &itin);
    CPPUNIT_ASSERT(trx.itin()[1] == &childItin4);
    CPPUNIT_ASSERT(trx.itin()[2] == &childItin3);
  }
  void setUpForRefundPlusUps()
  {
    _refundTrx = _memHandle.insert(new RefundPricingTrx);
    _refundTrx->exchangeItin().push_back(_excItin);
    _refundTrx->totalFareCalcAmount() = 0.0;

    CPPUNIT_ASSERT(!_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testPlusUpSumVsFCSum_zero()
  {
    setUpForRefundPlusUps();

    _itinAnalyzer->checkIfPenaltiesAndFCsEqualToSumFromFareCalc(*_refundTrx);

    CPPUNIT_ASSERT(_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testPlusUpSumVsFCSum_notHipExc()
  {
    setUpForRefundPlusUps();
    PlusUpOverride plusUp;
    plusUp.fromExchange() = true;
    plusUp.moduleName() = CTM;

    _refundTrx->exchangeOverrides().plusUpOverride().push_back(&plusUp);

    _itinAnalyzer->checkIfPenaltiesAndFCsEqualToSumFromFareCalc(*_refundTrx);

    CPPUNIT_ASSERT(!_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testPlusUpSumVsFCSum_HipExc()
  {
    setUpForRefundPlusUps();
    PlusUpOverride plusUp;
    plusUp.fromExchange() = true;
    plusUp.moduleName() = HIP;

    _refundTrx->exchangeOverrides().plusUpOverride().push_back(&plusUp);

    _itinAnalyzer->checkIfPenaltiesAndFCsEqualToSumFromFareCalc(*_refundTrx);

    CPPUNIT_ASSERT(_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testPlusUpSumVsFCSum_HipNotExc()
  {
    setUpForRefundPlusUps();
    PlusUpOverride plusUp;
    CPPUNIT_ASSERT(!plusUp.fromExchange());
    plusUp.moduleName() = HIP;

    _refundTrx->exchangeOverrides().plusUpOverride().push_back(&plusUp);

    _itinAnalyzer->checkIfPenaltiesAndFCsEqualToSumFromFareCalc(*_refundTrx);

    CPPUNIT_ASSERT(_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testPlusUpSumVsFCSum_DiffEqual()
  {
    setUpForRefundPlusUps();
    DifferentialOverride diff;
    diff.fromExchange() = true;
    diff.amount() = 20.0;

    DifferentialOverride diff2;
    CPPUNIT_ASSERT(!diff2.fromExchange());
    diff2.amount() = 15.0;

    _refundTrx->exchangeOverrides().differentialOverride().push_back(&diff);
    _refundTrx->exchangeOverrides().differentialOverride().push_back(&diff2);

    _refundTrx->totalFareCalcAmount() = 20.0;

    _itinAnalyzer->checkIfPenaltiesAndFCsEqualToSumFromFareCalc(*_refundTrx);

    CPPUNIT_ASSERT(_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testPlusUpSumVsFCSum_DiffNotEqual()
  {
    setUpForRefundPlusUps();
    DifferentialOverride diff;
    diff.fromExchange() = true;
    diff.amount() = 20.0;

    DifferentialOverride diff2;
    CPPUNIT_ASSERT(!diff2.fromExchange());
    diff2.amount() = 15.0;

    _refundTrx->exchangeOverrides().differentialOverride().push_back(&diff);
    _refundTrx->exchangeOverrides().differentialOverride().push_back(&diff2);

    _refundTrx->totalFareCalcAmount() = 19.0;

    _itinAnalyzer->checkIfPenaltiesAndFCsEqualToSumFromFareCalc(*_refundTrx);

    CPPUNIT_ASSERT(!_refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc());
  }

  void testDiscoverThroughFarePrecedenceEmpty()
  {
    setUpTFP_BrazilJJ();

    _itinAnalyzer->discoverThroughFarePrecedence(*_trx, *_newItin);

    CPPUNIT_ASSERT_EQUAL(false, _newItin->isThroughFarePrecedence());
  }

  void testDiscoverThroughFarePrecedenceUnrelated()
  {
    setUpTFP_BrazilJJ();

    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_LON);
      as->destination() = TestLocFactory::create(LOC_DFW);
      as->carrier() = "AA";
      _newItin->travelSeg().push_back(as);
    }
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_DFW);
      as->destination() = TestLocFactory::create(LOC_LON);
      as->carrier() = "AA";
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->discoverThroughFarePrecedence(*_trx, *_newItin);

    CPPUNIT_ASSERT_EQUAL(false, _newItin->isThroughFarePrecedence());
  }

  void testDiscoverThroughFarePrecedenceWrongCountry()
  {
    setUpTFP_BrazilJJ();

    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_LON);
      as->destination() = TestLocFactory::create(LOC_RIO);
      as->carrier() = "JJ";
      _newItin->travelSeg().push_back(as);
    }
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_RIO);
      as->destination() = TestLocFactory::create(LOC_SAO);
      as->carrier() = "JJ";
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->discoverThroughFarePrecedence(*_trx, *_newItin);

    CPPUNIT_ASSERT_EQUAL(false, _newItin->isThroughFarePrecedence());
  }

  void testDiscoverThroughFarePrecedenceWrongCarrier()
  {
    setUpTFP_BrazilJJ();

    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_SAO);
      as->destination() = TestLocFactory::create(LOC_RIO);
      as->carrier() = "XX";
      _newItin->travelSeg().push_back(as);
    }
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_RIO);
      as->destination() = TestLocFactory::create(LOC_SAO);
      as->carrier() = "JJ";
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->discoverThroughFarePrecedence(*_trx, *_newItin);

    CPPUNIT_ASSERT_EQUAL(false, _newItin->isThroughFarePrecedence());
  }

  void testDiscoverThroughFarePrecedenceOk()
  {
    setUpTFP_BrazilJJ();

    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_SAO);
      as->destination() = TestLocFactory::create(LOC_RIO);
      as->carrier() = "JJ";
      _newItin->travelSeg().push_back(as);
    }
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->origin() = TestLocFactory::create(LOC_RIO);
      as->destination() = TestLocFactory::create(LOC_SAO);
      as->carrier() = "JJ";
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->discoverThroughFarePrecedence(*_trx, *_newItin);

    CPPUNIT_ASSERT_EQUAL(true, _newItin->isThroughFarePrecedence());
  }

  void testFailIfThroughFarePrecedenceImpossibleNoTFP_NoForce()
  {
    _newItin->setThroughFarePrecedence(false);
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->forcedFareBrk() = 'F';
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->failIfThroughFarePrecedenceImpossible(*_newItin);
  }

  void testFailIfThroughFarePrecedenceImpossibleNoTFP_Force()
  {
    _newItin->setThroughFarePrecedence(false);
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->forcedFareBrk() = 'T';
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->failIfThroughFarePrecedenceImpossible(*_newItin);
  }

  void testFailIfThroughFarePrecedenceImpossibleTFP_NoForce()
  {
    _newItin->setThroughFarePrecedence(true);
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->forcedFareBrk() = 'F';
      _newItin->travelSeg().push_back(as);
    }

    _itinAnalyzer->failIfThroughFarePrecedenceImpossible(*_newItin);
  }

  void testFailIfThroughFarePrecedenceImpossibleTFP_Force()
  {
    _newItin->setThroughFarePrecedence(true);
    {
      AirSeg* as = _memHandle.create<AirSeg>();
      as->forcedFareBrk() = 'T';
      _newItin->travelSeg().push_back(as);
    }

    CPPUNIT_ASSERT_THROW(_itinAnalyzer->failIfThroughFarePrecedenceImpossible(*_newItin),
                         NonFatalErrorResponseException);
  }

private:
  ItinAnalyzerService* _itinAnalyzer;
  ItinAnalyzerServiceMock* _itinAnalyzerMock;
  RexPricingTrx* _rexTrx;
  ExchangePricingTrx* _excTrx;
  PaxType _paxType;
  PaxTypeInfo _paxTypeInfo;
  RefundPricingTrx* _refundTrx;
  ExcItin* _excItin;
  Itin* _newItin;
  PricingRequest* _request;
  PricingOptions* _option;
  MockTseServer* _tseServer;
  TestMemHandle _memHandle;
  PricingTrx* _pricingTrx;
  PricingTrx* _trx;
  TrxUtilMock* _trxUtilMock;

  void setUpTFP_BrazilJJ()
  {
    TestConfigInitializer::setValue("COUNTRIES", "BR", "THROUGH_FARE_PRECEDENCE");
    TestConfigInitializer::setValue("LIST_OF_CARRIERS", "JJ", "THROUGH_FARE_PRECEDENCE");
  }

  void preparePricingTrx()
  {
    _option->jpsEntered() = ' ';
    _pricingTrx->setOptions(_option);
    Agent* agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = agent;
    _request->lowFareRequested() = YES;
    agent->agentFunctions() = "YFH";
    agent->agentCity() = "HDQ";
    agent->agentDuty() = "8";
    agent->airlineDept() = "HDQ";
    agent->cxrCode() = "1S";
    agent->currencyCodeAgent() = "USD";
    // agent->agentLocation() = _pricingTrx->dataHandle().getLoc(agent->agentCity(), time(NULL));
    agent->agentLocation() = _memHandle.create<Loc>();
    _pricingTrx->setRequest(_request);

    // Need billing record
    Billing* billing = _memHandle.create<Billing>();
    billing->userPseudoCityCode() = "HDQ";
    billing->userStation() = "925";
    billing->userBranch() = "3470";
    billing->partitionID() = "AA";
    billing->userSetAddress() = "02BD09";
    billing->aaaCity() = "HDQ";
    billing->aaaSine() = "YFH";
    billing->serviceName() = "ITPRICE1";
    billing->actionCode() = "WPBET";
    _pricingTrx->billing() = billing;
  }

  Itin* prepareItinAndFareMarkets()
  {
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    AirSeg* a4 = _memHandle.create<AirSeg>();
    a1->carrier() = a2->carrier() = a3->carrier() = a4->carrier() = "AA";
    a1->marriageStatus() = 'S';
    a2->marriageStatus() = 'P';
    a3->marriageStatus() = 'E';
    a4->marriageStatus() = 'E';

    a1->origin() = a2->origin() = a3->origin() = a4->origin() = _memHandle.create<Loc>();
    a1->destination() = a2->destination() = a3->destination() = a4->destination() =
        _memHandle.create<Loc>();

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3, (TravelSeg*)a4;

    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3;

    FareMarket* fm2 = _memHandle.create<FareMarket>();
    fm2->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2;

    FareMarket* fm3 = _memHandle.create<FareMarket>();
    fm3->travelSeg() += (TravelSeg*)a2, (TravelSeg*)a3;

    FareMarket* fm4 = _memHandle.create<FareMarket>();
    fm4->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a3;

    FareMarket* fm5 = _memHandle.create<FareMarket>();
    fm5->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3, (TravelSeg*)a4;

    itin->fareMarket() += fm1, fm2, fm3, fm4, fm5;
    return itin;
  }

  // Prepared 2 Itins with 5 fare markets.
  // Fare Market 0: 3 segments (Seg0: DSM-CHI, Seg1:CHI-JFK, Seg2: JFK-PHL)
  // Fare Market 1: 2 Segments (Seg0: DSM-CHI, Seg3:CHI-PHL)
  // Fare Market 2: 2 Segments (Seg4: DSM-DFW, Seg5: DFW-PHL)
  // Fare Market 3: 2 Segments (Seg6: DSM-JFK, Seg2: JFK-PHL)
  // Fare Market 4: 3 Segments (Seg4: DSM-DFW, Seg7: DFW-JFK, Seg2: JFK-PHL)
  void prepareItinaryAndFMsForOldBrandedFare()
  {
    preparePricingTrx();

    std::vector<BookingCode> sbkc;
    sbkc.push_back(BookingCode('Y'));

    AirSeg* as[8];
    TravelSeg* ts[8];
    for (int i = 0; i < 8; i++)
    {
      as[i] = _memHandle.create<AirSeg>();
      as[i]->carrier() = "AR";
      as[i]->marriageStatus() = 'S';
      // By default all flights are domestic
      as[i]->origin() = _memHandle.create<Loc>();
      const_cast<NationCode&>(as[i]->origin()->nation()) = UNITED_STATES;
      as[i]->destination() = _memHandle.create<Loc>();
      const_cast<NationCode&>(as[i]->destination()->nation()) = UNITED_STATES;
      ts[i] = (TravelSeg*)as[i];
    }

    ts[0]->origAirport() = ts[4]->origAirport() = ts[6]->origAirport() = "DSM";
    ts[0]->destAirport() = ts[1]->origAirport() = ts[3]->origAirport() = "CHI";
    ts[4]->destAirport() = ts[5]->origAirport() = ts[7]->origAirport() = "DFW";
    ts[2]->origAirport() = ts[1]->destAirport() = ts[6]->destAirport() = ts[7]->destAirport() =
        "JFK";
    ts[2]->destAirport() = ts[3]->destAirport() = ts[5]->destAirport() = "PHL";

    // Create 24 cos with default I, W, Y, X booking codes
    ClassOfService* cos[24];
    for (int i = 0; i < 24; i++)
    {
      cos[i] = _memHandle.create<ClassOfService>();
      cos[i]->numSeats() = 9;
      (i < 8) ? cos[i]->bookingCode() = 'I' : i >= 8 && i < 16
          ? cos[i]->bookingCode() =
                'Y' : i >= 16 && i < 20 ? cos[i]->bookingCode() = 'W' : cos[i]->bookingCode() = 'X';
    }

    // Create 8 cosV same as number of segments
    std::vector<ClassOfService*>* cosV[8];
    for (int i = 0; i < 8; i++)
    {
      cosV[i] = new std::vector<ClassOfService*>;
      *cosV[i] += cos[i], cos[i + 8], cos[i + 16];
    }

    // Create 5 fare markets
    FareMarket* fm[5];
    for (int i = 0; i < 5; i++)
      fm[i] = _memHandle.create<FareMarket>();

    Itin* itin = _memHandle.create<Itin>();

    // begin data for Price_by Cabin Project
    cos[0]->cabin().setPremiumFirstClass();
    cos[4]->cabin().setEconomyClass();
    cos[8]->cabin().setEconomyClass();
    ts[0]->setBookingCode(cos[0]->bookingCode());
    ts[0]->classOfService().push_back(cos[0]);
    ts[0]->classOfService().push_back(cos[4]);
    // end data for Price_by Cabin Project

    itin->travelSeg() += ts[0], ts[1], ts[2], ts[3];

    // Fare Market 0: 3 segments (Seg0: DSM-CHI, Seg1:CHI-JFK, Seg2: JFK-PHL)
    fm[0]->travelSeg() += ts[0], ts[1], ts[2];
    fm[0]->classOfServiceVec().clear();
    fm[0]->classOfServiceVec().push_back(cosV[0]); // Booking Code used: I,Y,W
    fm[0]->classOfServiceVec().push_back(cosV[1]); // Booking Code used: I,Y,W
    fm[0]->classOfServiceVec().push_back(cosV[2]); // Booking Code used: I,Y,W

    // Fare Market 1: 2 Segments (Seg0: DSM-CHI, Seg3:CHI-PHL)
    fm[1]->travelSeg() += ts[0], ts[3];
    fm[1]->classOfServiceVec().clear();
    fm[1]->classOfServiceVec().push_back(cosV[0]); // Booking Code used: I,Y,W
    fm[1]->classOfServiceVec().push_back(cosV[3]); // Booking Code used: I,Y,W

    // Fare Market 2: 2 Segments (Seg4: DSM-DFW, Seg5: DFW-PHL)
    fm[2]->travelSeg() += ts[4], ts[5];
    fm[2]->classOfServiceVec().clear();
    fm[2]->classOfServiceVec().push_back(cosV[4]); // Booking Code used: I,Y,X
    fm[2]->classOfServiceVec().push_back(cosV[5]); // Booking Code used: I,Y,X

    // Fare Market 3: 2 Segments (Seg6: DSM-JFK, Seg2: JFK-PHL)
    fm[3]->travelSeg() += ts[6], ts[2];
    fm[3]->classOfServiceVec().clear();
    fm[3]->classOfServiceVec().push_back(cosV[6]); // Booking Code used: I,Y,X
    fm[3]->classOfServiceVec().push_back(cosV[2]); // Booking Code used: I,Y,W

    // Fare Market 4: 3 Segments (Seg4: DSM-DFW, Seg7: DFW-JFK, Seg2: JFK-PHL)
    fm[4]->travelSeg() += ts[4], ts[7], ts[2];
    fm[4]->classOfServiceVec().clear();
    fm[4]->classOfServiceVec().push_back(cosV[4]); // Booking Code used: I,Y,X
    fm[4]->classOfServiceVec().push_back(cosV[7]); // Booking Code used: I,Y,X
    fm[4]->classOfServiceVec().push_back(cosV[2]); // Booking Code used: I,Y,W

    Itin* itin1 = _memHandle.create<Itin>(); // FM 0,1,3
    Itin* itin2 = _memHandle.create<Itin>(); // FM 2,3,4

    // Itin and Itin2 shares same fare market 3 (fm[3])
    itin1->travelSeg() += ts[0], ts[1], ts[2], ts[3], ts[6];
    itin1->fareMarket() += fm[0], fm[1], fm[3];
    itin2->travelSeg() += ts[2], ts[4], ts[5], ts[6], ts[7];
    itin2->fareMarket() += fm[2], fm[3], fm[4];
    _pricingTrx->itin() += itin1, itin2;

    _pricingTrx->getRequest()->brandedFareSecondaryBookingCode() = sbkc;
    _pricingTrx->getRequest()->brandedFareEntry() = true;
    _pricingTrx->getRequest()->setBrandedFaresRequest(false);
  }

  void testBuildFareMarketForAncillaryPricingTrx()
  {
    AncillaryPricingTrx trx;
    testBuildFareMarket(trx);
  }

  void testIsJointUSCAFareMarket_Domestic()
  {
    preparePricingTrx();
    FareMarket fm;
    fm.geoTravelType() = GeoTravelType::Domestic;
    AirSeg s1, s2, s3;
    s1.carrier() = "F9";
    s2.carrier() = "AA";
    s3.carrier() = "F9";
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s2);
    fm.travelSeg().push_back(&s3);
    CPPUNIT_ASSERT(_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.travelSeg().clear();
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s3);
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
  }

  void testIsJointUSCAFareMarket_Transborder()
  {
    preparePricingTrx();
    FareMarket fm;
    fm.geoTravelType() = GeoTravelType::Transborder;
    AirSeg s1, s2, s3;
    s1.carrier() = "F9";
    s2.carrier() = "AA";
    s3.carrier() = "F9";
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s2);
    fm.travelSeg().push_back(&s3);
    CPPUNIT_ASSERT(_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.travelSeg().clear();
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s3);
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
  }

  void testIsJointUSCAFareMarket_SkipARNK()
  {
    preparePricingTrx();
    FareMarket fm;
    fm.geoTravelType() = GeoTravelType::Domestic;
    ArunkSeg a;
    AirSeg s1, s2;
    s1.carrier() = "F9";
    s2.carrier() = "AA";
    fm.travelSeg().push_back(&a);
    fm.travelSeg().push_back(&s1);
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.travelSeg().push_back(&s2);
    CPPUNIT_ASSERT(_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
  }
  void testIsJointUSCAFareMarket_NotUSCA()
  {
    preparePricingTrx();
    FareMarket fm;
    fm.geoTravelType() = GeoTravelType::International;
    AirSeg s1, s2, s3;
    s1.carrier() = "F9";
    s2.carrier() = "AA";
    s3.carrier() = "F9";
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s2);
    fm.travelSeg().push_back(&s3);
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.geoTravelType() = GeoTravelType::ForeignDomestic;
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.travelSeg().clear();
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s3);
    fm.geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.geoTravelType() = GeoTravelType::ForeignDomestic;
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
    fm.geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    CPPUNIT_ASSERT(!_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
  }

  void testIsJointUSCAFareMarket_FallBack()
  {
    preparePricingTrx();
    FareMarket fm;
    fm.geoTravelType() = GeoTravelType::Transborder;
    AirSeg s1, s2, s3;
    s1.carrier() = "F9";
    s2.carrier() = "AA";
    s3.carrier() = "F9";
    fm.travelSeg().push_back(&s1);
    fm.travelSeg().push_back(&s2);
    fm.travelSeg().push_back(&s3);

    CPPUNIT_ASSERT(_itinAnalyzerMock->_itinAnalyzerWrapper.isJointUSCAFareMarket(&fm));
  }

  void testSetInfoForSimilarItinsCallsSetFareFamilyIds()
  {
    PricingTrx trx;
    CPPUNIT_ASSERT(!_itinAnalyzerMock->isVisited(ItinAnalyzerServiceMock::SET_FARE_FAMILIES_IDS));

    PricingRequest request;
    trx.setRequest(&request);

    PricingOptions* option;
    trx.dataHandle().get(option);
    trx.setOptions(option);

    Itin itin;
    setUpItin(trx, itin);
    itin.itinNum() = 5;

    trx.itin().push_back(&itin);

    trx.travelSeg() += itin.travelSeg()[0], itin.travelSeg()[1], itin.travelSeg()[2],
        itin.travelSeg()[3];

    trx.setTrxType(PricingTrx::MIP_TRX);

    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    paxType.paxTypeInfo() = &paxTypeInfo;
    trx.paxType().push_back(&paxType);

    _itinAnalyzerMock->process(trx);
    CPPUNIT_ASSERT(itin.isHeadOfFamily());
    CPPUNIT_ASSERT(itin.getItinFamily() == 5);
  }

  void testSetFareFamilyIds()
  {
    PricingTrx trx;

    Itin* i1_1 = _memHandle.create<Itin>();
    Itin* i1_2 = _memHandle.create<Itin>();
    Itin* i1_3 = _memHandle.create<Itin>();
    Itin* i2_1 = _memHandle.create<Itin>();
    Itin* i3_1 = _memHandle.create<Itin>();
    Itin* i3_2 = _memHandle.create<Itin>();

    i1_1->itinNum() = 0;
    i1_2->itinNum() = 1;
    i1_3->itinNum() = 2;
    i2_1->itinNum() = 3;
    i3_1->itinNum() = 4;
    i3_2->itinNum() = 5;

    CPPUNIT_ASSERT(!i1_1->isHeadOfFamily());
    CPPUNIT_ASSERT(!i1_2->isHeadOfFamily());
    CPPUNIT_ASSERT(!i1_3->isHeadOfFamily());
    CPPUNIT_ASSERT(!i2_1->isHeadOfFamily());
    CPPUNIT_ASSERT(!i3_1->isHeadOfFamily());
    CPPUNIT_ASSERT(!i3_2->isHeadOfFamily());

    i1_1->addSimilarItin(i1_2);
    i1_1->addSimilarItin(i1_3);
    i3_1->addSimilarItin(i3_2);

    i1_1->setItinFamily(1);
    i1_2->setItinFamily(1);
    i1_3->setItinFamily(1);
    i2_1->setItinFamily(1);
    i3_1->setItinFamily(1);
    i3_2->setItinFamily(1);

    trx.itin().push_back(i1_1);
    trx.itin().push_back(i2_1);
    trx.itin().push_back(i3_1);

    FamilyLogicUtils::setFareFamiliesIds(trx);

    CPPUNIT_ASSERT(i1_1->isHeadOfFamily());
    CPPUNIT_ASSERT(!i1_2->isHeadOfFamily());
    CPPUNIT_ASSERT(!i1_3->isHeadOfFamily());
    CPPUNIT_ASSERT(i2_1->isHeadOfFamily());
    CPPUNIT_ASSERT(i3_1->isHeadOfFamily());
    CPPUNIT_ASSERT(!i3_2->isHeadOfFamily());

    int i1_family = i1_1->getItinFamily();
    int i2_family = i2_1->getItinFamily();
    int i3_family = i3_1->getItinFamily();

    CPPUNIT_ASSERT(i1_family != i2_family);
    CPPUNIT_ASSERT(i1_family != i3_family);
    CPPUNIT_ASSERT(i2_family != i3_family);
    CPPUNIT_ASSERT(i1_family == i1_2->getItinFamily());
    CPPUNIT_ASSERT(i1_family == i1_3->getItinFamily());
    CPPUNIT_ASSERT(i3_family == i3_2->getItinFamily());
  }

  void testBuildFareMarket(AncillaryPricingTrx& trx)
  {
    AncRequest request;
    trx.setRequest(&request);
    PricingOptions* option;
    trx.dataHandle().get(option);
    trx.setOptions(option);

    // Build Itin
    Itin itin;
    setUpItin(trx, itin);

    // Add Itin to Trx
    trx.itin().push_back(&itin);
    trx.travelSeg() += itin.travelSeg()[0], itin.travelSeg()[1], itin.travelSeg()[2],
        itin.travelSeg()[3];

    _itinAnalyzerMock->process(trx);

    std::vector<FareMarket*>& fareMarkets = itin.fareMarket();
    /*
      std::vector<FareMarket*>::iterator iter = fareMarkets.begin();

      int marketIndex = 1;
      while(iter != fareMarkets.end())
      {
      FareMarket *fm = (*iter);

      std::cout << "**************** FARE MARKET "<< marketIndex++ << std::endl;
      std::cout << "Board: "<<fm->boardMultiCity()<<"  Off: "<< fm->offMultiCity() << std::endl;
      std::cout << "  governing carrier: "<< fm->governingCarrier() << std::endl;
      std::cout << "  includes: "<< fm->travelSeg().size()<<" travel segments: "
      << itinAnalyzer.getTravelSegOrderStr(itin, fm->travelSeg()) << std::endl;
      std::cout << "  includes: "<< fm->sideTripTravelSeg().size()<<" side trips: "
      << itinAnalyzer.getTravelSegOrderStr(itin, fm->sideTripTravelSeg()) << std::endl;
      std::cout << "  breakIndicator: "<< fm->breakIndicator() << std::endl;

      iter++;
      }*/

    CPPUNIT_ASSERT(itin.fareMarket().size() == 8);

    // Test cloneFareMarket
    FareMarket newFareMarket;

    _itinAnalyzerMock->cloneFareMarket(*fareMarkets[2], newFareMarket, "UA");
    CPPUNIT_ASSERT(newFareMarket.governingCarrier() == "UA");
  }

  void setUpItin(AncillaryPricingTrx& trx, Itin& itin)
  {
    // Initialize Location
    const Loc* loc1 = trx.dataHandle().getLoc("SCL", DateTime::localTime());
    const Loc* loc2 = trx.dataHandle().getLoc("MIA", DateTime::localTime());
    const Loc* loc3 = trx.dataHandle().getLoc("SDQ", DateTime::localTime());
    const Loc* loc4 = trx.dataHandle().getLoc("CCS", DateTime::localTime());

    AirSeg* travelSeg1, *travelSeg2, *travelSeg3, *travelSeg4;

    trx.dataHandle().get(travelSeg1);
    trx.dataHandle().get(travelSeg2);
    trx.dataHandle().get(travelSeg3);
    trx.dataHandle().get(travelSeg4);
    // Build Travel Segments SCL-AA-MIA-AA-SDQ-AA-MIA-CCS

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 20);
    initAirSeg(*travelSeg2, GeoTravelType::International, loc2, loc3, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 21);
    initAirSeg(*travelSeg3, GeoTravelType::International, loc3, loc2, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 22);
    initAirSeg(*travelSeg4, GeoTravelType::International, loc2, loc4, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 23);

    itin.travelSeg() += travelSeg1, travelSeg2, travelSeg3, travelSeg4;

    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(travelSeg1->departureDT());
  }
  void testCloneFareMarket()
  {
    PricingTrx trx;
    FareCompInfo fci;
    FareMarket fm1, newFareMarket1;
    fm1.fareCompInfo() = &fci;

    _itinAnalyzerMock->cloneFareMarket(fm1, newFareMarket1, "UA");
    CPPUNIT_ASSERT(newFareMarket1.governingCarrier() == "UA");
    CPPUNIT_ASSERT(newFareMarket1.fareCompInfo() == &fci);
  }

  void testIsValidSideTripCombinationTrueWhenValid()
  {
    PricingTrx trx;
    // Build Itin AAA-BBB-CCC-DDD-BBB-EEE-FFF-EEE-GGG
    Itin itin;
    setUpItinForSideTrip(trx, itin);
    std::vector<std::vector<TravelSeg*>> sideTrips;
    std::vector<TravelSeg*> sideTrip;
    //  sideTrip  BBB-CCC-DDD-BBB
    sideTrip.push_back(itin.travelSeg()[1]);
    sideTrip.push_back(itin.travelSeg()[2]);
    sideTrip.push_back(itin.travelSeg()[3]);

    sideTrips.push_back(sideTrip);
    // new sideTrip EEE-FFF-EEE
    std::vector<TravelSeg*> newSt;
    newSt.push_back(itin.travelSeg()[5]);
    newSt.push_back(itin.travelSeg()[6]);

    CPPUNIT_ASSERT(ItinAnalyzerServiceWrapper::isValidSideTripCombination(itin, sideTrips, newSt));
  }

  void testIsValidSideTripCombinationTrueWhenValid1()
  {
    PricingTrx trx;
    // Build Itin AAA-BBB-CCC-DDD-BBB-EEE-FFF-EEE-GGG
    Itin itin;
    setUpItinForSideTrip(trx, itin);
    std::vector<std::vector<TravelSeg*>> sideTrips;
    std::vector<TravelSeg*> sideTrip;
    //  sideTrip  BBB-CCC-DDD-BBB
    sideTrip.push_back(itin.travelSeg()[1]);
    sideTrip.push_back(itin.travelSeg()[2]);
    sideTrip.push_back(itin.travelSeg()[3]);
    sideTrips.push_back(sideTrip);

    // modify itin: AAA-BBB-CCC-DDD-BBB-EEE-BBB-FFF-GGG
    itin.travelSeg()[5]->offMultiCity() = "BBB";
    itin.travelSeg()[6]->boardMultiCity() = "BBB";
    // new sideTrip BBB-EEE-BBB
    std::vector<TravelSeg*> newSt;
    newSt.push_back(itin.travelSeg()[4]);
    newSt.push_back(itin.travelSeg()[5]);

    CPPUNIT_ASSERT(ItinAnalyzerServiceWrapper::isValidSideTripCombination(itin, sideTrips, newSt));
  }

  void testIsValidSideTripCombinationTrueWhenValid2()
  {
    PricingTrx trx;
    // Build Itin AAA-BBB-CCC-DDD-CCC-BBB-FFF
    Itin itin;
    setUpItinForSideTrip2(trx, itin);
    std::vector<std::vector<TravelSeg*>> sideTrips;
    std::vector<TravelSeg*> sideTrip;
    //  sideTrip  CCC-DDD-CCC
    sideTrip.push_back(itin.travelSeg()[2]);
    sideTrip.push_back(itin.travelSeg()[3]);
    sideTrips.push_back(sideTrip);

    std::vector<TravelSeg*> newSt;
    //  sideTrip  BBB-CCC-DDD-CCC-BBB
    newSt.push_back(itin.travelSeg()[1]);
    newSt.push_back(itin.travelSeg()[2]);
    newSt.push_back(itin.travelSeg()[3]);
    newSt.push_back(itin.travelSeg()[4]);

    CPPUNIT_ASSERT(ItinAnalyzerServiceWrapper::isValidSideTripCombination(itin, sideTrips, newSt));
  }

  void testIsValidSideTripCombinationFalseWhenNotValid()
  {
    PricingTrx trx;
    // Build Itin AAA-BBB-CCC-DDD-BBB-EEE-FFF-EEE-GGG
    Itin itin;
    setUpItinForSideTrip(trx, itin);
    std::vector<std::vector<TravelSeg*>> sideTrips;
    std::vector<TravelSeg*> sideTrip;
    //  sideTrip  BBB-CCC-DDD-BBB
    sideTrip.push_back(itin.travelSeg()[1]);
    sideTrip.push_back(itin.travelSeg()[2]);
    sideTrip.push_back(itin.travelSeg()[3]);
    sideTrips.push_back(sideTrip);
    // modify itin: AAA-BBB-CCC-DDD-BBB-DDD-FFF-EEE-GGG
    itin.travelSeg()[4]->offMultiCity() = "DDD";
    itin.travelSeg()[5]->boardMultiCity() = "DDD";
    // new sideTrip DDD-BBB-DDD
    std::vector<TravelSeg*> newSt;
    newSt.push_back(itin.travelSeg()[3]);
    newSt.push_back(itin.travelSeg()[4]);

    CPPUNIT_ASSERT(!ItinAnalyzerServiceWrapper::isValidSideTripCombination(itin, sideTrips, newSt));
  }

  void testIsValidSideTripCombinationFalseWhenNotValid1()
  {
    PricingTrx trx;
    // Build Itin AAA-BBB-CCC-BBB-CCC-FFF
    Itin itin;
    setUpItinForSideTrip3(trx, itin);
    std::vector<std::vector<TravelSeg*>> sideTrips;

    std::vector<TravelSeg*> sideTrip;
    //  sideTrip  BBB-CCC-BBB
    sideTrip.push_back(itin.travelSeg()[1]);
    sideTrip.push_back(itin.travelSeg()[2]);
    sideTrips.push_back(sideTrip);
    // new sideTrip CCC-BBB-CCC
    std::vector<TravelSeg*> newSt;
    newSt.push_back(itin.travelSeg()[2]);
    newSt.push_back(itin.travelSeg()[3]);

    CPPUNIT_ASSERT(!ItinAnalyzerServiceWrapper::isValidSideTripCombination(itin, sideTrips, newSt));
  }

  void testIsValidSideTripCombinationFalseWhenNotValid2()
  {
    PricingTrx trx;
    // Build Itin AAA-BBB-CCC-BBB-DDD-CCC-FFF
    Itin itin;
    setUpItinForSideTrip4(trx, itin);
    std::vector<std::vector<TravelSeg*>> sideTrips;

    std::vector<TravelSeg*> sideTrip;
    //  sideTrip  BBB-CCC-BBB
    sideTrip.push_back(itin.travelSeg()[1]);
    sideTrip.push_back(itin.travelSeg()[2]);
    sideTrips.push_back(sideTrip);
    // new sideTrip CCC-BBB-DDD-CCC
    std::vector<TravelSeg*> newSt;
    newSt.push_back(itin.travelSeg()[2]);
    newSt.push_back(itin.travelSeg()[3]);
    newSt.push_back(itin.travelSeg()[4]);

    CPPUNIT_ASSERT(!ItinAnalyzerServiceWrapper::isValidSideTripCombination(itin, sideTrips, newSt));
  }

  void testCopyFromInvalidSideTripWhenEmptyNewSideTrip()
  {
    std::vector<std::vector<TravelSeg*>> sideTrips;
    std::vector<TravelSeg*> sideTripPrepare;
    std::vector<TravelSeg*> newSt;
    TravelSegs travelSegs;
    AirSeg tSeg1, tSeg2, tSeg3, tSeg4;
    sideTripPrepare.push_back(&tSeg1);
    sideTripPrepare.push_back(&tSeg2);
    sideTripPrepare.push_back(&tSeg3);
    sideTripPrepare.push_back(&tSeg4);
    sideTrips.push_back(sideTripPrepare);

    ItinAnalyzerServiceWrapper::copyFromInvalidSideTrip(sideTrips, newSt, travelSegs);

    CPPUNIT_ASSERT(travelSegs.empty());
  }

  void testCopyFromInvalidSideTripWhenInvalidNewSideTrip()
  {
    std::vector<std::vector<TravelSeg*>> sideTrips;
    std::vector<TravelSeg*> sideTripPrepare;
    std::vector<TravelSeg*> newSt;
    TravelSegs travelSegs;
    AirSeg tSeg1, tSeg2, tSeg3, tSeg4, tSeg5;

    sideTripPrepare.push_back(&tSeg2);
    sideTripPrepare.push_back(&tSeg3);
    sideTripPrepare.push_back(&tSeg4);

    sideTrips.push_back(sideTripPrepare);

    newSt.push_back(&tSeg3);
    newSt.push_back(&tSeg4);
    newSt.push_back(&tSeg5);

    travelSegs.push_back(&tSeg1);

    ItinAnalyzerServiceWrapper::copyFromInvalidSideTrip(sideTrips, newSt, travelSegs);

    CPPUNIT_ASSERT(travelSegs.size() == 2); // tSeg1 and tSeg5
  }

  void testSetValidatingCarrierWorksOnAltDatesItins()
  {
    preparePricingTrx();
    Itin* itin1 = prepareItinAndFareMarkets();
    Itin* itin2 = prepareItinAndFareMarkets();

    PricingTrx::AltDateInfo* altDateInfo =
        _pricingTrx->dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo->journeyItin = itin2;

    _pricingTrx->itin() += itin1;
    _pricingTrx->altDatePairs().insert(std::make_pair(DatePair(), altDateInfo));

    // make sure the carrier is not set when we start
    CPPUNIT_ASSERT(itin1->validatingCarrier().empty());
    CPPUNIT_ASSERT(itin2->validatingCarrier().empty());

    _itinAnalyzer->setValidatingCarrier(*_pricingTrx);

    // make sure the carrier is set for itins in trx.itin()
    // and those in altDatePairs
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), itin1->validatingCarrier());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), itin2->validatingCarrier());
  }

  void populateItin(const GeoTravelType& type)
  {
    _newItin->geoTravelType() = type;
    _newItin->travelSeg() += _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>();
    Loc* loc = _memHandle.create<Loc>();
    _newItin->travelSeg().front()->origin() = loc;
    loc->nation() = GlobalDirection::US;
    _newItin->travelSeg().front()->boardMultiCity() = "NYC";
  }

  FareMarket* createFareMarket(const GeoTravelType& type)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->geoTravelType() = type;
    fm->travelSeg() += _memHandle.create<AirSeg>();
    fm->destination() = fm->origin() = _newItin->travelSeg().front()->origin();
    return fm;
  }

  void testGetInboundOutbound_StartFromOriginPoint()
  {
    populateItin(GeoTravelType::International);
    FareMarket fm;
    fm.travelSeg() += _newItin->travelSeg().front();

    CPPUNIT_ASSERT(
        FMDirection::OUTBOUND ==
        _itinAnalyzer->getInboundOutbound(*_newItin, fm, _pricingTrx->dataHandle()));
  }

  void testGetInboundOutbound_InternationalJourney_ReturnToOrigin_Domestic()
  {
    populateItin(GeoTravelType::International);
    const FareMarket& fm = *createFareMarket(GeoTravelType::Domestic);

    CPPUNIT_ASSERT(
        FMDirection::OUTBOUND ==
        _itinAnalyzer->getInboundOutbound(*_newItin, fm, _pricingTrx->dataHandle()));
  }

  void testGetInboundOutbound_InternationalJourney_ReturnToOrigin_ForeignDomestic()
  {
    populateItin(GeoTravelType::International);
    const FareMarket& fm = *createFareMarket(GeoTravelType::ForeignDomestic);

    CPPUNIT_ASSERT(
        FMDirection::OUTBOUND ==
        _itinAnalyzer->getInboundOutbound(*_newItin, fm, _pricingTrx->dataHandle()));
  }

  void testGetInboundOutbound_InternationalJourney_ReturnToOrigin_Transborder()
  {
    populateItin(GeoTravelType::International);
    const FareMarket& fm = *createFareMarket(GeoTravelType::Transborder);

    CPPUNIT_ASSERT(
        FMDirection::OUTBOUND ==
        _itinAnalyzer->getInboundOutbound(*_newItin, fm, _pricingTrx->dataHandle()));
  }

  void testGetInboundOutbound_InternationalJourney_ReturnToOrigin_International()
  {
    populateItin(GeoTravelType::International);
    const FareMarket& fm = *createFareMarket(GeoTravelType::International);

    CPPUNIT_ASSERT(
        FMDirection::INBOUND ==
        _itinAnalyzer->getInboundOutbound(*_newItin, fm, _pricingTrx->dataHandle()));
  }

  TravelSeg* makeTravelSeg(const DateTime& departureDate)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->departureDT() = departureDate;
    airSeg->origin() = _memHandle.create<Loc>();
    return airSeg;
  }

  void testMarkFlownSegments()
  {
    DateTime localTime = DateTime::localTime();
    DateTime future = localTime.addDays(1);
    DateTime past = localTime.subtractDays(1);

    Itin firstItin;
    firstItin.travelSeg().push_back(makeTravelSeg(past));
    firstItin.travelSeg().push_back(makeTravelSeg(past));
    firstItin.travelSeg().push_back(makeTravelSeg(future));
    _pricingTrx->itin().push_back(&firstItin);

    Itin secondItin;
    secondItin.travelSeg().push_back(makeTravelSeg(past));
    secondItin.travelSeg().push_back(makeTravelSeg(future));
    secondItin.travelSeg().push_back(makeTravelSeg(future));
    _pricingTrx->itin().push_back(&secondItin);

    _itinAnalyzer->markFlownSegments(*_pricingTrx);

    CPPUNIT_ASSERT(!_pricingTrx->itin()[0]->travelSeg()[0]->unflown());
    CPPUNIT_ASSERT(!_pricingTrx->itin()[0]->travelSeg()[1]->unflown());
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->travelSeg()[2]->unflown());
    CPPUNIT_ASSERT(!_pricingTrx->itin()[1]->travelSeg()[0]->unflown());
    CPPUNIT_ASSERT(_pricingTrx->itin()[1]->travelSeg()[1]->unflown());
    CPPUNIT_ASSERT(_pricingTrx->itin()[1]->travelSeg()[2]->unflown());
  }

  void testProcessFMsForValidatingCarriers()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    FareMarket fm1, fm2;
    AirSeg tSeg1, tSeg2;
    tSeg1.legId() = 0;
    tSeg2.legId() = 1;

    Itin itin1, itin2;
    itin1.travelSeg().push_back(&tSeg1);
    itin2.travelSeg().push_back(&tSeg1);
    itin2.travelSeg().push_back(&tSeg2);

    fm1.travelSeg().push_back(&tSeg1);
    fm2.travelSeg().push_back(&tSeg2);

    PricingTrx trx;
    trx.setTrxType(PricingTrx::PRICING_TRX);

    trx.itin().push_back(&itin1);

    trx.fareMarket().push_back(&fm1);
    trx.fareMarket().push_back(&fm2);

    itin1.fareMarket().push_back(&fm1);

    vcx::ValidatingCxrData vcd;
    ValidatingCxrGSAData v1, v2, vc;

    v1.validatingCarriersData()["AA"] = vcd;

    itin1.validatingCxrGsaData() = &v1;

    _itinAnalyzer->processFMsForValidatingCarriers(trx);

    CPPUNIT_ASSERT(fm1.validatingCarriers().empty());
    CPPUNIT_ASSERT(fm2.validatingCarriers().empty());

    trx.itin().push_back(&itin2);
    v1.validatingCarriersData()["BB"] = vcd;

    itin2.fareMarket().push_back(&fm1);
    itin2.fareMarket().push_back(&fm2);

    // Test multiple itins, multiple VCs

    v2.validatingCarriersData()["AA"] = vcd;
    v2.validatingCarriersData()["CC"] = vcd;

    itin2.validatingCxrGsaData() = &v2;

    _itinAnalyzer->processFMsForValidatingCarriers(trx);

    CPPUNIT_ASSERT(fm1.validatingCarriers().size() == 3);
    CPPUNIT_ASSERT(fm2.validatingCarriers().size() == 2);

    CPPUNIT_ASSERT(fm1.validatingCarriers()[0] == "AA");
    CPPUNIT_ASSERT(fm1.validatingCarriers()[1] == "BB");
    CPPUNIT_ASSERT(fm1.validatingCarriers()[2] == "CC");

    CPPUNIT_ASSERT(fm2.validatingCarriers()[0] == "AA");
    CPPUNIT_ASSERT(fm2.validatingCarriers()[1] == "CC");

    // Test child itin
    Itin childItin;
    FareMarket fm3, fm4;

    AirSeg a1, a2, a3;
    a1.legId() = 2;
    a2.legId() = 3;
    a3.legId() = 3;

    // fm3 is thru, fm4 is not thru
    fm3.travelSeg().push_back(&a1);
    fm4.travelSeg().push_back(&a2);

    itin2.travelSeg().push_back(&a1);
    itin2.travelSeg().push_back(&a2);
    itin2.travelSeg().push_back(&a3);

    vc.validatingCarriersData()["AA"] = vcd;
    vc.validatingCarriersData()["BB"] = vcd;
    vc.validatingCarriersData()["NN"] = vcd;
    vc.validatingCarriersData()["ZZ"] = vcd;

    childItin.validatingCxrGsaData() = &vc;

    itin2.fareMarket().push_back(&fm3);
    itin2.fareMarket().push_back(&fm4);
    trx.fareMarket().push_back(&fm3);
    trx.fareMarket().push_back(&fm4);

    itin2.addSimilarItin(&childItin);
    _itinAnalyzer->processFMsForValidatingCarriers(trx);

    // Mother itin has AA, CC. Child itin has AA, BB, NN (thru),ZZ (non-thru)
    CPPUNIT_ASSERT(fm3.validatingCarriers().size() == 5);
    CPPUNIT_ASSERT(fm4.validatingCarriers().size() == 2);

    CPPUNIT_ASSERT(fm3.validatingCarriers()[0] == "AA");
    CPPUNIT_ASSERT(fm3.validatingCarriers()[1] == "BB");
    CPPUNIT_ASSERT(fm3.validatingCarriers()[2] == "CC");
    CPPUNIT_ASSERT(fm3.validatingCarriers()[3] == "NN");
    CPPUNIT_ASSERT(fm3.validatingCarriers()[4] == "ZZ");

    CPPUNIT_ASSERT(fm4.validatingCarriers()[0] == "AA");
    CPPUNIT_ASSERT(fm4.validatingCarriers()[1] == "CC");
  }

  void setSegmentInPricingTrx(PricingTrx* trx)
  {
    Itin* itin = _memHandle.create<Itin>();
    AirSeg* travelSeg = _memHandle.create<AirSeg>();

    travelSeg->arrivalDT() = DateTime(2014, 11, 20, 4, 30);
    travelSeg->departureDT() = DateTime(2014, 11, 20, 3, 30);
    itin->travelSeg() += travelSeg;
    trx->itin().clear();
    trx->itin().push_back(itin);
  }

  void testAddFakeTravelSeg_emptyDepartureDates()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime::emptyDate();
    _trx->inboundDepartureDate() = DateTime::emptyDate();

    CPPUNIT_ASSERT(!_itinAnalyzer->addFakeTravelSeg(*_trx));
  }

  void testAddFakeTravelSeg_bothDepartureDates()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime(2014, 11, 20, 3, 0);
    _trx->inboundDepartureDate() = DateTime(2014, 11, 20, 5, 0);

    CPPUNIT_ASSERT(!_itinAnalyzer->addFakeTravelSeg(*_trx));
  }

  void testAddFakeTravelSeg_emptyItin()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime(2014, 11, 20, 3, 0);
    _trx->inboundDepartureDate() = DateTime::emptyDate();
    _trx->itin().clear();

    CPPUNIT_ASSERT(_itinAnalyzer->addFakeTravelSeg(*_trx));
  }

  void testAddFakeTravelSeg_fakeOnOutbound_timeFromDepartureDate()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime(2014, 11, 19, 23, 29, 58);
    _trx->inboundDepartureDate() = DateTime::emptyDate();

    CPPUNIT_ASSERT(_itinAnalyzer->addFakeTravelSeg(*_trx));
    CPPUNIT_ASSERT(DateTime(2014, 11, 19, 23, 29, 58) ==
                   _trx->itin().front()->travelSeg().front()->toAirSeg()->departureDT());
    CPPUNIT_ASSERT(DateTime(2014, 11, 19, 23, 29, 58) ==
                   _trx->itin().front()->travelSeg().front()->toAirSeg()->arrivalDT());
  }

  void testAddFakeTravelSeg_fakeOnOutbound_timeDependsOnAdjacentSegment()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime(2014, 11, 19, 23, 30, 0);
    _trx->inboundDepartureDate() = DateTime::emptyDate();

    CPPUNIT_ASSERT(_itinAnalyzer->addFakeTravelSeg(*_trx));
    CPPUNIT_ASSERT(DateTime(2014, 11, 19, 23, 29, 59) ==
                   _trx->itin().front()->travelSeg().front()->toAirSeg()->departureDT());
    CPPUNIT_ASSERT(DateTime(2014, 11, 19, 23, 29, 59) ==
                   _trx->itin().front()->travelSeg().front()->toAirSeg()->arrivalDT());
  }

  void testAddFakeTravelSeg_fakeOnInbound_timeFromDepartureDate()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime::emptyDate();
    _trx->inboundDepartureDate() = DateTime(2014, 11, 20, 8, 30, 2);

    CPPUNIT_ASSERT(_itinAnalyzer->addFakeTravelSeg(*_trx));
    CPPUNIT_ASSERT(DateTime(2014, 11, 20, 8, 30, 2) ==
                   _trx->itin().front()->travelSeg().back()->toAirSeg()->departureDT());
    CPPUNIT_ASSERT(DateTime(2014, 11, 20, 8, 30, 2) ==
                   _trx->itin().front()->travelSeg().back()->toAirSeg()->arrivalDT());
  }

  void testAddFakeTravelSeg_fakeOnInbound_timeDependsOnAdjacentSegment()
  {
    setSegmentInPricingTrx(_trx);
    _trx->outboundDepartureDate() = DateTime::emptyDate();
    _trx->inboundDepartureDate() = DateTime(2014, 11, 20, 8, 30, 0);

    CPPUNIT_ASSERT(_itinAnalyzer->addFakeTravelSeg(*_trx));
    CPPUNIT_ASSERT(DateTime(2014, 11, 20, 8, 30, 1) ==
                   _trx->itin().front()->travelSeg().back()->toAirSeg()->departureDT());
    CPPUNIT_ASSERT(DateTime(2014, 11, 20, 8, 30, 1) ==
                   _trx->itin().front()->travelSeg().back()->toAirSeg()->arrivalDT());
  }

  void testIsPopulateFareMarkets_True()
  {
    FareMarket fm1, fm2;
    Itin itin1;

    PricingTrx trx;
    trx.setTrxType(PricingTrx::PRICING_TRX);

    trx.itin().push_back(&itin1);

    trx.fareMarket().push_back(&fm1);
    trx.fareMarket().push_back(&fm2);

    itin1.fareMarket().push_back(&fm1);

    vcx::ValidatingCxrData vcd;
    ValidatingCxrGSAData v1;

    v1.validatingCarriersData()["AA"] = vcd;

    SpValidatingCxrGSADataMap* spGsaDataMap =
        _trx->dataHandle().create<SpValidatingCxrGSADataMap>();
    spGsaDataMap->insert(std::pair<SettlementPlanType, const ValidatingCxrGSAData*>("BSP", &v1));
    spGsaDataMap->insert(std::pair<SettlementPlanType, const ValidatingCxrGSAData*>("TCH", &v1));
    itin1.spValidatingCxrGsaDataMap() = spGsaDataMap;

    bool isRepricingFromMIP = false;
    CPPUNIT_ASSERT_EQUAL(true, _itinAnalyzer->needToPopulateFareMarkets(trx, isRepricingFromMIP));
  }

  void testIsPopulateFareMarkets_False()
  {
    FareMarket fm1, fm2;
    Itin itin1;

    PricingTrx trx;
    trx.setTrxType(PricingTrx::PRICING_TRX);

    trx.itin().push_back(&itin1);

    trx.fareMarket().push_back(&fm1);
    trx.fareMarket().push_back(&fm2);

    itin1.fareMarket().push_back(&fm1);

    vcx::ValidatingCxrData vcd;
    ValidatingCxrGSAData v1, v2, vc;

    v1.validatingCarriersData()["AA"] = vcd;

    SpValidatingCxrGSADataMap* spGsaDataMap =
        _trx->dataHandle().create<SpValidatingCxrGSADataMap>();
    spGsaDataMap->insert(std::pair<SettlementPlanType, const ValidatingCxrGSAData*>("BSP", &v1));
    itin1.spValidatingCxrGsaDataMap() = spGsaDataMap;

    bool isRepricingFromMIP = false;
    CPPUNIT_ASSERT_EQUAL(false, _itinAnalyzer->needToPopulateFareMarkets(trx, isRepricingFromMIP));
  }

  void testIsItinHasValidatingCxrData_True()
  {
    FareMarket fm1, fm2;
    Itin itin1;

    PricingTrx trx;
    trx.setTrxType(PricingTrx::PRICING_TRX);

    trx.itin().push_back(&itin1);

    trx.fareMarket().push_back(&fm1);
    trx.fareMarket().push_back(&fm2);

    itin1.fareMarket().push_back(&fm1);

    vcx::ValidatingCxrData vcd;
    ValidatingCxrGSAData v1, v2, vc;

    v1.validatingCarriersData()["AA"] = vcd;

    SpValidatingCxrGSADataMap* spGsaDataMap =
        _trx->dataHandle().create<SpValidatingCxrGSADataMap>();
    spGsaDataMap->insert(std::pair<SettlementPlanType, const ValidatingCxrGSAData*>("BSP", &v1));
    itin1.spValidatingCxrGsaDataMap() = spGsaDataMap;

    CPPUNIT_ASSERT_EQUAL(true, _itinAnalyzer->itinHasValidatingCxrData(trx, itin1));
  }

  void testIsItinHasValidatingCxrData_False()
  {
    FareMarket fm1, fm2;
    Itin itin1;

    PricingTrx trx;
    trx.setTrxType(PricingTrx::PRICING_TRX);
    trx.itin().push_back(&itin1);
    trx.fareMarket().push_back(&fm1);
    trx.fareMarket().push_back(&fm2);
    itin1.fareMarket().push_back(&fm1);

    CPPUNIT_ASSERT_EQUAL(false, _itinAnalyzer->itinHasValidatingCxrData(trx, itin1));
  }

  // Atleast one flight in both of the itin is international with primary booking code
  void testZeroOutIntlFlightSecondaryRBD_IntlFlightWithPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('W'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Itin0: Setting Seg2 (JFK->PHL, Booking Code: I,Y,W) & Seg3 (CHI->PHL, Booking Code: I,Y,W) as
    // internationalwith primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;

    // Itin1: Setting Seg5 (DFW->PHL, Booking Code: I,Y,X) as international with no primary booking
    // code R/W
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): Atleast one international flight with primary booking code-------------
    // FM(0)->Seg2: JFK->PHL International Flight (with Primary Booking Code), Zero out seats for
    // secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->numSeats());

    // FM(1)-> Seg3: CHI->PHL International Flight (with Primary Booking Code), Zero out seats for
    // secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());

    // FM(3)-> Seg2: JFK->PHL International Flight (with Primary Booking Code), Zero out seats for
    // secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[1]))[1]->numSeats());

    //-------------Itin(1): Atleast one international flight with primary booking code-------------
    // FM(3)->Seg2: JFK->PHL International Flight (with Primary Booking Code), Zero out seats for
    // secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());

    // FM(4)->Seg2: JFK->PHL International Flight (with Primary Booking Code), Zero out seats for
    // secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->numSeats());
  }

  // Mixed International Flights with/without primary booking code
  void testDoNotZeroOutIntlFlightSecondaryRBD_IntlFlightWithoutPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('W'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Itin0: Setting Seg2 (JFK->PHL, Booking Code: I,Y,W) & Seg3 (CHI->PHL, Booking Code: I,Y,W) as
    // international with primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin1: Setting Seg5 (DFW->PHL, Booking Code: I,Y,X) as international with no primary booking
    // code R/W
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(1): Atleast one international flight with primary booking code-------------
    // FM(2)->Seg5: DFW->PHL International Flight (with No Primary Booking Code), Do not Zero out
    // seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());
  }

  // Atleast one flight in both of the itin is international with primary booking code
  void testDoNotZeroOutDomesticFlightSecondaryRBD_IntlFlightWithPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('W'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Itin0: Setting 3rd (JFK->PHL, Booking Code: I,Y,W) & 4th (CHI->PHL, Booking Code: I,Y,W)
    // segments as international segments, having primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin1: Setting 3rd segment (DFW->PHL, Booking Code: I,Y,X) as international flight which does
    // not have primary booking code R/W
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): Atleast one international flight with primary booking code-------------
    // FM(0)->Seg0: DSM->CHI Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(0)->Seg1: CHI->JFK Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());

    // FM(1)-> Seg0: DSM->CHI Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(3)-> Seg6: DSM->JFK Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[0]))[1]->numSeats());

    //-------------Itin(1): Atleast one international flight with primary booking code-------------
    // FM(2)->Seg4: DSM->DFW Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(3)->Seg6: DSM->JFK Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(4)->Seg4: DSM->DFW Domestic Flight, Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(4)->Seg7: DFW->JFK Domestic Flight , Do not Zero out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[1]))[1]->numSeats());
  }

  // Atleast one flight in both of the itin is international with primary booking code but is
  // dummyFare
  void testDoNotZeroOutDummyFlightSecondaryRBD_IntlFlightWithPrimaryRBD()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('W'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Itin0: Setting Seg2 (JFK->PHL, Booking Code: I,Y,W) & Seg3 (CHI->PHL, Booking Code: I,Y,W) as
    // international with primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin1: Setting Seg5 (DFW->PHL, Booking Code: I,Y,X) as international without primary booking
    // code W/R
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    // Set Itin0:Seg2 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[0]->travelSeg()[2])->makeFake();
    // Set Itin1:Seg2 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[0]->travelSeg()[0])->makeFake();

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    // FM(0)-> Seg2: JFK-PHL International Flight (DummyFare with Primary Booking Code), Do not Zero
    // out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->numSeats());

    // FM(3)-> Seg2: JFK-PHL International Flight (DummyFare with Primary Booking Code), Do not Zero
    // out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());
    // FM(4)-> Seg2: JFK-PHL International Flight (DummyFare with Primary Booking Code), Do not Zero
    // out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->numSeats());
  }

  // No flight is international with primary booking code in Itin(1)
  // but atleast one international flight with Primary Booking Code in Itin(2)
  void testZeroOutSecondaryRBD_GoodItinWithIntlFlightPrimaryRBD()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('X'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;
    // Itin1: Setting 3rd (JFK->PHL, Booking Code: I,Y,W) & 4th (CHI->PHL, Booking Code: I,Y,W)
    // segments as international segments, having primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin2: Setting 3rd segment (DFW->PHL, Booking Code: I,Y,X) as international flight which does
    // not have primary booking code R/W
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): International Flights but none with primary booking code Bad
    // Itin-------------
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->errResponseCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT_EQUAL(_pricingTrx->itin()[0]->errResponseMsg(),
                         std::string("INVALID ITIN FOR THE CLASS USED"));

    //-------------Itin(1): Atleast one international flight with primary booking code Good
    // Itin-------------
    // FM(2)->Seg5: DFW->PHL International Flight (with Primary Booking Code), Zero out seats for
    // secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());
  }

  // Fare Market 3: 2 Segments (Seg5: DSM-DFW, Seg6: DFW-PHL) is shared between Itin0 and Itin1
  // Test: Itin0 is invalid and Itin1 changes Fare Market 3. Verify, it is reflected in Itin0 too
  void testVerifyFMsSharedAcrossItins()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('X'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;
    // Itin1: Setting 3rd (JFK->PHL, Booking Code: I,Y,W) & 4th (CHI->PHL, Booking Code: I,Y,W)
    // segments as international segments, without primary booking code X
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin2: Setting 3rd segment (DFW->PHL, Booking Code: I,Y,X) as international flight which have
    // primary booking code X
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): International Flights but none with primary booking code Bad
    // Itin-------------
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->errResponseCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT_EQUAL(_pricingTrx->itin()[0]->errResponseMsg(),
                         std::string("INVALID ITIN FOR THE CLASS USED"));

    // FM(0), FM(1) are bad Fare Market but not FM(3) as it exists in good Itin 1
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->fareMarket()[0]->failCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->fareMarket()[1]->failCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->fareMarket()[2]->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    //-------------Itin(1): Atleast one international flight with primary booking code Good
    // Itin-------------
    // FM(3)->Seg2: JFK->PHL International Flight (no primar booking code), Good Fare Market
    CPPUNIT_ASSERT(_pricingTrx->itin()[1]->fareMarket()[1]->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
  }

  // Fare Market 3: 2 Segments (Seg5: DSM-DFW, Seg6: DFW-PHL) is shared between Itin1 and Itin2
  // Test: Itin1 is invalid and verify that FareMarket 3 does not exist in BadItinFMList
  void testVerifyBadItinContainsUniqueFMs()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('X'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;
    // Itin1: Setting 3rd (JFK->PHL, Booking Code: I,Y,W) & 4th (CHI->PHL, Booking Code: I,Y,W)
    // segments as international segments, having primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin2: Setting 3rd segment (DFW->PHL, Booking Code: I,Y,X) as international flight which does
    // not have primary booking code R/W
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(1): International Flights but none with primary booking code-------------
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->errResponseCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT_EQUAL(_pricingTrx->itin()[0]->errResponseMsg(),
                         std::string("INVALID ITIN FOR THE CLASS USED"));

    // Verify that FareMarket 3 is not set with failCode()
    CPPUNIT_ASSERT((_pricingTrx->itin()[0]->fareMarket()[2])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    // Verify that FareMarkets in valid ITin2 does not have failCode()
    CPPUNIT_ASSERT((_pricingTrx->itin()[1]->fareMarket()[0])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT((_pricingTrx->itin()[1]->fareMarket()[1])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT((_pricingTrx->itin()[1]->fareMarket()[2])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
  }

  // Atleast one flight in both of the itin is international with primary booking code but is
  // dummyFare
  void testVerifyInvalidItin_IntlDummyFlightWithPrimaryRBD()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('W'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Itin0: Setting Seg2 (JFK->PHL, Booking Code: I,Y,W) & Seg3 (CHI->PHL, Booking Code: I,Y,W) as
    // international with primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin1: Setting Seg5 (DFW->PHL, Booking Code: I,Y,X) as international without primary booking
    // code W/R
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    // Set Itin0:Seg2 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[0]->travelSeg()[2])->makeFake();
    // Set Itin1:Seg2 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[1]->travelSeg()[0])->makeFake();

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    // Itin0->Valid: has atleast one flight with primary booking code(Seg3:CHI->PHL) without dummy
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->errResponseCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    CPPUNIT_ASSERT((_pricingTrx->itin()[0]->fareMarket()[0])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT((_pricingTrx->itin()[0]->fareMarket()[1])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT((_pricingTrx->itin()[0]->fareMarket()[2])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    // Itin1->inValid: has atleast one flight with primary booking code (Seg2:JFK->PHL) which is
    // dummy
    CPPUNIT_ASSERT(_pricingTrx->itin()[1]->errResponseCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);

    CPPUNIT_ASSERT((_pricingTrx->itin()[1]->fareMarket()[0])->failCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    // FM2 is shared between Itin0 and Itin1
    CPPUNIT_ASSERT((_pricingTrx->itin()[1]->fareMarket()[1])->failCode() !=
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT((_pricingTrx->itin()[1]->fareMarket()[2])->failCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
  }

  // Atleast one flight in the itin is international but none has primary booking code
  void testVerifyInvalidItins_IntlFlightsWithNoPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('A'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;
    // Itin1: Setting 3rd (JFK->PHL, Booking Code: I,Y,W) & 4th (CHI->PHL, Booking Code: I,Y,W)
    // segments as international segments, having primary booking code W
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;
    const_cast<NationCode&>(_pricingTrx->itin()[0]->travelSeg()[3]->destination()->nation()) =
        AUSTRALIA;
    // Itin2: Setting 3rd segment (DFW->PHL, Booking Code: I,Y,X) as international flight which does
    // not have primary booking code R/W
    const_cast<NationCode&>(_pricingTrx->itin()[1]->travelSeg()[2]->destination()->nation()) =
        AUSTRALIA;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(1): International Flights but none with primary booking code-------------
    CPPUNIT_ASSERT(_pricingTrx->itin()[0]->errResponseCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT_EQUAL(_pricingTrx->itin()[0]->errResponseMsg(),
                         std::string("INVALID ITIN FOR THE CLASS USED"));

    //-------------Itin(2): International Flights but none with primary booking code-------------
    CPPUNIT_ASSERT(_pricingTrx->itin()[1]->errResponseCode() ==
                   ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    CPPUNIT_ASSERT_EQUAL(_pricingTrx->itin()[1]->errResponseMsg(),
                         std::string("INVALID ITIN FOR THE CLASS USED"));
  }

  // All flights in the itin are domestic and atleast one flight has primary booking code
  void testZeroOutSecondaryRBD_ItinWithDomesticFlights()
  {
    prepareItinaryAndFMsForOldBrandedFare();
    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('X'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): All Domestic Flights. Atleast one with primary booking code-------------
    // FM(3)-> Seg6: DSM->JFK Domestic Flight (Found Primary RBD 'X'), Zero out seats for secondary
    // RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[0]))[1]->numSeats());

    //-------------Itin(1): Atleast one international flight with primary booking code-------------
    // FM(2)->Seg4: DSM->DFW Domestic Flight (Found Primary RBD 'X'), Zero out seats for secondary
    // RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(2)->Seg5: DFW->PHL Domestic Flight (Found Primary RBD 'X'), Zero out seats for secondary
    // RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());

    // FM(3)->Seg6: DSM->JFK Domestic Flight (Found Primary RBD 'X'), Zero out seats for secondary
    // RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(4)->Seg14 DSM->DFW Domestic Flight (Found Primary RBD 'X'), Zero out seats for secondary
    // RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[0]))[1]->numSeats());

    // FM(4)->Seg7: DFW->JFK Domestic Flight (Found Primary RBD 'X'), Zero out seats for secondary
    // RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[1]))[1]->numSeats());
  }

  // All flights in the itin are domestic and atleast one flight has primary booking code
  void testDoNotZeroOutSecondaryRBD_DummyDomesticFlightWithPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();
    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('W'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Set Itin0:Seg2 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[0]->travelSeg()[2])->makeFake();
    // Set Itin1:Seg2 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[0]->travelSeg()[0])->makeFake();
    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    // Itin0->Valid: has atleast one flight with primary booking code which is not dummy
    // FM(0)-> Seg2: JFK-PHL International Flight (DummyFare with Primary Booking Code), Do not Zero
    // out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->numSeats());

    // FM(3)-> Seg2: JFK-PHL International Flight (DummyFare with Primary Booking Code), Do not Zero
    // out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());
    // FM(4)-> Seg2: JFK-PHL International Flight (DummyFare with Primary Booking Code), Do not Zero
    // out seats for secondary RBD
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)9,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->numSeats());
  }

  // All flights in the itin are domestic and flights that have primary booking code are dummyFare
  void testZeroOutSecondaryRBD_DummyDomesticFlightWithPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();
    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('X'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    // Set Itin0:Seg6 (JFK->PHL) as dummy Segment
    static_cast<AirSeg*>(_pricingTrx->itin()[0]->travelSeg()[4])->makeFake();

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): All Domestic Flights. with 1 flight primary booking code
    //(dummyFare)-------------
    // Zero out seats for Secondary RBD FM0 and FM1, FM3 for all flights except the dummyFare with
    // Primary Booking Code
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->numSeats());

    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());

    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[1]))[1]->numSeats());
  }

  // All flights in the itin are domestic and none has primary booking code
  void testZeroOutSecondaryRBD_ItinWithDomesticFlights_NoPrimaryBC()
  {
    prepareItinaryAndFMsForOldBrandedFare();
    std::vector<BookingCode> pbkc;
    pbkc.push_back(BookingCode('P'));
    _pricingTrx->getRequest()->brandedFareBookingCode() = pbkc;

    _itinAnalyzer->processOldBrandedFareSecondaryRBD(*_pricingTrx);

    //-------------Itin(0): All Domestic Flights.None with primary booking code-------------
    // Zero out seats for Secondary RBD, for all flights
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[0]->classOfServiceVec()[2]))[1]->numSeats());

    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());

    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[0]->fareMarket()[2]->classOfServiceVec()[1]))[1]->numSeats());

    //-------------Itin(1): All Domestic Flights.None with primary booking code-------------
    // Zero out seats for Secondary RBD, for all flights
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[0]->classOfServiceVec()[1]))[1]->numSeats());

    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[1]->classOfServiceVec()[1]))[1]->numSeats());

    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[0]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[1]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[1]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        BookingCode('Y'),
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->bookingCode());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0,
        (*(_pricingTrx->itin()[1]->fareMarket()[2]->classOfServiceVec()[2]))[1]->numSeats());
  }

  void testsetAirSegWithReqSpecificCabin()
  {
    prepareItinaryAndFMsForOldBrandedFare();
    // Prepared 2 Itins with 5 fare markets.
    // Fare Market 0: 3 segments (Seg0: DSM-CHI, Seg1:CHI-JFK, Seg2: JFK-PHL)
    // Fare Market 1: 2 Segments (Seg0: DSM-CHI, Seg3:CHI-PHL)
    // Fare Market 2: 2 Segments (Seg4: DSM-DFW, Seg5: DFW-PHL)
    // Fare Market 3: 2 Segments (Seg6: DSM-JFK, Seg2: JFK-PHL)
    // Fare Market 4: 3 Segments (Seg4: DSM-DFW, Seg7: DFW-JFK, Seg2: JFK-PHL)

    _pricingTrx->getOptions()->cabin().setAllCabin();

    _pricingTrx->fareMarket().push_back(_pricingTrx->itin()[0]->fareMarket()[1]);

    _pricingTrx->fareMarket()[0]->travelSeg()[0]->classOfService().clear();

    ClassOfService* cos[5];
    for (int i = 0; i < 5; i++)
      cos[i] = _memHandle.create<ClassOfService>();
    cos[0]->bookingCode() = 'J';
    cos[0]->numSeats() = 3;
    cos[0]->cabin().setPremiumFirstClass();
    cos[1]->bookingCode() = 'R';
    cos[1]->numSeats() = 0;
    cos[1]->cabin().setPremiumFirstClass();
    cos[2]->bookingCode() = 'D';
    cos[2]->numSeats() = 0;
    cos[2]->cabin().setPremiumFirstClass();
    cos[3]->bookingCode() = 'I';
    cos[3]->numSeats() = 0;
    cos[3]->cabin().setFirstClass();
    cos[4]->bookingCode() = 'Y';
    cos[4]->numSeats() = 0;
    cos[4]->cabin().setEconomyClass();
    for (int i = 0; i < 5; i++)
    {
      _pricingTrx->fareMarket()[0]->travelSeg()[0]->classOfService().push_back(cos[i]);
      _pricingTrx->fareMarket()[0]->travelSeg()[1]->classOfService().push_back(cos[i]);
    }
    _pricingTrx->travelSeg().push_back(_pricingTrx->fareMarket()[0]->travelSeg()[0]);
    _pricingTrx->travelSeg().push_back(_pricingTrx->fareMarket()[0]->travelSeg()[1]);
    _pricingTrx->fareMarket()[0]->travelSeg()[0]->bookedCabin().setPremiumFirstClass();
    _itinAnalyzerMock->_requestedNumberOfSeats = 2;
    _itinAnalyzerMock->setAirSegsWithRequestedCabin(*_pricingTrx);
    CPPUNIT_ASSERT_EQUAL(BookingCode('I'),
                         (_pricingTrx->fareMarket()[0]->travelSeg()[0]->getBookingCode()));
  }

  void testsetAirSegWithReqSpecificCabinThrowException()
  {
    prepareItinaryAndFMsForOldBrandedFare();

    _pricingTrx->getOptions()->cabin().setPremiumEconomyClass();
    _pricingTrx->fareMarket().push_back(_pricingTrx->itin()[0]->fareMarket()[0]);
    _pricingTrx->fareMarket()[0]->travelSeg()[0]->classOfService().clear();

    ClassOfService* cos[5];
    for (int i = 0; i < 5; i++)
      cos[i] = _memHandle.create<ClassOfService>();
    cos[0]->bookingCode() = 'J';
    cos[0]->numSeats() = 3;
    cos[0]->cabin().setPremiumFirstClass();
    cos[1]->bookingCode() = 'R';
    cos[1]->numSeats() = 0;
    cos[1]->cabin().setPremiumFirstClass();
    cos[2]->bookingCode() = 'D';
    cos[2]->numSeats() = 0;
    cos[2]->cabin().setPremiumFirstClass();
    cos[3]->bookingCode() = 'I';
    cos[3]->numSeats() = 0;
    cos[3]->cabin().setFirstClass();
    cos[4]->bookingCode() = 'Y';
    cos[4]->numSeats() = 0;
    cos[4]->cabin().setPremiumEconomyClass();
    for (int i = 0; i < 5; i++)
    {
      _pricingTrx->fareMarket()[0]->travelSeg()[0]->classOfService().push_back(cos[i]);
      _pricingTrx->fareMarket()[0]->travelSeg()[1]->classOfService().push_back(cos[i]);
    }
    _pricingTrx->travelSeg().push_back(_pricingTrx->fareMarket()[0]->travelSeg()[0]);
    _pricingTrx->travelSeg().push_back(_pricingTrx->fareMarket()[0]->travelSeg()[1]);
    _pricingTrx->fareMarket()[0]->travelSeg()[0]->bookedCabin().setPremiumFirstClass();
    _itinAnalyzer->_requestedNumberOfSeats = 2;
    try
    {
      _itinAnalyzer->setAirSegsWithRequestedCabin(*_pricingTrx);
    }
    catch (const ErrorResponseException& e)
    {
      if (e.code() == ErrorResponseException::INVALID_INPUT)
      {
        CPPUNIT_ASSERT_EQUAL(e.message(), std::string("CABIN REQUESTED IS NOT OFFERED/AVAILABLE"));
        return;
      }
    }
    CPPUNIT_FAIL(
        "No Seats available for Cabin requested should throw NonFatalErrorResponseException");
  }

  void testisItinOutsideNetherlandAntilles()
  {
    Itin* itin = _memHandle.create<Itin>();

    // Initialize Location
    const Loc* loc1 = _trx->dataHandle().getLoc("CUR", DateTime::localTime());
    const Loc* loc2 = _trx->dataHandle().getLoc("SXM", DateTime::localTime());
    const Loc* loc3 = _trx->dataHandle().getLoc("SAB", DateTime::localTime());
    const Loc* loc4 = _trx->dataHandle().getLoc("CUR", DateTime::localTime());
    const Loc* loc5 = _trx->dataHandle().getLoc("BON", DateTime::localTime());

    AirSeg* travelSeg1, *travelSeg2, *travelSeg3, *travelSeg4;

    _trx->dataHandle().get(travelSeg1);
    _trx->dataHandle().get(travelSeg2);
    _trx->dataHandle().get(travelSeg3);
    _trx->dataHandle().get(travelSeg4);

    // Build Travel Segments CUR-7I-SXM-SAB-WN-SXM-7I-CUR-7I-BON

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "7I");
    travelSeg1->departureDT() = DateTime(2012, 12, 31);
    initAirSeg(*travelSeg2, GeoTravelType::International, loc3, loc2, "WN");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg3, GeoTravelType::International, loc1, loc4, "7I");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg4, GeoTravelType::International, loc4, loc5, "7I");

    itin->travelSeg() += travelSeg1, travelSeg2, travelSeg3, travelSeg4;

    itin->geoTravelType() = GeoTravelType::International;
    itin->setTravelDate(travelSeg1->departureDT());
    bool aaa = _itinAnalyzer->isItinOutsideNetherlandAntilles(*itin);
    CPPUNIT_ASSERT_EQUAL(false, aaa);
    itin->geoTravelType() = GeoTravelType::Domestic;
    aaa = _itinAnalyzer->isItinWhollyWithInSameNation(*itin);
    CPPUNIT_ASSERT_EQUAL(false, aaa);
  }
  void testisItinOutsideEurope()
  {
    Itin* itin = _memHandle.create<Itin>();

    // Initialize Location
    const Loc* loc1 = _trx->dataHandle().getLoc("LHR", DateTime::localTime());
    const Loc* loc2 = _trx->dataHandle().getLoc("FRA", DateTime::localTime());

    AirSeg* travelSeg1;

    _trx->dataHandle().get(travelSeg1);

    // Build Travel Segments LHR-LH-FRA

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "LH");
    travelSeg1->departureDT() = DateTime(2012, 12, 31);

    itin->travelSeg() += travelSeg1;

    itin->geoTravelType() = GeoTravelType::International;
    itin->setTravelDate(travelSeg1->departureDT());
    bool aaa = _itinAnalyzer->isItinOutsideEurope(*itin);
    CPPUNIT_ASSERT_EQUAL(false, aaa);
  }
  void testisInternationalItin()
  {
    Itin* itin = _memHandle.create<Itin>();

    // Initialize Location
    const Loc* loc1 = _trx->dataHandle().getLoc("DFW", DateTime::localTime());
    const Loc* loc2 = _trx->dataHandle().getLoc("LHR", DateTime::localTime());

    AirSeg* travelSeg1;
    _trx->dataHandle().get(travelSeg1);

    // Build Travel Segments DFW-LH-LHR

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "LH");
    travelSeg1->departureDT() = DateTime(2012, 12, 31);

    itin->travelSeg() += travelSeg1;

    itin->setTravelDate(travelSeg1->departureDT());
    _trx->itin().push_back(itin);
    bool aaa = _itinAnalyzer->isInternationalItin(*_trx);
    CPPUNIT_ASSERT_EQUAL(false, aaa);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinAnalyzerServiceTest);

} // tse
