#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/Trx.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/PU.h"
#include "Pricing/SurchargesPreValidator.h"
#include "Pricing/test/MockFareMarket.h"
#include "Pricing/test/FactoriesConfigStub.h"
#include "Server/TseServer.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{

using boost::assign::operator+=;

class SurchargesPreValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SurchargesPreValidatorTest);

  CPPUNIT_TEST(testConstructor1);
  CPPUNIT_TEST(testConstructor2);
  CPPUNIT_TEST(testProcessFareMarket1);
  CPPUNIT_TEST(testProcessFareMarket2);
  CPPUNIT_TEST(testProcessFareMarketXf);

  // TODO add multi pax unit tests.

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());
    FareCalcConfig* fcConfig = _memHandle.create<FareCalcConfig>();
    _trx->fareCalcConfig() = fcConfig;
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = TestLocFactory::create("/vobs/atseintl/test/sampledata/DFW_Loc.xml");
    _trx->getRequest()->ticketingAgent() = agent;
    _factoriesConfig = _memHandle.create<test::FactoriesConfigStub>();

    Loc* locPKC = _memHandle.create<LocMock>("PKC");
    Loc* locDME = _memHandle.create<LocMock>("DME");
    Loc* locOVB = _memHandle.create<LocMock>("OVB");

    TravelSegMock* tvlsegPKCDME = _memHandle(new TravelSegMock(locPKC, locDME, 0));
    TravelSegMock* tvlsegPKCOVB = _memHandle(new TravelSegMock(locPKC, locOVB, 0));
    TravelSegMock* tvlsegOVBDME = _memHandle(new TravelSegMock(locOVB, locDME, 0));

    _trx->travelSeg().push_back(tvlsegPKCDME);
    _trx->travelSeg().push_back(tvlsegPKCOVB);
    _trx->travelSeg().push_back(tvlsegOVBDME);

    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->number() = 1;
    paxType->paxType() = "ADT";
    PaxTypeInfo pti;
    paxType->paxTypeInfo() = &pti;

    std::vector<PaxType*>* actualPaxType = _memHandle.create<std::vector<PaxType*>>();
    actualPaxType->push_back(paxType);
    paxType->actualPaxType()["**"] = actualPaxType;

    _trx->paxType().push_back(paxType);

    GlobalDirection gdFE = GlobalDirection::FE;
    GlobalDirection gdEH = GlobalDirection::EH;

    CarrierCode un = "UN";
    CarrierCode s7 = "S7";

    FareMarketMock* mktPKCDME =
        _memHandle.create<FareMarketMock>(locPKC, locDME, paxType, gdFE, un);
    mktPKCDME->travelSeg().push_back(tvlsegPKCDME);
    mktPKCDME->allPaxTypeFare().push_back(PTF(298.74, ROUND_TRIP_MAYNOT_BE_HALVED));
    mktPKCDME->allPaxTypeFare().push_back(PTF(330.34, ROUND_TRIP_MAYNOT_BE_HALVED));
    mktPKCDME->allPaxTypeFare().push_back(PTF(354.98, ONE_WAY_MAYNOT_BE_DOUBLED));
    mktPKCDME->allPaxTypeFare().push_back(PTF(365.92, ROUND_TRIP_MAYNOT_BE_HALVED));
    mktPKCDME->allPaxTypeFare().push_back(PTF(384.97, ONE_WAY_MAYNOT_BE_DOUBLED));
    mktPKCDME->allPaxTypeFare().push_back(PTF(420.47, ONE_WAY_MAYNOT_BE_DOUBLED));
    mktPKCDME->allPaxTypeFare().push_back(PTF(422.33, ROUND_TRIP_MAYNOT_BE_HALVED));
    for (PaxTypeFare* const paxTypeFare : mktPKCDME->allPaxTypeFare())
    {
      paxTypeFare->fareMarket() = mktPKCDME;
    }
    mktPKCDME->initialize(*_trx);
    mktPKCDME->paxTypeCortege()[0].paxTypeFare() = mktPKCDME->allPaxTypeFare();

    FareMarketMock* mktPKCOVB =
        _memHandle.create<FareMarketMock>(locPKC, locOVB, paxType, gdEH, s7);
    mktPKCOVB->travelSeg().push_back(tvlsegPKCOVB);
    mktPKCOVB->allPaxTypeFare().push_back(PTF(349.47, ROUND_TRIP_MAYNOT_BE_HALVED));
    mktPKCOVB->allPaxTypeFare().push_back(PTF(358.22, ROUND_TRIP_MAYNOT_BE_HALVED));
    mktPKCOVB->allPaxTypeFare().push_back(PTF(371.52, ROUND_TRIP_MAYNOT_BE_HALVED));
    for (PaxTypeFare* const paxTypeFare : mktPKCOVB->allPaxTypeFare())
    {
      paxTypeFare->fareMarket() = mktPKCOVB;
    }
    mktPKCOVB->initialize(*_trx);
    mktPKCOVB->paxTypeCortege()[0].paxTypeFare() = mktPKCOVB->allPaxTypeFare();

    FareMarketMock* mktOVBDME =
        _memHandle.create<FareMarketMock>(locOVB, locDME, paxType, gdFE, s7);
    mktOVBDME->travelSeg().push_back(tvlsegOVBDME);
    mktOVBDME->allPaxTypeFare().push_back(PTF(237.7, ONE_WAY_MAYNOT_BE_DOUBLED)); // yqyr: 37.9
    mktOVBDME->allPaxTypeFare().push_back(PTF(260.5, ROUND_TRIP_MAYNOT_BE_HALVED)); // yqyr: 37.9
    for (PaxTypeFare* const paxTypeFare : mktOVBDME->allPaxTypeFare())
    {
      paxTypeFare->fareMarket() = mktOVBDME;
    }
    mktOVBDME->initialize(*_trx);
    mktOVBDME->paxTypeCortege()[0].paxTypeFare() = mktOVBDME->allPaxTypeFare();

    Itin* itin_1 = _memHandle.create<Itin>();

    itin_1->travelSeg().push_back(tvlsegPKCOVB);
    itin_1->travelSeg().push_back(tvlsegPKCDME);
    itin_1->travelSeg().push_back(tvlsegOVBDME);

    itin_1->fareMarket().push_back(mktPKCDME);

    Itin* itin_2 = _memHandle.create<Itin>();

    itin_2->travelSeg().push_back(tvlsegPKCOVB);
    itin_2->travelSeg().push_back(tvlsegPKCDME);
    itin_2->travelSeg().push_back(tvlsegOVBDME);

    itin_2->fareMarket().push_back(mktPKCOVB);
    itin_2->fareMarket().push_back(mktOVBDME);

    _trx->itin().push_back(itin_1);
    _trx->itin().push_back(itin_2);

    _trx->diagnostic().diagnosticType() = DiagnosticNone;
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor1()
  {
    Itin* itin = _trx->itin().front();
    SurchargesPreValidator validator(*_trx, *_factoriesConfig, *itin, 0, 0);

    CPPUNIT_ASSERT_EQUAL(size_t(1), validator._numFaresForCAT12Estimation);
  }

  void testConstructor2()
  {
    Itin* itin = _trx->itin().front();
    SurchargesPreValidator validator(*_trx, *_factoriesConfig, *itin, 0, 5);

    CPPUNIT_ASSERT_EQUAL(size_t(5), validator._numFaresForCAT12Estimation);
  }

  void testProcessFareMarket1()
  {
    const std::vector<Itin*>& itinVec = _trx->itin();
    std::vector<MoneyAmount> surTaxAm = { 10.0, 13.0, 5.6, 32.7, 11.0, 55.4, 8.0 };

    Itin* itin = itinVec[0];
    SurchargesPreValidatorProcessMock validatorMock(
        *_trx, *_factoriesConfig, *itin, 0, 4, surTaxAm, surTaxAm);
    SurchargesPreValidator* validator = &validatorMock;
    FareMarket* fm = *(itin->fareMarket().begin());

    validator->process(*fm);

    const PrecalculatedTaxes& taxes = fm->paxTypeCortege()[0].cxrPrecalculatedTaxes()[""];
    CPPUNIT_ASSERT(!taxes.empty());

    CPPUNIT_ASSERT_EQUAL(4, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(1, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::YQYR));

    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::YQYR));

    CPPUNIT_ASSERT_EQUAL(32.7, taxes.getDefaultAmount(PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(10.0, taxes.getDefaultAmount(PrecalculatedTaxesAmount::YQYR));
  }

  void testProcessFareMarket2()
  {
    const std::vector<Itin*>& itinVec = _trx->itin();
    std::vector<MoneyAmount> surTaxAm = { 10.0, 13.0, 5.6, 32.7, 11.0, 55.4, 8.0 };

    Itin* itin = itinVec[1];
    SurchargesPreValidatorProcessMock validatorMock(
        *_trx, *_factoriesConfig, *itin, 0, 4, surTaxAm, surTaxAm);
    SurchargesPreValidator* validator = &validatorMock;
    FareMarket* fm = *(itin->fareMarket().begin());

    validator->process(*fm);

    const PrecalculatedTaxes& taxes = fm->paxTypeCortege()[0].cxrPrecalculatedTaxes()[""];
    CPPUNIT_ASSERT(!taxes.empty());

    CPPUNIT_ASSERT_EQUAL(3, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(1, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::YQYR));

    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::YQYR));

    CPPUNIT_ASSERT_EQUAL(13.0, taxes.getDefaultAmount(PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(10.0, taxes.getDefaultAmount(PrecalculatedTaxesAmount::YQYR));
  }

  void testProcessFareMarketXf()
  {
    const std::vector<Itin*>& itinVec = _trx->itin();
    std::vector<MoneyAmount> surTaxAm = { 10.0, 13.0, 5.6, 32.7, 11.0, 55.4, 8.0 };

    Itin* itin = itinVec[1];
    SurchargesPreValidatorProcessMock validatorMock(
        *_trx, *_factoriesConfig, *itin, 0, 4, surTaxAm, surTaxAm, { 5.0, 7.0 });
    SurchargesPreValidator* validator = &validatorMock;
    FareMarket* fm = *(itin->fareMarket().begin());

    validator->process(*fm);

    const PrecalculatedTaxes& taxes = fm->paxTypeCortege()[0].cxrPrecalculatedTaxes()[""];
    CPPUNIT_ASSERT(!taxes.empty());

    CPPUNIT_ASSERT_EQUAL(3, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(1, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::YQYR));
    CPPUNIT_ASSERT_EQUAL(0, precalculatedTaxTypeCount(taxes, PrecalculatedTaxesAmount::XF));

    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::YQYR));
    CPPUNIT_ASSERT_EQUAL(true, taxes.isProcessed(PrecalculatedTaxesAmount::XF));

    CPPUNIT_ASSERT_EQUAL(13.0, taxes.getDefaultAmount(PrecalculatedTaxesAmount::CAT12));
    CPPUNIT_ASSERT_EQUAL(10.0, taxes.getDefaultAmount(PrecalculatedTaxesAmount::YQYR));
    CPPUNIT_ASSERT_EQUAL(5.0, taxes.getDefaultAmount(PrecalculatedTaxesAmount::XF));
  }

