#include <string>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "Common/DateTime.h"
#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/MetricsUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"

#include "DataModel/Agent.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TicketingCxrRequest.h"

#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/GenSalesAgentInfo.h"

#include "TicketingCxrService/TicketingCxrService.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag191Collector.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  CountrySettlementPlanInfo* getUS_ARC()
  {
    CountrySettlementPlanInfo* info = _memHandle.create<CountrySettlementPlanInfo>();
    info->setCountryCode("US");
    info->setSettlementPlanTypeCode("ARC");
    info->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    return info;
  }
  CountrySettlementPlanInfo* getUS_SAT()
  {
    CountrySettlementPlanInfo* info = _memHandle.create<CountrySettlementPlanInfo>();
    info->setCountryCode("US");
    info->setSettlementPlanTypeCode("SAT");
    info->setPreferredTicketingMethod('E');
    return info;
  }
  CountrySettlementPlanInfo* getBM_BSP()
  {
    CountrySettlementPlanInfo* info = _memHandle.create<CountrySettlementPlanInfo>();
    info->setCountryCode("BM");
    info->setSettlementPlanTypeCode("BSP");
    info->setPreferredTicketingMethod('E');
    return info;
  }
  CountrySettlementPlanInfo* getAU_BSP()
  {
    CountrySettlementPlanInfo* info = _memHandle.create<CountrySettlementPlanInfo>();
    info->setCountryCode("AU");
    info->setSettlementPlanTypeCode("BSP");
    info->setPreferredTicketingMethod('E');
    return info;
  }
  CountrySettlementPlanInfo* getBM_KRY()
  {
    CountrySettlementPlanInfo* info = _memHandle.create<CountrySettlementPlanInfo>();
    info->setCountryCode("BM");
    info->setSettlementPlanTypeCode("KRY");
    info->setPreferredTicketingMethod('E');
    return info;
  }

  AirlineCountrySettlementPlanInfo* getCoSpFor(const NationCode& nation, const CarrierCode& airline )
  {
    AirlineCountrySettlementPlanInfo* info = _memHandle.create<AirlineCountrySettlementPlanInfo>();
    info->setCountryCode(nation);
    info->setGds("1S");
    info->setAirline(airline);
    info->setSettlementPlanType("BSP");
    info->setPreferredTicketingMethod('E');
    info->setRequiredTicketingMethod('E');
    return info;
  }

  void setCountrySelttlementPlanForAU(AirlineCountrySettlementPlanInfo& info,
      const CarrierCode& cxr)
  {
    info.setCountryCode("AU");
    info.setGds("1S");
    info.setAirline(cxr);
    info.setSettlementPlanType("BSP");
    info.setPreferredTicketingMethod('E');
    info.setRequiredTicketingMethod('E');
  }

  AirlineInterlineAgreementInfo* getAIAgmtFor(const CarrierCode& valCxr,
      const CarrierCode& participatingCxr,
      const Alpha3Char& agmtType)
  {
    AirlineInterlineAgreementInfo* info = _memHandle.create<AirlineInterlineAgreementInfo>();
    info->setCountryCode("US");
    info->setGds("1S");
    info->setValidatingCarrier(valCxr);
    info->setParticipatingCarrier(participatingCxr);
    info->setAgreementTypeCode(agmtType);
    return info;
  }

  AirlineInterlineAgreementInfo* getAIAgmtDLAndKL(const CarrierCode& cxr)
  {
    AirlineInterlineAgreementInfo* info = _memHandle.create<AirlineInterlineAgreementInfo>();
    info->setCountryCode("BM");
    info->setGds("1S");
    info->setValidatingCarrier((cxr == "DL") ? "DL" : "KL");
    info->setAgreementTypeCode(vcx::AGR_THIRD_PARTY);
    info->setParticipatingCarrier((cxr == "DL") ? "KL" : "DL");
    return info;
  }

  // Multiple GSA interline agmt
  void setIAForLHinAU(AirlineInterlineAgreementInfo& info,
      const CarrierCode& cxr,
      const CarrierCode& pcxr,
      const Alpha3Char& agmt)
  {
    info.setCountryCode("AU");
    info.setGds("1S");
    info.setValidatingCarrier(cxr);
    info.setParticipatingCarrier(pcxr);
    info.setAgreementTypeCode(agmt);
  }

  // DL and KL has interline agmt with LH
  AirlineInterlineAgreementInfo* getAIAgmtDLAndLH()
  {
    AirlineInterlineAgreementInfo* info = _memHandle.create<AirlineInterlineAgreementInfo>();
    info->setCountryCode("BM");
    info->setGds("1S");
    info->setValidatingCarrier("DL");
    info->setAgreementTypeCode(vcx::AGR_THIRD_PARTY);
    info->setParticipatingCarrier("LH");
    return info;
  }

  GenSalesAgentInfo* getGSAForKLInBM()
  {
    GenSalesAgentInfo* info = _memHandle.create<GenSalesAgentInfo>();
    info->setCountryCode("BM");
    info->setGDSCode("1S");
    info->setSettlementPlanCode("BSP");
    info->setCxrCode("DL");
    return info;
  }

  void setGSAForB1InUS(std::vector<GenSalesAgentInfo*>& col)
  {
    GenSalesAgentInfo* info1 = _memHandle.create<GenSalesAgentInfo>();
    info1->setCountryCode("US");
    info1->setGDSCode("1S");
    info1->setSettlementPlanCode("ARC");
    info1->setNonParticipatingCxr("B1");
    info1->setCxrCode("G1");
    col.push_back(info1);

    GenSalesAgentInfo* info2 = _memHandle.create<GenSalesAgentInfo>();
    info2->setCountryCode("AU");
    info2->setGDSCode("1S");
    info2->setSettlementPlanCode("BSP");
    info2->setNonParticipatingCxr("B1");
    info2->setCxrCode("G2");
    col.push_back(info2);
  }

  void setGSAForLHInAU(std::vector<GenSalesAgentInfo*>& col)
  {
    GenSalesAgentInfo* info1 = _memHandle.create<GenSalesAgentInfo>();
    info1->setCountryCode("AU");
    info1->setGDSCode("1S");
    info1->setSettlementPlanCode("BSP");
    info1->setNonParticipatingCxr("LH");
    info1->setCxrCode("TG");
    col.push_back(info1);

    GenSalesAgentInfo* info2 = _memHandle.create<GenSalesAgentInfo>();
    info2->setCountryCode("AU");
    info2->setGDSCode("1S");
    info2->setSettlementPlanCode("BSP");
    info2->setNonParticipatingCxr("LH");
    info2->setCxrCode("UA");
    col.push_back(info2);
  }

