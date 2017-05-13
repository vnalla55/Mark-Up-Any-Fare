//----------------------------------------------------------------------------
//
// Copyright Sabre 2013
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/FreqFlyerUtils.h"
#include "DBAccess/FreqFlyerStatusSeg.h"
#include "DBAccess/FreqFlyerStatus.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/test/Diag852ParsedParamsTester.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include <gmock/gmock.h>

using namespace ::testing;

namespace tse
{
class BaggageTravelTestMock : public DataHandleMock
{
  FreqFlyerStatus status;
  std::vector<const FreqFlyerStatus*> statuses;

public:
  BaggageTravelTestMock() : DataHandleMock()
  {
    status._carrier = "AA";
    statuses.push_back(&status);
    ON_CALL(*this, getFreqFlyerStatuses(_, _, _)).WillByDefault(Return(statuses));
  }
  MOCK_METHOD3(getFreqFlyerStatuses,
               std::vector<const FreqFlyerStatus*>(const CarrierCode carrier,
                                                   const DateTime& date,
                                                   bool useHistorical));
  MOCK_METHOD2(getFreqFlyerStatusSegs,
               std::vector<const FreqFlyerStatusSeg*>(const CarrierCode carrier,
                                                      const DateTime& date));
};

class BaggageTravelTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageTravelTest);

  CPPUNIT_TEST(test_shouldAttachToDisclosure_YES_for_pricingTrx);
  CPPUNIT_TEST(test_shouldAttachToDisclosure_YES_for_rexPricing_transactions_with_unflownSegment);
  CPPUNIT_TEST(test_shouldAttachToDisclosure_NO_for_rexPricing_transactions_with_unflownSegment);

  CPPUNIT_TEST(testShouldNotReadGetFFStatusSeg);
  CPPUNIT_TEST(testShouldNotReadGetFFStatusSegMultipleLevels);
  CPPUNIT_TEST(testShouldReadA03TableGetFFStatus);
  CPPUNIT_TEST(testShouldNotReadGetFFStatusSegNoDiag);
  CPPUNIT_TEST(testNoA03Table);
  CPPUNIT_TEST_SUITE_END();

