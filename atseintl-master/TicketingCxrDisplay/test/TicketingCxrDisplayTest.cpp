#include <string>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/MetricsUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"

#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrDisplayRequest.h"

#include "DBAccess/AirlineInterlineAgreementInfo.h"

#include "TicketingCxrDisplay/TicketingCxrDisplay.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag191Collector.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  void setCommonData(AirlineInterlineAgreementInfo& info,
      NationCode country,
      CrsCode gds,
      CarrierCode valCxr,
      const Alpha3Char agmtType)
  {
    info.setCountryCode(country);
    info.setGds(gds);
    info.setValidatingCarrier(valCxr);
    info.setAgreementTypeCode(agmtType);
  }

  void set3PTAgmt(std::vector<AirlineInterlineAgreementInfo*>& col,
      const NationCode& country,
      const CrsCode& gds,
      const CarrierCode& vcxr)
  {
    for (size_t i=0; i<5; ++i)
    {
      AirlineInterlineAgreementInfo* info =
        _memHandle.create<AirlineInterlineAgreementInfo>();
      setCommonData(*info, country, gds, vcxr, "3PT");

      std::stringstream ss("");
      ss<<"T"<<i;
      info->setParticipatingCarrier(ss.str());
      col.push_back(info);
    }
  }

  void setSTDAgmt(std::vector<AirlineInterlineAgreementInfo*>& col,
      const NationCode& country,
      const CrsCode& gds,
      const CarrierCode& vcxr)
  {
    for (size_t i=0; i<5; ++i)
    {
      AirlineInterlineAgreementInfo* info =
        _memHandle.create<AirlineInterlineAgreementInfo>();
      setCommonData(*info, country, gds, vcxr, "STD");

      std::stringstream ss("");
      ss<<"S"<<i;
      info->setParticipatingCarrier(ss.str());
      col.push_back(info);
    }
  }

  void setPPRAgmt(std::vector<AirlineInterlineAgreementInfo*>& col,
      const NationCode& country,
      const CrsCode& gds,
      const CarrierCode& vcxr)
  {
    for (size_t i=0; i<5; ++i)
    {
      AirlineInterlineAgreementInfo* info =
        _memHandle.create<AirlineInterlineAgreementInfo>();
      setCommonData(*info, country, gds, vcxr, "PPR");

      std::stringstream ss("");
      ss<<"P"<<i;
      info->setParticipatingCarrier(ss.str());
      col.push_back(info);
    }
  }

public:
    const std::vector<CountrySettlementPlanInfo*>&
    getCountrySettlementPlans(const NationCode& nation)
    {
      std::vector<CountrySettlementPlanInfo*>* ret =
        _memHandle.create<std::vector<CountrySettlementPlanInfo*> >();

      if (nation == "US")
      {
        CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
        cspi->setSettlementPlanTypeCode("ARC");
        cspi->setCountryCode("US");
        cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi);
      }
      else if (nation == "AU")
      {
        CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
        cspi->setSettlementPlanTypeCode("BSP");
        cspi->setCountryCode(nation);
        cspi->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
        ret->push_back(cspi);
      }

      return *ret;
    }

  // DL and KL has interline agreement between them
  const std::vector<AirlineInterlineAgreementInfo*>&
  getAirlineInterlineAgreements(const NationCode& country,
                                const CrsCode& gds,
                                const CarrierCode& vcxr)
  {
    std::vector<AirlineInterlineAgreementInfo*>* col =
      _memHandle.create<std::vector<AirlineInterlineAgreementInfo*> >();
    if (vcxr=="AA" && country=="US" && gds=="1S")
    {
      setSTDAgmt(*col, vcxr, country, gds);
      set3PTAgmt(*col, vcxr, country, gds);
      setPPRAgmt(*col, vcxr, country, gds);
    }
    else if (vcxr=="AA" && country=="CA" && gds=="1S")
    {
      set3PTAgmt(*col, vcxr, country, gds);
    }
    else if (vcxr=="MA" && country=="ZZ" && gds=="1B")
    {
      setSTDAgmt(*col, vcxr, country, gds);
      set3PTAgmt(*col, vcxr, country, gds);
    }
    return *col;
  }

  const std::vector<AirlineCountrySettlementPlanInfo*>&
  getAirlineCountrySettlementPlans(const CrsCode& gds,
                                   const NationCode& country,
                                   const CarrierCode& vcxr)
  {
    std::vector<AirlineCountrySettlementPlanInfo*>* acspList =
      _memHandle.create<std::vector<AirlineCountrySettlementPlanInfo*> >();
    if ( (vcxr == "AA") && (country == "US") && (gds == "1S") )
    {
      AirlineCountrySettlementPlanInfo* acsp =
          createairlinecountrysettlementplan( vcxr, country, gds, "GEN" );
      acspList->push_back( acsp );
    }
    else if ( (vcxr == "AA") && (country == "CA") && (gds == "1S") )
    {
      AirlineCountrySettlementPlanInfo* acsp =
          createairlinecountrysettlementplan( vcxr, country, gds, "GEN" );
      acspList->push_back( acsp );
    }
    else if ( (vcxr == "MA") && (country == "FR") && (gds == "1B") )
    {
      AirlineCountrySettlementPlanInfo* acsp =
          createairlinecountrysettlementplan( vcxr, country, gds, "GEN" );
      acspList->push_back( acsp );
    }
    return *acspList;
  }

  AirlineCountrySettlementPlanInfo* createairlinecountrysettlementplan(
      const CarrierCode& vcxr,
      const NationCode& country,
      const CrsCode& gds,
      const SettlementPlanType& spType)
  {
    AirlineCountrySettlementPlanInfo* acsp = _memHandle.create<AirlineCountrySettlementPlanInfo>();
    acsp->setCountryCode( country );
    acsp->setGds( gds );
    acsp->setAirline( vcxr );
    acsp->setSettlementPlanType( spType );
    return acsp;
  }

};
}

class TicketingCxrDisplayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketingCxrDisplayTest);
  CPPUNIT_TEST(testValidateTrue);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcessDisplayInterline_NoInterlineAgmts);
  CPPUNIT_TEST(testProcessDisplayInterline_POS);
  CPPUNIT_TEST(testProcessDisplayInterline_Specified);
  CPPUNIT_TEST(testProcessDisplayInterline_Zz);
  CPPUNIT_TEST(testProcessDisplayInterline_ZzButNoAcsp);
  CPPUNIT_TEST(testProcessDisplayValCxr_NoSettlementPlanInCntry);
  CPPUNIT_TEST(testProcessDisplayValCxr_RequestedSettlementPlanDoesNotExistsInCntry);
  CPPUNIT_TEST(testSetValidatingCxrMap);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  TicketingCxrDisplayTrx* _trx;
  TicketingCxrDisplayRequest* _request;
  TicketingCxrDisplay* _tcdSvc;
  MockTseServer* _svr;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _svr = _memHandle.insert(new MockTseServer);
    _tcdSvc = new tse::TicketingCxrDisplay("TICKETING_CXR_DISPLAY_SERVICE", *_svr);
    _tcdSvc->initialize(0, 0);
    _trx = _memHandle.insert(new TicketingCxrDisplayTrx);
    _request = _memHandle.insert(new TicketingCxrDisplayRequest);
    _trx->getRequest() = _request;
  }

  void tearDown()
  {
    delete _tcdSvc;
    _memHandle.clear();
  }

  // tests
