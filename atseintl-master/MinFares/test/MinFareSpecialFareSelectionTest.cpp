//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "MinFares/MinFareSpecialFareSelection.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "Common/TseConsts.h"
#include "Common/ErrorResponseException.h"
#include "MinFares/MinimumFare.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DataModel/Itin.h"
#include "Common/DateTime.h"
#include <fstream>
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/MinFareFareTypeGrp.h"
#include "MinFares/test/MinFareDataHandleTest.h"

namespace tse
{

class MinFareSpecialFareSelectionDerived : public MinFareSpecialFareSelection
{
public:
  MinFareSpecialFareSelectionDerived(MinimumFareModule module,
                                     EligibleFare eligibleFare,
                                     FareDirectionChoice fareDirection,
                                     const FareType& fareType,
                                     PricingTrx& trx,
                                     const Itin& itin,
                                     const std::vector<TravelSeg*>& travelSegs,
                                     const std::vector<PricingUnit*>& pricingUnits,
                                     const PaxType* paxType,
                                     DateTime travelDate,
                                     const FarePath* farePath = 0,
                                     const PaxTypeFare* thruFare = 0,
                                     const MinFareAppl* minFareAppl = 0,
                                     const MinFareDefaultLogic* minFareDefaultLogic = 0,
                                     const RepricingTrx* repricingTrx = 0,
                                     const PaxTypeCode& actualPaxType = "")
    : MinFareSpecialFareSelection(module,
                                  eligibleFare,
                                  fareDirection,
                                  fareType,
                                  trx,
                                  itin,
                                  travelSegs,
                                  pricingUnits,
                                  paxType,
                                  travelDate,
                                  farePath,
                                  thruFare,
                                  minFareAppl,
                                  minFareDefaultLogic,
                                  repricingTrx,
                                  actualPaxType)
  {
  }
  ~MinFareSpecialFareSelectionDerived() {}

  virtual bool ruleValidated(const std::vector<uint16_t>& categories,
                             const PaxTypeFare& paxTypeFare,
                             bool puScope = false)
  {
    return true;
  }

protected:
  TestMemHandle _memHandle;
};

class MinFareSpecialFareSelectionDerived2 : public MinFareSpecialFareSelectionDerived
{
public:
  MinFareSpecialFareSelectionDerived2(MinimumFareModule module,
                                      EligibleFare eligibleFare,
                                      FareDirectionChoice fareDirection,
                                      const FareType& fareType,
                                      PricingTrx& trx,
                                      const Itin& itin,
                                      const std::vector<TravelSeg*>& travelSegs,
                                      const std::vector<PricingUnit*>& pricingUnits,
                                      const PaxType* paxType,
                                      DateTime travelDate,
                                      const FarePath* farePath = 0,
                                      const PaxTypeFare* thruFare = 0,
                                      const MinFareAppl* minFareAppl = 0,
                                      const MinFareDefaultLogic* minFareDefaultLogic = 0,
                                      const RepricingTrx* repricingTrx = 0,
                                      const PaxTypeCode& actualPaxType = "")
    : MinFareSpecialFareSelectionDerived(module,
                                         eligibleFare,
                                         fareDirection,
                                         fareType,
                                         trx,
                                         itin,
                                         travelSegs,
                                         pricingUnits,
                                         paxType,
                                         travelDate,
                                         farePath,
                                         thruFare,
                                         minFareAppl,
                                         minFareDefaultLogic,
                                         repricingTrx,
                                         actualPaxType)
  {
  }
  ~MinFareSpecialFareSelectionDerived2() {}