protected:
  // Data members
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FactoriesConfig* _factoriesConfig;

  int
  precalculatedTaxTypeCount(const PrecalculatedTaxes& taxes, PrecalculatedTaxesAmount::Type type)
  {
    typedef PrecalculatedTaxes::FareToAmountMap::value_type PtfMoney;

    return std::count_if(taxes.begin(), taxes.end(), [=](const PtfMoney& m) -> bool
    {
      return m.second.has(type);
    });
  }

  // class overrides
  class LocMock : public Loc
  {
  public:
    LocMock(const LocCode& code) { _loc = code; }
  };

  class TravelSegMock : public AirSeg
  {
  public:
    TravelSegMock(const Loc* orig, const Loc* dest, int order)
    {
      origin() = orig;
      destination() = dest;
      origAirport() = orig->loc();
      destAirport() = dest->loc();
      boardMultiCity() = orig->loc();
      offMultiCity() = dest->loc();
      segmentOrder() = order;
      departureDT() = DateTime::localTime();
    }
  };

  PaxTypeFare* PTF(const MoneyAmount ptfAmt, const Indicator tagType)
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFareMock>(ptfAmt, tagType);
    return ptf;
  }

  class PaxTypeFareMock : public PaxTypeFare
  {
  public:
    PaxTypeFareMock() : PaxTypeFare() {}
    PaxTypeFareMock(const MoneyAmount ptfAmt, const Indicator tagType) : PaxTypeFare()
    {
      FareInfo* fareInfo = new FareInfo();
      fareInfo->owrt() = tagType;
      Fare* fare = new Fare();
      fare->setFareInfo(fareInfo);
      fare->nucFareAmount() = ptfAmt;
      setFare(fare);

      _isShoppingFare = true;
    }
  };

  class FareMarketMock : public FareMarket
  {
  public:
    FareMarketMock(Loc* orig, Loc* dest, PaxType* paxType, GlobalDirection& gd, CarrierCode& cc)
    {
      _origin = orig;
      _destination = dest;
      boardMultiCity() = orig->loc();
      offMultiCity() = dest->loc();

      setGlobalDirection(gd);
      governingCarrier() = cc;

      legIndex() = 0;
    }

    ~FareMarketMock() { _memHandle.clear(); }

  protected:
    TestMemHandle _memHandle;
  };

  class SurchargesPreValidatorProcessMock : public SurchargesPreValidator
  {
  public:
    SurchargesPreValidatorProcessMock(PricingTrx& trx,
                                      const FactoriesConfig& factoriesConfig,
                                      Itin& itin,
                                      DiagCollector* diag,
                                      size_t numFaresForCAT12Estimation,
                                      const std::vector<MoneyAmount>& surAm,
                                      const std::vector<MoneyAmount>& taxAm,
                                      const std::vector<MoneyAmount>& xfAm = {})
      : SurchargesPreValidator(trx, factoriesConfig, itin, diag, numFaresForCAT12Estimation),
        _surAm(surAm),
        _taxAm(taxAm),
        _xfAm(xfAm)
    {
    }

  protected:
    size_t _surIndex = 0, _taxIndex = 0, _xfIndex = 0;
    std::vector<MoneyAmount> _surAm;
    std::vector<MoneyAmount> _taxAm;
    std::vector<MoneyAmount> _xfAm;

    virtual boost::optional<MoneyAmount>
    calculatePtfSurcharges(PaxTypeFare& paxTypeFare, const FarePathWrapper& farePathWrapper)
    {
      return (_surIndex < _surAm.size()) ? _surAm[_surIndex++] : 0.0;
    }

    virtual boost::optional<MoneyAmount>
    calculatePtfYqyr(PaxTypeFare& paxTypeFare, FarePath& farePath)
    {
      return (_taxIndex < _taxAm.size()) ? _taxAm[_taxIndex++] : 0.0;
    }

    virtual boost::optional<MoneyAmount>
    calculatePtfXFTax(PaxTypeFare& paxTypeFare, FarePath& farePath)
    {
      return (_xfIndex < _xfAm.size()) ? _xfAm[_xfIndex++] : 0.0;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(SurchargesPreValidatorTest);
}