public:
  BaggageTravelTestMock* myDataHandleMock;
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    myDataHandleMock = _memHandle.create<BaggageTravelTestMock>();

    _baggageTravel = _memHandle.create<BaggageTravel>();
    FarePath* fp = _memHandle.create<FarePath>();
    fp->itin() = _memHandle.create<Itin>();
    _baggageTravel->setupTravelData(*fp);
  }

  void tearDown()
  {
    _baggageTravelSegments.clear();
    _memHandle.clear();
  }

  void createSegmentsAllFlown()
  {
    for (size_t segIndex = 0; segIndex < 5; ++segIndex)
    {
      _baggageTravelSegments.push_back(_memHandle.create<AirSeg>());
      _baggageTravelSegments.back()->unflown() = false;
    }
  }

  void test_shouldAttachToDisclosure_YES_for_pricingTrx()
  {
    PricingTrx pricingTrx;
    pricingTrx.setRequest(_memHandle.create<PricingRequest>());
    pricingTrx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _baggageTravel->_trx = &pricingTrx;

    createSegmentsAllFlown();

    _baggageTravel->updateSegmentsRange(_baggageTravelSegments.begin(),
                                        _baggageTravelSegments.end());

    CPPUNIT_ASSERT_EQUAL(true, _baggageTravel->shouldAttachToDisclosure());

    _baggageTravelSegments.back()->unflown() = true;

    CPPUNIT_ASSERT_EQUAL(true, _baggageTravel->shouldAttachToDisclosure());
  }

  void test_shouldAttachToDisclosure_YES_for_rexPricing_transactions_with_unflownSegment()
  {
    RexPricingTrx rexTrx;
    rexTrx.prepareRequest();
    rexTrx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _baggageTravel->_trx = &rexTrx;

    createSegmentsAllFlown();
    _baggageTravelSegments.back()->unflown() = true;

    _baggageTravel->updateSegmentsRange(_baggageTravelSegments.begin(),
                                        _baggageTravelSegments.end());

    CPPUNIT_ASSERT_EQUAL(true, _baggageTravel->shouldAttachToDisclosure());
  }

  void test_shouldAttachToDisclosure_NO_for_rexPricing_transactions_with_unflownSegment()
  {
    RexPricingTrx rexTrx;
    rexTrx.prepareRequest();
    rexTrx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _baggageTravel->_trx = &rexTrx;

    createSegmentsAllFlown();
    _baggageTravel->updateSegmentsRange(_baggageTravelSegments.begin(),
                                        _baggageTravelSegments.end());

    CPPUNIT_ASSERT_EQUAL(false, _baggageTravel->shouldAttachToDisclosure());
  }
  class TestDiagCollector : public Diag852Collector
  {
  public:
    TestDiagCollector(Diag852Collector::DiagType diagType)
      : _diagParamsHandler(Diag852Collector::_params)
    {
      rootDiag() = _memHandle.create<Diagnostic>();
      _diagParamsHandler.updateDiagType(diagType);
    }

  private:
    TestMemHandle _memHandle;
    Diag852ParsedParamsTester _diagParamsHandler;
  };

  TestDiagCollector* prepare852FFDiag()
  {
    TestDiagCollector* diag = _memHandle.create<TestDiagCollector>(Diag852Collector::FFACTIVE);
    diag->diagnosticType() = Diagnostic852;
    diag->enable(Diagnostic852);
    diag->activate();

    return diag;
  }

  void createBaggageTravel(CarrierCode carrier)
  {
    PaxType* paxType = _memHandle.create<PaxType>();
    _baggageTravel->farePath()->paxType() = paxType;
    _baggageTravel->setPaxType(*paxType);
    _baggageTravel->setCarrier(carrier);
    PricingTrx* pricingTrx = _memHandle.create<PricingTrx>();
    pricingTrx->setRequest(_memHandle.create<PricingRequest>());
    pricingTrx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _baggageTravel->_trx = pricingTrx;
  }

  void addFFCarrierWithLevel(CarrierCode carrier, uint16_t level)
  {
    PaxType::FreqFlyerTierWithCarrier* data =
        _memHandle.create<PaxType::FreqFlyerTierWithCarrier>();
    data->setCxr(carrier);
    data->setFreqFlyerTierLevel(level);
    _baggageTravel->paxType()->freqFlyerTierWithCarrier().push_back(data);
  }

  void testShouldNotReadGetFFStatusSeg()
  {
    createBaggageTravel("AA");
    addFFCarrierWithLevel("AA", 2);

    TestDiagCollector* diag = prepare852FFDiag();
    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("DO NOT READ A03 FOR AA") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 2") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: 2") != std::string::npos);
    CPPUNIT_ASSERT(_baggageTravel->getFreqFlyerTierLevel() == 2);
  }

  void testShouldReadA03TableGetFFStatus()
  {
    FreqFlyerStatusSeg* tierStatus = _memHandle.create<FreqFlyerStatusSeg>();
    tierStatus->addPartnerLevel("LH", 2, 7);
    std::vector<const FreqFlyerStatusSeg*> tierStatusVec(1, tierStatus);

    createBaggageTravel("AA");
    addFFCarrierWithLevel("LH", 2);
    addFFCarrierWithLevel("UA", 3);
    TestDiagCollector* diag = prepare852FFDiag();

    EXPECT_CALL(*myDataHandleMock, getFreqFlyerStatusSegs(CarrierCode("AA"), _))
        .WillOnce(Return(tierStatusVec));

    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("DO NOT READ A03 FOR AA") == std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("READ A03 TABLE FOR AA AGAINST CARRIERS LH UA") !=
                   std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("LH TIER 2 - AA TIER 7") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("UA TIER 3 - NO AA TIER") != std::string::npos);
  }

  void testChoosingTheHighesetFrequentFlyerLevel_SpecifiedWinning()
  {
    FreqFlyerStatusSeg* tierStatus = _memHandle.create<FreqFlyerStatusSeg>();
    tierStatus->addPartnerLevel("LH", 9, 8);
    tierStatus->addPartnerLevel("UA", 8, 8);
    std::vector<const FreqFlyerStatusSeg*> tierStatusVec(1, tierStatus);

    createBaggageTravel("AA");
    addFFCarrierWithLevel("LH", 9);
    addFFCarrierWithLevel("UA", 8);
    addFFCarrierWithLevel("AA", 3);
    TestDiagCollector* diag = prepare852FFDiag();

    EXPECT_CALL(*myDataHandleMock, getFreqFlyerStatusSegs(CarrierCode("AA"), _))
        .WillOnce(Return(tierStatusVec));

    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("DO NOT READ A03 FOR AA") == std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("READ A03 TABLE FOR AA AGAINST CARRIERS LH UA") !=
                   std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("LH TIER 9 - AA TIER 8") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("UA TIER 8 - AA TIER 7") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 3") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: 3") != std::string::npos);
  }

  void testChoosingTheHighesetFrequentFlyerLevel_A03TableWinning()
  {
    FreqFlyerStatusSeg* tierStatus = _memHandle.create<FreqFlyerStatusSeg>();
    tierStatus->addPartnerLevel("LH", 9, 9);
    tierStatus->addPartnerLevel("UA", 1, 1);
    std::vector<const FreqFlyerStatusSeg*> tierStatusVec(1, tierStatus);

    createBaggageTravel("AA");
    addFFCarrierWithLevel("LH", 9);
    addFFCarrierWithLevel("UA", 1);
    addFFCarrierWithLevel("AA", 3);
    TestDiagCollector* diag = prepare852FFDiag();

    EXPECT_CALL(*myDataHandleMock, getFreqFlyerStatusSegs(CarrierCode("AA"), _))
        .WillOnce(Return(tierStatusVec));

    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("DO NOT READ A03 FOR AA") == std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("READ A03 TABLE FOR AA AGAINST CARRIERS LH UA") !=
                   std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("LH TIER 9 - AA TIER 9") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("UA TIER 1 - AA TIER 1") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 3") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: 1") != std::string::npos);
  }

  void testChoosingTheHighesetFrequentFlyerLevel_Level_0_notRespected()
  {
    createBaggageTravel("AA");
    addFFCarrierWithLevel("LH", 2);
    addFFCarrierWithLevel("UA", 1);
    addFFCarrierWithLevel("AA", 3);
    TestDiagCollector* diag = prepare852FFDiag();

    EXPECT_CALL(*myDataHandleMock, getFreqFlyerStatusSegs(CarrierCode("AA"), _))
        .WillOnce(Return(std::vector<const FreqFlyerStatusSeg*>()));

    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("DO NOT READ A03 FOR AA") == std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("READ A03 TABLE FOR AA AGAINST CARRIERS LH UA") !=
                   std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("LH TIER 2 - NO AA TIER") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("UA TIER 1 - NO AA TIER") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 3") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: 3") != std::string::npos);
  }

  void testShouldNotReadGetFFStatusSegNoDiag()
  {
    createBaggageTravel("AA");
    addFFCarrierWithLevel("AA", 2);

    TestDiagCollector* diag = nullptr;
    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(_baggageTravel->getFreqFlyerTierLevel() == 2);
  }

  void testShouldNotReadGetFFStatusSegMultipleLevels()
  {
    createBaggageTravel("AA");
    addFFCarrierWithLevel("AA", 2);
    addFFCarrierWithLevel("AA", 3);
    TestDiagCollector* diag = prepare852FFDiag();
    auto& freqFlyerData = _baggageTravel->paxType()->freqFlyerTierWithCarrier();
    if (!freqFlyerData.empty())
      _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
          diag, freqFlyerData, _baggageTravel->getCarrier(), _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("DO NOT READ A03 FOR AA") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 2") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 3") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: 2") != std::string::npos);
  }

  void testShouldReadGetFFStatusSegMultipleCarriers()
  {
    createBaggageTravel("AA");
    addFFCarrierWithLevel("LH", 2);
    addFFCarrierWithLevel("LH", 3);
    addFFCarrierWithLevel("LO", 1);
    TestDiagCollector* diag = prepare852FFDiag();
    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));
    CPPUNIT_ASSERT(diag->str().find("READ A03 TABLE FOR AA AGAINST CARRIERS LH LO") !=
                   std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 2") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: NOT DETERMINED") != std::string::npos);
  }

  void testNoA03Table()
  {
    createBaggageTravel("AA");
    addFFCarrierWithLevel("AA", 5);
    addFFCarrierWithLevel("LZ", 2);
    TestDiagCollector* diag = prepare852FFDiag();

    EXPECT_CALL(*myDataHandleMock, getFreqFlyerStatusSegs(CarrierCode("AA"), _))
        .WillOnce(Return(std::vector<const FreqFlyerStatusSeg*>()));

    EXPECT_CALL(*myDataHandleMock, getFreqFlyerStatuses(CarrierCode("AA"), _, _))
        .WillOnce(Return(std::vector<const FreqFlyerStatus*>()));

    _baggageTravel->setFreqFlyerTierLevel(freqflyerutils::determineFreqFlyerTierLevel(
        diag,
        _baggageTravel->paxType()->freqFlyerTierWithCarrier(),
        _baggageTravel->getCarrier(),
        _baggageTravel->_trx));

    CPPUNIT_ASSERT(diag->str().find("READ A03 TABLE FOR AA AGAINST CARRIERS LZ") !=
                   std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("SPECIFIED AA TIER 5") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("LZ TIER 2 - NO AA A03") != std::string::npos);
    CPPUNIT_ASSERT(diag->str().find("TIER LEVEL FOR AA: 5") != std::string::npos);
  }

  std::vector<TravelSeg*> _baggageTravelSegments;

  TestMemHandle _memHandle;
  BaggageTravel* _baggageTravel;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageTravelTest);
}