  MinFareFareTypeGrp* createMinFareFareTypeGrp()
  {
    MinFareFareTypeGrp* grp = _memHandle.create<MinFareFareTypeGrp>();
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 1, "AAA"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 1, "BBB"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 1, "CCC"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 1, "DDD"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 2, "EEE"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 2, "FFF"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 2, "GGG"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 2, "HHH"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 3, "III"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 3, "JJJ"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 3, "KKK"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 3, "LLL"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 4, "MMM"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 4, "NNN"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 4, "OOO"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(1, 4, "PPP"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(2, 1, "QQQ"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(2, 2, "RRR"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(2, 3, "SSS"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(2, 4, "TTT"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(3, 1, "UUU"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(3, 1, "VVV"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(3, 1, "WWW"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(3, 3, "XXX"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(3, 3, "YYY"));
    grp->segs().push_back(createMinFareFareTypeGrpSeg(3, 6, "ZZZ"));

    return grp;
  }
  MinFareFareTypeGrpSeg*
  createMinFareFareTypeGrpSeg(int grpsetno, int setno, std::string fareType, int grpType = 1)
  {
    // need to use new. segmensts will be deleted in MinFareFareTypeGrp destructor
    MinFareFareTypeGrpSeg* seg = new MinFareFareTypeGrpSeg();
    seg->setNo() = setno;
    seg->grpSetNo() = grpsetno;
    seg->grpType() = grpType;
    seg->fareType() = fareType;
    return seg;
  }
  const MinFareFareTypeGrp* getMinFareFareTypeGrp() { return createMinFareFareTypeGrp(); }
  const PaxTypeFare* selectSpecialFare(const std::vector<TravelSeg*>& travelSegs)
  {
    double value = 999999999;
    PaxTypeFare* ptf = 0;
    std::map<FareType, PaxTypeFare*>::iterator it = _fareMap.begin();
    std::map<FareType, PaxTypeFare*>::iterator ie = _fareMap.end();
    for (; it != ie; it++)
    {
      if (!matchFareType(it->first))
        continue;
      if (it->second->notValidForInclusionCode())
        continue;
      if (it->second->nucFareAmount() < value)
      {
        ptf = it->second;
        value = ptf->nucFareAmount();
      }
    }
    return ptf;
  }
  const FareTypeMatrix* getFareTypeMatrix(const FareType& fareType, const DateTime& date)
  {
    if (fareType == "AAA" || fareType == "BBB")
      _ftm.fareTypeAppl() = 'S';
    else
      _ftm.fareTypeAppl() = 'N';
    return &_ftm;
  }
  std::map<FareType, PaxTypeFare*> _fareMap;
  FareTypeMatrix _ftm;
};

class MinFareSpecialFareSelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinFareSpecialFareSelectionTest);
  CPPUNIT_TEST(testSameFareType);
  CPPUNIT_TEST(testSpclDiffFareType);
  CPPUNIT_TEST(testSpclFareHierachy);
  CPPUNIT_TEST(testCTMAlternateFareSelect);

  CPPUNIT_TEST(testHigherFareType_NoMatchOnFareType);
  CPPUNIT_TEST(testHigherFareType_Match_Grp1_Set1);
  CPPUNIT_TEST(testHigherFareType_Match_Grp1_Set1_Bump2);
  CPPUNIT_TEST(testHigherFareType_Match_Grp1_Set1_Bump3);
  CPPUNIT_TEST(testHigherFareType_Match_Grp1_Set1_Bump4);
  CPPUNIT_TEST(testHigherFareType_Match_Grp1_Set1_NoBump);

  CPPUNIT_TEST(testHigherFareType_Match_Grp2_Set1);
  CPPUNIT_TEST(testHigherFareType_Match_Grp2_Set1_Bump2);
  CPPUNIT_TEST(testHigherFareType_Match_Grp2_Set1_Bump3);
  CPPUNIT_TEST(testHigherFareType_Match_Grp2_Set1_NoBump);

  CPPUNIT_TEST(testHigherFareType_Match_Grp3_Set1);
  CPPUNIT_TEST(testHigherFareType_Match_Grp3_Set1_Bump3);
  CPPUNIT_TEST(testHigherFareType_Match_Grp3_Set1_Bump6);
  CPPUNIT_TEST(testHigherFareType_Match_Grp3_set1_NoBump);

  CPPUNIT_TEST(testProcessHigherFareTypeNew_NoMatch);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_CurFTNotValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_CCC_CurFTNotValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_CCC_DDD_CurFTNotValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_BBB_DDD_CurFTNotValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_CurFTValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_CCC_CurFTValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_CCC_DDD_CurFTValid);
  CPPUNIT_TEST(testProcessHigherFareTypeNew_MatchAAA_BBB_DDD_CurFTValid);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MinFareDataHandleTest>();

    _fareMarket = TestFareMarketFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/fareMarket.xml");
    _paxType = TestPaxTypeFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/paxType.xml");
    _paxTypeFare1 = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/paxTypeFare1.xml");
    _airSeg =
        TestAirSegFactory::create("/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/"
                                  "MinFarePromFareSelection_AirSeg0.xml");

    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _itin = _memHandle.create<Itin>();
    _travelSegs = _memHandle.create<std::vector<TravelSeg*> >();
    _pricingUnits = _memHandle.create<std::vector<PricingUnit*> >();
    _minFareAppl = _memHandle.create<MinFareAppl>();
    _defaultLogic = _memHandle.create<MinFareDefaultLogic>();

    _trx->diagnostic().diagnosticType() = Diagnostic710;
    _trx->diagnostic().activate();
    _trx->itin().push_back(_itin);
    _trx->setRequest(_request);
    _trx->travelSeg().push_back(_airSeg);
    _trx->fareMarket().push_back(_fareMarket);
    _itin->travelSeg().push_back(_airSeg);
    _travelSegs->push_back(_airSeg);

    _minFareAppl->seqNo() = 11111;
    _minFareAppl->applyDefaultLogic() = MinimumFare::NO;
    _minFareAppl->spclHipTariffCatInd() = MinimumFare::YES;
    _minFareAppl->spclHipRuleTrfInd() = MinimumFare::YES;
    _minFareAppl->spclHipFareClassInd() = MinimumFare::NO;
    _minFareAppl->spclHip1stCharInd() = MinimumFare::YES;
    _minFareAppl->spclHipStopCompInd() = MinimumFare::YES;
    _minFareAppl->spclCtmTariffCatInd() = MinimumFare::NO;
    _minFareAppl->spclCtmRuleTrfInd() = MinimumFare::NO;
    _minFareAppl->spclCtmFareClassInd() = MinimumFare::NO;
    _minFareAppl->spclSame1stCharFBInd2() = MinimumFare::YES;
    _minFareAppl->spclCtmStopCompInd() = MinimumFare::NO;
    _minFareAppl->specialProcessName() = "INDUSTRY";

    _defaultLogic->seqNo() = 22222;
    _defaultLogic->spclHipTariffCatInd() = MinimumFare::YES;
    _defaultLogic->spclHipRuleTrfInd() = MinimumFare::YES;
    _defaultLogic->spclHipFareClassInd() = MinimumFare::NO;
    _defaultLogic->spclHip1stCharInd() = MinimumFare::YES;
    _defaultLogic->spclHipStopCompInd() = MinimumFare::YES;
    _defaultLogic->spclCtmTariffCatInd() = MinimumFare::NO;
    _defaultLogic->spclCtmRuleTrfInd() = MinimumFare::NO;
    _defaultLogic->spclCtmFareClassInd() = MinimumFare::NO;
    _defaultLogic->spclSame1stCharFBInd2() = MinimumFare::YES;
    _defaultLogic->spclCtmStopCompInd() = MinimumFare::NO;
    _defaultLogic->specialProcessName() = "INDUSTRY";
  }
  void tearDown() { _memHandle.clear(); }

  void testSameFareType()
  {
    PaxTypeFare* paxTypeFarePtr = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/paxTypeFare.xml");

    MinFareSpecialFareSelectionDerived fareSel(HIP,
                                               MinFareFareSelection::ONE_WAY,
                                               MinFareFareSelection::OUTBOUND,
                                               paxTypeFarePtr->fcaFareType(),
                                               *_trx,
                                               *_itin,
                                               *_travelSegs,
                                               *_pricingUnits,
                                               _paxType,
                                               DateTime::emptyDate(),
                                               0,
                                               paxTypeFarePtr,
                                               _minFareAppl);

    CPPUNIT_ASSERT_NO_THROW(fareSel.selectFare());

    std::string str = _trx->diagnostic().toString();
    // std::cout << str << std::endl;
  }

  void testSpclDiffFareType()
  {
    PaxTypeFare* paxTypeFarePtr = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/paxTypeFare1.xml");

    _minFareAppl->applyDefaultLogic() = MinimumFare::YES;

    MinFareSpecialFareSelectionDerived fareSel1(CTM,
                                                MinFareFareSelection::HALF_ROUND_TRIP,
                                                MinFareFareSelection::OUTBOUND,
                                                paxTypeFarePtr->fcaFareType(),
                                                *_trx,
                                                *_itin,
                                                *_travelSegs,
                                                *_pricingUnits,
                                                _paxType,
                                                DateTime::emptyDate(),
                                                0,
                                                paxTypeFarePtr,
                                                _minFareAppl,
                                                _defaultLogic);

    CPPUNIT_ASSERT_NO_THROW(fareSel1.selectFare());

    std::string str = _trx->diagnostic().toString();
    // std::cout << str << std::endl;
  }

  void testSpclFareHierachy()
  {
    PaxTypeFare* paxTypeFarePtr = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/paxTypeFare2.xml");

    MinFareSpecialFareSelectionDerived fareSel(HIP,
                                               MinFareFareSelection::ONE_WAY,
                                               MinFareFareSelection::OUTBOUND,
                                               paxTypeFarePtr->fcaFareType(),
                                               *_trx,
                                               *_itin,
                                               *_travelSegs,
                                               *_pricingUnits,
                                               _paxType,
                                               DateTime::emptyDate(),
                                               0,
                                               paxTypeFarePtr,
                                               _minFareAppl);

    CPPUNIT_ASSERT_NO_THROW(fareSel.selectFare());

    std::string str = _trx->diagnostic().toString();
    // std::cout << str << std::endl;
  }

  void testCTMAlternateFareSelect()
  {
    PricingTrx trx;
    trx.setRequest(_request);
    trx.diagnostic().diagnosticType() = Diagnostic710;
    trx.diagnostic().activate();

    PaxType paxType;
    paxType.paxType() = "ADT";

    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");

    // Travel segment is LON-NYC, NYC-DFW, DFW-NYC. Alternate fare market retrival should get
    // LON-NYC fare
    // which matches first segment and destination point
    AirSeg airSeg;
    airSeg.segmentOrder() = 0;
    airSeg.geoTravelType() = GeoTravelType::International;
    airSeg.origin() = loc;
    airSeg.destination() = loc1;
    airSeg.boardMultiCity() = loc->loc();
    airSeg.offMultiCity() = loc1->loc();
    airSeg.carrier() = "BA";

    trx.travelSeg().push_back(&airSeg);

    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

    AirSeg airSeg2;
    airSeg2.segmentOrder() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = loc1;
    airSeg2.destination() = loc2;
    airSeg2.boardMultiCity() = loc1->loc();
    airSeg2.offMultiCity() = loc2->loc();
    airSeg2.carrier() = "BA";

    trx.travelSeg().push_back(&airSeg2);

    AirSeg airSeg3;
    airSeg3.segmentOrder() = 2;
    airSeg3.geoTravelType() = GeoTravelType::International;
    airSeg3.origin() = loc2;
    airSeg3.destination() = loc1;
    airSeg3.boardMultiCity() = loc2->loc();
    airSeg3.offMultiCity() = loc1->loc();
    airSeg3.carrier() = "BA";

    trx.travelSeg().push_back(&airSeg3);

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = loc;
    fareMarket.destination() = loc1;

    fareMarket.boardMultiCity() = loc->loc();
    fareMarket.offMultiCity() = loc1->loc();

    GlobalDirection globleDirection = GlobalDirection::AT;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "BA";

    // LON-BA-PAR   /CXR-BA/ #GI-XX#  .OUT.
    // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   2000.26

    // Create Thru PaxTypeFare
    Fare fare;
    fare.nucFareAmount() = 2000.26;

    FareInfo fareInfo;
    fareInfo._carrier = "BA";
    fareInfo._market1 = "LON";
    fareInfo._market2 = "PAR";
    fareInfo._fareClass = "WMLUQOW";
    fareInfo._fareAmount = 999.00;
    fareInfo._currency = "GBP";
    fareInfo._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo._ruleNumber = "5135";
    fareInfo._routingNumber = "XXXX";
    fareInfo._directionality = FROM;
    fareInfo._globalDirection = GlobalDirection::AT;
    fareInfo._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._tariffCat = 0;
    tariffRefInfo._ruleTariff = 23456;

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    PaxTypeFare paxTypeFare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);

    FareClassAppInfo appInfo;
    appInfo._fareType = "EU";
    appInfo._pricingCatType = 'S';
    appInfo._dowType = ' ';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setEconomyClass();

    // LON-BA-NYC    /CXR-BA/ #GI-XX#  .OUT.
    // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   1845.26

    // Create first PaxTypeFare
    Fare fare1;
    fare1.nucFareAmount() = 1845.26;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "LON";
    fareInfo1._market2 = "NYC";
    fareInfo1._fareClass = "WMLUQOW";
    fareInfo1._fareAmount = 999.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo1._ruleNumber = "5135";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::AT;
    fareInfo1._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._tariffCat = 1;
    tariffRefInfo1._ruleTariff = 12345;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket, &tariffRefInfo1);

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "ER";
    appInfo1._pricingCatType = 'S';
    appInfo1._dowType = ' ';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.cabin().setEconomyClass();

    // BA  AT A 5135   FMLUQOW  TAFPBA O I   2835.00 GBP FU   ADT   5236.55

    // Create second PaxTypeFare

    Fare fare2;
    fare2.nucFareAmount() = 5236.55;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "LON";
    fareInfo2._market2 = "NYC";
    fareInfo2._fareClass = "FMLUQOW";
    fareInfo2._fareAmount = 2835.00;
    fareInfo2._currency = "GBP";
    fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::AT;
    fareInfo2._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 23456;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket, &tariffRefInfo2);

    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "EU";
    appInfo2._pricingCatType = 'S';
    appInfo2._dowType = ' ';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setEconomyClass();

    // BA  AT A 2738   JUP1     TAFP   R O   7746.00 USD BR   ADT     7746.00
    //-------------------------------------------------------------------------

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = &paxType;
    paxTypeCortege.paxTypeFare().push_back(&paxTypeFare1);
    paxTypeCortege.paxTypeFare().push_back(&paxTypeFare2);
    fareMarket.paxTypeCortege().push_back(paxTypeCortege);

    fareMarket.governingCarrier() = "BA";

    trx.fareMarket().push_back(&fareMarket);

    Itin itin;
    itin.travelSeg().push_back(&airSeg);
    trx.itin().push_back(&itin);

    FareUsage fareUsage;
    fareUsage.paxTypeFare() = &paxTypeFare2;

    // Create MinFareAppl item
    MinFareAppl minFareAppl;
    minFareAppl.seqNo() = 11111;
    // Use Override Logic
    minFareAppl.applyDefaultLogic() = MinimumFare::NO;
    minFareAppl.nmlFareCompareInd() = MinimumFare::NO; // MinimumFare::YES;
    minFareAppl.nmlMpmBeforeRtgInd() = MinimumFare::YES;
    minFareAppl.nmlRtgBeforeMpmInd() = MinimumFare::NO;

    minFareAppl.nmlHipTariffCatInd() = -1;
    minFareAppl.nmlHipRestrCompInd() = 'S';
    minFareAppl.nmlHipUnrestrCompInd() = 'S';
    minFareAppl.nmlHipRbdCompInd() = MinimumFare::YES;
    minFareAppl.nmlHipStopCompInd() = MinimumFare::YES;

    minFareAppl.nmlCtmTariffCatInd() = -1;
    minFareAppl.nmlCtmRestrCompInd() = 'S';
    minFareAppl.nmlCtmUnrestrCompInd() = 'S';
    minFareAppl.nmlCtmRbdCompInd() = MinimumFare::YES;
    minFareAppl.nmlCtmStopCompInd() = MinimumFare::YES;

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back(&airSeg);
    travelSegs.push_back(&airSeg2);
    travelSegs.push_back(&airSeg3);

    std::vector<PricingUnit*> pricingUnits;

    MinFareSpecialFareSelectionDerived fareSel(CTM,
                                               MinFareFareSelection::ONE_WAY,
                                               MinFareFareSelection::OUTBOUND,
                                               paxTypeFare.fcaFareType(),
                                               trx,
                                               itin,
                                               travelSegs,
                                               pricingUnits,
                                               &paxType,
                                               DateTime::emptyDate(),
                                               0,
                                               &paxTypeFare,
                                               &minFareAppl);

    const PaxTypeFare* selectedFare = fareSel.selectFare();
    CPPUNIT_ASSERT(selectedFare != 0);

    std::string str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;
  }
  /////////////////////////////////////////////////////
  void testHigherFareType_NoMatchOnFareType()
  {
    // no match in any groupset, return false
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = -1;
    CPPUNIT_ASSERT(!fareSel->higherFareType("ABC", workingseg));
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 0);
  }
  void testHigherFareType_Match_Grp1_Set1()
  {
    // match on groupset1, setno 1
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = -1;
    CPPUNIT_ASSERT(fareSel->higherFareType("AAA", workingseg));
    CPPUNIT_ASSERT(workingseg == 1);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 3 && fareSel->_curFareTypeVec[0] == "BBB" &&
                   fareSel->_curFareTypeVec[1] == "CCC" && fareSel->_curFareTypeVec[2] == "DDD");
  }
  void testHigherFareType_Match_Grp1_Set1_Bump2()
  {
    // if call scond time, we should move from setno1 to setno2
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 1;
    CPPUNIT_ASSERT(fareSel->higherFareType("AAA", workingseg));
    CPPUNIT_ASSERT(workingseg == 2);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 4 && fareSel->_curFareTypeVec[0] == "EEE" &&
                   fareSel->_curFareTypeVec[1] == "FFF" && fareSel->_curFareTypeVec[2] == "GGG" &&
                   fareSel->_curFareTypeVec[3] == "HHH");
  }
  void testHigherFareType_Match_Grp1_Set1_Bump3()
  {
    // if call third time, should get setno3
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 2;
    CPPUNIT_ASSERT(fareSel->higherFareType("AAA", workingseg));
    CPPUNIT_ASSERT(workingseg == 3);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 4 && fareSel->_curFareTypeVec[0] == "III" &&
                   fareSel->_curFareTypeVec[1] == "JJJ" && fareSel->_curFareTypeVec[2] == "KKK" &&
                   fareSel->_curFareTypeVec[3] == "LLL");
  }
  void testHigherFareType_Match_Grp1_Set1_Bump4()
  {
    // forth call, get response from setno4
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 3;
    CPPUNIT_ASSERT(fareSel->higherFareType("AAA", workingseg));
    CPPUNIT_ASSERT(workingseg == 4);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 4 && fareSel->_curFareTypeVec[0] == "MMM" &&
                   fareSel->_curFareTypeVec[1] == "NNN" && fareSel->_curFareTypeVec[2] == "OOO" &&
                   fareSel->_curFareTypeVec[3] == "PPP");
  }
  void testHigherFareType_Match_Grp1_Set1_NoBump()
  {
    // fifth call, no setno5, groupset 2 is ignored
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 4;
    CPPUNIT_ASSERT(!fareSel->higherFareType("AAA", workingseg));
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 0);
  }
  void testHigherFareType_Match_Grp2_Set1()
  {
    // match on groupset2, setno 2
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = -1;
    CPPUNIT_ASSERT(fareSel->higherFareType("RRR", workingseg));
    CPPUNIT_ASSERT(workingseg == 2);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 1 && fareSel->_curFareTypeVec[0] == "RRR");
  }
  void testHigherFareType_Match_Grp2_Set1_Bump2()
  {
    // if call scond time, we should move from setno2 to setno3
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 2;
    CPPUNIT_ASSERT(fareSel->higherFareType("RRR", workingseg));
    CPPUNIT_ASSERT(workingseg == 3);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 1 && fareSel->_curFareTypeVec[0] == "SSS");
  }
  void testHigherFareType_Match_Grp2_Set1_Bump3()
  {
    // if call scond time, we should move from setno3 to setno4
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 3;
    CPPUNIT_ASSERT(fareSel->higherFareType("RRR", workingseg));
    CPPUNIT_ASSERT(workingseg == 4);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 1 && fareSel->_curFareTypeVec[0] == "TTT");
  }
  void testHigherFareType_Match_Grp2_Set1_NoBump()
  {
    // no more sets in this groupset
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 4;
    CPPUNIT_ASSERT(!fareSel->higherFareType("RRR", workingseg));
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 0);
  }
  void testHigherFareType_Match_Grp3_Set1()
  {
    // match on groupset3, setno 1
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = -1;
    CPPUNIT_ASSERT(fareSel->higherFareType("VVV", workingseg));
    CPPUNIT_ASSERT(workingseg == 1);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 3 && fareSel->_curFareTypeVec[0] == "UUU" &&
                   fareSel->_curFareTypeVec[1] == "VVV" && fareSel->_curFareTypeVec[2] == "WWW");
  }
  void testHigherFareType_Match_Grp3_Set1_Bump3()
  {
    // match on groupset3, no setno 2, go to setno3
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 1;
    CPPUNIT_ASSERT(fareSel->higherFareType("VVV", workingseg));
    CPPUNIT_ASSERT(workingseg == 3);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 2 && fareSel->_curFareTypeVec[0] == "XXX" &&
                   fareSel->_curFareTypeVec[1] == "YYY");
  }
  void testHigherFareType_Match_Grp3_Set1_Bump6()
  {
    // match on groupset3, no setno 4,5 , go to setno6
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 3;
    CPPUNIT_ASSERT(fareSel->higherFareType("VVV", workingseg));
    CPPUNIT_ASSERT(workingseg == 6);
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 1 && fareSel->_curFareTypeVec[0] == "ZZZ");
  }
  void testHigherFareType_Match_Grp3_set1_NoBump()
  {
    // no more sets in this groupset
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    int32_t workingseg = 6;
    CPPUNIT_ASSERT(!fareSel->higherFareType("RRR", workingseg));
    CPPUNIT_ASSERT(fareSel->_curFareTypeVec.size() == 0);
  }
  void testProcessHigherFareTypeNew_NoMatch()
  {
    // no match on fare type, return null
    MinFareSpecialFareSelectionDerived2* fareSel = selector("ABC");
    CPPUNIT_ASSERT(!fareSel->processHigherFareType(*_travelSegs));
  }
  void testProcessHigherFareTypeNew_MatchAAA_CurFTNotValid()
  {
    // match on AAA, but AAA is not valid
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 1, false);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(!ptf);
  }
  void testProcessHigherFareTypeNew_MatchAAA_CCC_CurFTNotValid()
  {
    // match on AAA, from table AAA, BBB, CCC, DDD. Only AAA CCC exists (CCC is valid)
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 10, false);
    addPaxTypeFareToMockMap(fareSel, "CCC", 15);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(15), ptf->nucFareAmount());
  }
  void testProcessHigherFareTypeNew_MatchAAA_CCC_DDD_CurFTNotValid()
  {
    // match on AAA, from table AAA, BBB, CCC, DDD. Only AAA CCC DDD exists
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 10, false);
    addPaxTypeFareToMockMap(fareSel, "CCC", 15);
    addPaxTypeFareToMockMap(fareSel, "DDD", 5);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(5), ptf->nucFareAmount());
  }
  void testProcessHigherFareTypeNew_MatchAAA_BBB_DDD_CurFTNotValid()
  {
    // match on AAA, from table AAA, BBB, CCC, DDD. Only AAA BBB DDD exists
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 10, false);
    addPaxTypeFareToMockMap(fareSel, "BBB", 1);
    addPaxTypeFareToMockMap(fareSel, "DDD", 50);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1), ptf->nucFareAmount());
  }

  void testProcessHigherFareTypeNew_MatchAAA_CurFTValid()
  {
    // match on AAA, but AAA is not valid
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 1);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(1), ptf->nucFareAmount());
  }
  void testProcessHigherFareTypeNew_MatchAAA_CCC_CurFTValid()
  {
    // match on AAA, from table AAA, BBB, CCC, DDD. Only AAA CCC exists
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 10);
    addPaxTypeFareToMockMap(fareSel, "CCC", 15);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(10), ptf->nucFareAmount());
  }
  void testProcessHigherFareTypeNew_MatchAAA_CCC_DDD_CurFTValid()
  {
    // match on AAA, from table AAA, BBB, CCC, DDD. Only AAA CCC DDD exists
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 10);
    addPaxTypeFareToMockMap(fareSel, "CCC", 15);
    addPaxTypeFareToMockMap(fareSel, "DDD", 5);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(10), ptf->nucFareAmount());
  }
  void testProcessHigherFareTypeNew_MatchAAA_BBB_DDD_CurFTValid()
  {
    // match on AAA, from table AAA, BBB, CCC, DDD. Only AAA BBB DDD exists
    MinFareSpecialFareSelectionDerived2* fareSel = selector("AAA");
    addPaxTypeFareToMockMap(fareSel, "AAA", 10);
    addPaxTypeFareToMockMap(fareSel, "BBB", 1);
    addPaxTypeFareToMockMap(fareSel, "DDD", 50);
    const PaxTypeFare* ptf = fareSel->processHigherFareType(*_travelSegs);
    CPPUNIT_ASSERT(ptf);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(10), ptf->nucFareAmount());
  }

  MinFareSpecialFareSelectionDerived2* selector(const char* faretype)
  {
    static FareType fareType = faretype;
    PaxTypeFare* paxTypeFarePtr = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/MinFares/test/data/MinFarePromFareSelection/paxTypeFare.xml");
    MinFareSpecialFareSelectionDerived2* fareSel =
        new MinFareSpecialFareSelectionDerived2(HIP,
                                                MinFareFareSelection::ONE_WAY,
                                                MinFareFareSelection::OUTBOUND,
                                                fareType,
                                                *_trx,
                                                *_itin,
                                                *_travelSegs,
                                                *_pricingUnits,
                                                _paxType,
                                                DateTime::emptyDate(),
                                                0,
                                                paxTypeFarePtr,
                                                _minFareAppl);
    _memHandle.insert(fareSel);
    return fareSel;
  }
  void addPaxTypeFareToMockMap(MinFareSpecialFareSelectionDerived2* fs,
                               FareType fareType,
                               double nucAmount,
                               bool fareValid = true)
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    ptf->setFare(fare);
    ptf->nucFareAmount() = nucAmount;
    fs->_fareMap[fareType] = ptf;
    if (!fareValid)
    {
      ptf->invalidateFare(PaxTypeFare::FD_Inclusion_Code);
    }
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  Itin* _itin;
  FareMarket* _fareMarket;
  PaxType* _paxType;
  PaxTypeFare* _paxTypeFare1;
  PaxTypeFare* _paxTypeFare2;
  AirSeg* _airSeg;
  std::vector<TravelSeg*>* _travelSegs;
  std::vector<PricingUnit*>* _pricingUnits;
  MinFareAppl* _minFareAppl;
  MinFareDefaultLogic* _defaultLogic;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MinFareSpecialFareSelectionTest);
} // namespace