public:
  // BSP and KRY exists in BM
  const std::vector<CountrySettlementPlanInfo*>&
  getCountrySettlementPlans(const NationCode& countryCode)
  {
    std::vector<CountrySettlementPlanInfo*>& col =
        *_memHandle.create<std::vector<CountrySettlementPlanInfo*> >();
    if (countryCode == "US")
    {
      col.push_back(getUS_ARC());
      col.push_back(getUS_SAT());
      return col;
    }
    else if (countryCode == "BM")
    {
      col.push_back(getBM_BSP());
      col.push_back(getBM_KRY());
      return col;
    }
    else if (countryCode == "AU")
    {
      col.push_back(getAU_BSP());
      return col;
    }
    else if (countryCode == "SA")
    {
      CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
      cspi->setSettlementPlanTypeCode("GTC");
      cspi->setCountryCode("SA");
      cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
      col.push_back(cspi);

      CountrySettlementPlanInfo* cspi2 =
        _memHandle.create<CountrySettlementPlanInfo>();
      cspi2->setSettlementPlanTypeCode("BSP");
      cspi2->setCountryCode("SA");
      cspi2->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
      col.push_back(cspi2);
    }
    return col;
  }

  // KL does not participates in BM
  const std::vector<AirlineCountrySettlementPlanInfo*>&
  getAirlineCountrySettlementPlans(const NationCode& country,
                                   const CrsCode& gds,
                                   const CarrierCode& airline,
                                   const SettlementPlanType& spType)
  {
    std::vector<AirlineCountrySettlementPlanInfo*>* col =
      _memHandle.create<std::vector<AirlineCountrySettlementPlanInfo*> >();

    if (country == "US" && (airline == "G1" || airline == "G2"))
    {
      col->push_back(getCoSpFor(country, airline));
      return *col;
    }

    if (country == "BM" && airline == "DL")
    {
      col->push_back(getCoSpFor(country, airline));
      return *col;
    }

    if (country == "AU")
    {
      AirlineCountrySettlementPlanInfo* info = _memHandle.create<AirlineCountrySettlementPlanInfo>();
      setCountrySelttlementPlanForAU(*info, airline);
      col->push_back(info);
      return *col;
    }

    if (country == "SA" && spType == "GTC")
    {
      AirlineCountrySettlementPlanInfo* info = _memHandle.create<AirlineCountrySettlementPlanInfo>();
      info->setCountryCode("SA");
      info->setGds("1S");
      info->setAirline("8P");
      info->setSettlementPlanType("GTC");
      info->setPreferredTicketingMethod('E');
      info->setRequiredTicketingMethod('E');
      col->push_back(info);
      return *col;
    }

    return *col;
  }

  // DL and KL has interline agreement between them
  const std::vector<AirlineInterlineAgreementInfo*>&
  getAirlineInterlineAgreements(const NationCode& country,
                                const CrsCode& gds,
                                const CarrierCode& vcxr)
  {
    CarrierCode TG("TG");
    CarrierCode UA("UA");
    CarrierCode LH("LH");

    std::vector<AirlineInterlineAgreementInfo*>* col =
        _memHandle.create<std::vector<AirlineInterlineAgreementInfo*> >();

    if (country == "US" && (vcxr == "G1" || vcxr == "G2"))
    {
      col->push_back(getAIAgmtFor(vcxr, "B1", vcx::AGR_THIRD_PARTY));
      //col->push_back(getAIAgmtFor(vcxr, "P1", vcx::AGR_STANDARD));
      //col->push_back(getAIAgmtFor(vcxr, "P2", vcx::AGR_STANDARD));
      return *col;
    }
    else if (country == "BM" && (vcxr == "DL" || vcxr == "KL"))
    {
      col->push_back(getAIAgmtDLAndKL(vcxr));
      col->push_back(getAIAgmtDLAndLH());
      return *col;
    }
    else if (country == "AU" && vcxr=="LH")
    {
      AirlineInterlineAgreementInfo* info = _memHandle.create<AirlineInterlineAgreementInfo>();
      setIAForLHinAU(*info, vcxr, TG, vcx::AGR_THIRD_PARTY);
      col->push_back(info);

      AirlineInterlineAgreementInfo* info2 = _memHandle.create<AirlineInterlineAgreementInfo>();
      setIAForLHinAU(*info2, vcxr, UA, vcx::AGR_THIRD_PARTY);
      col->push_back(info2);
      return *col;
    }
    else if (country == "AU" && vcxr=="TG")
    {
      AirlineInterlineAgreementInfo* info = _memHandle.create<AirlineInterlineAgreementInfo>();
      setIAForLHinAU(*info, vcxr, LH, vcx::AGR_THIRD_PARTY);
      col->push_back(info);

      AirlineInterlineAgreementInfo* info2 = _memHandle.create<AirlineInterlineAgreementInfo>();
      setIAForLHinAU(*info2, vcxr, UA, vcx::AGR_STANDARD);
      col->push_back(info);
      return *col;
    }
    else if (country == "AU" && vcxr=="UA")
    {
      AirlineInterlineAgreementInfo* info = _memHandle.create<AirlineInterlineAgreementInfo>();
      setIAForLHinAU(*info, vcxr, LH, vcx::AGR_THIRD_PARTY);
      col->push_back(info);

      AirlineInterlineAgreementInfo* info2 = _memHandle.create<AirlineInterlineAgreementInfo>();
      setIAForLHinAU(*info2, vcxr, TG, vcx::AGR_STANDARD);
      col->push_back(info);
      return *col;
    }
    else
    {
      return *col;
    }
  }

  // KL has designated DL as its GSA in BM
  const std::vector<GenSalesAgentInfo*>&
    getGenSalesAgents(const CrsCode& gds,
        const NationCode& country,
        const SettlementPlanType& sp,
        const CarrierCode& vcxr)
  {
    std::vector<GenSalesAgentInfo*>* col = _memHandle.create<std::vector<GenSalesAgentInfo*> >();
    if (country == "BM" && vcxr == "KL")
    {
      col->push_back(getGSAForKLInBM());
      return *col;
    }
    else if (country == "AU" && vcxr == "LH")
    {
      setGSAForLHInAU(*col);
      return *col;
    }
    else if (country == "US" && vcxr == "B1")
    {
      setGSAForB1InUS(*col);
      return *col;
    }
    else
    {
      return *col;
    }
  }

  const std::vector<Customer*>& getCustomer(const PseudoCityCode& pcc)
  {
    std::vector<Customer*>* list = _memHandle.create<std::vector<Customer*> >();
    Customer* customer = _memHandle.create<Customer>();
    customer->pseudoCity() = pcc;
    if ( pcc == "A0B0" )
    {
      customer->pricingApplTag2() = 'Y'; // ARC user
    }
    list->push_back(customer);
    return *list;
  }

};
}

class TicketingCxrServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketingCxrServiceTest);
  CPPUNIT_TEST(testValidateTrue);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcess_ArcUser_Plausibility);
  CPPUNIT_TEST(testProcess_ArcUser_Settlement);
  CPPUNIT_TEST(testProcessPlausibilityCheck_Valid);
  CPPUNIT_TEST(testProcessPlausibilityCheck_InValid);
  CPPUNIT_TEST(testCheckCountrySettlementPlanTrue);
  CPPUNIT_TEST(testCheckCountrySettlementPlanFalse);
  CPPUNIT_TEST(testCheckForValCxrPaperConflictTrue);
  CPPUNIT_TEST(testCheckForValCxrPaperConflictFalse);
  CPPUNIT_TEST(testCheckForValCxrAmongGSATrue);
  CPPUNIT_TEST(testCheckForValCxrAmongGSAFalse);
  CPPUNIT_TEST(testCheckForValCxrAmongGSAMultipleGSASwaps);
  CPPUNIT_TEST(testNoValidTkTAgmtFoundAmongGSA);
  CPPUNIT_TEST(testCheckForValCxr_GTCCarrier);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  TicketingCxrTrx* _trx;
  TicketingCxrRequest* _request;
  TicketingCxrService* _tcs;
  MockTseServer* _svr;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _svr = _memHandle.insert(new MockTseServer);
    _tcs = new tse::TicketingCxrService("TICKETING_CXR_SERVICE", *_svr);
    _tcs->initialize(0, 0);
    _trx = _memHandle.insert(new TicketingCxrTrx);
    _request = _memHandle.insert(new TicketingCxrRequest);
  }

  void tearDown()
  {
    delete _tcs;
    _memHandle.clear();
  }

  void setTrx(const vcx::ParticipatingCxr& p1,
              const vcx::ParticipatingCxr& p2,
              const vcx::ParticipatingCxr& p3,
              NationCode country = "US",
              SettlementPlanType sp = "ARC",
              CarrierCode valCxr = "AA",
              PseudoCityCode pcc = "80k2",
              CrsCode ph = "1S")
  {
    setTrx(p1, p2, country, sp, valCxr, pcc, ph);
    _request->participatingCxrs().push_back(p3);
  }
  void setTrx(const vcx::ParticipatingCxr& p1,
              const vcx::ParticipatingCxr& p2,
              NationCode country = "US",
              SettlementPlanType sp = "ARC",
              CarrierCode valCxr = "AA",
              PseudoCityCode pcc = "80k2",
              CrsCode ph = "1S")
  {
    _request->ticketingAgent() = _memHandle.insert(new Agent);
    _request->setSettlementPlan(sp);
    _request->setPosCountry(country);
    _request->setPcc(pcc);
    _request->setValidatingCxr(valCxr);
    _request->participatingCxrs().push_back(p1);
    _request->participatingCxrs().push_back(p2);
    _request->setTicketType(vcx::ETKT_PREF);
    _request->setMultiHost(ph);

    _trx->setRequest(_request);
    _trx->inputDT() = DateTime(boost::gregorian::date(2014, 3, 27));
  }
  void setTrx(const vcx::ParticipatingCxr& p1,
              NationCode country = "US",
              SettlementPlanType sp = "ARC",
              CarrierCode valCxr = "AA",
              PseudoCityCode pcc = "80k2",
              CrsCode ph = "1S")
  {
    _request->ticketingAgent() = _memHandle.insert(new Agent);
    _request->setSettlementPlan(sp);
    _request->setPosCountry(country);
    _request->setPcc(pcc);
    _request->setValidatingCxr(valCxr);
    _request->participatingCxrs().push_back(p1);
    _request->setTicketType(vcx::ETKT_PREF);
    _request->setMultiHost(ph);

    _trx->setRequest(_request);
    _trx->inputDT() = DateTime(boost::gregorian::date(2014, 3, 27));
  }

  void setTrxWithDiag191()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic191;
    _trx->diagnostic().activate();
  }

  // tests
