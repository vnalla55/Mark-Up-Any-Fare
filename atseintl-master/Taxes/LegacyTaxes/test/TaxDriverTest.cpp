#include <string>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag804Collector.h"
#include "Diagnostic/Diag817Collector.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxDriver.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestXMLHelper.h"

namespace tse
{

using boost::assign::operator+=;

class TaxDriverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDriverTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxDriver);
  CPPUNIT_TEST(testTaxDriverBucketsAllItin);
  CPPUNIT_TEST(testTaxDriverBucketsAllOC);
  CPPUNIT_TEST(testTaxDriverBucketsItinOC);
  CPPUNIT_TEST(testTaxDriverBucketsOCItin);
  CPPUNIT_TEST(testBuildTaxNationVector);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxDriver taxDriver; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxDriver()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;

    //   trx.requestType() = TAX_DISPLAY;

    DateTime dt;

    trx.getRequest()->ticketingDT() = dt.localTime();

    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    request.ticketingAgent()->agentLocation() = loc;

    PricingOptions options;
    trx.setOptions(&options);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_HNL.xml");

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(500.00);

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);
    farePath.itin()->travelSeg().push_back(airSeg2);

    itin.farePath().push_back(&farePath);

    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag817Collector diag(*diagroot);

    TaxResponse taxResponse;

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg2);

    fare.nucFareAmount() = 1000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 1000.00;
    fareInfo._currency = "USD";
    fareInfo._fareTariff = 0;
    fareInfo._fareClass = "AA";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariff = 0;

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);

    fareUsage.travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml");

    TaxItem taxItem;

    Tax tax;

    taxItem.buildTaxItem(trx, tax, taxResponse, *taxCodeReg);

    taxItem.taxAmount() = 100.00;

    taxResponse.taxItemVector().push_back(&taxItem);

    TaxCodeReg* taxCodeReg2 =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    Tax& tax2 = *taxMap.getSpecialTax(taxCodeReg2->specialProcessNo());

    TaxItem taxItem2;

    taxItem2.buildTaxItem(trx, tax2, taxResponse, *taxCodeReg2);

    taxItem2.taxAmount() = 40.00;

    taxResponse.taxItemVector().push_back(&taxItem2);

    TaxItem taxItem3;

    taxItem3.buildTaxItem(trx, tax2, taxResponse, *taxCodeReg2);

    taxItem3.taxAmount() = 20.00;

    taxResponse.taxItemVector().push_back(&taxItem3);

    TaxItem taxItem4;

    taxItem4.buildTaxItem(trx, tax2, taxResponse, *taxCodeReg2);

    taxItem4.taxAmount() = 10.00;

    taxResponse.taxItemVector().push_back(&taxItem4);

    TaxMap::TaxFactoryMap taxFactoryMap;
    TaxMap::buildTaxFactoryMap(trx.dataHandle(), taxFactoryMap);
    TaxDriver taxDriver;

    taxDriver.ProcessTaxesAndFees(trx, taxResponse, taxFactoryMap, nullptr);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegHNL_MKK_AQ.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    fareInfo._fareTariff = 773;
    fareInfo._fareClass = "YAQAP";

    taxDriver.ProcessTaxesAndFees(trx, taxResponse, taxFactoryMap, nullptr);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegAMA_DFW_F9.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    fareInfo._fareTariff = 0;
    fareInfo._fareClass = "";

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    taxDriver.ProcessTaxesAndFees(trx, taxResponse, taxFactoryMap, nullptr);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX_HP.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    fareInfo._fareTariff = 745;
    fareInfo._ruleNumber = "3600";

    taxDriver.ProcessTaxesAndFees(trx, taxResponse, taxFactoryMap, nullptr);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegMKE_X_PHX_HP.xml");
    AirSeg* airSegX;
    airSegX = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegPHX_X_ONT_HP.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);
    farePath.itin()->travelSeg().push_back(airSegX);
    fareUsage.travelSeg().push_back(airSegX);
    fareMarket.travelSeg().push_back(airSegX);

    fareInfo._fareTariff = 0;
    fareInfo._ruleNumber = "";

    taxDriver.ProcessTaxesAndFees(trx, taxResponse, taxFactoryMap, nullptr);
  }

  void taxDriverBucketsSetUp()
  {
    _trx = _memHandle.create<PricingTrx>();

    Agent* agent = _memHandle.create<Agent>();
    agent->currencyCodeAgent() = "HNL";

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = agent;

    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();
    _trx->getRequest()->ticketingAgent()->agentLocation() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHND.xml");

    _trx->setOptions(_memHandle.create<PricingOptions>());

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCUN_MEX.xml");

    Itin* itin = _memHandle.create<Itin>();
    itin->originationCurrency() = "HNL";
    itin->calculationCurrency() = "HNL";
    itin->anciliaryServiceCode() = "ZYZ";

    ServiceFeesGroup* sfg = _memHandle.create<ServiceFeesGroup>();
    itin->ocFeesGroup().push_back(sfg);

    _trx->itin().push_back(itin);

    FarePath* farePath = _memHandle.create<FarePath>();

    farePath->setTotalNUCAmount(500.00);
    farePath->itin() = itin;
    farePath->itin()->travelSeg().push_back(airSeg);

    itin->farePath().push_back(farePath);

    std::vector<OCFees*>* ocFeesVec = _memHandle.create<std::vector<OCFees*> >();
    _ocFees = _memHandle.create<OCFees>();
    ocFeesVec->push_back(_ocFees);

    _ocFees->travelStart() = airSeg;
    _ocFees->travelEnd() = airSeg;

    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    _ocFees->subCodeInfo() = s5;

    OptionalServicesInfo* osi = _memHandle.create<OptionalServicesInfo>();
    _ocFees->optFee() = osi;

    sfg->ocFeesMap().insert(std::pair<FarePath*, std::vector<OCFees*> >(farePath, *ocFeesVec));

    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    diagroot->diagParamMap().insert(std::pair<std::string, std::string>("OC", ""));
    Diag817Collector* diag = _memHandle.insert(new Diag817Collector(*diagroot));

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->diagCollector() = diag;

    _taxResponse->farePath() = farePath;

    PricingUnit* pricingUnit = _memHandle.create<PricingUnit>();
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    Fare* fare = _memHandle.create<Fare>();
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg().push_back(airSeg);

    fare->nucFareAmount() = 1000.00;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 1000.00;
    fareInfo->_currency = "HNL";
    fareInfo->_fareTariff = 0;
    fareInfo->_fareClass = "AA";

    TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    tariffRefInfo->_fareTariff = 0;

    fare->initialize(Fare::FS_Domestic, fareInfo, *fareMarket, tariffRefInfo);

    fareUsage->paxTypeFare() = paxTypeFare;
    fareUsage->paxTypeFare()->setFare(fare);

    fareUsage->travelSeg().push_back(airSeg);

    _taxResponse->farePath()->pricingUnit().push_back(pricingUnit);
    _taxResponse->farePath()->pricingUnit()[0]->fareUsage().push_back(fareUsage);

    _taxFactoryMap = _memHandle.create<TaxMap::TaxFactoryMap>();
    TaxMap::buildTaxFactoryMap(_trx->dataHandle(), *_taxFactoryMap);

    _taxDriver = _memHandle.create<TaxDriver>();

    _mdh->reset();
  }

  void taxDriverBucketsVerify(size_t itinTaxes, size_t ocTaxes)
  {
    CPPUNIT_ASSERT(_taxResponse->taxItemVector().size() == itinTaxes);
    CPPUNIT_ASSERT(_ocFees->getTaxes().size() == ocTaxes);
  }

  void testTaxDriverBucketsAllItin()
  {
    taxDriverBucketsSetUp();

    _taxDriver->ProcessTaxesAndFees(*_trx, *_taxResponse, *_taxFactoryMap, nullptr);

    taxDriverBucketsVerify(1, 0);
  }

  void testTaxDriverBucketsAllOC()
  {
    taxDriverBucketsSetUp();
    _mdh->setOC(0);
    _mdh->setOC(1);
    _mdh->setOC(2);

    _taxDriver->ProcessTaxesAndFees(*_trx, *_taxResponse, *_taxFactoryMap, nullptr);

    taxDriverBucketsVerify(0, 1);
  }

  void testTaxDriverBucketsItinOC()
  {
    taxDriverBucketsSetUp();
    _mdh->setOC(1);
    _mdh->setOC(2);

    _taxDriver->ProcessTaxesAndFees(*_trx, *_taxResponse, *_taxFactoryMap, nullptr);

    taxDriverBucketsVerify(1, 1);
  }

  void testTaxDriverBucketsOCItin()
  {
    taxDriverBucketsSetUp();
    _mdh->setOC(0);
    _mdh->setOC(1);

    _taxDriver->ProcessTaxesAndFees(*_trx, *_taxResponse, *_taxFactoryMap, nullptr);

    taxDriverBucketsVerify(1, 1);
  }

  void testBuildTaxNationVector()
  {
    taxDriverBucketsSetUp();
    _trx->getRequest()->ticketPointOverride() = "BUE";
    _trx->getRequest()->salePointOverride() = "LON";
    _taxDriver->buildTaxNationVector(*_trx, *_taxResponse, nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(3), _taxDriver->_taxNationVector.size());
    CPPUNIT_ASSERT(_taxDriver->findNation(*_trx, NationCode("AR")));
    CPPUNIT_ASSERT(_taxDriver->findNation(*_trx, NationCode("GB")));
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;
    std::vector<TaxCodeReg*> _taxCodeRegVector;

  public:
    MyDataHandle() : _memHandle(), _taxCodeRegVector()
    {
      _taxCodeRegVector.push_back(TestTaxCodeRegFactory::create(
          "/vobs/atseintl/test/testdata/data/TaxCodeReg_HNA_Seq100.xml"));
      _taxCodeRegVector.push_back(TestTaxCodeRegFactory::create(
          "/vobs/atseintl/test/testdata/data/TaxCodeReg_HNA_Seq200.xml"));
      _taxCodeRegVector.push_back(TestTaxCodeRegFactory::create(
          "/vobs/atseintl/test/testdata/data/TaxCodeReg_HNA_Seq300.xml"));
    }

    void setOC(int index)
    {
      _taxCodeRegVector[index]->specConfigName() = "ANCILLARYOC";
      _taxCodeRegVector[index]->specialProcessNo() = 0;
    }

    void reset()
    {
      for (int index = 0; index < 3; index++)
      {
        _taxCodeRegVector[index]->specConfigName() = "";
        _taxCodeRegVector[index]->specialProcessNo() = 23;
      }
    }

    const Currency* getCurrency(const CurrencyCode& currency, const DateTime& date)
    {
      if (currency == "HNL")
        return _memHandle.create<Currency>();
      return 0;
    }

    std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name)
    {
      std::vector<TaxSpecConfigReg*>& ret = *_memHandle.create<std::vector<TaxSpecConfigReg*> >();
      if (name == "ANCILLARYOC")
      {
        TaxSpecConfigReg* specConfigReg = _memHandle.create<TaxSpecConfigReg>();
        specConfigReg->effDate() = DateTime(2000, 1, 20);
        specConfigReg->discDate() = DateTime(2030, 1, 20);
        ret.push_back(specConfigReg);
        TaxSpecConfigReg::TaxSpecConfigRegSeq* seq = new TaxSpecConfigReg::TaxSpecConfigRegSeq();
        ret.front()->taxSpecConfigName() = name;
        seq->paramName() = "TAXBASE";
        seq->paramValue() = "OC";
        ret.front()->seqs().push_back(seq);
      }
      return ret;
    }

    const TaxNation* getTaxNation(const NationCode& nation, const DateTime& date)
    {
      if (nation == "US")
      {
        TaxNation* ret = _memHandle.create<TaxNation>();
        ret->roundingRule() = NEAREST;
        ret->taxRoundingOverrideInd() = 'E';
        ret->taxCollectionInd() = 'A';
        ret->taxFarequoteInd() = 'I';
        ret->taxCodeOrder() += "US1", "ZP";
        ret->nation() = nation;
        return ret;
      }
      else if (nation == "CA")
      {
        TaxNation* ret = _memHandle.create<TaxNation>();
        ret->roundingRule() = NEAREST;
        ret->taxRoundingOverrideInd() = ' ';
        ret->taxCollectionInd() = 'A';
        ret->taxFarequoteInd() = 'N';
        ret->taxCodeOrder() += "CA1", "CA2", "CA3", "SQ", "SQ3", "XG1";
        ret->nation() = nation;
        return ret;
      }
      else if (nation == "MX" || nation == "HN")
      {
        TaxNation* ret = _memHandle.create<TaxNation>();
        ret->roundingRule() = NEAREST;
        ret->taxRoundingOverrideInd() = ' ';
        ret->taxCollectionInd() = 'A';
        ret->taxFarequoteInd() = 'N';
        ret->taxCodeOrder() += "HNA";
        ret->nation() = nation;
        return ret;
      }
      else if (nation == "AR" || nation == "GB")
      {
        TaxNation* ret = _memHandle.create<TaxNation>();
        ret->roundingRule() = NEAREST;
        ret->taxRoundingOverrideInd() = ' ';
        ret->taxCollectionInd() = 'A';
        ret->taxFarequoteInd() = 'N';
        ret->taxCodeOrder() += "ZK";
        ret->nation() = nation;
        return ret;
      }

      else if (nation == "")
      {
        return 0;
      }
      return DataHandleMock::getTaxNation(nation, date);
    }
    const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& taxCode, const DateTime& date)
    {
      std::vector<TaxCodeReg*>& ret = *_memHandle.create<std::vector<TaxCodeReg*> >();
      if (taxCode == "US1")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml"));
        return ret;
      }
      else if (taxCode == "ZP")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml"));
        return ret;
      }
      else if (taxCode == "CA1")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_CA1.xml"));
        return ret;
      }
      else if (taxCode == "CA2")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_CA2.xml"));
        return ret;
      }
      else if (taxCode == "CA3")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_CA3.xml"));
        return ret;
      }
      else if (taxCode == "SQ")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ.xml"));
        return ret;
      }
      else if (taxCode == "SQ3")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ3.xml"));
        return ret;
      }
      else if (taxCode == "XG1")
      {
        ret.push_back(
            TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_XG1.xml"));
        return ret;
      }
      else if (taxCode == "HNA")
      {
        return _taxCodeRegVector;
      }

      return DataHandleMock::getTaxCode(taxCode, date);
    }
    const std::vector<TaxSegAbsorb*>&
    getTaxSegAbsorb(const CarrierCode& carrier, const DateTime& date)
    {
      if (carrier == "AA" || carrier == "AQ" || carrier == "F9" || carrier == "HP")
        return *_memHandle.create<std::vector<TaxSegAbsorb*> >();
      return DataHandleMock::getTaxSegAbsorb(carrier, date);
    }
  };

  TestMemHandle _memHandle;
  MyDataHandle* _mdh;
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  TaxMap::TaxFactoryMap* _taxFactoryMap;
  TaxDriver* _taxDriver;
  OCFees* _ocFees;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDriverTest);

} // tse
