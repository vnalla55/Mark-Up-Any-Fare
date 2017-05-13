//----------------------------------------------------------------------------
//  File: FDSuppressFareControllerTest.cpp
//
//  Author: Partha Kumar Chakraborti
//  Created:      04/12/2005
//  Description:  This is a unit test class for FDSuppressFareControllerTest.cpp
//
//  Copyright Sabre 2005
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

#include "Common/Config/ConfigMan.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/Customer.h"
#include "ItinAnalyzer/FDSuppressFareController.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FDSuppressFareControllerMock : public FDSuppressFareController
{
public:
  FDSuppressFare* addSuppressFare(const SuppressFareMatchLevel source,
                                  const Indicator& pseudoCityType,
                                  const PseudoCityCode& pseudoCityCode,
                                  const CarrierCode& carrier,
                                  const Indicator fareDisplayType = 'M',
                                  const Directionality directionality = (Directionality)0,
                                  const LocCode origCode = "",
                                  const LocTypeCode origTypeCode = ' ',
                                  const LocCode destCode = "",
                                  const LocTypeCode destTypeCode = ' ')
  {
    FDSuppressFare* tmp = _memHandle.create<FDSuppressFare>();
    tmp->pseudoCityType() = pseudoCityType;
    tmp->carrier() = carrier;
    tmp->fareDisplayType() = fareDisplayType;
    tmp->directionality() = directionality;
    tmp->loc1().loc() = origCode;
    tmp->loc1().locType() = origTypeCode;
    tmp->loc2().loc() = destCode;
    tmp->loc2().locType() = destTypeCode;

    switch (source)
    {
    case SFS_AGENTPCC:
      _fdSuppressFareAgentPcc.push_back(tmp);
      break;
    case SFS_MAINPCC:
      _fdSuppressFareHomePcc.push_back(tmp);
      break;
    case SFS_TJRGROUPNO:
      _fdSuppressFareTjr.push_back(tmp);
      break;
    }
    return tmp;
  }

  std::vector<const FDSuppressFare*>& getSuppressFareList(const FareDisplayTrx& trx,
                                                          const PseudoCityCode& pCC,
                                                          const Indicator pCCType,
                                                          const TJRGroup& ssgGroupNo,
                                                          const CarrierCode& carrier,
                                                          const DateTime& date)
  {
    switch (pCCType)
    {
    case 'T':
      return _fdSuppressFareAgentPcc;
    case 'U':
      return _fdSuppressFareHomePcc;
    case ' ':
      return _fdSuppressFareTjr;
    }
    return _fdSuppressFareEmpty;
  }

  bool _useTjr;

private:
  std::vector<const FDSuppressFare*> _fdSuppressFareAgentPcc;
  std::vector<const FDSuppressFare*> _fdSuppressFareHomePcc;
  std::vector<const FDSuppressFare*> _fdSuppressFareTjr;
  std::vector<const FDSuppressFare*> _fdSuppressFareEmpty;
  TestMemHandle _memHandle;
};

class FDSuppressFareControllerTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(FDSuppressFareControllerTest);
  CPPUNIT_TEST(testPassWorldwide);
  CPPUNIT_TEST(testPassDirWithin);
  CPPUNIT_TEST(testFailDirWithin);
  CPPUNIT_TEST(testPassDirFrom1);
  CPPUNIT_TEST(testPassDirFrom2);
  CPPUNIT_TEST(testFailDirFrom1);
  CPPUNIT_TEST(testFailDirFrom2);
  CPPUNIT_TEST(testPassDirBetween1);
  CPPUNIT_TEST(testPassDirBetween2);
  CPPUNIT_TEST(testPassDirBetween3);
  CPPUNIT_TEST(testPassDirBetween4);
  CPPUNIT_TEST(testFailDirBetween1);
  CPPUNIT_TEST(testFailDirBetween2);
  CPPUNIT_TEST(testFailDirBetween3);
  CPPUNIT_TEST(testPassFareDisplayTypeB);
  CPPUNIT_TEST(testPassFareDisplayTypeM);
  CPPUNIT_TEST(testPassFareDisplayTypeS);
  CPPUNIT_TEST(testFailFareDisplayType);
  CPPUNIT_TEST(testFailAgentPCCFailMainPCC);
  CPPUNIT_TEST(testFailAgentPCCPassMainPCC);
  CPPUNIT_TEST(testFailAgentPCCFailMainPCCFailTjr);
  CPPUNIT_TEST(testFailAgentPCCFailMainPCCPassTjr);
  CPPUNIT_TEST(testFailAgentPCCFailMainPCCDisableTjr1);
  CPPUNIT_TEST(testFailAgentPCCFailMainPCCDisableTjr2);
  CPPUNIT_TEST(test2CFailAgentPCCPassMainPCC);
  CPPUNIT_TEST(test2CPassAgentPCCFailMainPCC);
  CPPUNIT_TEST(test2CPassAgentPCCPassMainPCC);
  CPPUNIT_TEST(test2CFailAgentPCCFailMainPCCFailTjr);
  CPPUNIT_TEST(test2CFailAgentPCCFailMainPCCPassTjr);
  CPPUNIT_TEST(test2CFailAgentPCCPassMainPCCPassTjr);
  CPPUNIT_TEST(test2CPassAgentPCCFailMainPCCPassTjr);
  CPPUNIT_TEST(test3CFailAgentPCCPassMainPCCPassTjr);
  CPPUNIT_TEST(testCarrierRemoving1);
  CPPUNIT_TEST(testCarrierRemoving2);
  CPPUNIT_TEST(testCarrierRemoving3);
  CPPUNIT_TEST(testCarrierRemoving4);
  CPPUNIT_TEST(testCarrierRemoving5);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdSFCMock = _memHandle.create<FDSuppressFareControllerMock>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _options = _memHandle.create<FareDisplayOptions>();
    _request = _memHandle.create<FareDisplayRequest>();
    _tickAgent = _memHandle.create<Agent>();
    _tjrAgent = _memHandle.create<Customer>();
    _segment = _memHandle.create<AirSeg>();
    _locOrig = _memHandle.create<Loc>();
    _locDest = _memHandle.create<Loc>();

    _trx->setOptions(_options);
    _options->allCarriers() = 'N';

    _trx->setRequest(_request);
    _request->ticketingAgent() = _tickAgent;
    _tickAgent->mainTvlAgencyPCC() = "A1B1";
    _tickAgent->tvlAgencyPCC() = "A2B2";
    _tickAgent->agentTJR() = _tjrAgent;
    _tjrAgent->ssgGroupNo() = 2;

    _locOrig->loc() = "KRK";
    _locOrig->area() = 2;
    _locOrig->city() = "KRK";
    _locOrig->nation() = "PL";
    _locDest->loc() = "DFW";
    _locDest->area() = 1;
    _locDest->city() = "DAL";
    _locDest->nation() = "US";
    _segment->origin() = _locOrig;
    _segment->destination() = _locDest;
    _trx->travelSeg().push_back(_segment);

    _trx->multipleCarriersEntered() = true;
    _fdSFCMock->_useTjr = true;

    _trx->preferredCarriers().insert("C1");
  }

  void tearDown() { _memHandle.clear(); }

