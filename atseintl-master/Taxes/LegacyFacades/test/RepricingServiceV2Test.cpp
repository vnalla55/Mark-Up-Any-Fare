// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"

#include "Common/MoneyUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NUCInfo.h"
#include "Taxes/LegacyFacades/RepricingServiceV2.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class RepricingServiceV2Mock : public RepricingServiceV2
{
public:
  RepricingServiceV2Mock(PricingTrx& trx, const tax::V2TrxMappingDetails* v2Mapping)
    : RepricingServiceV2(trx, v2Mapping), _counter(0) {}

  void setDistance(LocCode origin, LocCode destination, uint16_t distance)
  {
    _miles[std::make_pair(origin, destination)] = distance;
  }

  void addPartialFare(MoneyAmount fareAmount) { _partialFares.push_back(fareAmount); }

private:
  uint16_t calculateMiles(FarePath& /*farePath*/,
                          std::vector<TravelSeg*>& /*travelSegs*/,
                          const Loc* origin,
                          const Loc* destination) const
  {
    try
    {
      return _miles.at(std::make_pair(origin->loc(), destination->loc()));
    }
    catch (std::out_of_range&)
    {
      return 0;
    }
  }

  bool subtractPartialFare(FarePath& /*farePath*/,
                           const MoneyAmount& /*totalFareAmount*/,
                           MoneyAmount& taxableAmount,
                           FareUsage& /*fareUsage*/,
                           std::vector<TravelSeg*>& travelSegsEx,
                           CurrencyCode& /*paymentCurrency*/) const
  {
    if (travelSegsEx.size() == 0)
      return true;

    if (_counter == _partialFares.size())
      throw std::out_of_range("Not enough partialFare test data");

    taxableAmount -= _partialFares[_counter++];
    return true;
  }

  bool findRepricedFare(const PaxTypeCode& /*paxTypeCode*/,
                        const FareUsage& /*fareUsage*/,
                        TravelSeg* travelSeg,
                        bool /*changeDate*/,
                        Indicator /*wpncsFlagIndicator*/,
                        MoneyAmount& taxableFare,
                        const DateTime& /*travelDate*/,
                        bool /*ignoreCabinCheck*/,
                        bool /*privateFareCheck = false*/) const
  {
    if (travelSeg->origin()->nation() == JAPAN &&
        travelSeg->destination()->nation() == JAPAN)
    {
      if (_counter == _partialFares.size())
        throw std::out_of_range("Not enough partialFare test data");

      taxableFare = _partialFares[_counter++];
      return true;
    }

    return false;
  }

  std::map<std::pair<LocCode, LocCode>, uint16_t> _miles;
  std::vector<MoneyAmount> _partialFares;
  mutable size_t _counter;
};

