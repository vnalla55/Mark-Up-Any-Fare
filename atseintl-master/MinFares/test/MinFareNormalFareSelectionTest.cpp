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
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "Diagnostic/DCFactory.h"
#include "MinFares/MinFareChecker.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/test/MinFareDataHandleTest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{

class MinFareNormalFareSelectionDerived : public MinFareNormalFareSelection
{
public:
  MinFareNormalFareSelectionDerived(MinimumFareModule module,
                                    EligibleFare eligibleFare,
                                    FareDirectionChoice fareDirection,
                                    CabinType cabin,
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
    : MinFareNormalFareSelection(module,
                                 eligibleFare,
                                 fareDirection,
                                 cabin,
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

  virtual bool ruleValidated(const std::vector<uint16_t>& categories,
                             const PaxTypeFare& paxTypeFare,
                             bool puScope = false)
  {
    return true;
  }

  virtual const PaxTypeFare* selectFareForCabin(const std::vector<TravelSeg*>& travelSegs,
                                                const CabinType cabin,
                                                bool selectLowest)
  {
    if (selectLowest)
    {
      static const PaxTypeFare selectedFare;
      return &selectedFare;
    }
    else
    {
      return 0;
    }
  }
};

class MinFareNormalFareSelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinFareNormalFareSelectionTest);
  CPPUNIT_TEST(test);
  CPPUNIT_TEST(testNextCabin);
  CPPUNIT_TEST(testCTMAlternateFareSelect);
  CPPUNIT_TEST(test_getLowerCabin_UnKnownCabin);
  CPPUNIT_TEST(test_getLowerCabin_PremiumFirstCabin);
  CPPUNIT_TEST(test_getLowerCabin_FirstCabin);
  CPPUNIT_TEST(test_getLowerCabin_PremiumBusinessCabin);
  CPPUNIT_TEST(test_getLowerCabin_BusinessCabin);
  CPPUNIT_TEST(test_getLowerCabin_PremiumEconomyCabin);
  CPPUNIT_TEST(test_getLowerCabin_EconomyCabin);
  CPPUNIT_TEST(test_getLowerCabin_UnDefinedCabin);
  CPPUNIT_TEST(test_selectFareForLowerCabin_NoFareWhenPremiumFirst);
  CPPUNIT_TEST(test_selectFareForLowerCabin_NoFareWhenFirst);
  CPPUNIT_TEST(test_selectFareForLowerCabin_NoFareWhenPremiumBusiness);
  CPPUNIT_TEST(test_selectFareForLowerCabin_NoFareWhenBusiness);
  CPPUNIT_TEST(test_selectFareForLowerCabin_ReturnFareCabinIsFirst);
  CPPUNIT_TEST(test_selectFareForLowerCabin_ReturnFareCabinIsBusiness);
  CPPUNIT_TEST(test_selectFareForLowerCabin_ReturnFareCabinIsPremiumEconomy);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MinFareDataHandleTest>();
  }

  void tearDown() { _memHandle.clear(); }

  void test()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);

    PricingRequest request;
    trx.setRequest(&request);
    trx.diagnostic().diagnosticType() = Diagnostic710;
    trx.diagnostic().activate();

    // Create PaxType
    PaxType paxType;
    paxType.paxType() = "ADT";

    // Create one FareMarket
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");

    AirSeg airSeg;
    airSeg.segmentOrder() = 0;
    airSeg.geoTravelType() = GeoTravelType::International;
    airSeg.origin() = loc;
    airSeg.destination() = loc1;
    airSeg.boardMultiCity() = loc->loc();
    airSeg.offMultiCity() = loc1->loc();

    airSeg.carrier() = "BA";
    trx.travelSeg().push_back(&airSeg);

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = loc;
    fareMarket.destination() = loc1;

    fareMarket.boardMultiCity() = loc->loc();
    fareMarket.offMultiCity() = loc1->loc();

    GlobalDirection globleDirection = GlobalDirection::AT;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "BA";

    //-------------------------------------------------------------------------

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
    fareInfo._globalDirection = AT;
    fareInfo._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._tariffCat = 0;
    tariffRefInfo._ruleTariff = 23456;

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    PaxTypeFare paxTypeFare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);

    FareClassAppInfo appInfo;
    appInfo._fareType = "EU";
    appInfo._pricingCatType = 'N';
    appInfo._dowType = ' ';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setEconomyClass();

    //-------------------------------------------------------------------------

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
    fareInfo1._globalDirection = AT;
    fareInfo1._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._tariffCat = 1;
    tariffRefInfo1._ruleTariff = 12345;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket, &tariffRefInfo1);

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "ER";
    appInfo1._pricingCatType = 'N';
    appInfo1._dowType = ' ';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.cabin().setBusinessClass();

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
    fareInfo2._globalDirection = AT;
    fareInfo2._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 23456;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket, &tariffRefInfo2);
    // fare2.setIsMileage();

    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "EU";
    appInfo2._pricingCatType = 'N';
    appInfo2._dowType = ' ';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setBusinessClass();

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

    CabinType cabin;
    cabin.setEconomyClass();

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

    std::vector<PricingUnit*> pricingUnits;

    MinFareNormalFareSelectionDerived normalfareSel(HIP,
                                                    MinFareFareSelection::ONE_WAY,
                                                    MinFareFareSelection::OUTBOUND,
                                                    cabin,
                                                    trx,
                                                    itin,
                                                    travelSegs,
                                                    pricingUnits,
                                                    &paxType,
                                                    DateTime::emptyDate(),
                                                    0,
                                                    &paxTypeFare,
                                                    &minFareAppl);

    const PaxTypeFare* selectedFare = normalfareSel.selectFare();

    // std::string str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;
    CPPUNIT_ASSERT(selectedFare != 0);
  }

  void testNextCabin()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);

    trx.diagnostic().diagnosticType() = Diagnostic710;
    trx.diagnostic().activate();
    _request.ticketingDT() = DateTime(2009, 05, 20);
    trx.setRequest(&_request);

    // Create PaxType
    PaxType paxType;
    paxType.paxType() = "ADT";

    // Create one FareMarket
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");

    AirSeg airSeg;
    airSeg.segmentOrder() = 0;
    airSeg.geoTravelType() = GeoTravelType::International;
    airSeg.origin() = loc;
    airSeg.destination() = loc1;
    airSeg.boardMultiCity() = loc->loc();
    airSeg.offMultiCity() = loc1->loc();
    airSeg.carrier() = "BA";
    trx.travelSeg().push_back(&airSeg);

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&airSeg);

    fareMarket.origin() = loc;
    fareMarket.destination() = loc1;

    fareMarket.boardMultiCity() = loc->loc();
    fareMarket.offMultiCity() = loc1->loc();

    GlobalDirection globleDirection = GlobalDirection::AT;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "BA";

    //-------------------------------------------------------------------------

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
    appInfo._pricingCatType = 'N';
    appInfo._dowType = ' ';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setEconomyClass();

    //-------------------------------------------------------------------------

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
    fareInfo1._owrt = ONE_WAY_MAY_BE_DOUBLED;
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
    appInfo1._fareType = "BU";
    appInfo1._pricingCatType = 'N';
    appInfo1._dowType = ' ';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.cabin().setPremiumEconomyClass();

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
    fareInfo2._owrt = ONE_WAY_MAY_BE_DOUBLED;
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
    appInfo2._fareType = "BR";
    appInfo2._pricingCatType = 'N';
    appInfo2._dowType = ' ';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setPremiumEconomyClass();

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

    CabinType cabin;
    cabin.setEconomyClass();

    // Create MinFareAppl item
    MinFareAppl minFareAppl;
    minFareAppl.seqNo() = 11111;
    // Use default Logic
    minFareAppl.applyDefaultLogic() = MinimumFare::YES;
    MinFareDefaultLogic defaultLogic;
    defaultLogic.seqNo() = 22222;
    defaultLogic.nmlFareCompareInd() = MinimumFare::YES;
    defaultLogic.nmlMpmBeforeRtgInd() = MinimumFare::YES;
    defaultLogic.nmlRtgBeforeMpmInd() = MinimumFare::NO;

    defaultLogic.nmlHipTariffCatInd() = -1;
    defaultLogic.nmlHipRestrCompInd() = 'S';
    defaultLogic.nmlHipUnrestrCompInd() = 'S';
    defaultLogic.nmlHipRbdCompInd() = MinimumFare::YES;
    defaultLogic.nmlHipStopCompInd() = MinimumFare::YES;

    defaultLogic.nmlCtmTariffCatInd() = -1;
    defaultLogic.nmlCtmRestrCompInd() = 'S';
    defaultLogic.nmlCtmUnrestrCompInd() = 'S';
    defaultLogic.nmlCtmRbdCompInd() = MinimumFare::YES;
    defaultLogic.nmlCtmStopCompInd() = MinimumFare::YES;

    // Test CTM Fare Sel with diff constructor

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back(&airSeg);
    std::vector<PricingUnit*> pricingUnits;

    MinFareNormalFareSelection normalfareSel1(CTM,
                                              MinFareFareSelection::HALF_ROUND_TRIP,
                                              MinFareFareSelection::OUTBOUND,
                                              cabin,
                                              trx,
                                              itin,
                                              travelSegs,
                                              pricingUnits,
                                              &paxType,
                                              DateTime::emptyDate(),
                                              0,
                                              &paxTypeFare,
                                              &minFareAppl,
                                              &defaultLogic);
    // Test functions
    normalfareSel1._govCxrNonValidatedFares.insert(&paxTypeFare1);
    normalfareSel1._govCxrNonValidatedFares.insert(&paxTypeFare2);
    CPPUNIT_ASSERT(normalfareSel1.selectNonValidatedFare(normalfareSel1._govCxrNonValidatedFares) !=
                   0);

    CPPUNIT_ASSERT(normalfareSel1.selectValidatedFare(
                       normalfareSel1._govCxrNonValidatedFares, false, false) != 0);
  }

  void testCTMAlternateFareSelect()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);

    trx.diagnostic().diagnosticType() = Diagnostic710;
    trx.diagnostic().activate();

    _request.ticketingDT() = DateTime(2009, 05, 20);
    trx.setRequest(&_request);

    PaxType paxType;
    paxType.paxType() = "ADT";

    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");

    // Travel segment is LON-NYC, NYC-DFW, DFW-NYC.
    // Alternate fare market retrival should get LON-NYC fare
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
    appInfo._pricingCatType = 'N';
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
    appInfo1._pricingCatType = 'N';
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
    appInfo2._pricingCatType = 'N';
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

    CabinType cabin;
    cabin.setEconomyClass();

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

    MinFareNormalFareSelection normalfareSel(CTM,
                                             MinFareFareSelection::ONE_WAY,
                                             MinFareFareSelection::OUTBOUND,
                                             cabin,
                                             trx,
                                             itin,
                                             travelSegs,
                                             pricingUnits,
                                             &paxType,
                                             DateTime::emptyDate(),
                                             0,
                                             &paxTypeFare,
                                             &minFareAppl);

    const PaxTypeFare* selectedFare = normalfareSel.selectFare();
    // std::string str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;

    CPPUNIT_ASSERT(selectedFare != 0);
  }

  void buildFareInfo(FareInfo& fareInfo)
  {
    // LON-BA-PAR   /CXR-BA/ #GI-XX#  .OUT.
    // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   2000.26
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
  }

  void buildTariffCrossRefInfo(TariffCrossRefInfo& tariffRefInfo)
  {
    tariffRefInfo._tariffCat = 0;
    tariffRefInfo._ruleTariff = 23456;
  }

  void enableDiagnostics710(PricingTrx& trx)
  {
    trx.diagnostic().diagnosticType() = Diagnostic710;
    trx.diagnostic().activate();
  }

  void buildFareMarket(FareMarket& fareMarket, GlobalDirection& globleDirection)
  {
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "BA";
  }

  void createPaxTypeFare(PaxTypeFare& paxTypeFare, PaxType& paxType)
  {
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    GlobalDirection globleDirection = GlobalDirection::AT;

    // Initialize
    buildFareInfo(*fareInfo);
    buildTariffCrossRefInfo(*tariffRefInfo);
    buildFareMarket(*fareMarket, globleDirection);

    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffRefInfo);
    paxTypeFare.initialize(fare, &paxType, fareMarket);
  }

  void createNormalFareSel()
  {
    createPaxTypeFare(paxTypeFareA, paxTypeA);
    _request.ticketingDT() = DateTime(2009, 05, 30);
    trxA.setRequest(&_request);
    _normalfareSel = _memHandle.insert<MinFareNormalFareSelectionDerived>(
        new MinFareNormalFareSelectionDerived(CTM,
                                              MinFareFareSelection::ONE_WAY,
                                              MinFareFareSelection::OUTBOUND,
                                              cabinA,
                                              trxA,
                                              itinA,
                                              travelSegsA,
                                              pricingUnitsA,
                                              &paxTypeA,
                                              DateTime::emptyDate(),
                                              0,
                                              &paxTypeFareA,
                                              &minFareApplA));
  }

  void createNormalFareSel_COM()
  {
    createPaxTypeFare(paxTypeFareA, paxTypeA);
    _request.ticketingDT() = DateTime(2009, 05, 30);
    trxA.setRequest(&_request);
    _normalfareSel = _memHandle.insert<MinFareNormalFareSelectionDerived>(
        new MinFareNormalFareSelectionDerived(COM,
                                              MinFareFareSelection::ONE_WAY,
                                              MinFareFareSelection::OUTBOUND,
                                              cabinA,
                                              trxA,
                                              itinA,
                                              travelSegsA,
                                              pricingUnitsA,
                                              &paxTypeA,
                                              DateTime::emptyDate(),
                                              0,
                                              &paxTypeFareA,
                                              &minFareApplA));
  }

  void test_getLowerCabin_UnKnownCabin()
  {
    cabinA.setUnknownClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(!_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isUnknownClass());
  }

  void test_getLowerCabin_PremiumFirstCabin()
  {
    cabinA.setPremiumFirstClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isFirstClass());
  }

  void test_getLowerCabin_FirstCabin()
  {
    cabinA.setFirstClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isPremiumBusinessClass());
  }

  void test_getLowerCabin_PremiumBusinessCabin()
  {
    cabinA.setPremiumBusinessClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isBusinessClass());
  }

  void test_getLowerCabin_BusinessCabin()
  {
    cabinA.setBusinessClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isPremiumEconomyClass());
  }

  void test_getLowerCabin_PremiumEconomyCabin()
  {
    cabinA.setPremiumEconomyClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isEconomyClass());
  }

  void test_getLowerCabin_EconomyCabin()
  {
    cabinA.setEconomyClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(!_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isEconomyClass());
  }

  void test_getLowerCabin_UnDefinedCabin()
  {
    cabinA.setUndefinedClass();
    createNormalFareSel();
    CabinType modifiedCabin = cabinA;

    CPPUNIT_ASSERT(!_normalfareSel->getLowerCabin(modifiedCabin));
    CPPUNIT_ASSERT(modifiedCabin.isUndefinedClass());
  }

  void test_selectFareForLowerCabin_NoFareWhenPremiumFirst()
  {
    cabinA.setPremiumFirstClass();

    const PaxTypeFare* selectedFare = 0;
    createNormalFareSel();
    selectedFare = _normalfareSel->selectFareForLowerCabin(travelSegsA);
    CPPUNIT_ASSERT(selectedFare == 0);
  }

  void test_selectFareForLowerCabin_NoFareWhenFirst()
  {
    cabinA.setFirstClass();

    const PaxTypeFare* selectedFare = 0;
    createNormalFareSel();
    selectedFare = _normalfareSel->selectFareForLowerCabin(travelSegsA);
    CPPUNIT_ASSERT(selectedFare == 0);
  }

  void test_selectFareForLowerCabin_NoFareWhenPremiumBusiness()
  {
    cabinA.setPremiumBusinessClass();

    createNormalFareSel();
    CPPUNIT_ASSERT(!_normalfareSel->selectFareForLowerCabin(travelSegsA));
  }

  void test_selectFareForLowerCabin_NoFareWhenBusiness()
  {
    cabinA.setBusinessClass();

    createNormalFareSel();
    CPPUNIT_ASSERT(!_normalfareSel->selectFareForLowerCabin(travelSegsA));
  }

  void test_selectFareForLowerCabin_ReturnFareCabinIsFirst()
  {
    cabinA.setFirstClass();

    createNormalFareSel_COM();
    CPPUNIT_ASSERT(_normalfareSel->selectFareForLowerCabin(travelSegsA));
  }

  void test_selectFareForLowerCabin_ReturnFareCabinIsBusiness()
  {
    cabinA.setBusinessClass();

    createNormalFareSel_COM();
    CPPUNIT_ASSERT(_normalfareSel->selectFareForLowerCabin(travelSegsA));
  }

  void test_selectFareForLowerCabin_ReturnFareCabinIsPremiumEconomy()
  {
    cabinA.setPremiumEconomyClass();

    createNormalFareSel_COM();
    CPPUNIT_ASSERT(_normalfareSel->selectFareForLowerCabin(travelSegsA));
  }

protected:
  CabinType cabinA;
  Itin itinA;
  std::vector<TravelSeg*> travelSegsA;
  MinFareAppl minFareApplA;
  PricingTrx trxA;
  std::vector<PricingUnit*> pricingUnitsA;
  PaxType paxTypeA;
  PaxTypeFare paxTypeFareA;
  MinFareNormalFareSelection* _normalfareSel;

  TestMemHandle _memHandle;
  PricingRequest _request;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MinFareNormalFareSelectionTest);

} // tse