private:
  void testPassWorldwide()
  {
    // test worldwide directionality check
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1");
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassDirWithin()
  {
    // test direcitionality within
    // within US
    _locOrig->loc() = "ORD";
    _locOrig->area() = 1;
    _locOrig->city() = "CHI";
    _locOrig->nation() = "US";
    _fdSFCMock->addSuppressFare(
        FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1", 'M', WITHIN, "US", 'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testFailDirWithin()
  {
    // test directionality within
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                WITHIN,
                                "US",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testPassDirFrom1()
  {
    // test directionality from PL - US
    _fdSFCMock->addSuppressFare(
        FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1", 'M', FROM, "PL", 'N', "US", 'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassDirFrom2()
  {
    // test directionality from KRK - DFW
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                FROM,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testFailDirFrom1()
  {
    // test directionaluity from US - PL (reversed)
    _fdSFCMock->addSuppressFare(
        FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1", 'M', FROM, "US", 'N', "PL", 'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailDirFrom2()
  {
    // test directionality from DFW - KRK (reversed)
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                FROM,
                                "DFW",
                                'C',
                                "KRK",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testPassDirBetween1()
  {
    // test directionality between PL - US
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassDirBetween2()
  {
    // test directionality between US - PL
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "US",
                                'N',
                                "PL",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassDirBetween3()
  {
    // test directionality between KRK - US
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "KRK",
                                'C',
                                "US",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassDirBetween4()
  {
    // test directionality between DFW - PL
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "DFW",
                                'C',
                                "PL",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testFailDirBetween1()
  {
    // test directionality between PL - CA (Fail CA)
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "CA",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }
  void testFailDirBetween2()
  {
    // test directionality between KTW - US (Fail KTW)
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "KTW",
                                'C',
                                "US",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailDirBetween3()
  {
    // test directionlity KRK - NYC (Fail NYC)
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testPassFareDisplayTypeB()
  {
    // test fare display type BLANK - PASS
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassFareDisplayTypeM()
  {
    // test fare display type M - M
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'M',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testPassFareDisplayTypeS()
  {
    // test fare display type S - S
    _trx->multipleCarriersEntered() = false;
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'S',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
  }

  void testFailFareDisplayType()
  {
    // test fare display Type M - S (not eqal)
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                'S',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailAgentPCCFailMainPCC()
  {
    // test failing Agent PCC level, and Main PCC
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailAgentPCCPassMainPCC()
  {
    // test failing Agent PCC and not proceed to Main PCC
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailAgentPCCFailMainPCCFailTjr()
  {
    // test failinig on all levels
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "JP",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO,
                                ' ',
                                "",
                                "C1",
                                ' ',
                                BETWEEN,
                                "WAW",
                                'C',
                                "DFW",
                                'C');

    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailAgentPCCFailMainPCCPassTjr()
  {
    // test failing Agent PCC, iand not proceed to Main PCC and TJR
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "JP",
                                'N');
    _fdSFCMock->addSuppressFare(
        FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1", 'M', FROM, "PL", 'N', "US", 'N');

    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailAgentPCCFailMainPCCDisableTjr1()
  {
    // test diasbling TJR by configuration
    _fdSFCMock->_useTjr = false;
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "JP",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1");

    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void testFailAgentPCCFailMainPCCDisableTjr2()
  {
    // test not processing when tjr group = 0
    _tjrAgent->ssgGroupNo() = 0;
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "JP",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1");

    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
  }

  void test2CFailAgentPCCPassMainPCC()
  {
    // test removing carriers when pass Main PCC
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "NYC",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC, 'U', "A1B1", "C2");
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test2CPassAgentPCCFailMainPCC()
  {
    // test removing carrier when pass on main PCC
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "JP",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test2CPassAgentPCCPassMainPCC()
  {
    // test removing carrier when pass on both PCC
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test2CFailAgentPCCFailMainPCCFailTjr()
  {
    // test removing carrier when failing all levels
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "WAW",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "UK",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1", 'S');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C2", 'S');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test2CFailAgentPCCFailMainPCCPassTjr()
  {
    // test not removing carrier when pass tjr, wehn match was found in PCC
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "WAW",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "UK",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1", 'M');
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test2CFailAgentPCCPassMainPCCPassTjr()
  {
    // test removing carrier when pass on main PCC and not process tjr
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "TYO",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1");
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test2CPassAgentPCCFailMainPCCPassTjr()
  {
    // test removing carrier when pass on agent PCC and tjr
    _trx->preferredCarriers().insert("C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC,
                                'T',
                                "A1B1",
                                "C1",
                                ' ',
                                BETWEEN,
                                "KRK",
                                'C',
                                "DFW",
                                'C');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C2");
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
  }

  void test3CFailAgentPCCPassMainPCCPassTjr()
  {
    // test removing carrier when pass on main PCC and not proceesed tjr
    _trx->preferredCarriers().insert("C2");
    _trx->preferredCarriers().insert("C3");

    _fdSFCMock->addSuppressFare(
        FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1", ' ', WITHIN, "PL", 'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC,
                                'U',
                                "A1B1",
                                "C2",
                                'M',
                                BETWEEN,
                                "PL",
                                'N',
                                "US",
                                'N');
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, ' ', "", "C3");
    _fdSFCMock->suppressFare(*_trx);
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C3") != _trx->preferredCarriers().end());
  }

  void testCarrierRemoving1()
  {
    // test of multiple carriers match and remove
    _trx->preferredCarriers().insert("C2");
    _trx->preferredCarriers().insert("C3");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C3");
    _fdSFCMock->suppressFare(*_trx);

    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C3") == _trx->preferredCarriers().end());
  }

  void testCarrierRemoving2()
  {
    // test of multiple carriers match and remove

    _trx->preferredCarriers().insert("C2");
    _trx->preferredCarriers().insert("C3");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C4");
    _fdSFCMock->suppressFare(*_trx);

    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C3") != _trx->preferredCarriers().end());
  }

  void testCarrierRemoving3()
  {
    // test of multiple carriers match and remove

    _trx->preferredCarriers().insert("C2");
    _trx->preferredCarriers().insert("C3");
    _trx->preferredCarriers().insert("C4");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C3");
    _fdSFCMock->suppressFare(*_trx);

    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C3") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C4") != _trx->preferredCarriers().end());
  }

  void testCarrierRemoving4()
  {
    // test of multiple carriers match and remove

    _trx->preferredCarriers().insert("C2");
    _trx->preferredCarriers().insert("C3");
    _trx->preferredCarriers().insert("C4");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "A1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "A2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "A3");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "A4");

    _fdSFCMock->suppressFare(*_trx);

    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C3") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C4") != _trx->preferredCarriers().end());
  }

  void testCarrierRemoving5()
  {
    // test of multiple carriers match and remove

    _trx->preferredCarriers().insert("C2");
    _trx->preferredCarriers().insert("C3");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_AGENTPCC, 'T', "A1B1", "C1");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_MAINPCC, 'T', "A1B1", "C2");
    _fdSFCMock->addSuppressFare(FDSuppressFareController::SFS_TJRGROUPNO, 'T', "A1B1", "C3");
    _fdSFCMock->suppressFare(*_trx);

    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C1") == _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C2") != _trx->preferredCarriers().end());
    CPPUNIT_ASSERT(_trx->preferredCarriers().find("C3") != _trx->preferredCarriers().end());
  }

private:
  FDSuppressFareControllerMock* _fdSFCMock;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  AirSeg* _segment;
  Loc* _locOrig;
  Loc* _locDest;
  Agent* _tickAgent;
  Customer* _tjrAgent;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FDSuppressFareControllerTest);
}