class RepricingServiceV2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RepricingServiceV2Test);
  CPPUNIT_TEST(testDestinationOutsideUS);
  CPPUNIT_TEST(testBothInUS);
  CPPUNIT_TEST(testRepricingJapan);
  CPPUNIT_TEST(testLocatePaxTypeFare);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _dataHandleMock = _memHandle.create<DataHandleMock>();
    _memHandle.create<SpecificTestConfigInitializer>();

    _trx = new PricingTrx();
    _billing = new Billing();
    _pricingRequest = new PricingRequest();
    _ticketingAgent = new Agent();
    _pricingOptions = new PricingOptions();
    _itin = new Itin();
    for (uint16_t i = 0; i < 4; ++i)
    {
      _airSegs[i] = new AirSeg();
    }
    for (uint16_t i = 0; i < 5; ++i)
    {
      _locs[i] = new Loc();
    }
    _hidden = new Loc();
    _agentLocation = new Loc();
    _fareMarket = new FareMarket();
    _paxType = new PaxType();
    _farePath = new FarePath();
    _pricingUnit = new PricingUnit();
    _fareUsage = new FareUsage();
    _paxTypeFare = new PaxTypeFare();
    _fare = new Fare();
    _fareInfo = new FareInfo();

    _trx->billing() = _billing;
    _trx->setRequest(_pricingRequest);
    _trx->getRequest()->ticketingAgent() = _ticketingAgent;
    _trx->setOptions(_pricingOptions);
    _trx->itin().push_back(_itin);
    for (uint16_t i = 0; i < 4; ++i)
    {
      _itin->travelSeg().push_back(_airSegs[i]);
      _itin->travelSeg()[i]->origin() = _locs[i];
      _itin->travelSeg()[i]->destination() = _locs[i + 1];
      _fareUsage->travelSeg().push_back(_airSegs[i]);
    }
    _itin->farePath().push_back(_farePath);
    _fareMarket->paxTypeCortege().push_back(PaxTypeBucket());
    _fareMarket->paxTypeCortege()[0].requestedPaxType() = _paxType;
    _paxType->paxType() = "ADT";
    _farePath->itin() = _itin;
    _farePath->pricingUnit().push_back(_pricingUnit);
    _pricingUnit->fareUsage().push_back(_fareUsage);
    _fareUsage->paxTypeFare() = _paxTypeFare;
    _paxTypeFare->setFare(_fare);
    _paxTypeFare->segmentStatus().resize(_itin->travelSeg().size());
    _fare->setFareInfo(_fareInfo);

    _billing->userPseudoCityCode() = "KRK";
    _pricingOptions->currencyOverride() = "PLN";
    _pricingRequest->ticketPointOverride() = "KTW";
    _ticketingAgent->currencyCodeAgent() = "USD";
    _trx->ticketingDate() = tse::DateTime(2014, 9, 19, 14, 16, 0);
    for (uint16_t i = 0; i < 4; ++i)
    {
      _airSegs[i]->equipmentType() = "767";
      _airSegs[i]->setOperatingCarrierCode("LA");
      _airSegs[i]->setMarketingCarrierCode("LX");
      _airSegs[i]->marketingFlightNumber() = 1111 * (i + 1);
      _airSegs[i]->departureDT() = tse::DateTime(2014, 9, static_cast<uint16_t>(20 + i), 9, 30, 0);
      _airSegs[i]->arrivalDT() = tse::DateTime(2014, 9, static_cast<uint16_t>(20 + i), 15, 30, 0);
    }
    _itin->validatingCarrier() = "LA";
    _farePath->setTotalNUCAmount(160);
    _farePath->baseFareCurrency() = "USD";
    _farePath->calculationCurrency() = "USD";

    _agentLocation->loc() = std::string("KRK");
    _agentLocation->nation() = std::string("PL");
    _trx->getRequest()->ticketingAgent()->agentLocation() = _agentLocation;

    _fareInfo->owrt() = 'O';
    _fareInfo->directionality() = tse::BOTH;
    _fareInfo->market1() = "KRK";
    _fareInfo->market2() = "TYO";
    _fare->nucFareAmount() = 120;

    _sellRates.push_back(new BankerSellRate());
    _sellRates.back()->primeCur() = "PLN";
    _sellRates.back()->cur() = "USD";
    _sellRates.back()->rate() = 1.0 / 3.0;
    _sellRates.back()->rateNodec() = 1;
    _sellRates.push_back(new BankerSellRate());
    _sellRates.back()->primeCur() = "PLN";
    _sellRates.back()->cur() = "JPY";
    _sellRates.back()->rate() = 33.0;
    _sellRates.back()->rateNodec() = 1;
    _sellRates.push_back(new BankerSellRate());
    _sellRates.back()->primeCur() = "USD";
    _sellRates.back()->cur() = "PLN";
    _sellRates.back()->rate() = 3.7;
    _sellRates.back()->rateNodec() = 1;
    _sellRates.push_back(new BankerSellRate());
    _sellRates.back()->primeCur() = "JPY";
    _sellRates.back()->cur() = "PLN";
    _sellRates.back()->rate() = 0.03;
    _sellRates.back()->rateNodec() = 1;
    _dataHandleMock->set_getBankerSellRate(&_sellRates);

    _nucInfo = new NUCInfo();
    _nucInfo->_cur = "PLN";
    _nucInfo->_roundingRule = UP;
    _dataHandleMock->set_getNUCFirst(&_nucInfo);

    _v2Mapping = new tax::V2TrxMappingDetails;
    tax::type::CarrierCode cc{"XX"};
    _v2Mapping->_itinFarePathMapping.push_back(
        std::make_tuple(_itin, _farePath, cc, 0));

    _repricingServiceV2 = new RepricingServiceV2Mock(*_trx, _v2Mapping);
  }

  void tearDown()
  {
    delete _repricingServiceV2;
    delete _trx;
    delete _billing;
    delete _pricingRequest;
    delete _ticketingAgent;
    delete _pricingOptions;
    delete _itin;
    for (uint16_t i = 0; i < 4; ++i)
    {
      delete _airSegs[i];
    }
    for (uint16_t i = 0; i < 5; ++i)
    {
      delete _locs[i];
    }
    delete _hidden;
    delete _agentLocation;
    delete _fareMarket;
    delete _paxType;
    delete _farePath;
    delete _pricingUnit;
    delete _fareUsage;
    delete _paxTypeFare;
    delete _fare;
    delete _fareInfo;
    delete _v2Mapping;

    for (uint16_t i = 0; i < _sellRates.size(); ++i)
    {
      delete _sellRates[i];
    }
    _sellRates.clear();
    delete _nucInfo;
    _memHandle.clear();
  }

  void fillLocs(std::vector<std::pair<std::string, std::string> > cities)
  {
    for (uint16_t i = 0; i < 5; ++i)
    {
      _locs[i]->loc() = cities[i].first;
      _locs[i]->city() = cities[i].first;
      _locs[i]->nation() = cities[i].second;
    }
  }

  void testDestinationOutsideUS()
  {
    std::vector<std::pair<std::string, std::string> > cities;
    cities.push_back(std::make_pair("KRK", "PL"));
    cities.push_back(std::make_pair("DFW", "US"));
    cities.push_back(std::make_pair("IAD", "US"));
    cities.push_back(std::make_pair("YYZ", "CA"));
    cities.push_back(std::make_pair("TYO", "JP"));
    fillLocs(cities);
    _airSegs[2]->hiddenStops().push_back(_hidden);
    _hidden->loc() = "NYC";
    _hidden->city() = "NYC";
    _hidden->nation() = "US";

    dynamic_cast<RepricingServiceV2Mock*>(_repricingServiceV2)->setDistance("KRK", "TYO", 250);
    dynamic_cast<RepricingServiceV2Mock*>(_repricingServiceV2)->setDistance("DFW", "NYC", 100);

    const tax::type::MoneyAmount result = _repricingServiceV2->getFareUsingUSDeductMethod(2, 7, 0);

    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      const tax::type::MoneyAmount expected = tax::doubleToAmount( 177.6 );
      CPPUNIT_ASSERT_EQUAL( expected, result );
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(144), result);
    }
  }

  void testBothInUS()
  {
    std::vector<std::pair<std::string, std::string> > cities;
    cities.push_back(std::make_pair("KRK", "PL"));
    cities.push_back(std::make_pair("DFW", "US"));
    cities.push_back(std::make_pair("IAD", "US"));
    cities.push_back(std::make_pair("NYC", "US"));
    cities.push_back(std::make_pair("TYO", "JP"));
    fillLocs(cities);

    dynamic_cast<RepricingServiceV2Mock*>(_repricingServiceV2)->addPartialFare(69);
    dynamic_cast<RepricingServiceV2Mock*>(_repricingServiceV2)->addPartialFare(93);

    const tax::type::MoneyAmount result = _repricingServiceV2->getFareUsingUSDeductMethod(2, 5, 0);

    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      const tax::type::MoneyAmount expected = tax::doubleToAmount( 282 );
      CPPUNIT_ASSERT_EQUAL( expected, result );
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(198), result);
    }
  }

  void testRepricingJapan()
  {
    std::vector<std::pair<std::string, std::string> > cities;
    cities.push_back(std::make_pair("KRK", "PL"));
    cities.push_back(std::make_pair("HIJ", "JP"));
    cities.push_back(std::make_pair("KIX", "JP"));
    cities.push_back(std::make_pair("NRT", "JP"));
    cities.push_back(std::make_pair("LAX", "US"));
    fillLocs(cities);
    _ticketingAgent->currencyCodeAgent() = "JPY";
    _farePath->baseFareCurrency() = "JPY";
    _farePath->calculationCurrency() = "JPY";

    dynamic_cast<RepricingServiceV2Mock*>(_repricingServiceV2)->addPartialFare(220);
    dynamic_cast<RepricingServiceV2Mock*>(_repricingServiceV2)->addPartialFare(440);

    const tax::type::MoneyAmount result = _repricingServiceV2->getFareFromFareList(2, 5, 0);

    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      const tax::type::MoneyAmount expected = tax::doubleToAmount( 19.8 );
      CPPUNIT_ASSERT_EQUAL( expected, result );
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(20), result);
    }
  }

  void testLocatePaxTypeFare()
  {
    PaxTypeCode paxTypeCode = "ADT";
    const std::vector<PaxTypeFare*>* paxTypeFare = 0;
    paxTypeFare = _repricingServiceV2->locatePaxTypeFare(_fareMarket, paxTypeCode);
    CPPUNIT_ASSERT_EQUAL(const_cast<const std::vector<PaxTypeFare*>*>(&_fareMarket->paxTypeCortege()[0].paxTypeFare()),
        paxTypeFare);
  }

private:
  PricingTrx* _trx;
  Billing* _billing;
  PricingRequest* _pricingRequest;
  Agent* _ticketingAgent;
  PricingOptions* _pricingOptions;
  Itin* _itin;
  AirSeg* _airSegs[4];
  Loc* _locs[5];
  Loc* _hidden;
  Loc* _agentLocation;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  PaxType* _paxType;
  FareMarket* _fareMarket;
  FareUsage* _fareUsage;
  PaxTypeFare* _paxTypeFare;
  Fare* _fare;
  FareInfo* _fareInfo;
  RepricingServiceV2* _repricingServiceV2;
  std::vector<BankerSellRate*> _sellRates;
  NUCInfo* _nucInfo;
  tax::V2TrxMappingDetails* _v2Mapping;
  DataHandleMock* _dataHandleMock;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RepricingServiceV2Test);

} // namespace tse
