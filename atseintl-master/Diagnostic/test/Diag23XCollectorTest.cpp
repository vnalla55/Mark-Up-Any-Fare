//-----------------------------------------------------------------------------
//
//  File:     Diag23XCollectorTest.cpp
//
//  Author :  Grzegorz Wanke
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/RexPricingOptions.h"
#include "Diagnostic/Diag23XCollector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{

class Diag23XCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag23XCollectorTest);
  CPPUNIT_TEST(testPrintAmountWithVariance_noVariance);
  CPPUNIT_TEST(testPrintAmountWithVariance_withVariance);
  CPPUNIT_TEST(testPrintNetAmountLine_noIndicator);
  CPPUNIT_TEST(testPrintNetAmountLine_withIndicator);
  CPPUNIT_TEST(testFareMarket_FirstCrossing);
  CPPUNIT_TEST(testFareMarket_HighestTpm);
  CPPUNIT_TEST(testFareMarket_FirstCrossing_And_HighestTpm);
  CPPUNIT_TEST(testFareMarket_FareSelectionNotActive);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diag23XCollector = _memHandle.create<Diag23XCollector>();
    _diag23XCollector->_active = true;
    _diag23XCollector->_trx = _memHandle.create<RefundPricingTrx>();
    _diag23XCollector->_trx->setOptions(_options = _memHandle.create<RexPricingOptions>());

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->setFare(_memHandle.create<Fare>());
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->currency() = "USD";
    _paxTypeFare->fare()->setFareInfo(fi);
  }

  void tearDown() { _memHandle.clear(); }

  void setUpWriteAmountWithVariance()
  {
    _diag23XCollector->_trx->exchangeItin().push_back(_memHandle.create<ExcItin>());
    // NUC to avoid database call
    _diag23XCollector->_trx->exchangeItin()[0]->calculationCurrency() = "NUC";
    _diag23XCollector->_variance = -1;
    _paxTypeFare->nucFareAmount() = 6969.6969;
  }

  void populateNetLevelAmount(const MoneyAmount& amt)
  {
    NegPaxTypeFareRuleData* data = _memHandle.create<NegPaxTypeFareRuleData>();
    data->netAmount() = amt;
    data->nucNetAmount() = amt/2.0;
    PaxTypeFare::PaxTypeFareAllRuleData*
      allData = _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    allData->fareRuleData = data;
    (*_paxTypeFare->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allData;
    _paxTypeFare->status().set(PaxTypeFare::PTF_Negotiated);
  }


  void testPrintAmountWithVariance_noVariance()
  {
    setUpWriteAmountWithVariance();
    _diag23XCollector->printAmountWithVariance(*_paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(std::string(" 6969.70  N/A\n"), _diag23XCollector->str());
  }

  void testPrintAmountWithVariance_withVariance()
  {
    setUpWriteAmountWithVariance();
    _diag23XCollector->_variance = 0.06969;
    _diag23XCollector->printAmountWithVariance(*_paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(std::string(" 6969.70  0.0697\n"), _diag23XCollector->str());
  }

  void testPrintNetAmountLine_noIndicator()
  {
    setUpWriteAmountWithVariance();
    populateNetLevelAmount(133.333);
    _options->setNetSellingIndicator(false);
    _diag23XCollector->printNetAmountLine(*_paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag23XCollector->str());
  }

  void testPrintNetAmountLine_withIndicator()
  {
    setUpWriteAmountWithVariance();
    populateNetLevelAmount(133.333);
    _options->setNetSellingIndicator(true);

   _diag23XCollector->printNetAmountLine(*_paxTypeFare);
    CPPUNIT_ASSERT_EQUAL(std::string(" NET AMOUNT: 133.33 USD CNV: 66.67\n"),
                         _diag23XCollector->str());
  }

  void testFareMarket_FirstCrossing()
  {
    FareMarket fm;
    fm.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    fm.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");

    PaxType paxType;
    _diag23XCollector->_trx->paxType().push_back( &paxType );
    _diag23XCollector->_trx->setIataFareSelectionApplicable( true );

    *_diag23XCollector << fm;

    std::stringstream expectedDiag;
    expectedDiag << "HIGHEST TPM GOVERNING CARRIER:  N" << std::endl
                 << " " << std::endl
                 << "    /CXR-/ #GI-XX#  .UNKNWN." << std::endl << " " << std::endl
                 << "GEOTRAVELTYPE : UNKNOWN" << std::endl << " " << std::endl
                 << " " << std::endl
                 << "NO FARES FOUND FOR MARKET : LON-BOS. REQUESTED PAXTYPE : " << std::endl
                 << std::endl << " " << std::endl;

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _diag23XCollector->str() );
  }

  void testFareMarket_HighestTpm()
  {
    FareMarket fm;
    fm.setHighTPMGoverningCxr( true );
    fm.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    fm.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");

    PaxType paxType;
    _diag23XCollector->_trx->paxType().push_back( &paxType );
    _diag23XCollector->_trx->setIataFareSelectionApplicable( true );

    *_diag23XCollector << fm;

    std::stringstream expectedDiag;
    expectedDiag << "HIGHEST TPM GOVERNING CARRIER:  Y" << std::endl
                 << " " << std::endl
                 << "    /CXR-/ #GI-XX#  .UNKNWN." << std::endl << " " << std::endl
                 << "GEOTRAVELTYPE : UNKNOWN" << std::endl << " " << std::endl
                 << " " << std::endl
                 << "NO FARES FOUND FOR MARKET : LON-BOS. REQUESTED PAXTYPE : " << std::endl
                 << std::endl << " " << std::endl;

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _diag23XCollector->str() );
  }

  void testFareMarket_FirstCrossing_And_HighestTpm()
  {
    FareMarket fm;
    fm.setFirstCrossingAndHighTpm( true );
    fm.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    fm.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");

    PaxType paxType;
    _diag23XCollector->_trx->paxType().push_back( &paxType );
    _diag23XCollector->_trx->setIataFareSelectionApplicable( true );

    *_diag23XCollector << fm;

    std::stringstream expectedDiag;
    expectedDiag << "HIGHEST TPM GOVERNING CARRIER:  B" << std::endl
                 << " " << std::endl
                 << "    /CXR-/ #GI-XX#  .UNKNWN." << std::endl << " " << std::endl
                 << "GEOTRAVELTYPE : UNKNOWN" << std::endl << " " << std::endl
                 << " " << std::endl
                 << "NO FARES FOUND FOR MARKET : LON-BOS. REQUESTED PAXTYPE : " << std::endl
                 << std::endl << " " << std::endl;

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _diag23XCollector->str() );
  }

  void testFareMarket_FareSelectionNotActive()
  {
    FareMarket fm;
    fm.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    fm.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");

    PaxType paxType;
    _diag23XCollector->_trx->paxType().push_back( &paxType );
    _diag23XCollector->_trx->setIataFareSelectionApplicable( false );

    *_diag23XCollector << fm;

    std::stringstream expectedDiag;
    expectedDiag << " " << std::endl
                 << "    /CXR-/ #GI-XX#  .UNKNWN." << std::endl << " " << std::endl
                 << "GEOTRAVELTYPE : UNKNOWN" << std::endl << " " << std::endl
                 << " " << std::endl
                 << "NO FARES FOUND FOR MARKET : LON-BOS. REQUESTED PAXTYPE : " << std::endl
                 << std::endl << " " << std::endl;

    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _diag23XCollector->str() );
  }

private:
  TestMemHandle _memHandle;
  Diag23XCollector* _diag23XCollector;
  PaxTypeFare* _paxTypeFare;
  RexPricingOptions* _options;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag23XCollectorTest);
} // tse