public:
  void testValidateTrue() { CPPUNIT_ASSERT(_tcs->process(*_trx)); }
  void testProcess() { _tcs->process(*_trx); }

  void testProcess_ArcUser_Plausibility()
  {
    const CarrierCode participatingCxr = "P1";
    const CarrierCode validatingCxr = "DL";
    const SettlementPlanType spType;
    const NationCode nation = "BM";
    const CrsCode gds = "1S";
    const PseudoCityCode pcc = "A0B0";
    setTrx( participatingCxr, nation, spType, validatingCxr, pcc, gds );
    _trx->getRequest()->setRequestType( vcx::PLAUSIBILITY_CHECK );
    CPPUNIT_ASSERT( nation == _trx->getRequest()->getPosCountry() );
    _tcs->process( *_trx );
    CPPUNIT_ASSERT( "US" == _trx->getRequest()->getPosCountry() );
  }

  void testProcess_ArcUser_Settlement()
  {
    const CarrierCode participatingCxr = "P1";
    const CarrierCode validatingCxr = "DL";
    const SettlementPlanType spType;
    const NationCode nation = "BM";
    const CrsCode gds = "1S";
    const PseudoCityCode pcc = "A0B0";
    setTrx( participatingCxr, nation, spType, validatingCxr, pcc, gds );
    _trx->getRequest()->setRequestType( vcx::SETTLEMENTPLAN_CHECK );
    CPPUNIT_ASSERT( nation == _trx->getRequest()->getPosCountry() );
    _tcs->process( *_trx );
    CPPUNIT_ASSERT( "US" == _trx->getRequest()->getPosCountry() );
  }

  void testProcessPlausibilityCheck_Valid()
  {
    CarrierCode valCxr("DL");
    vcx::ParticipatingCxr p1("DL");
    vcx::ParticipatingCxr p2("KL");
    setTrx(p1, p2, "BM", "BSP", valCxr);

    SettlementPlanType sp("BSP");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, NULL, sp);
    _trx->ctrySettlementInfo() = spInfo;
    _trx->vcxrData().ticketType = vcx::ETKT_PREF;
    _trx->vcxrData().participatingCxrs.push_back(p1);
    _trx->vcxrData().participatingCxrs.push_back(p2);
    CPPUNIT_ASSERT(_tcs->processPlausibilityCheck(*_trx, NULL));

    std::string msg("VALIDATING CARRIER - ");
    _trx->buildMessageText(msg);
    CPPUNIT_ASSERT_EQUAL(std::string("VALIDATING CARRIER - DL"), msg);
  }

  // Invalid becuse of no interline agmt
  void testProcessPlausibilityCheck_InValid()
  {
    CarrierCode valCxr("DL");
    vcx::ParticipatingCxr p1("DL");
    vcx::ParticipatingCxr p2("AA");
    setTrx(p1, p2, "BM", "BSP", valCxr);

    SettlementPlanType sp("BSP");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, NULL, sp);
    _trx->ctrySettlementInfo() = spInfo;
    _trx->vcxrData().ticketType = vcx::ETKT_PREF;
    _trx->vcxrData().participatingCxrs.push_back(p1);
    _trx->vcxrData().participatingCxrs.push_back(p2);

    std::string emsg = "DL HAS NO INTERLINE TICKETING AGREEMENT WITH AA";
    CPPUNIT_ASSERT(!_tcs->processPlausibilityCheck(*_trx, NULL));

    std::string msg("VALIDATING CARRIER - ");
    _trx->buildMessageText(msg);
    CPPUNIT_ASSERT_EQUAL(emsg, msg);
  }

  // US: ARC SAT
  void testCheckCountrySettlementPlanTrue()
  {
    vcx::ParticipatingCxr p1("AA", vcx::STANDARD), p2("AB", vcx::STANDARD);
    setTrx(p1, p2);
    SettlementPlanType sp("ARC");
    CPPUNIT_ASSERT(_tcs->checkCountrySettlementPlan(*_trx, NULL, sp) != 0);
  }

  void testCheckCountrySettlementPlanFalse()
  {
    vcx::ParticipatingCxr p1("AA", vcx::STANDARD), p2("AB", vcx::STANDARD);
    setTrx(p1, p2);
    SettlementPlanType sp("BSP");
    CPPUNIT_ASSERT(_tcs->checkCountrySettlementPlan(*_trx, NULL, sp) == 0);
  }

  // cxr participates, has interline agmt and not paper conflict
  void testCheckForValCxrPaperConflictFalse()
  {
    CarrierCode valCxr("DL");
    vcx::ParticipatingCxr p1("DL");
    vcx::ParticipatingCxr p2("KL");
    vcx::Pos pos("BM", "1S");
    SettlementPlanType sp("BSP");
    setTrx(p1, p2, "BM", "BSP", valCxr);

    _trx->vcxrData().ticketType = vcx::ETKT_PREF;
    _trx->vcxrData().participatingCxrs.push_back(p1);
    _trx->vcxrData().participatingCxrs.push_back(p2);

    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, NULL, sp);
    _trx->ctrySettlementInfo() = spInfo;
    CPPUNIT_ASSERT(_tcs->checkForValCxr(*_trx, NULL, pos, valCxr));
  }

  // cxr participates but has paper tkt conflict (user wants paper but etkt required at stml plan
  void testCheckForValCxrPaperConflictTrue()
  {
    CarrierCode valCxr("DL");
    vcx::ParticipatingCxr p1("DL");
    vcx::ParticipatingCxr p2("KL");
    SettlementPlanType sp("BSP");
    setTrx(p1, p2, "BM", "BSP", valCxr);

    vcx::Pos pos("BM", "1S");
    _trx->getRequest()->setTicketType(vcx::PAPER_TKT_REQ);
    _trx->vcxrData().ticketType = vcx::ETKT_REQ;
    _trx->vcxrData().participatingCxrs.push_back(p1);
    _trx->vcxrData().participatingCxrs.push_back(p2);

    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, NULL, sp);
    _trx->ctrySettlementInfo() = spInfo;
    CPPUNIT_ASSERT(spInfo);
    vcx::ValidationStatus vstatus = _tcs->checkForValCxr(*_trx, NULL, pos, valCxr);
    _trx->setValidationStatus(vstatus);

    std::string msg;
    _trx->buildMessageText(msg);
    CPPUNIT_ASSERT_EQUAL(std::string("PAPER TICKET NOT PERMITTED"), msg);
  }

  void testCheckForValCxrAmongGSATrue()
  {
    CarrierCode valCxr = "KL";
    vcx::ParticipatingCxr p1("KL", vcx::STANDARD), p2("LH", vcx::STANDARD);
    setTrx(p1, p2, "BM", "BSP", valCxr);

    vcx::Pos pos("BM", "1S");
    SettlementPlanType sp("BSP");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, NULL, sp);
    _trx->ctrySettlementInfo() = spInfo;

    vcx::ValidationStatus vstatus = _tcs->checkForValCxrAmongGSA(*_trx, NULL, pos);
    _trx->setValidationStatus(vstatus);
    _trx->setValidationResult(_trx->getResultType(vstatus));

    std::string msg("VALIDATING CARRIER - "); _trx->buildMessageText(msg);
    std::string emsg("VALIDATING CARRIER - DL PER GSA AGREEMENT WITH KL");
    CPPUNIT_ASSERT_EQUAL(emsg, msg);
  }

  void testCheckForValCxrAmongGSAMultipleGSASwaps()
  {
    CarrierCode valCxr = "LH";
    vcx::ParticipatingCxr p1("LH", vcx::STANDARD);
    setTrx(p1,"AU", "BSP", valCxr);

    vcx::Pos pos("AU", "1S");
    SettlementPlanType sp("BSP");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, NULL, sp);
    CPPUNIT_ASSERT(spInfo != 0);

    _trx->ctrySettlementInfo() = spInfo;
    vcx::ValidationStatus vstatus = _tcs->checkForValCxrAmongGSA(*_trx, NULL, pos);
    _trx->setValidationStatus(vstatus);
    _trx->setValidationResult(_trx->getResultType(vstatus));

    std::string msg("VALIDATING CARRIER - ");
    _trx->buildMessageText(msg);
    std::string emsg("ALTERNATE VALIDATING CARRIER/S - TG UA");
    CPPUNIT_ASSERT_EQUAL(emsg, msg);
  }

  void testCheckForValCxrAmongGSAFalse()
  {
    // CarrierCode valCxr = "KL";
    vcx::ParticipatingCxr p1("KL", vcx::STANDARD), p2("AF", vcx::STANDARD);
    setTrx(p1, p2, "BM", "BSP", "KL");

    Diag191Collector diag;
    diag.activate();
    vcx::Pos pos("BM", "1S");
    _trx->vcxrData().ticketType = vcx::ETKT_PREF;
    _trx->vcxrData().participatingCxrs.push_back(p1);
    _trx->vcxrData().participatingCxrs.push_back(p2);
    SettlementPlanType sp("BSP");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, &diag, sp);
    _trx->ctrySettlementInfo() = spInfo;
    vcx::ValidationStatus vstatus = _tcs->checkForValCxrAmongGSA(*_trx, &diag, pos);
    _trx->setValidationStatus(vstatus);
    std::string msg; _trx->buildMessageText(msg);
    CPPUNIT_ASSERT_EQUAL(std::string("NO VALID TICKETING AGREEMENTS FOUND"), msg);
    //CPPUNIT_ASSERT_EQUAL(std::string("KL NOT VALID FOR SETTLEMENT METHOD"), msg);
  }

  void testCheckForValCxr_GTCCarrier()
  {
    CarrierCode valCxr = "8P";
    vcx::ParticipatingCxr p1("8P", vcx::STANDARD), p2("AF", vcx::STANDARD);
    setTrx(p1, p2, "SA", "BSP", "8P");

    Diag191Collector diag;
    diag.activate();
    vcx::Pos pos("SA", "1S");
    _trx->vcxrData().ticketType = vcx::ETKT_PREF;
    _trx->vcxrData().participatingCxrs.push_back(p1);
    _trx->vcxrData().participatingCxrs.push_back(p2);

    SettlementPlanType sp("BSP");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, &diag, sp);
    CPPUNIT_ASSERT(spInfo != nullptr);

    _trx->ctrySettlementInfo() = spInfo;
    CPPUNIT_ASSERT(!_tcs->processPlausibilityCheck(*_trx, NULL));

    std::string msg;
    _trx->buildMessageText(msg);
    CPPUNIT_ASSERT_EQUAL(std::string("8P NOT VALID FOR SETTLEMENT METHOD - GTC CARRIER"), msg);
  }

  // B1 does not participate in SettlementPlan and has one or more GSA
  //  1. GSAs have no interline agreement
  //    >> Expected Result: "NO VALID TICKETING AGREEMENTS FOUND"
  void testNoValidTkTAgmtFoundAmongGSA()
  {
    Diag191Collector diag;
    diag.activate();
    CarrierCode valCxr = "B1";
    vcx::ParticipatingCxr p1("P1", vcx::STANDARD);
    setTrx(p1, "US", "ARC", valCxr);

    vcx::Pos pos("US", "1S");
    _trx->vcxrData().participatingCxrs = _trx->getRequest()->participatingCxrs();
    _trx->vcxrData().ticketType = vcx::ETKT_PREF;

    SettlementPlanType sp("ARC");
    CountrySettlementPlanInfo* spInfo = _tcs->checkCountrySettlementPlan(*_trx, &diag, sp);
    CPPUNIT_ASSERT(spInfo != 0);

    _trx->ctrySettlementInfo() = spInfo;
    vcx::ValidationStatus vstatus = _tcs->checkForValCxrAmongGSA(*_trx, &diag, pos);

    //std::cerr << "status : " << vstatus << std::endl;
    //std::cerr << diag.str() << std::endl;

    CPPUNIT_ASSERT(vcx::NO_VALID_TKT_AGMT_FOUND==vstatus);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TicketingCxrServiceTest);
} // tse