public:
  void testValidateTrue()
  {
    _trx->getRequest()->specifiedCountry() = "US";
    CPPUNIT_ASSERT(_tcdSvc->process(*_trx));
  }
  void testProcess()
  {
    _trx->getRequest()->specifiedCountry() = "US";
    _tcdSvc->process(*_trx);
  }

  // If we find no interline agmts, we throw "NO VALID TICKETING AGREEMENTS FOUND"
  void testProcessDisplayInterline_NoInterlineAgmts()
  {
    _trx->getRequest()->specifiedCountry() = "US";
    _trx->getRequest()->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    _tcdSvc->process(*_trx);
    CPPUNIT_ASSERT_EQUAL(_trx->getValidationStatus(), vcx::NO_VALID_TKT_AGMT_FOUND);
  }

  //country and carrier are required fields for interline
  //primeHost and settlementPlan are optional
  void testProcessDisplayInterline_POS()
  {
    //set POS
    PointOfSale pos("1S", "CA", "K25H");
    _request->pointOfSale()=pos;
    _request->validatingCxr()="AA";
    _request->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    try {
      CPPUNIT_ASSERT(_tcdSvc->processDisplayInterline(*_trx, NULL));
      CPPUNIT_ASSERT_MESSAGE("Expecting 1 Agmt Type",
          _trx->getResponse().interlineAgreements().size()==1);

      CPPUNIT_ASSERT_MESSAGE("Expecting 5 3PT Interline Agmts",
          _trx->getResponse().interlineAgreements()["3PT"].size()==5);

      InterlineAgreements& agreements = _trx->getResponse().interlineAgreements();
      InterlineAgreementsIter it = agreements.find("3PT");
      CPPUNIT_ASSERT_MESSAGE(
          "Expecting Carrier T0 to have 3PT Agmt with AA", it != agreements.end());
      std::set<CarrierCode> carriers = it->second;
      CPPUNIT_ASSERT_MESSAGE(
          "Expecting Carrier T0 to have 3PT Agmt with AA", carriers.find("T0") != carriers.end());
    } catch (...) {
      CPPUNIT_ASSERT_MESSAGE("Not expecting Exception", false);
    }
  }

  void testProcessDisplayInterline_Specified()
  {
    //set POS
    PointOfSale pos("1S", "CA", "K25H");
    _request->pointOfSale()=pos;
    _request->specifiedCountry()="US";
    _request->specifiedPrimeHost()="1S";
    _request->validatingCxr()="AA";
    _request->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    CPPUNIT_ASSERT(_tcdSvc->processDisplayInterline(*_trx, NULL));

    InterlineAgreements& agreements = _trx->getResponse().interlineAgreements();

    CPPUNIT_ASSERT_MESSAGE("Expecting 3 Agmt Types", agreements.size()==3);

    CPPUNIT_ASSERT_MESSAGE("Expecting 5 STD Interline Agmts",
        agreements["STD"].size()==5);
    InterlineAgreementsIter it = agreements.find("STD");
    CPPUNIT_ASSERT_MESSAGE(
        "Expecting Carrier S0 to have STD Agmt with AA", it != agreements.end());
    std::set<CarrierCode> carriers = it->second;
    CPPUNIT_ASSERT_MESSAGE(
        "Expecting Carrier S0 to have STD Agmt with AA", carriers.find("S0") != carriers.end());

    CPPUNIT_ASSERT_MESSAGE("Expecting 5 3PT Interline Agmts",
        agreements["3PT"].size()==5);
    it = agreements.find("3PT");
    CPPUNIT_ASSERT_MESSAGE(
        "Expecting Carrier T0 to have 3PT Agmt with AA", it != agreements.end());
    carriers = it->second;
    CPPUNIT_ASSERT_MESSAGE(
        "Expecting Carrier T0 to have 3PT Agmt with AA", carriers.find("T0") != carriers.end());

    CPPUNIT_ASSERT_MESSAGE("Expecting 5 PPR Interline Agmts",
        agreements["PPR"].size()==5);
    it = agreements.find("PPR");
    CPPUNIT_ASSERT_MESSAGE("Expecting Carrier P0 to have PPR Agmt with AA", it != agreements.end());
    carriers = it->second;
    CPPUNIT_ASSERT_MESSAGE(
        "Expecting Carrier P0 to have PPR Agmt with AA", carriers.find("P0") != carriers.end());
  }

  void testProcessDisplayInterline_Zz()
  {
    PointOfSale pos( "1B", "FR", "ABCD" );
    _request->pointOfSale()=pos;
    _request->validatingCxr()="MA";
    _request->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    CPPUNIT_ASSERT( _tcdSvc->processDisplayInterline(*_trx, NULL) );

    InterlineAgreements& agreements = _trx->getResponse().interlineAgreements();

    CPPUNIT_ASSERT( agreements.size() == 2 );
    CPPUNIT_ASSERT( agreements["STD"].size() == 5 );
    InterlineAgreementsIter it = agreements.find( "STD" );
    CPPUNIT_ASSERT( it != agreements.end() );
    std::set<CarrierCode> carriers = it->second;
    CPPUNIT_ASSERT( carriers.find("S0") != carriers.end() );

    CPPUNIT_ASSERT( agreements["3PT"].size() == 5 );
    it = agreements.find("3PT");
    CPPUNIT_ASSERT( it != agreements.end() );
    carriers = it->second;
    CPPUNIT_ASSERT( carriers.find("T0") != carriers.end() );

    CPPUNIT_ASSERT( agreements["PPR"].size() == 0 );
  }

  void testProcessDisplayInterline_ZzButNoAcsp()
  {
    PointOfSale pos( "1B", "AU", "ABCD" );
    _request->pointOfSale()=pos;
    _request->validatingCxr()="MA";
    _request->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    CPPUNIT_ASSERT( !_tcdSvc->processDisplayInterline(*_trx, NULL) );

    InterlineAgreements& agreements = _trx->getResponse().interlineAgreements();
    CPPUNIT_ASSERT( agreements.size() == 0 );
  }

  void testProcessDisplayValCxr_NoSettlementPlanInCntry()
  {
    Diag191Collector diag191;
    diag191.activate();
    PointOfSale pos( "1B", "GU", "ABCD" );
    _request->pointOfSale()=pos;
    _request->validatingCxr()="MA";
    _request->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    CPPUNIT_ASSERT( !_tcdSvc->processDisplayValCxr(*_trx, &diag191) );
    CPPUNIT_ASSERT_EQUAL(std::string("NO SETTLEMENT PLAN FOR COUNTRY GU"), diag191.str());
  }

  void testProcessDisplayValCxr_RequestedSettlementPlanDoesNotExistsInCntry()
  {
    Diag191Collector diag191;
    diag191.activate();
    PointOfSale pos( "1S", "US", "ABCD" );
    _request->pointOfSale()=pos;
    _request->validatingCxr()="AA";
    _request->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    _trx->getRequest()->settlementPlan()="BSP";
    CPPUNIT_ASSERT( !_tcdSvc->processDisplayValCxr(*_trx, &diag191) );
    CPPUNIT_ASSERT_EQUAL(std::string("BSP NOT FOUND IN THE COUNTRY US"), diag191.str());
  }

  void testSetValidatingCxrMap()
  {
    TicketingCxrValidatingCxrDisplay tcvcd;
    std::vector<AirlineCountrySettlementPlanInfo*> acspList;
    CountrySettlementPlanInfo csp;
    csp.setCountryCode( "US" );
    csp.setSettlementPlanTypeCode( "ARC" );
    csp.setPreferredTicketingMethod( vcx::TM_ELECTRONIC );

    AirlineCountrySettlementPlanInfo acsp1;
    acsp1.setAirline( "AA" );
    acsp1.setRequiredTicketingMethod( vcx::TM_ELECTRONIC );
    AirlineCountrySettlementPlanInfo acsp2;
    acsp2.setAirline( "BB" );
    acsp2.setPreferredTicketingMethod( vcx::TM_PAPER );
    AirlineCountrySettlementPlanInfo acsp3;
    acsp3.setAirline( "C1" );
    acsp3.setPreferredTicketingMethod( vcx::TM_ELECTRONIC );
    AirlineCountrySettlementPlanInfo acsp4;
    acsp4.setAirline( "C2" );
    acsp4.setPreferredTicketingMethod( vcx::TM_ELECTRONIC );

    acspList.push_back(&acsp1);
    acspList.push_back(&acsp2);
    acspList.push_back(&acsp3);
    acspList.push_back(&acsp4);

    tcvcd.setValidatingCxrMap( acspList, &csp );

    const std::size_t expectedMapSize = 3;
    CPPUNIT_ASSERT_EQUAL( expectedMapSize, tcvcd.validatingCarrierMap().size() );
    CPPUNIT_ASSERT( 1 == tcvcd.validatingCarrierMap().count( vcx::ETKT_REQ ) );
    CPPUNIT_ASSERT( 1 == tcvcd.validatingCarrierMap().count( vcx::PAPER_TKT_PREF ) );
    CPPUNIT_ASSERT( 1 == tcvcd.validatingCarrierMap().count( vcx::ETKT_PREF ) );
    CPPUNIT_ASSERT( 0 == tcvcd.validatingCarrierMap().count( vcx::PAPER_TKT_REQ ) );

    ValidatingCarrierMapIter it = tcvcd.validatingCarrierMap().find( vcx::ETKT_REQ );
    CPPUNIT_ASSERT( it != tcvcd.validatingCarrierMap().end() );
    CPPUNIT_ASSERT( 1 == it->second.size() );

    it = tcvcd.validatingCarrierMap().find( vcx::PAPER_TKT_PREF );
    CPPUNIT_ASSERT( it != tcvcd.validatingCarrierMap().end() );
    CPPUNIT_ASSERT( 1 == it->second.size() );

    it = tcvcd.validatingCarrierMap().find( vcx::ETKT_PREF );
    CPPUNIT_ASSERT( it != tcvcd.validatingCarrierMap().end() );
    CPPUNIT_ASSERT( 2 == it->second.size() );
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TicketingCxrDisplayTest);
} // tse
