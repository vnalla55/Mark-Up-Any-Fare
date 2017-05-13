//----------------------------------------------------------------------------
//  Copyright Sabre 2008
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

#include "test/include/CppUnitHelperMacros.h"
#include "MinFares/RoundTripFareSelection.h"
#include "MinFares/RoundTripFareSelection.cpp"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "DataModel/PaxType.h"
#include "MinFares/test/MinFareDataHandleTest.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
class RoundTripFareSelectionDerived : public RoundTripFareSelection
{
public:
  static bool derivedFlag;

  RoundTripFareSelectionDerived(MinimumFareModule module,
                                DiagCollector* diag,
                                PricingTrx& trx,
                                const FarePath& farePath,
                                const PricingUnit& pu,
                                const std::vector<TravelSeg*>& obTvlSegs,
                                const std::vector<TravelSeg*>& ibTvlSegs,
                                CabinType lowestCabin,
                                const PaxType* paxType,
                                const DateTime& travelDate,
                                const PaxTypeFare* obThruFare = 0,
                                const PaxTypeFare* ibThruFare = 0,
                                const MinFareAppl* minFareAppl = 0,
                                const MinFareDefaultLogic* minFareDefLogic = 0,
                                const RepricingTrx* obRepricingTrx = 0,
                                const RepricingTrx* ibRepricingTrx = 0,
                                FMDirection selectFareForDirection = FMDirection::UNKNOWN)
    : RoundTripFareSelection(module,
                             diag,
                             trx,
                             farePath,
                             pu,
                             obTvlSegs,
                             ibTvlSegs,
                             lowestCabin,
                             paxType,
                             travelDate,
                             obThruFare,
                             ibThruFare,
                             minFareAppl,
                             minFareDefLogic,
                             obRepricingTrx,
                             ibRepricingTrx,
                             selectFareForDirection)
  {
  }

  ~RoundTripFareSelectionDerived() {}

  void reuseThroughFare() { return reuseThruFare(); }
  const PaxTypeFare* obFare() { return _obFare; }
  const PaxTypeFare* ibFare() { return _ibFare; }

  void setObIbFareAndGI(const PaxTypeFare* obFare,
                        GlobalDirection obGi,
                        const PaxTypeFare* ibFare,
                        GlobalDirection ibGi)
  {
    _obFare = obFare;
    _obGi = obGi;
    _ibFare = ibFare;
    _ibGi = ibGi;
  }

  const PaxTypeFare* selectFare(PaxTypeStatus status, bool nextCabin) { return 0; }

  void getFareDiffGI() { selectFareDiffGI(*this, *this, PAX_TYPE_STATUS_ADULT); }

  void setStatusTrue() { _status = true; }

  void setStatusFalse() { _status = false; }

  virtual bool isThruFareValidForReUse(const PaxTypeFare& paxTypeFare)
  {
    if (!RoundTripFareSelectionDerived::derivedFlag)
      return RoundTripFareSelection::isThruFareValidForReUse(paxTypeFare);
    else
      return !_status;
  }
};

bool RoundTripFareSelectionDerived::derivedFlag = true;

class RoundTripFareSelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RoundTripFareSelectionTest);
  CPPUNIT_TEST(testReuseOutboundThruFare);
  CPPUNIT_TEST(testReuseInboundThruFare);
  CPPUNIT_TEST(testReuseOutboundAndInboundThruFare);
  CPPUNIT_TEST(testNoThruFareReused);
  CPPUNIT_TEST(testObIbFareSavedInRtFares);
  CPPUNIT_TEST(testObFareSavedInRtFaresWhenIbFareNotAvailable);
  CPPUNIT_TEST(testIbFareSavedInRtFaresWhenObFareNotAvailable);
  CPPUNIT_TEST(testNoObIbFareSavedInRtFares);
  CPPUNIT_TEST(test_isThruFareValidForReUse);
  CPPUNIT_TEST(test_isThruFareValidForReUse_SpecialFare);
  CPPUNIT_TEST(test_isThruFareValidForReUse_NormalFare);
  CPPUNIT_TEST_SUITE_END();

  CabinType _ecoCabin;
  PaxType paxType;
  FareInfo fareInfo;
  TariffCrossRefInfo tariffRefInfo;
  Fare fare;
  PaxTypeFare ptf;
  DateTime tktDate;
  PricingRequest* _request;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MinFareDataHandleTest>();
    // GlobalDirection gd = tse::XX;
    fm1.travelSeg().push_back(&tvlSeg1);
    fm1.travelSeg().push_back(&tvlSeg2);
    fm2.travelSeg().push_back(&tvlSeg3);

    fare1.fareMarket() = &fm1;
    fare2.fareMarket() = &fm2;

    fu1.paxTypeFare() = &fare1;
    fu2.paxTypeFare() = &fare2;

    PaxTypeCode paxTypeCode = ADULT;
    paxType.paxType() = paxTypeCode;
    paxType.vendorCode() = ATPCO_VENDOR_CODE;

    trx.setOptions(&options);

    pu.fareUsage().push_back(&fu1);
    pu.fareUsage().push_back(&fu2);
    _ecoCabin.setEconomyClass();
    rtSel = _memHandle.insert(new RoundTripFareSelectionDerived(
        CTM, 0, trx, fp, pu, obTvlSegs, ibTvlSegs, _ecoCabin, &paxType, DateTime::localTime()));

    tariffRefInfo._fareTariffCode = "TAFPBA";
    tariffRefInfo._tariffCat = 0;
    tariffRefInfo._ruleTariff = 304;

    fare.initialize(Fare::FS_International, &fareInfo, fm1, &tariffRefInfo);
    paxType.paxType() = paxTypeCode;

    fare1.initialize(&fare, &paxType, &fm1);
    fare2.initialize(&fare, &paxType, &fm2);

    fu1.paxTypeFare() = &fare1;
    fu2.paxTypeFare() = &fare2;

    appInfo = _memHandle.create<FareClassAppInfo>();
    ptf.initialize(&fare, &paxType, &fm1);
    tariffRefInfo1 = _memHandle.create<TariffCrossRefInfo>();
    _request = _memHandle.create<PricingRequest>();

    tktDate = DateTime(2009, 4, 16, 8, 15, 0);
    _request->ticketingDT() = tktDate;
    trx.setRequest(_request);
  }

  void tearDown() { _memHandle.clear(); }

  void testReuseOutboundThruFare()
  {
    obTvlSegs.push_back(&tvlSeg1);
    obTvlSegs.push_back(&tvlSeg2);
    ibTvlSegs.push_back(&tvlSeg4);
    rtSel->reuseThroughFare();
    CPPUNIT_ASSERT(rtSel->roundTripFare().front() == &fare1);
    CPPUNIT_ASSERT(rtSel->roundTripFare().back() == &fare1);
  }

  void testReuseInboundThruFare()
  {
    obTvlSegs.push_back(&tvlSeg1);
    ibTvlSegs.push_back(&tvlSeg3);
    rtSel->reuseThroughFare();
    CPPUNIT_ASSERT(!rtSel->roundTripFare().empty());
    CPPUNIT_ASSERT(rtSel->roundTripFare().front() == &fare2);
    CPPUNIT_ASSERT(rtSel->roundTripFare().back() == &fare2);
  }

  void testReuseOutboundAndInboundThruFare()
  {
    obTvlSegs.push_back(&tvlSeg1);
    obTvlSegs.push_back(&tvlSeg2);
    ibTvlSegs.push_back(&tvlSeg3);
    rtSel->reuseThroughFare();
    CPPUNIT_ASSERT(rtSel->roundTripFare().front() == &fare1);
    CPPUNIT_ASSERT(rtSel->roundTripFare().back() == &fare1);
  }

  void testNoThruFareReused()
  {
    obTvlSegs.push_back(&tvlSeg3);
    ibTvlSegs.push_back(&tvlSeg4);
    rtSel->reuseThroughFare();
    CPPUNIT_ASSERT(rtSel->roundTripFare().empty());
  }

  void testObIbFareSavedInRtFares()
  {
    rtSel->setObIbFareAndGI(&fare1, GlobalDirection::EH, &fare2, GlobalDirection::TS);
    rtSel->getFareDiffGI();
    CPPUNIT_ASSERT(rtSel->roundTripFare().front() == &fare1);
    CPPUNIT_ASSERT(rtSel->roundTripFare().back() == &fare2);
  }

  void testObFareSavedInRtFaresWhenIbFareNotAvailable()
  {
    rtSel->setObIbFareAndGI(&fare1, GlobalDirection::EH, 0, GlobalDirection::XX);
    rtSel->getFareDiffGI();
    CPPUNIT_ASSERT(rtSel->roundTripFare().front() == &fare1);
  }

  void testIbFareSavedInRtFaresWhenObFareNotAvailable()
  {
    rtSel->setObIbFareAndGI(0, GlobalDirection::XX, &fare2, GlobalDirection::EH);
    rtSel->getFareDiffGI();
    CPPUNIT_ASSERT(rtSel->roundTripFare().front() == &fare2);
  }

  void testNoObIbFareSavedInRtFares()
  {
    rtSel->setObIbFareAndGI(0, GlobalDirection::XX, 0, GlobalDirection::XX);
    rtSel->getFareDiffGI();
    CPPUNIT_ASSERT(rtSel->roundTripFare().empty());
  }

  void test_isThruFareValidForReUse()
  {
    obTvlSegs.push_back(&tvlSeg1);
    ibTvlSegs.push_back(&tvlSeg3);
    rtSel->setStatusFalse();
    rtSel->reuseThroughFare();
    CPPUNIT_ASSERT(!rtSel->roundTripFare().empty());
  }

  void setPaxTypeFareSpecial()
  {
    appInfo->_pricingCatType = 'S'; // PaxTypeFare::PRICING_CATTYPE_NORMAL;
    ptf.fareClassAppInfo() = appInfo;
    tariffRefInfo1->_ruleTariff = 304;
  }

  void setPaxTypeFareNornal()
  {
    appInfo->_pricingCatType = 'N'; // PaxTypeFare::PRICING_CATTYPE_NORMAL;
    ptf.fareClassAppInfo() = appInfo;
    tariffRefInfo1->_ruleTariff = 304;
  }

  void test_isThruFareValidForReUse_SpecialFare()
  {
    rtSel->derivedFlag = false;
    setPaxTypeFareSpecial();
    CPPUNIT_ASSERT(rtSel->isThruFareValidForReUse(ptf));
  }

  void test_isThruFareValidForReUse_NormalFare()
  {
    rtSel->derivedFlag = false;
    setPaxTypeFareNornal();
    CPPUNIT_ASSERT(rtSel->isThruFareValidForReUse(ptf));
  }

protected:
  PricingTrx trx;
  PricingOptions options;
  FarePath fp;
  PricingUnit pu;
  FareUsage fu1, fu2;
  std::vector<TravelSeg*> obTvlSegs;
  std::vector<TravelSeg*> ibTvlSegs;
  AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4;
  FareMarket fm1, fm2;
  PaxTypeFare fare1, fare2;
  RoundTripFareSelectionDerived* rtSel;
  FareClassAppInfo* appInfo;
  TariffCrossRefInfo* tariffRefInfo1;
  TestMemHandle _memHandle;
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(tse::RoundTripFareSelectionTest);
