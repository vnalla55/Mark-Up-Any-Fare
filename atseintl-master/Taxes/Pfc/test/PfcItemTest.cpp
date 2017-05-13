#include <string>
#include <iostream>
#include <vector>

#include "Taxes/Pfc/PfcItem.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"

#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"

#include "DataModel/PricingRequest.h"
#include "Common/DateTime.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diagnostic.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/DiscountInfo.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "test/include/MockTseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include <unistd.h>
#include "DBAccess/PfcCoterminal.h"
#include "DBAccess/PfcMultiAirport.h"
#include <boost/assign/std/vector.hpp>
#include "DBAccess/PfcPFC.h"
#include "DBAccess/PfcCollectMeth.h"

using namespace boost::assign;
using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  PfcCoterminal* getCoT(int seq, LocCode lc)
  {
    PfcCoterminal* ret = new PfcCoterminal;
    ret->orderNo() = seq;
    ret->cotermLoc() = lc;
    ret->vendor() = "ATP";
    return ret;
  }
  PfcPFC* getPfc(Indicator pfcAirTaxExcp)
  {
    PfcPFC* ret = _memHandle.create<PfcPFC>();
    ret->pfcAmt1() = 4.50;
    ret->pfcAirTaxExcp() = pfcAirTaxExcp;
    ret->pfcCharterExcp() = 'N';
    ret->freqFlyerInd() = 'Y';
    ret->pfcCur1() = "USD";
    ret->vendor() = "ATP";
    return ret;
  }

public:
  const PfcMultiAirport* getPfcMultiAirport(const LocCode& key, const DateTime& date)
  {
    if (key == "MIA")
    {
      PfcMultiAirport* ret = _memHandle.create<PfcMultiAirport>();
      ret->segCnt() = 2;
      ret->coterminals() += getCoT(1, "MPB"), getCoT(2, "OPF");
      return ret;
    }
    else if (key == "LHR")
    {
      PfcMultiAirport* ret = _memHandle.create<PfcMultiAirport>();
      ret->segCnt() = 6;
      ret->coterminals() += getCoT(1, "BQH"), getCoT(2, "LCY"), getCoT(3, "LGW"), getCoT(4, "LON"),
          getCoT(5, "LTN"), getCoT(6, "STN");
      return ret;
    }
    return DataHandleMock::getPfcMultiAirport(key, date);
  }
  const PfcPFC* getPfcPFC(const LocCode& key, const DateTime& date)
  {
    if (key == "MIA")
      return getPfc('Y');
    else if (key == "DFW" || key == "LAX" || key == "SEA")
      return getPfc('N');
    return DataHandleMock::getPfcPFC(key, date);
  }
  const std::vector<PfcEssAirSvc*>&
  getPfcEssAirSvc(const LocCode& easHubArpt, const LocCode& easArpt, const DateTime& date)
  {
    if ((easHubArpt == "MIA" && easArpt == "DFW") || (easHubArpt == "DFW" && easArpt == "LAX") ||
        (easHubArpt == "LAX" && easArpt == "SEA") || (easHubArpt == "SEA" && easArpt == "MIA"))
      return *_memHandle.create<std::vector<PfcEssAirSvc*> >();
    return DataHandleMock::getPfcEssAirSvc(easHubArpt, easArpt, date);
  }
  const std::vector<PfcCollectMeth*>&
  getPfcCollectMeth(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "IB")
    {
      std::vector<PfcCollectMeth*>& ret = *_memHandle.create<std::vector<PfcCollectMeth*> >();
      ret.push_back(_memHandle.create<PfcCollectMeth>());
      ret.front()->collectOption() = 1;
      return ret;
    }
    return DataHandleMock::getPfcCollectMeth(carrier, date);
  }
  const PaxTypeInfo* getPaxType(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
  {
    PaxTypeInfo* pti = _memHandle.create<PaxTypeInfo>();
    pti->infantInd() = 'Y';
    pti->initPsgType();
    return pti;
  }
};
}

class PfcItemTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PfcItemTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testPfcItemCase0);
  CPPUNIT_TEST(testPfcItemCase1);
  CPPUNIT_TEST(testPfcItem_isInfantExempt_true_dummyFare_zeroFareAmount);
  //   CPPUNIT_TEST(testPfcItem_isInfantExempt_false_noDummyFare);
  CPPUNIT_TEST(testPfcItem_isInfantExempt_false_not_zeroFareAmount);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
  }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { PfcItem pfcItem; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testPfcItemCase0()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingOptions options;
    trx.setOptions(&options);

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Loc origin;
    Loc destination;

    origin.loc() = std::string("MIA");
    origin.nation() = std::string("US");

    destination.loc() = std::string("DFW");
    destination.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination;

    AirSeg airSeg;
    TravelSeg* travelSeg = &airSeg;

    travelSeg->origin() = &origin;
    travelSeg->destination() = &destination;
    travelSeg->departureDT() = dt.localTime();

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    PaxType pt;
    PaxTypeInfo pti;
    pt.paxTypeInfo() = &pti;

    FarePath farePath;

    farePath.paxType() = &pt;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(travelSeg);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("MY");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = tse::LocCode("MY");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

    PfcItem pfcItem;

    pfcItem.build(trx, taxResponse);

    Loc origin2;
    Loc destination2;

    origin2.loc() = std::string("DFW");
    origin2.nation() = std::string("US");

    destination2.loc() = std::string("LAX");
    destination2.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination2;

    AirSeg airSeg2;
    TravelSeg* travelSeg2 = &airSeg2;

    travelSeg2->origin() = &origin2;
    travelSeg2->destination() = &destination2;
    travelSeg2->departureDT() = dt.localTime();

    farePath.itin()->travelSeg().push_back(travelSeg2);

    Loc origin3;
    Loc destination3;

    origin3.loc() = std::string("LAX");
    origin3.nation() = std::string("US");

    destination3.loc() = std::string("SEA");
    destination3.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination3;

    AirSeg airSeg3;
    TravelSeg* travelSeg3 = &airSeg3;

    travelSeg3->origin() = &origin3;
    travelSeg3->destination() = &destination3;
    travelSeg3->departureDT() = dt;

    farePath.itin()->travelSeg().push_back(travelSeg3);

    pfcItem.build(trx, taxResponse);

    Loc origin4;
    Loc destination4;

    origin4.loc() = std::string("SEA");
    origin4.nation() = std::string("US");

    destination4.loc() = std::string("MIA");
    destination4.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination4;

    AirSeg airSeg4;
    TravelSeg* travelSeg4 = &airSeg4;

    travelSeg4->origin() = &origin4;
    travelSeg4->destination() = &destination4;
    travelSeg4->departureDT() = dt.localTime();

    farePath.itin()->travelSeg().push_back(travelSeg4);

    pfcItem.build(trx, taxResponse);
  }

  void testPfcItemCase1()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingOptions options;
    trx.setOptions(&options);

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Loc origin;
    Loc destination;

    origin.loc() = std::string("LHR");
    origin.nation() = std::string("GB");

    destination.loc() = std::string("CDG");
    destination.nation() = std::string("FR");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination;

    AirSeg airSeg;

    airSeg.setMarketingCarrierCode(std::string("IB"));
    TravelSeg* travelSeg = &airSeg;

    travelSeg->origin() = &origin;
    travelSeg->destination() = &destination;
    travelSeg->departureDT() = dt.localTime();

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    PaxType pt;
    PaxTypeInfo pti;
    pt.paxTypeInfo() = &pti;

    FarePath farePath;

    farePath.paxType() = &pt;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(travelSeg);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("MY");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = tse::LocCode("MY");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

    PfcItem pfcItem;

    pfcItem.build(trx, taxResponse);
  }

  void testPfcItem_isInfantExempt_true_dummyFare_zeroFareAmount()
  {
    PricingTrx trx;

    AirSeg airSeg;
    TravelSeg* travelSeg = &airSeg;
    travelSeg->segmentOrder() = 0;

    PaxType pt;
    PaxTypeInfo pti;
    pt.paxTypeInfo() = &pti;
    pti.infantInd() = 'Y';
    pti.initPsgType();

    FarePath farePath;
    farePath.paxType() = &pt;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    PricingUnit pu;
    farePath.pricingUnit().push_back(&pu);
    FareUsage fu;
    pu.fareUsage().push_back(&fu);

    PaxTypeFare ptf;
    fu.paxTypeFare() = &ptf;
    FareClassAppSegInfo fcai;
    ptf.fareClassAppSegInfo() = &fcai;
    ptf.nucFareAmount() = 0;

    fu.travelSeg().push_back(travelSeg);

    FareMarket fm;
    ptf.fareMarket() = &fm;
    fm.fareBasisCode() = "QAZ";
    fm.fareCalcFareAmt() = "100";

    PfcItem pfcItem;
    bool res = pfcItem.isInfantExempt(trx, taxResponse, *travelSeg);
    CPPUNIT_ASSERT(res);
  }

  void testPfcItem_isInfantExempt_false_noDummyFare()
  {
    PricingTrx trx;

    AirSeg airSeg;
    TravelSeg* travelSeg = &airSeg;
    travelSeg->segmentOrder() = 0;

    PaxType pt;
    PaxTypeInfo pti;
    pt.paxTypeInfo() = &pti;
    pti.infantInd() = 'Y';
    pti.initPsgType();

    FarePath farePath;
    farePath.paxType() = &pt;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    PricingUnit pu;
    farePath.pricingUnit().push_back(&pu);
    FareUsage fu;
    pu.fareUsage().push_back(&fu);

    PaxTypeFare ptf;
    fu.paxTypeFare() = &ptf;
    FareClassAppSegInfo fcai;
    ptf.fareClassAppSegInfo() = &fcai;
    ptf.nucFareAmount() = 0;

    fu.travelSeg().push_back(travelSeg);

    FareMarket fm;
    ptf.fareMarket() = &fm;
    fm.fareBasisCode() = "";
    fm.fareCalcFareAmt() = "";

    PfcItem pfcItem;
    bool res = pfcItem.isInfantExempt(trx, taxResponse, *travelSeg);
    CPPUNIT_ASSERT(!res);
  }

  void testPfcItem_isInfantExempt_false_not_zeroFareAmount()
  {
    PricingTrx trx;

    AirSeg airSeg;
    TravelSeg* travelSeg = &airSeg;
    travelSeg->segmentOrder() = 0;

    PaxType pt;
    PaxTypeInfo pti;
    pt.paxTypeInfo() = &pti;
    pti.infantInd() = 'Y';
    pti.initPsgType();

    FarePath farePath;
    farePath.paxType() = &pt;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    PricingUnit pu;
    farePath.pricingUnit().push_back(&pu);
    FareUsage fu;
    pu.fareUsage().push_back(&fu);

    PaxTypeFare ptf;
    fu.paxTypeFare() = &ptf;
    FareClassAppSegInfo fcai;
    ptf.fareClassAppSegInfo() = &fcai;
    ptf.nucFareAmount() = 1;

    fu.travelSeg().push_back(travelSeg);

    FareMarket fm;
    ptf.fareMarket() = &fm;
    fm.fareBasisCode() = "QAZ";
    fm.fareCalcFareAmt() = "100";

    PfcItem pfcItem;
    bool res = pfcItem.isInfantExempt(trx, taxResponse, *travelSeg);
    CPPUNIT_ASSERT(!res);
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PfcItemTest);
}
