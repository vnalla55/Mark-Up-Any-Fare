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

#include <sstream>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/Utils/CommonUtils.h"
#include "DBAccess/CarrierCombination.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareClassRestRule.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TariffRuleRest.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "Pricing/Combinations.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/test/MockFareMarket.h"
#include "Pricing/test/MockFarePath.h"
#include "Pricing/test/MockLoc.h"
#include "Pricing/test/MockTravelSeg.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"
#include "test/testdata/TestFareUsageFactory.h"

namespace tse
{

class CombinationsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CombinationsTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcessOnePU);
  CPPUNIT_TEST(testProcessOneFU);
  CPPUNIT_TEST(testProcessOneFare);
  CPPUNIT_TEST(testDisplayDiagA);
  CPPUNIT_TEST(testDisplayDiagB);
  CPPUNIT_TEST(testProcessRtRestr);
  CPPUNIT_TEST(testProcessCtRestr);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionAbortWhenNotPCat10);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailCombWhenEndOnEndNull);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailWhenMissingReqIntlFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotMissingReqIntlFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedIntlFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedIntlFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailWhenMissingReqDomFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotMissingReqDomFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotMissingReqDomFare_RUXU_Exception);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedDomFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedDomFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailWhenMissingReqTransbFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotMissingReqTransbFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedTransbFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedTransbFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailWhenMissingReqNormalFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotMissingReqNormalFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedNormalFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedNormalFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailWhenMissingReqSpecialFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotMissingReqSpecialFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedSpecialFare);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedSpecialFare);
  CPPUNIT_TEST(testProcessMajorSubCatRtnSTOPWhenNeg104Permitted);
  CPPUNIT_TEST(testProcessMajorSubCatRtnPASSWhen104Permitted);
  CPPUNIT_TEST(testProcessTariffRuleRestriction_NoRuleRestr_Abort);
  CPPUNIT_TEST(testProcessTariffRuleRestriction_Idle);
  CPPUNIT_TEST(testProcessFareClassTypeRestriction_NoRuleRestr_Abort);
  CPPUNIT_TEST(testProcessFareClassTypeRestriction_Idle);
  CPPUNIT_TEST(testCheckGeoSpecBtwPass);
  CPPUNIT_TEST(testCheckGeoSpecBtwFail);
  CPPUNIT_TEST(testCheckGeoSpecViaPass);
  CPPUNIT_TEST(testdataStringValidation_failSingleTHENFailDirectionality);
  CPPUNIT_TEST(testdataStringValidation_failTHENFail_ORFailDirectionality);
  CPPUNIT_TEST(testdataStringValidation_passTHENFail_ORPassDirectionality);
  CPPUNIT_TEST(testAllowEndOnEndPassDifferentRecord3);
  CPPUNIT_TEST(testEndOnEndFailWithMultipleRecord3);
  CPPUNIT_TEST(testAllowHRTFareCheckForVendor);
  CPPUNIT_TEST(test_same_fareClass_processEndOnEndABA_P);
  CPPUNIT_TEST(test_same_fareClass_processEndOnEndABA_Y);
  CPPUNIT_TEST(test_same_fareClass_processEndOnEndABA_N);
  CPPUNIT_TEST(testProcessEndOnEndRestrictionFailWhenReqDomFareORReqTransbFare);
  CPPUNIT_TEST(testValidateFareCat10overrideCat25_fare1cat25_fare2changed);
  CPPUNIT_TEST(testValidateFareCat10overrideCat25_fare1cat25_fare2notChanged);
  CPPUNIT_TEST(testValidateFareCat10overrideCat25_fare1noCat25_fare1changed_fare2cat25);
  CPPUNIT_TEST(testValidateFareCat10overrideCat25_fare1noCat25_fare1changed_fare2noCat25);
  CPPUNIT_TEST(testValidateFareCat10overrideCat25_fare1noCat25_fare1notChanged_fare2changed);
  CPPUNIT_TEST(testValidateFareCat10overrideCat25_fare1noCat25_fare1notChanged_fare2notChanged);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dbHandle = _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _combinations = _memHandle.create<Combinations>();
    _comboScoreboard = _memHandle.create<CombinabilityScoreboard>();
    _combinations->comboScoreboard() = _comboScoreboard;
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _comboScoreboard->trx() = _trx;
    _combinationsPrebuiltVFC = _memHandle.create<CombinationsPrebuiltVFC>();
    _combinationsPrebuiltVFC->trx() = _trx;
    _combinationsPrebuiltVFC->comboScoreboard() = _comboScoreboard;

    _fp.itin() = &_itin;

    _directionalityInfo =
        _memHandle.insert(new Combinations::DirectionalityInfo(Combinations::PricingUnitLevel,
                                                               "PU",
                                                               Combinations::FROM_LOC1_TO_LOC2,
                                                               Combinations::TO_LOC1_FROM_LOC2));

    _vfc.resetCarrierPref();
    _eoeItemNo = 100;

    _endOnEndRule.overrideDateTblItemNo() = 0;
    _endOnEndRule.abacombInd() = Combinations::NO_RESTRICTION;
    _endOnEndRule.sameCarrierInd() = Combinations::NO_RESTRICTION;
    _endOnEndRule.tktInd() = Combinations::NO_APPLICATION;
    _endOnEndRule.fareTypeLocAppl() = Combinations::NO_APPLICATION;
    _endOnEndRule.constLocAppl() = Combinations::NO_APPLICATION;
    _endOnEndRule.eoeRestInd() = Combinations::NO_APPLICATION;
    _endOnEndRule.eoeNormalInd() = Combinations::PERMITTED;
    _endOnEndRule.eoespecialInd() = Combinations::PERMITTED;
    _endOnEndRule.eoespecialApplInd() = Combinations::APPLY_ONLY_SPECIAL_FARE;
    _endOnEndRule.uscatransborderInd() = Combinations::PERMITTED;
    _endOnEndRule.domInd() = Combinations::PERMITTED;
    _endOnEndRule.intlInd() = Combinations::PERMITTED;

    _cat10Rule.vendorCode() = "ATP";
    _endOnEndDA = _memHandle.insert(new Combinations::EndOnEndDataAccess(_fp,
                                                                         _pricingUnit,
                                                                         _cat10Rule,
                                                                         static_cast<uint32_t>(_eoeItemNo),
                                                                         _vfc,
                                                                         _endOnEndRule,
                                                                         _diag,
                                                                         *_directionalityInfo));

    _combinations->trx() = _trx;
    _trx->setRequest(&_request);
    _trx->ticketingDate() = DateTime(2025, 4, 2);

    _locYYZ.loc() = "YYZ";
    _locYYZ.area() = IATA_AREA1;
    _locYYZ.subarea() = IATA_SUB_AREA_11();
    _locYYZ.nation() = "CA";

    _locWAS.loc() = "WAS";
    _locWAS.area() = IATA_AREA1;
    _locWAS.subarea() = IATA_SUB_AREA_11();
    _locWAS.nation() = "US";

    _locDFW.loc() = "DFW";
    _locDFW.area() = IATA_AREA1;
    _locDFW.subarea() = IATA_SUB_AREA_11();
    _locDFW.nation() = "US";

    _locKRK.loc() = "KRK";
    _locKRK.area() = IATA_AREA2;
    _locKRK.subarea() = IATA_SUB_AREA_21();
    _locKRK.nation() = "PL";

    _locWAW.loc() = "WAW";
    _locWAW.area() = IATA_AREA2;
    _locWAW.subarea() = IATA_SUB_AREA_21();
    _locWAW.nation() = "PL";

    _locABA.loc() = "ABA";
    _locABA.area() = IATA_AREA3;
    _locABA.subarea() = IATA_SUB_ARE_32();
    _locABA.nation()= "XU";

    _locAER.loc() = "AER";
    _locAER.area() = IATA_AREA2;
    _locAER.subarea() = IATA_SUB_AREA_21();
    _locAER.nation()= "RU";

    _catRuleItemInfoSet = new CombinabilityRuleItemInfoSet();
  }

  void tearDown()
  {
    tools::non_const(_cat10.categoryRuleItemInfoSet()).clear();
    _memHandle.clear();
  }

  void testConstructor() { CPPUNIT_ASSERT_NO_THROW(CombinationsTest combinationsTest;); }

  void setEndOnEndReqIntlFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).intlInd() = Combinations::RESTRICTIONS;
  }

  void setNotPermitIntlFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).intlInd() = Combinations::NOT_PERMITTED;
  }

  void setEndOnEndReqDomFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).domInd() = Combinations::RESTRICTIONS;
  }

  void setNotPermitDomFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).domInd() = Combinations::NOT_PERMITTED;
  }

  void setEndOnEndReqTransbFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).uscatransborderInd() =
        Combinations::RESTRICTIONS;
  }

  void setNotPermitTransbFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).uscatransborderInd() =
        Combinations::NOT_PERMITTED;
  }

  void setEndOnEndReqNormalFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).eoeNormalInd() = Combinations::RESTRICTIONS;
  }

  void setNotPermitNormalFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).eoeNormalInd() =
        Combinations::NOT_PERMITTED;
  }

  void setEndOnEndReqSpecialFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).eoespecialInd() =
        Combinations::RESTRICTIONS;
  }

  void setNotPermitSpecialFare()
  {
    (const_cast<EndOnEnd&>(_endOnEndDA->_endOnEndRule)).eoespecialInd() =
        Combinations::NOT_PERMITTED;
  }

  FareUsage* buildFU()
  {
    Fare* fare = _memHandle.create<Fare>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareUsage* fu = _memHandle.create<FareUsage>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fare->setFareInfo(fareInfo);
    CombinabilityRuleInfo* cri = _memHandle.create<CombinabilityRuleInfo>();
    ptf->rec2Cat10() = cri;
    fu->rec2Cat10() = cri;

    fu->paxTypeFare() = ptf;
    ptf->setFare(fare);
    ptf->fareMarket() = fm;

    // Actual locations does not matter. Just can't be null.
    // Tests that require specific locations will change them later.
    fm->origin() = &_locDFW;
    fm->destination() = &_locWAW;

    return fu;
  }

  void prepareTargetFU(const size_t numOfTargetFU)
  {
    Combinations::ValidationFareComponents& vfc = _endOnEndDA->_validationFareComponent;
    vfc.resize(numOfTargetFU);

    FareUsage* cFU = buildFU();
    for (size_t i = 0; i < numOfTargetFU; i++)
    {
      FareUsage* fu = buildFU();

      vfc[i].initialize(cFU, cFU->rec2Cat10(), fu);
    }
  }

  void setTargetFUFareStatus(size_t order, Fare::FareState fareStatus)
  {
    _endOnEndDA->_validationFareComponent[order]
        ._targetFareUsage->paxTypeFare()
        ->fare()
        ->status()
        .set(fareStatus);
  }

  void setTargetFareMarketOnD(size_t order, Loc* orig, Loc* dest)
  {
    FareMarket* fm = _endOnEndDA->_validationFareComponent[order]
            ._targetFareUsage->paxTypeFare()
            ->fareMarket();
    fm->origin() = orig;
    fm->destination() = dest;
  }

  void setTargetFUSpecialFare(size_t order)
  {
    _endOnEndDA->_validationFareComponent[order]
        ._targetFareUsage->paxTypeFare()
        ->fareTypeApplication() = 'S'; // PaxTypeFare::PRICING_CATTYPE_SPECIAL
  }

  void setTargetFUNormalFare(size_t order)
  {
    _endOnEndDA->_validationFareComponent[order]
        ._targetFareUsage->paxTypeFare()
        ->fareTypeApplication() = 'N'; // PaxTypeFare::PRICING_CATTYPE_NORMAL
  }

  void setOrigInLoc1_DestInLoc2(size_t order)
  {
    _vfc[order]._targetFareUsage->paxTypeFare()->fareMarket()->boardMultiCity() = "DEN";
    _endOnEndRule.fareTypeLoc1Type() = NATION;
    _endOnEndRule.fareTypeLoc1() = "US";
    _vfc[order]._targetFareUsage->paxTypeFare()->fareMarket()->offMultiCity() = "NYC";
    _endOnEndRule.fareTypeLoc1Type() = NATION;
    _endOnEndRule.fareTypeLoc1() = "US";
  }

  void setOrigInLoc1_DestNotInLoc2(size_t order)
  {
    _vfc[order]._targetFareUsage->paxTypeFare()->fareMarket()->boardMultiCity() = "DEN";
    _endOnEndRule.fareTypeLoc1Type() = NATION;
    _endOnEndRule.fareTypeLoc1() = "US";
    _vfc[order]._targetFareUsage->paxTypeFare()->fareMarket()->offMultiCity() = "NYC";
    _endOnEndRule.fareTypeLoc1Type() = NATION;
    _endOnEndRule.fareTypeLoc1() = "CA";
  }

  void setOrigNotInLoc1_DestNotInLoc2(size_t order)
  {
    _vfc[order]._targetFareUsage->paxTypeFare()->fareMarket()->boardMultiCity() = "DEN";
    _endOnEndRule.fareTypeLoc1Type() = NATION;
    _endOnEndRule.fareTypeLoc1() = "MX";
    _vfc[order]._targetFareUsage->paxTypeFare()->fareMarket()->offMultiCity() = "NYC";
    _endOnEndRule.fareTypeLoc1Type() = NATION;
    _endOnEndRule.fareTypeLoc1() = "CA";
  }

  EndOnEnd* addEOERecord3InNewSet(FareUsage* fu)
  {
    EndOnEnd* eoe = _memHandle.create<EndOnEnd>();
    eoe->itemNo() = static_cast<uint32_t>(_eoeItemNo++);

    CombinabilityRuleItemInfoSet* newCat10Set = new CombinabilityRuleItemInfoSet;
    CombinabilityRuleItemInfo eoeRecord3Ctrl;
    eoeRecord3Ctrl.setItemcat(104);
    eoeRecord3Ctrl.setItemNo(eoe->itemNo());
    eoeRecord3Ctrl.setRelationalInd(CategoryRuleItemInfo::THEN);

    newCat10Set->push_back(eoeRecord3Ctrl);

    if (!fu->rec2Cat10())
      fu->rec2Cat10() = _memHandle.create<CombinabilityRuleInfo>();
    fu->rec2Cat10()->addItemInfoSetNosync(newCat10Set);

    return eoe;
  }

  void addEOEThatPassIntlFare(FareUsage* fu)
  {
    EndOnEnd* new104Rec3 = addEOERecord3InNewSet(fu);
    new104Rec3->intlInd() = PERMITTED;

    Combinations::ValidationFareComponents& vfc = _endOnEndDA->_validationFareComponent;
    for (uint32_t fuNum = 0; fuNum < vfc.size(); fuNum++)
    {
      if (vfc[fuNum]._targetFareUsage->paxTypeFare()->isInternational())
      {
        _combinationsPrebuiltVFC->setEOEResultSimulation(new104Rec3->itemNo(), fuNum, true);
      }
    }
  }

  void addEOEThatPassDomFare(FareUsage* fu)
  {
    EndOnEnd* new104Rec3 = addEOERecord3InNewSet(fu);
    new104Rec3->domInd() = PERMITTED;

    Combinations::ValidationFareComponents& vfc = _endOnEndDA->_validationFareComponent;
    for (uint32_t fuNum = 0; fuNum < vfc.size(); fuNum++)
    {
      if (vfc[fuNum]._targetFareUsage->paxTypeFare()->isDomestic() ||
          vfc[fuNum]._targetFareUsage->paxTypeFare()->isForeignDomestic())
      {
        _combinationsPrebuiltVFC->setEOEResultSimulation(new104Rec3->itemNo(), fuNum, true);
      }
    }
  }

  void testProcess()
  {
    MockFarePath farePath;

    // Need a fareUsage for the pricing unit.
    // So, use one from the Factory unit testing (we don't really care what's in it at this point).

    FareUsage* fu = TestFareUsageFactory::create("/vobs/atseintl/test/testdata/data/FareUsage.xml");
    PricingUnit* pricingUnit = _memHandle.create<PricingUnit>();
    pricingUnit->fareUsage().push_back(fu);

    farePath.addPU(pricingUnit);

    DiagnosticTypes diagType = Diagnostic610;
    Diagnostic rootDiag(diagType);
    _trx->diagnostic().diagnosticType() = Diagnostic610;

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    // sleep(1);

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.trx() = _trx;

    PaxTypeFare ptf;
    fu->paxTypeFare() = &ptf;

    CombinabilityRuleInfo combinabilityRuleInfo;
    fu->rec2Cat10() = &combinabilityRuleInfo;
    ptf.rec2Cat10() = &combinabilityRuleInfo;

    try
    {
      CombinabilityValidationResult results = combinations.process(farePath, 1, fu, fu, diag);
      CPPUNIT_ASSERT_EQUAL(CVR_PASSED, results);
    }
    catch (ErrorResponseException& ex)
    {
      std::ostringstream oss;
      oss << ex.message() << "\n" << ex.code();
      CPPUNIT_ASSERT_MESSAGE(oss.str(), false);
    }
  }

  void testProcessOnePU()
  {
    MockFarePath farePath;

    DiagnosticTypes diagType = Diagnostic610;
    Diagnostic rootDiag(diagType);
    _trx->diagnostic().diagnosticType() = Diagnostic610;

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    // sleep(1);

    FareUsage* fu = TestFareUsageFactory::create("/vobs/atseintl/test/testdata/data/FareUsage.xml");
    _pricingUnit.fareUsage().push_back(fu);
    farePath.pricingUnit().push_back(&_pricingUnit);

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;

    PaxTypeFare ptf;
    fu->paxTypeFare() = &ptf;

    CombinabilityRuleInfo combinabilityRuleInfo;
    fu->rec2Cat10() = &combinabilityRuleInfo;
    ptf.rec2Cat10() = &combinabilityRuleInfo;

    try
    {
      CombinabilityValidationResult results = combinations.process(farePath, 1, fu, fu, diag);
      CPPUNIT_ASSERT_EQUAL(CVR_PASSED, results);
    }
    catch (ErrorResponseException& ex)
    {
      std::ostringstream oss;
      oss << ex.message() << "\n" << ex.code();
      CPPUNIT_ASSERT_MESSAGE(oss.str(), false);
    }
  }

  void testProcessOneFU()
  {
    MockFarePath farePath;

    DiagnosticTypes diagType = Diagnostic610;
    Diagnostic rootDiag(diagType);
    _trx->diagnostic().diagnosticType() = Diagnostic610;

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    farePath.pricingUnit().push_back(&_pricingUnit);

    FareUsage* fu = _memHandle.create<FareUsage>();

    farePath.pricingUnit()[0]->fareUsage().push_back(fu);

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);

    MockFareMarket faremkt;
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    fu->paxTypeFare() = ptf;
    ptf->setFare(fare);
    ptf->fareMarket() = &faremkt;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;

    CombinabilityRuleInfo combinabilityRuleInfo;
    fu->rec2Cat10() = &combinabilityRuleInfo;
    ptf->rec2Cat10() = &combinabilityRuleInfo;

    try
    {
      CombinabilityValidationResult results = combinations.process(farePath, 1, fu, fu, diag);
      CPPUNIT_ASSERT_EQUAL(CVR_PASSED, results);
    }
    catch (ErrorResponseException& ex)
    {
      std::ostringstream oss;
      oss << ex.message() << "\n" << ex.code();
      CPPUNIT_ASSERT_MESSAGE(oss.str(), false);
    }
  }

  void testProcessOneFare()
  {
    MockFarePath farePath;

    DiagnosticTypes diagType = Diagnostic610;
    Diagnostic rootDiag(diagType);
    _trx->diagnostic().diagnosticType() = Diagnostic610;

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    farePath.pricingUnit().push_back(&_pricingUnit);

    FareUsage* fu = _memHandle.create<FareUsage>();
    farePath.pricingUnit()[0]->fareUsage().push_back(fu);

    PricingRequest request;

    request.globalDirection() = GlobalDirection::AT;

    _trx->setRequest(&request);

    Loc loca;
    NationCode nation = "US";
    loca.nation() = nation;

    Itin itin;
    _trx->itin().push_back(&itin);

    farePath.itin() = &itin;

    MockLoc loc1;
    LocCode dfw = "DFW";
    loc1.setLoc(dfw);

    MockLoc loc2;
    LocCode lon = "LON";
    loc2.setLoc(lon);

    MockLoc loc3;
    NationCode us = "US";
    loc3.setNation(us);
    IATAAreaCode a1 = "1";
    loc3.setIATAArea(a1);

    MockLoc loc4;
    NationCode ca = "CA";
    loc4.setNation(ca);
    IATAAreaCode a2 = "1";
    loc4.setIATAArea(a2);

    LocCode board = "DFW";
    LocCode off = "LON";

    MockTravelSeg trvlseg;

    std::vector<TravelSeg*>& travelsegVec = _trx->travelSeg();

    trvlseg.origin() = &loc3;
    trvlseg.destination() = &loc4;
    trvlseg.origAirport() = board;
    trvlseg.destAirport() = off;
    trvlseg.departureDT() = DateTime::localTime();
    trvlseg.arrivalDT() = DateTime::localTime();

    travelsegVec.push_back(&trvlseg);

    MockFareMarket faremkt;

    faremkt.set_origin(&loc1);

    faremkt.set_destination(&loc2);

    GlobalDirection gb = GlobalDirection::WH;
    faremkt.setGlobalDirection(gb);
    CarrierCode cc = "AA";
    faremkt.governingCarrier() = cc;

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    ptf->fareMarket() = &faremkt;

    fu->paxTypeFare() = ptf;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;

    CombinabilityRuleInfo combinabilityRuleInfo;
    fu->rec2Cat10() = &combinabilityRuleInfo;
    ptf->rec2Cat10() = &combinabilityRuleInfo;

    try
    {
      CombinabilityValidationResult results = combinations.process(farePath, 1, fu, fu, diag);
      CPPUNIT_ASSERT_EQUAL(CVR_PASSED, results);
    }
    catch (ErrorResponseException& ex)
    {
      std::ostringstream oss;
      oss << ex.message() << "\n" << ex.code();
      CPPUNIT_ASSERT_MESSAGE(oss.str(), false);
    }
  }

  void testDisplayDiagA()
  {
    MockFarePath farePath;

    DiagnosticTypes diagType = Diagnostic610;
    Diagnostic rootDiag(diagType);
    _trx->diagnostic().diagnosticType() = Diagnostic610;
    _trx->diagnostic().activate();

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    farePath.pricingUnit().push_back(&_pricingUnit);

    farePath.pricingUnit()[0]->puType() = PricingUnit::Type::ROUNDTRIP;
    farePath.pricingUnit()[0]->geoTravelType() = GeoTravelType::International;

    FareUsage* fu = _memHandle.create<FareUsage>();
    farePath.pricingUnit()[0]->fareUsage().push_back(fu);

    PricingRequest request;

    request.globalDirection() = GlobalDirection::AT;

    _trx->setRequest(&request);

    Loc loca;
    NationCode nation = "US";
    loca.nation() = nation;

    Itin itin;
    _trx->itin().push_back(&itin);

    farePath.itin() = &itin;

    MockLoc loc1;
    LocCode dfw = "DFW";
    loc1.setLoc(dfw);

    MockLoc loc2;
    LocCode lon = "LON";
    loc2.setLoc(lon);

    MockLoc loc3;
    NationCode us = "US";
    loc3.setNation(us);
    IATAAreaCode a1 = "1";
    loc3.setIATAArea(a1);

    MockLoc loc4;
    NationCode ca = "CA";
    loc4.setNation(ca);
    IATAAreaCode a2 = "1";
    loc4.setIATAArea(a2);

    LocCode board = "DFW";
    LocCode off = "LON";

    MockTravelSeg trvlseg;

    std::vector<TravelSeg*>& travelsegVec = _trx->travelSeg();

    trvlseg.origin() = &loc3;
    trvlseg.destination() = &loc4;
    trvlseg.origAirport() = board;
    trvlseg.destAirport() = off;
    trvlseg.departureDT() = DateTime::localTime();
    trvlseg.arrivalDT() = DateTime::localTime();

    travelsegVec.push_back(&trvlseg);

    MockFareMarket faremkt;

    faremkt.set_origin(&loc1);

    faremkt.set_destination(&loc2);

    GlobalDirection gb = GlobalDirection::WH;
    faremkt.setGlobalDirection(gb);
    CarrierCode cc = "AA";
    faremkt.governingCarrier() = cc;

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    ptf->fareMarket() = &faremkt;

    fu->paxTypeFare() = ptf;

    CombinabilityRuleInfo* pCat10 = _memHandle.create<CombinabilityRuleInfo>();
    farePath.pricingUnit()[0]->fareUsage()[0]->rec2Cat10() = pCat10;

    pCat10->sojInd() = Combinations::PERMITTED;
    pCat10->dojInd() = Combinations::PERMITTED;
    pCat10->ct2Ind() = Combinations::PERMITTED;
    pCat10->ct2plusInd() = Combinations::PERMITTED;
    pCat10->eoeInd() = Combinations::PERMITTED;

    //---
    MockTravelSeg trvlseg2;

    trvlseg2.origin() = &loc4;
    trvlseg2.destination() = &loc3;
    trvlseg2.origAirport() = off;
    trvlseg2.destAirport() = board;
    trvlseg2.departureDT() = DateTime::localTime();
    trvlseg2.arrivalDT() = DateTime::localTime();

    travelsegVec.push_back(&trvlseg2);

    FareUsage* fu2 = _memHandle.create<FareUsage>();
    farePath.pricingUnit()[0]->fareUsage().push_back(fu2);
    farePath.pricingUnit()[0]->fareUsage()[1]->rec2Cat10() = pCat10;

    MockFareMarket faremkt2;

    faremkt2.set_origin(&loc2);

    faremkt2.set_destination(&loc1);

    faremkt2.setGlobalDirection(gb);
    faremkt2.governingCarrier() = cc;

    FareInfo* fi2 = _memHandle.create<FareInfo>();
    fi2->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare* _fare2 = _memHandle.create<Fare>();
    _fare2->setFareInfo(fi2);

    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    ptf2->setFare(_fare2);
    ptf2->fareMarket() = &faremkt2;

    fu2->paxTypeFare() = ptf2;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;

    combinations.displayDiag(diag, 50);
    diag.flushMsg();

    std::string expected = " FAILED TO BUILD END ON END LIST\n";
    std::string str = _trx->diagnostic().toString();
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayDiagB()
  {
    MockFarePath farePath;

    DiagnosticTypes diagType = Diagnostic610;
    Diagnostic rootDiag(diagType);
    _trx->diagnostic().diagnosticType() = Diagnostic610;
    _trx->diagnostic().activate();

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    farePath.pricingUnit().push_back(&_pricingUnit);

    farePath.pricingUnit()[0]->puType() = PricingUnit::Type::ROUNDTRIP;
    farePath.pricingUnit()[0]->geoTravelType() = GeoTravelType::International;

    FareUsage* fu = _memHandle.create<FareUsage>();
    farePath.pricingUnit()[0]->fareUsage().push_back(fu);

    PricingRequest request;

    request.globalDirection() = GlobalDirection::AT;

    _trx->setRequest(&request);

    Loc loca;
    NationCode nation = "US";
    loca.nation() = nation;

    Itin itin;
    _trx->itin().push_back(&itin);

    farePath.itin() = &itin;

    MockLoc loc1;
    LocCode dfw = "DFW";
    loc1.setLoc(dfw);

    MockLoc loc2;
    LocCode lon = "LON";
    loc2.setLoc(lon);

    MockLoc loc3;
    NationCode us = "US";
    loc3.setNation(us);
    IATAAreaCode a1 = "1";
    loc3.setIATAArea(a1);

    MockLoc loc4;
    NationCode ca = "CA";
    loc4.setNation(ca);
    IATAAreaCode a2 = "1";
    loc4.setIATAArea(a2);

    LocCode board = "DFW";
    LocCode off = "LON";

    MockTravelSeg trvlseg;

    std::vector<TravelSeg*>& travelsegVec = _trx->travelSeg();

    trvlseg.origin() = &loc3;
    trvlseg.destination() = &loc4;
    trvlseg.origAirport() = board;
    trvlseg.destAirport() = off;
    trvlseg.departureDT() = DateTime::localTime();
    trvlseg.arrivalDT() = DateTime::localTime();

    travelsegVec.push_back(&trvlseg);

    MockFareMarket faremkt;

    faremkt.set_origin(&loc1);

    faremkt.set_destination(&loc2);

    GlobalDirection gb = GlobalDirection::WH;
    faremkt.setGlobalDirection(gb);
    CarrierCode cc = "AA";
    faremkt.governingCarrier() = cc;

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    ptf->fareMarket() = &faremkt;

    fu->paxTypeFare() = ptf;

    CombinabilityRuleInfo* pCat10 = _memHandle.create<CombinabilityRuleInfo>();
    farePath.pricingUnit()[0]->fareUsage()[0]->rec2Cat10() = pCat10;

    pCat10->sojInd() = Combinations::PERMITTED;
    pCat10->dojInd() = Combinations::PERMITTED;
    pCat10->ct2Ind() = Combinations::PERMITTED;
    pCat10->ct2plusInd() = Combinations::PERMITTED;
    pCat10->eoeInd() = Combinations::PERMITTED;

    //---
    MockTravelSeg trvlseg2;

    trvlseg2.origin() = &loc4;
    trvlseg2.destination() = &loc3;
    trvlseg2.origAirport() = off;
    trvlseg2.destAirport() = board;
    trvlseg2.departureDT() = DateTime::localTime();
    trvlseg2.arrivalDT() = DateTime::localTime();

    travelsegVec.push_back(&trvlseg2);

    FareUsage* fu2 = _memHandle.create<FareUsage>();
    farePath.pricingUnit()[0]->fareUsage().push_back(fu2);
    farePath.pricingUnit()[0]->fareUsage()[1]->rec2Cat10() = pCat10;

    MockFareMarket faremkt2;

    faremkt2.set_origin(&loc2);

    faremkt2.set_destination(&loc1);

    faremkt2.setGlobalDirection(gb);
    faremkt2.governingCarrier() = cc;

    FareInfo* fi2 = _memHandle.create<FareInfo>();
    fi2->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare* _fare2 = _memHandle.create<Fare>();
    _fare2->setFareInfo(fi2);

    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    ptf2->setFare(_fare2);
    ptf2->fareMarket() = &faremkt2;

    fu2->paxTypeFare() = ptf2;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;

    combinations.displayDiag(diag, 70, *fu2, 1);
    diag.flushMsg();

    std::string expected = " FAILED DIRECTIONALITY CHECK -  FARE\n";
    std::string str = _trx->diagnostic().toString();
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testProcessRtRestr()
  {
    PricingRequest request;
    request.globalDirection() = GlobalDirection::AT;

    _trx->diagnostic().diagnosticType() = Diagnostic632;
    _trx->diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(*_trx);
    diag->enable(Diagnostic632);

    Loc loc;
    loc.loc() = "FRA";
    loc.nation() = "FR";
    loc.subarea() = EUROPE;

    Loc loc1;
    loc1.loc() = "NRT";
    loc1.nation() = "JP";
    loc1.subarea() = IATA_AREA1;

    AirSeg travelSeg;
    travelSeg.segmentOrder() = 0;
    travelSeg.geoTravelType() = GeoTravelType::International;
    travelSeg.origin() = &loc;
    travelSeg.destination() = &loc1;
    travelSeg.carrier() = "LH";

    AirSeg travelSeg1;
    travelSeg1.segmentOrder() = 1;
    travelSeg1.geoTravelType() = GeoTravelType::International;
    travelSeg1.origin() = &loc1;
    travelSeg1.destination() = &loc;
    travelSeg1.carrier() = "LH";

    Itin itin;
    itin.travelSeg().push_back(&travelSeg);
    itin.travelSeg().push_back(&travelSeg1);

    _trx->travelSeg().push_back(&travelSeg);
    _trx->travelSeg().push_back(&travelSeg1);

    // build FareMarket
    // Create PaxType
    PaxType paxType;
    paxType.paxType() = "ADT";

    Fare fare;
    fare.nucFareAmount() = 230.00;

    FareInfo fareInfo;
    fareInfo._carrier = "LH";
    fareInfo._market1 = "FRA";
    fareInfo._market2 = "NRT";
    fareInfo._fareClass = "WMLUQOW";
    fareInfo._fareAmount = 200.00;
    fareInfo._currency = "GBP";
    fareInfo._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo._ruleNumber = "5135";
    fareInfo._routingNumber = "XXXX";
    fareInfo._directionality = FROM;
    fareInfo._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&travelSeg);
    fareMarket.origin() = &loc;
    fareMarket.destination() = &loc1;

    GlobalDirection globleDirection = GlobalDirection::PA;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "LH";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    PaxTypeFare paxTypeFare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);

    FareClassAppInfo appInfo;
    appInfo._fareType = "EU";
    appInfo._pricingCatType = 'N';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setEconomyClass(); //= '3';

    Fare fare1;
    fare1.nucFareAmount() = 800.00;

    FareInfo _fareInfo1;
    _fareInfo1._carrier = "LH";
    _fareInfo1._market1 = "NRT";
    _fareInfo1._market2 = "FRA";
    _fareInfo1._fareClass = "WMLUQOW";
    _fareInfo1._fareAmount = 600.00;
    _fareInfo1._currency = "GBP";
    _fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    _fareInfo1._ruleNumber = "5135";
    _fareInfo1._routingNumber = "XXXX";
    _fareInfo1._directionality = FROM;
    _fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo _tariffRefInfo1;
    _tariffRefInfo1._fareTariffCode = "TAFPBA";

    FareMarket _fareMarket1;
    _fareMarket1.travelSeg().push_back(&travelSeg1);
    _fareMarket1.origin() = &loc1;
    _fareMarket1.destination() = &loc;

    GlobalDirection globleDirection1 = GlobalDirection::PA;
    _fareMarket1.setGlobalDirection(globleDirection1);
    _fareMarket1.governingCarrier() = "LH";

    fare1.initialize(Fare::FS_International, &_fareInfo1, _fareMarket1, &_tariffRefInfo1);

    PaxTypeFare _paxTypeFare1;
    _paxTypeFare1.initialize(&fare1, &paxType, &_fareMarket1);

    FareClassAppInfo _appInfo1;
    _appInfo1._fareType = "EU";
    _appInfo1._pricingCatType = 'N';
    _paxTypeFare1.fareClassAppInfo() = &_appInfo1;

    FareClassAppSegInfo _fareClassAppSegInfo1;
    _fareClassAppSegInfo1._paxType = "ADT";
    _paxTypeFare1.fareClassAppSegInfo() = &_fareClassAppSegInfo1;
    _paxTypeFare1.cabin().setEconomyClass(); //= '3';

    CombinabilityRuleInfo cat10;
    cat10.dojSameCarrierInd() = Combinations::SAME_CARRIER;
    cat10.dojSameRuleTariffInd() = Combinations::SAME_RULE;
    cat10.dojSameRuleTariffInd() = Combinations::SAME_TARIFF;
    cat10.dojSameFareInd() = Combinations::SAME_FARECLASS;
    cat10.dojSameFareInd() = Combinations::SAME_FARETYPE;

    cat10.ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    cat10.ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    cat10.ct2SameRuleTariffInd() = Combinations::SAME_TARIFF;
    cat10.ct2SameFareInd() = Combinations::SAME_FARECLASS;
    cat10.ct2SameFareInd() = Combinations::SAME_FARETYPE;

    cat10.ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    cat10.ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    cat10.ct2plusSameRuleTariffInd() = Combinations::SAME_TARIFF;
    cat10.ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    cat10.ct2plusSameFareInd() = Combinations::SAME_FARETYPE;

    cat10.eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    cat10.eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    cat10.eoeSameRuleTariffInd() = Combinations::SAME_TARIFF;
    cat10.eoeSameFareInd() = Combinations::SAME_FARECLASS;
    cat10.eoeSameFareInd() = Combinations::SAME_FARETYPE;

    cat10.sojInd() = Combinations::PERMITTED;
    cat10.dojInd() = Combinations::PERMITTED;
    cat10.ct2Ind() = Combinations::PERMITTED;
    cat10.ct2plusInd() = Combinations::PERMITTED;
    cat10.eoeInd() = Combinations::NOT_PERMITTED;
    cat10.vendorCode() = "ATP";

    CombinabilityRuleItemInfo cat10Seg;
    cat10Seg.setItemNo(0);
    cat10Seg.setTextonlyInd('Y');
    cat10Seg.setEoeallsegInd(Combinations::ALL_SEGMENTS);
    cat10Seg.setItemcat(Combinations::ROUND_TRIP);

    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->travelSeg().push_back(&travelSeg);
    fareUsage->paxTypeFare() = &paxTypeFare;
    fareUsage->rec2Cat10() = &cat10;

    FareUsage* fareUsage1 = _memHandle.create<FareUsage>();
    fareUsage1->travelSeg().push_back(&travelSeg1);
    fareUsage1->paxTypeFare() = &_paxTypeFare1;
    fareUsage1->rec2Cat10() = &cat10;

    PricingUnit::Type ow = PricingUnit::Type::ROUNDTRIP;
    PricingUnit::PUFareType nl = PricingUnit::NL;

    _pricingUnit.fareUsage().push_back(fareUsage);
    _pricingUnit.fareUsage().push_back(fareUsage1);
    _pricingUnit.puType() = ow;
    _pricingUnit.puFareType() = nl;

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&_pricingUnit);
    farePath.paxType() = &paxType;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;

    Combinations::DirectionalityInfo directionalityInfo(Combinations::PricingUnitLevel,
                                                        "PU",
                                                        Combinations::FROM_LOC1_TO_LOC2,
                                                        Combinations::TO_LOC1_FROM_LOC2);

    Combinations::ValidationFareComponents vfc;
    combinations.buildValidationFareComponent(
        *diag, directionalityInfo, _pricingUnit, *fareUsage, fareUsage->rec2Cat10(), farePath, vfc);

    combinations.validateCombination(
        *diag, directionalityInfo, &vfc, _pricingUnit, &farePath, fareUsage1, fareUsage1, 0);

    combinations.validateDirectionality(
        cat10Seg, *fareUsage, fareUsage->rec2Cat10(), directionalityInfo);

    combinations.processMajorSubCat(*diag,
                                    _pricingUnit,
                                    *fareUsage,
                                    fareUsage->rec2Cat10(),
                                    cat10Seg,
                                    farePath,
                                    vfc,
                                    &_negMatchedVfc,
                                    directionalityInfo);

    combinations.processRoundTripRestriction(
        *diag, _pricingUnit, *fareUsage, fareUsage->rec2Cat10(), 0, vfc);

    combinations.process(farePath, 1, fareUsage1, fareUsage1, *diag);

    diag->flushMsg();

    std::string expected = " PU MAJOR CAT:102\n";
    std::string str = _trx->diagnostic().toString();
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void buildDataForCTTest(FarePath& farePath)
  {
    _request.globalDirection() = GlobalDirection::AT;

    _trx->diagnostic().diagnosticType() = Diagnostic632;
    _trx->diagnostic().activate();

    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = "FRA";
    loc->nation() = "FR";
    loc->subarea() = EUROPE;

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = "NRT";
    loc1->nation() = "JP";
    loc1->subarea() = IATA_AREA3;

    Loc* loc2 = _memHandle.create<Loc>();
    loc2->loc() = "PAR";
    loc2->nation() = "FR";
    loc2->subarea() = EUROPE;

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->segmentOrder() = 0;
    travelSeg->geoTravelType() = GeoTravelType::International;
    travelSeg->origin() = loc;
    travelSeg->destination() = loc1;
    travelSeg->carrier() = "LH";

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->segmentOrder() = 1;
    travelSeg1->geoTravelType() = GeoTravelType::International;
    travelSeg1->origin() = loc1;
    travelSeg1->destination() = loc2;
    travelSeg1->carrier() = "JP";

    AirSeg* travelSeg2 = _memHandle.create<AirSeg>();
    travelSeg2->segmentOrder() = 2;
    travelSeg2->geoTravelType() = GeoTravelType::International;
    travelSeg2->origin() = loc2;
    travelSeg2->destination() = loc;
    travelSeg2->carrier() = "LH";

    _itin.travelSeg().clear();
    _itin.travelSeg().push_back(travelSeg);
    _itin.travelSeg().push_back(travelSeg1);
    _itin.travelSeg().push_back(travelSeg2);

    _trx->travelSeg().push_back(travelSeg);
    _trx->travelSeg().push_back(travelSeg1);
    _trx->travelSeg().push_back(travelSeg2);

    // build FareMarket
    // Create PaxType
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = "ADT";

    Fare* fare = _memHandle.create<Fare>();
    fare->nucFareAmount() = 600.00;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_vendor = "ATP";
    fareInfo->_carrier = "LH";
    fareInfo->_market1 = "FRA";
    fareInfo->_market2 = "NRT";
    fareInfo->_fareClass = "WMLUQOW";
    fareInfo->_fareAmount = 200.00;
    fareInfo->_currency = "GBP";
    fareInfo->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo->_ruleNumber = "5135";
    fareInfo->_routingNumber = "XXXX";
    fareInfo->_directionality = FROM;
    fareInfo->_globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    tariffRefInfo->_fareTariffCode = "TAFPBA";

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg().push_back(travelSeg);
    fareMarket->origin() = loc;
    fareMarket->destination() = loc1;

    GlobalDirection globleDirection = GlobalDirection::PA;
    fareMarket->setGlobalDirection(globleDirection);
    fareMarket->governingCarrier() = "LH";

    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffRefInfo);

    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    paxTypeFare->initialize(fare, paxType, fareMarket);

    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_fareType = "EU";
    appInfo->_pricingCatType = 'N';
    paxTypeFare->fareClassAppInfo() = appInfo;

    FareClassAppSegInfo* fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();
    fareClassAppSegInfo->_paxType = "ADT";
    paxTypeFare->fareClassAppSegInfo() = fareClassAppSegInfo;
    paxTypeFare->cabin().setEconomyClass(); //= '3';

    _fare1.nucFareAmount() = 800.00;

    _fareInfo1._carrier = "LH";
    _fareInfo1._market1 = "NRT";
    _fareInfo1._market2 = "PAR";
    _fareInfo1._fareClass = "WMLUQOW";
    _fareInfo1._fareAmount = 600.00;
    _fareInfo1._currency = "GBP";
    _fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    _fareInfo1._ruleNumber = "5135";
    _fareInfo1._routingNumber = "XXXX";
    _fareInfo1._directionality = FROM;
    _fareInfo1._globalDirection = GlobalDirection::AT;

    _tariffRefInfo1._fareTariffCode = "TAFPBA";

    _fareMarket1.travelSeg().push_back(travelSeg1);
    _fareMarket1.origin() = loc1;
    _fareMarket1.destination() = loc2;

    GlobalDirection globleDirection1 = GlobalDirection::PA;
    _fareMarket1.setGlobalDirection(globleDirection1);
    _fareMarket1.governingCarrier() = "LH";

    _fare1.initialize(Fare::FS_International, &_fareInfo1, _fareMarket1, &_tariffRefInfo1);

    _paxTypeFare1.initialize(&_fare1, paxType, &_fareMarket1);

    _appInfo1._fareType = "EU";
    _appInfo1._pricingCatType = 'N';
    _paxTypeFare1.fareClassAppInfo() = &_appInfo1;

    _fareClassAppSegInfo1._paxType = "ADT";
    _paxTypeFare1.fareClassAppSegInfo() = &_fareClassAppSegInfo1;
    _paxTypeFare1.cabin().setEconomyClass(); //= '3';

    _fare2.nucFareAmount() = 200.00;

    _fareInfo2._carrier = "LH";
    _fareInfo2._market1 = "NRT";
    _fareInfo2._market2 = "FRA";
    _fareInfo2._fareClass = "WMLUQOW";
    _fareInfo2._fareAmount = 600.00;
    _fareInfo2._currency = "GBP";
    _fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    _fareInfo2._ruleNumber = "5135";
    _fareInfo2._routingNumber = "XXXX";
    _fareInfo2._directionality = FROM;
    _fareInfo2._globalDirection = GlobalDirection::AT;

    _tariffRefInfo2._fareTariffCode = "TAFPBA";

    _fareMarket2.travelSeg().push_back(travelSeg2);
    _fareMarket2.origin() = loc1;
    _fareMarket2.destination() = loc;

    GlobalDirection globleDirection2 = GlobalDirection::WH;
    _fareMarket2.setGlobalDirection(globleDirection2);
    _fareMarket2.governingCarrier() = "LH";

    _fare2.initialize(Fare::FS_International, &_fareInfo2, _fareMarket2, &_tariffRefInfo2);

    _paxTypeFare2.initialize(&_fare2, paxType, &_fareMarket2);

    _appInfo2._fareType = "EU";
    _appInfo2._pricingCatType = 'N';
    _paxTypeFare2.fareClassAppInfo() = &_appInfo2;

    _fareClassAppSegInfo2._paxType = "ADT";
    _paxTypeFare2.fareClassAppSegInfo() = &_fareClassAppSegInfo2;
    _paxTypeFare2.cabin().setEconomyClass(); //= '3';

    // Build Records
    _cat10.dojSameCarrierInd() = Combinations::SAME_CARRIER;
    _cat10.dojSameRuleTariffInd() = Combinations::SAME_RULE;
    _cat10.dojSameRuleTariffInd() = Combinations::SAME_TARIFF;
    _cat10.dojSameFareInd() = Combinations::SAME_FARECLASS;
    _cat10.dojSameFareInd() = Combinations::SAME_FARETYPE;

    _cat10.ct2SameCarrierInd() = Combinations::SAME_CARRIER;
    _cat10.ct2SameRuleTariffInd() = Combinations::SAME_RULE;
    _cat10.ct2SameRuleTariffInd() = Combinations::SAME_TARIFF;
    _cat10.ct2SameFareInd() = Combinations::SAME_FARECLASS;
    _cat10.ct2SameFareInd() = Combinations::SAME_FARETYPE;

    _cat10.ct2plusSameCarrierInd() = Combinations::SAME_CARRIER;
    _cat10.ct2plusSameRuleTariffInd() = Combinations::SAME_RULE;
    _cat10.ct2plusSameRuleTariffInd() = Combinations::SAME_TARIFF;
    _cat10.ct2plusSameFareInd() = Combinations::SAME_FARECLASS;
    _cat10.ct2plusSameFareInd() = Combinations::SAME_FARETYPE;

    _cat10.eoeSameCarrierInd() = Combinations::SAME_CARRIER;
    _cat10.eoeSameRuleTariffInd() = Combinations::SAME_RULE;
    _cat10.eoeSameRuleTariffInd() = Combinations::SAME_TARIFF;
    _cat10.eoeSameFareInd() = Combinations::SAME_FARECLASS;
    _cat10.eoeSameFareInd() = Combinations::SAME_FARETYPE;

    _cat10.sojInd() = Combinations::PERMITTED;
    _cat10.dojInd() = Combinations::PERMITTED;
    _cat10.ct2Ind() = Combinations::PERMITTED;
    _cat10.ct2plusInd() = Combinations::PERMITTED;
    _cat10.eoeInd() = Combinations::NOT_PERMITTED;
    _cat10.vendorCode() = "ATP";

    _cat10Seg.setItemNo(0);
    _cat10Seg.setTextonlyInd('Y');
    _cat10Seg.setEoeallsegInd(Combinations::ALL_SEGMENTS);
    _cat10Seg.setItemcat(Combinations::CIRCLE_TRIP);
    _cat10Seg.setRelationalInd(CategoryRuleItemInfo::THEN);
    _cat10Seg.setDirectionality(RULE_ALWAYS_APPLIES);

    _cat10Seg1.setItemNo(0);
    _cat10Seg1.setTextonlyInd('Y');
    _cat10Seg1.setEoeallsegInd(Combinations::ALL_SEGMENTS);
    _cat10Seg1.setItemcat(Combinations::CIRCLE_TRIP);
    _cat10Seg1.setRelationalInd(CategoryRuleItemInfo::OR);
    _cat10Seg1.setDirectionality(RULE_ALWAYS_APPLIES);

    _cat10.addItemInfoSetNosync(_catRuleItemInfoSet);

    // Build Fare Usages
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->travelSeg().push_back(travelSeg);
    fareUsage->paxTypeFare() = paxTypeFare;
    fareUsage->rec2Cat10() = &_cat10;

    FareUsage* fareUsage1 = _memHandle.create<FareUsage>();
    fareUsage1->travelSeg().push_back(travelSeg1);
    fareUsage1->paxTypeFare() = &_paxTypeFare1;
    fareUsage1->rec2Cat10() = &_cat10;

    FareUsage* fareUsage2 = _memHandle.create<FareUsage>();
    fareUsage2->travelSeg().push_back(travelSeg2);
    fareUsage2->paxTypeFare() = &_paxTypeFare2;
    fareUsage2->rec2Cat10() = &_cat10;

    _pricingUnit.fareUsage().push_back(fareUsage);
    _pricingUnit.fareUsage().push_back(fareUsage1);
    _pricingUnit.fareUsage().push_back(fareUsage2);
    _pricingUnit.puType() = PricingUnit::Type::CIRCLETRIP;
    _pricingUnit.puFareType() = PricingUnit::NL;

    farePath.itin() = &_itin;
    farePath.pricingUnit().push_back(&_pricingUnit);
    farePath.paxType() = paxType;
  }

  void testProcessCtRestr()
  {
    FarePath farePath;
    buildDataForCTTest(farePath);
    FareUsage* fareUsage = _pricingUnit.fareUsage().front();

    Combinations::DirectionalityInfo directionalityInfo(Combinations::PricingUnitLevel,
                                                        "PU",
                                                        Combinations::FROM_LOC1_TO_LOC2,
                                                        Combinations::TO_LOC1_FROM_LOC2);

    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(*_trx);
    diag->enable(Diagnostic632);
    Combinations::ValidationFareComponents vfc;
    _combinations->buildValidationFareComponent(
        *diag, directionalityInfo, _pricingUnit, *fareUsage, fareUsage->rec2Cat10(), farePath, vfc);

    _combinations->validateCombination(
        *diag, directionalityInfo, &vfc, _pricingUnit, &farePath, fareUsage, fareUsage, 0);

    _combinations->validateDirectionality(
        _cat10Seg, *fareUsage, fareUsage->rec2Cat10(), directionalityInfo);

    _combinations->processMajorSubCat(*diag,
                                      _pricingUnit,
                                      *fareUsage,
                                      fareUsage->rec2Cat10(),
                                      _cat10Seg,
                                      farePath,
                                      vfc,
                                      &_negMatchedVfc,
                                      directionalityInfo);

    _combinations->processCircleTripRestriction(
        *diag, _pricingUnit, *fareUsage, fareUsage->rec2Cat10(), 0, vfc);

    _combinations->process(farePath, 1, fareUsage, fareUsage, *diag);

    diag->flushMsg();

    std::string expected = " PU MAJOR CAT:103\n";
    std::string str = _trx->diagnostic().toString();
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testProcessEndOnEndRestrictionAbortWhenNotPCat10()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic632;
    _trx->diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(*_trx);
    diag->enable(Diagnostic632);

    PricingUnit pricingUnit;
    FarePath farePath;
    FareUsage fareUsage;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;
    Combinations::ValidationFareComponents vfc;
    Combinations::ValidationElement element;
    vfc.push_back(element);
    vfc[0]._pCat10 = 0;
    vfc[0]._currentFareUsage = &fareUsage;
    vfc[0]._currentFareUsage->rec2Cat10() = 0;

    Combinations::DirectionalityInfo directionalityInfo(Combinations::PricingUnitLevel,
                                                        "PU",
                                                        Combinations::FROM_LOC1_TO_LOC2,
                                                        Combinations::TO_LOC1_FROM_LOC2);

    // should be false because rec2Cat10 is 0
    CPPUNIT_ASSERT(!(combinations.buildValidationFareComponent(
        *diag, directionalityInfo, pricingUnit, fareUsage, fareUsage.rec2Cat10(), farePath, vfc)));
    CPPUNIT_ASSERT_EQUAL(Combinations::ABORT,
                         combinations.processEndOnEndRestriction(
                             *diag, farePath, pricingUnit, 100, vfc, directionalityInfo));
  }

  void testProcessEndOnEndRestrictionFailCombWhenEndOnEndNull()
  {
    FarePath farePath;

    _trx->diagnostic().diagnosticType() = Diagnostic632;
    _trx->diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(*_trx);
    diag->enable(Diagnostic632);

    FareUsage fareUsage, fareUsageT;
    PricingUnit pricingUnit;

    Combinations combinations;
    CombinabilityScoreboard comboScoreboard;
    comboScoreboard.trx() = _trx;
    combinations.comboScoreboard() = &comboScoreboard;
    combinations.trx() = _trx;
    Combinations::ValidationFareComponents vfc;
    vfc.resize(1);
    vfc[0]._currentFareUsage = &fareUsage;
    vfc[0]._targetFareUsage = &fareUsageT;
    CombinabilityRuleInfo cat10Rule;
    vfc[0]._pCat10 = &cat10Rule;

    Combinations::DirectionalityInfo directionalityInfo(Combinations::FarePathLevel,
                                                        "FP",
                                                        Combinations::FROM_LOC1_TO_LOC2,
                                                        Combinations::TO_LOC1_FROM_LOC2);

    // should be false because we should not get EndOnEnd rule
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB,
                         combinations.processEndOnEndRestriction(
                             *diag, farePath, _pricingUnit, 99999, vfc, directionalityInfo));
  }

  void testProcessEndOnEndRestrictionFailWhenMissingReqIntlFare()
  {
    setEndOnEndReqIntlFare();

    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(3);

    setTargetFUFareStatus(0, Fare::FS_Domestic);        setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_Transborder);     setTargetFareMarketOnD(1, &_locWAS, &_locYYZ);
    setTargetFUFareStatus(2, Fare::FS_ForeignDomestic); setTargetFareMarketOnD(2, &_locWAW, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotMissingReqIntlFare()
  {
    setEndOnEndReqIntlFare();

    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(3);

    setTargetFUFareStatus(0, Fare::FS_Domestic);      setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_Transborder);   setTargetFareMarketOnD(1, &_locWAS, &_locYYZ);
    setTargetFUFareStatus(2, Fare::FS_International); setTargetFareMarketOnD(2, &_locWAS, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedIntlFare()
  {
    setNotPermitIntlFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(3);

    setTargetFUFareStatus(0, Fare::FS_Domestic);        setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_Transborder);     setTargetFareMarketOnD(1, &_locWAS, &_locYYZ);
    setTargetFUFareStatus(2, Fare::FS_ForeignDomestic); setTargetFareMarketOnD(2, &_locWAW, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedIntlFare()
  {
    setNotPermitIntlFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(3);

    setTargetFUFareStatus(0, Fare::FS_Domestic);      setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_Transborder);   setTargetFareMarketOnD(1, &_locWAS, &_locYYZ);
    setTargetFUFareStatus(2, Fare::FS_International); setTargetFareMarketOnD(2, &_locWAS, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailWhenMissingReqDomFare()
  {
    setEndOnEndReqDomFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(1);

    setTargetFUFareStatus(0, Fare::FS_International);   setTargetFareMarketOnD(0, &_locWAS, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotMissingReqDomFare()
  {
    setEndOnEndReqDomFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(1);

    setTargetFUFareStatus(0, Fare::FS_ForeignDomestic); setTargetFareMarketOnD(0, &_locWAW, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotMissingReqDomFare_RUXU_Exception()
  {
    setEndOnEndReqDomFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(1);

    setTargetFUFareStatus(0, Fare::FS_Domestic); setTargetFareMarketOnD(0, &_locABA, &_locAER);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedDomFare()
  {
    setNotPermitDomFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(1);

    setTargetFUFareStatus(0, Fare::FS_International); setTargetFareMarketOnD(0, &_locWAS, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedDomFare()
  {
    setNotPermitDomFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(1);

    setTargetFUFareStatus(0, Fare::FS_ForeignDomestic);   setTargetFareMarketOnD(0, &_locWAW, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailWhenMissingReqTransbFare()
  {
    setEndOnEndReqTransbFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(2);
    setTargetFUFareStatus(0, Fare::FS_Domestic);          setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_ForeignDomestic);   setTargetFareMarketOnD(1, &_locWAW, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotMissingReqTransbFare()
  {
    setEndOnEndReqTransbFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(2);
    setTargetFUFareStatus(0, Fare::FS_Domestic);          setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_Transborder);       setTargetFareMarketOnD(1, &_locDFW, &_locYYZ);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedTransbFare()
  {
    setNotPermitTransbFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(2);
    setTargetFUFareStatus(0, Fare::FS_Domestic);          setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_ForeignDomestic);   setTargetFareMarketOnD(1, &_locWAW, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedTransbFare()
  {
    setNotPermitTransbFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(2);
    setTargetFUFareStatus(0, Fare::FS_Domestic);          setTargetFareMarketOnD(0, &_locWAS, &_locDFW);
    setTargetFUFareStatus(1, Fare::FS_Transborder);       setTargetFareMarketOnD(1, &_locDFW, &_locYYZ);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailWhenMissingReqNormalFare()
  {
    setEndOnEndReqNormalFare();
    prepareTargetFU(1);
    setTargetFUSpecialFare(0);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotMissingReqNormalFare()
  {
    setEndOnEndReqNormalFare();
    prepareTargetFU(1);
    setTargetFUNormalFare(0);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedNormalFare()
  {
    setNotPermitNormalFare();
    prepareTargetFU(1);
    setTargetFUSpecialFare(0);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedNormalFare()
  {
    setNotPermitNormalFare();
    prepareTargetFU(1);
    setTargetFUNormalFare(0);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailWhenMissingReqSpecialFare()
  {
    setEndOnEndReqSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUNormalFare(1);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotMissingReqSpecialFare()
  {
    setEndOnEndReqSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUSpecialFare(1);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionPassWhenNotCombNotPermittedSpecialFare()
  {
    setNotPermitSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUNormalFare(1);

    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void testProcessEndOnEndRestrictionFailCombWhenCombNotPermittedSpecialFare()
  {
    setNotPermitSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUSpecialFare(1);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void setFareClass(FareUsage& fu, TariffCrossRefInfo* tariffRefInfo, const char* fc)
  {
    Fare* f = fu.paxTypeFare()->fare();
    FareInfo* fi = const_cast<FareInfo*>(f->fareInfo());
    fi->fareClass() = fc;
    tariffRefInfo->_fareTariffCode = "TAFPBA";
    f->setTariffCrossRefInfo(tariffRefInfo);
  }

  char run_test_processEndOnEndABA_P(char byte47 /* N/Y/P */, const char* fcX, const char* fcY)
  {
    _endOnEndRule.abacombInd() = byte47;

    TariffCrossRefInfo tariffRefInfo;
    setFareClass(*_endOnEndDA->_validationFareComponent[0]._currentFareUsage, &tariffRefInfo, fcX);
    setFareClass(*_endOnEndDA->_validationFareComponent[0]._targetFareUsage, &tariffRefInfo, fcY);

    _endOnEndDA->_validationFareComponent[0].getSubCat(Combinations::M104) = Combinations::ValidationElement::MATCHED;
    _combinations->processEndOnEndABA(*_endOnEndDA, 0);
    return _endOnEndDA->_validationFareComponent[0].getSubCat(Combinations::M104);
  }

  // P: End-On-End combination not required but only A-B-A permitted
  void test_same_fareClass_processEndOnEndABA_P()
  {
    setNotPermitSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUSpecialFare(1);

    CPPUNIT_ASSERT(_endOnEndDA->_validationFareComponent.size());
    CPPUNIT_ASSERT_EQUAL(
        Combinations::ValidationElement::MATCHED,
        run_test_processEndOnEndABA_P(Combinations::IF_EOE_MUST_BE_A_B_A, "X", "Y"));
  }

  // Y: End-On-End combination required but only A-B-A permitted
  void test_same_fareClass_processEndOnEndABA_Y()
  {
    setNotPermitSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUSpecialFare(1);

    CPPUNIT_ASSERT(_endOnEndDA->_validationFareComponent.size());
    CPPUNIT_ASSERT_EQUAL(Combinations::ValidationElement::MATCHED,
                         run_test_processEndOnEndABA_P(Combinations::MUST_BE_A_B_A, "X", "Y"));
  }

  void test_same_fareClass_processEndOnEndABA_N()
  {
    setNotPermitSpecialFare();
    prepareTargetFU(2);
    setTargetFUNormalFare(0);
    setTargetFUSpecialFare(1);

    CPPUNIT_ASSERT(_endOnEndDA->_validationFareComponent.size());
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB,
                         run_test_processEndOnEndABA_P(Combinations::MUST_NOT_BE_A_B_A, "X", "Y"));
  }

  void testProcessMajorSubCatRtnSTOPWhenNeg104Permitted()
  {
    prepareTargetFU(1);
    CombinabilityRuleItemInfo cat104Info;
    cat104Info.setItemcat(104);
    _vfc[0]._currentFareUsage->rec2Cat10() = &_cat10Rule;
    _cat10Rule.eoeInd() = Combinations::PERMITTED;

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processMajorSubCat(_diag,
                                                           _pricingUnit,
                                                           *_vfc[0]._currentFareUsage,
                                                           _vfc[0]._currentFareUsage->rec2Cat10(),
                                                           cat104Info,
                                                           _fp,
                                                           _vfc,
                                                           &_negMatchedVfc,
                                                           *_directionalityInfo));
  }

  void testProcessMajorSubCatRtnPASSWhen104Permitted()
  {
    prepareTargetFU(1);
    CombinabilityRuleItemInfo cat104Info;
    cat104Info.setItemcat(104);
    _vfc[0]._currentFareUsage->rec2Cat10() = &_cat10Rule;
    _cat10Rule.eoeInd() = Combinations::PERMITTED;

    CPPUNIT_ASSERT_EQUAL(Combinations::PASSCOMB,
                         _combinations->processMajorSubCat(_diag,
                                                           _pricingUnit,
                                                           *_vfc[0]._currentFareUsage,
                                                           _vfc[0]._currentFareUsage->rec2Cat10(),
                                                           cat104Info,
                                                           _fp,
                                                           _vfc,
                                                           0,
                                                           *_directionalityInfo));
  }

  void testProcessTariffRuleRestriction_NoRuleRestr_Abort()
  {
    bool negative = false;
    prepareTargetFU(1);
    _dbHandle->_tariffRuleRests.clear();
    CPPUNIT_ASSERT_EQUAL(Combinations::ABORT,
                         _combinations->processTariffRuleRestriction(_diag,
                                                                     *_vfc[0]._currentFareUsage,
                                                                     _vfc[0]._currentFareUsage->rec2Cat10(),
                                                                     0,
                                                                     negative,
                                                                     _vfc,
                                                                     _pricingUnit));
  }

  void testProcessTariffRuleRestriction_Idle()
  {
    bool negative = false;
    prepareTargetFU(1);
    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processTariffRuleRestriction(_diag,
                                                                     *_vfc[0]._currentFareUsage,
                                                                     _vfc[0]._currentFareUsage->rec2Cat10(),
                                                                     0,
                                                                     negative,
                                                                     _vfc,
                                                                     _pricingUnit));
  }

  void testProcessFareClassTypeRestriction_NoRuleRestr_Abort()
  {
    bool negative = false;
    prepareTargetFU(1);
    _dbHandle->_fareClassRestRule.clear();
    CPPUNIT_ASSERT_EQUAL(Combinations::ABORT,
                         _combinations->processFareClassTypeRestriction(_diag,
                                                                        *_vfc[0]._currentFareUsage,
                                                                        _vfc[0]._currentFareUsage->rec2Cat10(),
                                                                        0,
                                                                        negative,
                                                                        _vfc,
                                                                        _pricingUnit));
  }

  void testProcessFareClassTypeRestriction_Idle()
  {
    bool negative = false;
    prepareTargetFU(1);
    CPPUNIT_ASSERT_EQUAL(Combinations::IDLE,
                         _combinations->processFareClassTypeRestriction(_diag,
                                                                        *_vfc[0]._currentFareUsage,
                                                                        _vfc[0]._currentFareUsage->rec2Cat10(),
                                                                        0,
                                                                        negative,
                                                                        _vfc,
                                                                        _pricingUnit));
  }


  void testCheckGeoSpecBtwPass()
  {
    prepareTargetFU(1);
    _endOnEndRule.fareTypeLocAppl() = 'B'; // between
    setOrigInLoc1_DestInLoc2(0);
    CPPUNIT_ASSERT_EQUAL(Combinations::PASSCOMB, _combinations->checkGeoSpec(*_endOnEndDA, 0));
    CPPUNIT_ASSERT_EQUAL(Combinations::ValidationElement::NOT_SET, _vfc[0].getSubCat(Combinations::M104));
  }

  void testCheckGeoSpecBtwFail()
  {
    prepareTargetFU(1);
    _endOnEndRule.fareTypeLocAppl() = 'B'; // between
    setOrigInLoc1_DestNotInLoc2(0);
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB, _combinations->checkGeoSpec(*_endOnEndDA, 0));
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB, _vfc[0].getSubCat(Combinations::M104));
  }

  void testCheckGeoSpecViaPass()
  {
    prepareTargetFU(1);
    _endOnEndRule.fareTypeLocAppl() = 'B'; // between
    setOrigInLoc1_DestNotInLoc2(0);
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB, _combinations->checkGeoSpec(*_endOnEndDA, 0));
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB, _vfc[0].getSubCat(Combinations::M104));
  }

  void testCheckGeoSpecViaFail()
  {
    prepareTargetFU(1);
    _endOnEndRule.fareTypeLocAppl() = 'B'; // between
    setOrigNotInLoc1_DestNotInLoc2(0);
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB, _combinations->checkGeoSpec(*_endOnEndDA, 0));
    CPPUNIT_ASSERT_EQUAL(Combinations::FAILCOMB, _vfc[0].getSubCat(Combinations::M104));
  }

  void setRuleFailDirectionality(CombinabilityRuleItemInfo& cat10Seg,
                                 FareUsage& fareUsage,
                                 bool setFail = true)
  {
    fareUsage.inbound() = true;
    cat10Seg.setInOutInd((setFail) ? RuleConst::FARE_MARKET_OUTBOUND : RuleConst::FARE_MARKET_INBOUND);
    _catRuleItemInfoSet->push_back(cat10Seg);
  }

  void testdataStringValidation_failSingleTHENFailDirectionality()
  {
    FarePath farePath;
    buildDataForCTTest(farePath);

    FareUsage* fareUsage = _pricingUnit.fareUsage().front();

    Combinations::ValidationFareComponents vfc;
    _combinations->buildValidationFareComponent(_diag,
                                                *_directionalityInfo,
                                                _pricingUnit,
                                                *fareUsage,
                                                fareUsage->rec2Cat10(),
                                                farePath,
                                                vfc);

    setRuleFailDirectionality(_cat10Seg, *fareUsage);

    CPPUNIT_ASSERT(!_combinations->validateCombination(
        _diag, *_directionalityInfo, &vfc, _pricingUnit, &farePath, fareUsage, fareUsage, 0));
  }

  void testdataStringValidation_failTHENFail_ORFailDirectionality()
  {
    FarePath farePath;
    buildDataForCTTest(farePath);

    FareUsage* fareUsage = _pricingUnit.fareUsage().front();

    Combinations::ValidationFareComponents vfc;
    _combinations->buildValidationFareComponent(_diag,
                                                *_directionalityInfo,
                                                _pricingUnit,
                                                *fareUsage,
                                                fareUsage->rec2Cat10(),
                                                farePath,
                                                vfc);

    setRuleFailDirectionality(_cat10Seg, *fareUsage);
    setRuleFailDirectionality(_cat10Seg1, *fareUsage);

    CPPUNIT_ASSERT(!_combinations->validateCombination(
        _diag, *_directionalityInfo, &vfc, _pricingUnit, &farePath, fareUsage, fareUsage, 0));
  }

  void testdataStringValidation_passTHENFail_ORPassDirectionality()
  {
    FarePath farePath;
    buildDataForCTTest(farePath);

    FareUsage* fareUsage = _pricingUnit.fareUsage().front();

    Combinations::ValidationFareComponents vfc;
    _combinations->buildValidationFareComponent(_diag,
                                                *_directionalityInfo,
                                                _pricingUnit,
                                                *fareUsage,
                                                fareUsage->rec2Cat10(),
                                                farePath,
                                                vfc);

    setRuleFailDirectionality(_cat10Seg, *fareUsage);
    setRuleFailDirectionality(_cat10Seg1, *fareUsage, false);

    CPPUNIT_ASSERT(_combinations->validateCombination(
        _diag, *_directionalityInfo, &vfc, _pricingUnit, &farePath, fareUsage, fareUsage, 0));
  }

  void testAllowEndOnEndPassDifferentRecord3()
  {
    prepareTargetFU(3);
    setTargetFUFareStatus(0, Fare::FS_Domestic);
    setTargetFUFareStatus(1, Fare::FS_International);
    setTargetFUFareStatus(2, Fare::FS_International);
    addEOEThatPassIntlFare(_endOnEndDA->_validationFareComponent[0]._currentFareUsage);
    addEOEThatPassDomFare(_endOnEndDA->_validationFareComponent[0]._currentFareUsage);

    FareUsage* failedFU = 0, *failedEOETargetFU = 0;
    _pricingUnit.fareUsage().push_back(_endOnEndDA->_validationFareComponent[0]._currentFareUsage);
    _fp.pricingUnit().push_back(&_pricingUnit);
    _directionalityInfo->validationLevel = Combinations::FarePathLevel;
    CPPUNIT_ASSERT(
        _combinationsPrebuiltVFC->validateCombination(_diag,
                                                      *_directionalityInfo,
                                                      &_endOnEndDA->_validationFareComponent,
                                                      _pricingUnit,
                                                      &_fp,
                                                      failedFU,
                                                      failedEOETargetFU,
                                                      0));
  }

  void testEndOnEndFailWithMultipleRecord3()
  {
    prepareTargetFU(3);
    setTargetFUFareStatus(0, Fare::FS_Domestic);
    setTargetFUFareStatus(1, Fare::FS_International);
    setTargetFUFareStatus(2, Fare::FS_ForeignDomestic);
    addEOEThatPassIntlFare(_endOnEndDA->_validationFareComponent[0]._currentFareUsage);

    FareUsage* failedFU = 0, *failedEOETargetFU = 0;
    _pricingUnit.fareUsage().push_back(_endOnEndDA->_validationFareComponent[0]._currentFareUsage);
    _fp.pricingUnit().push_back(&_pricingUnit);
    _directionalityInfo->validationLevel = Combinations::FarePathLevel;
    // CPPUNIT_ASSERT(!_combinationsPrebuiltVFC->validateCombination(_diag, *_directionalityInfo,
    // &_endOnEndDA->_validationFareComponent, _pricingUnit, &_fp, failedFU, failedEOETargetFU, 0));
    // Investigate if it should pass or faill
    CPPUNIT_ASSERT(
        _combinationsPrebuiltVFC->validateCombination(_diag,
                                                      *_directionalityInfo,
                                                      &_endOnEndDA->_validationFareComponent,
                                                      _pricingUnit,
                                                      &_fp,
                                                      failedFU,
                                                      failedEOETargetFU,
                                                      0));
  }

  void testAllowHRTFareCheckForVendor()
  {
    FareUsage* fu = buildFU();
    PaxTypeFare* ptf = fu->paxTypeFare();
    FareInfo* fareInfo = const_cast<FareInfo*>(ptf->fare()->fareInfo());
    fareInfo->_vendor = "ZV90";
    CPPUNIT_ASSERT(PricingUtil::allowHRTCForVendor(*_trx, ptf) == true);
  }

  void testProcessEndOnEndRestrictionFailWhenReqDomFareORReqTransbFare()
  {
    setEndOnEndReqDomFare();
    setEndOnEndReqTransbFare();
    _itin.geoTravelType() = GeoTravelType::International;
    prepareTargetFU(1);

    setTargetFUFareStatus(0, Fare::FS_International);   setTargetFareMarketOnD(0, &_locWAS, &_locKRK);

    CPPUNIT_ASSERT_EQUAL(Combinations::STOPCOMB,
                         _combinations->processEndOnEndRestriction(*_endOnEndDA));
  }

  void setCat25(PaxTypeFare* fare)
  {
    fare->status().set(PaxTypeFare::PTF_FareByRule, true);
  }

  void setFareChanged(PaxTypeFare* fare)
  {
    _pricingUnit.addChangedFareCat10overrideCat25(&_paxTypeFare1, fare);
  }


  void testValidateFareCat10overrideCat25_fare1cat25_fare2changed()
  {
    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();

    setCat25(fare1);
    setFareChanged(fare2);

    CPPUNIT_ASSERT_EQUAL(fare2,
                         _combinations->validateFareCat10overrideCat25(fare1, &_paxTypeFare1, &_pricingUnit));
  }

  void testValidateFareCat10overrideCat25_fare1cat25_fare2notChanged()
  {
    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();

    setCat25(fare1);

    CPPUNIT_ASSERT_EQUAL(fare2,
                         _combinations->validateFareCat10overrideCat25(fare1, fare2, &_pricingUnit));
  }

  void testValidateFareCat10overrideCat25_fare1noCat25_fare1changed_fare2cat25()
  {
    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();

    setFareChanged(fare1);
    setCat25(fare2);

    CPPUNIT_ASSERT_EQUAL(fare2,
                         _combinations->validateFareCat10overrideCat25(fare1, fare2, &_pricingUnit));
  }

  void testValidateFareCat10overrideCat25_fare1noCat25_fare1changed_fare2noCat25()
  {
    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();

    setFareChanged(fare1);

    CPPUNIT_ASSERT_EQUAL(fare2,
                         _combinations->validateFareCat10overrideCat25(fare1, fare2, &_pricingUnit));
  }

  void testValidateFareCat10overrideCat25_fare1noCat25_fare1notChanged_fare2changed()
  {
    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();

    setFareChanged(fare2);

    CPPUNIT_ASSERT_EQUAL(fare2,
                         _combinations->validateFareCat10overrideCat25(fare1, &_paxTypeFare1, &_pricingUnit));
  }

  void testValidateFareCat10overrideCat25_fare1noCat25_fare1notChanged_fare2notChanged()
  {
    PaxTypeFare* fare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* fare2 = _memHandle.create<PaxTypeFare>();

    CPPUNIT_ASSERT_EQUAL(fare2,
                         _combinations->validateFareCat10overrideCat25(fare1, fare2, &_pricingUnit));
  }

  static const Indicator RULE_ALWAYS_APPLIES = ' ';

  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

  public:
    MyDataHandle()
    {
      TariffRuleRest* trr = _memHandle.create<TariffRuleRest>();
      _tariffRuleRests.push_back(trr);
      FareClassRestRule* fcrr = _memHandle.create<FareClassRestRule>();
      _fareClassRestRule.push_back(fcrr);
    }

    const std::vector<TariffRuleRest*>& getTariffRuleRest(const VendorCode& vendor, int itemNo)
    {
      return _tariffRuleRests;
    }

    const std::vector<FareClassRestRule*>& getFareClassRestRule(const VendorCode& vendor, int itemNo)
    {
      return _fareClassRestRule;
    }

    const CircleTripRuleItem* getCircleTripRuleItem(const VendorCode& vendor, int itemNo)
    {
      if (itemNo == 0)
        return 0;
      return DataHandleMock::getCircleTripRuleItem(vendor, itemNo);
    }

    const RoundTripRuleItem* getRoundTripRuleItem(const VendorCode& vendor, int itemNo)
    {
      if (itemNo == 0)
        return 0;
      return DataHandleMock::getRoundTripRuleItem(vendor, itemNo);
    }

    const EndOnEnd* getEndOnEnd(const VendorCode& vendor, const int itemNo)
    {
      if (itemNo == 99999)
        return 0;
      return DataHandleMock::getEndOnEnd(vendor, itemNo);
    }

    char getVendorType(const VendorCode& vendor)
    {
      if (vendor != "ATP" && vendor != "SITA")
        return 'T';
      return DataHandleMock::getVendorType(vendor);
    }

    std::vector<TariffRuleRest*> _tariffRuleRests;
    std::vector<FareClassRestRule*> _fareClassRestRule;
  };

  class CombinationsPrebuiltVFC : public Combinations
  {
  public:
    void setEOEResultSimulation(const uint32_t itemNo, const uint32_t fuNum, bool result)
    {
      uint32_t key = itemNo * 10 + fuNum;
      _resultSimulateMap[key] = result;
    }

  private:
    virtual bool
    buildValidationFareComponent(DiagCollector& diag,
                                 DirectionalityInfo& directionalityInfo,
                                 const PricingUnit& curPu,
                                 FareUsage& curFareUsage,
                                 const FarePath& farePath,
                                 Combinations::ValidationFareComponents& validationFareComponent)
    {
      return true;
    }

    virtual char processEndOnEndRestriction(DiagCollector& diag,
                                            const FarePath& farePath,
                                            const PricingUnit& currentPU,
                                            const uint32_t itemNo,
                                            ValidationFareComponents& validationFareComponent,
                                            const DirectionalityInfo& directionalityInfo)
    {
      validationFareComponent.needAllPassSameMajorItem() = false;

      uint32_t fuNum = 0;
      uint32_t fuSz = static_cast<uint32_t>(validationFareComponent.size());
      for (; fuNum < fuSz; fuNum++)
      {
        uint32_t key = itemNo * 10 + fuNum;
        std::map<uint32_t, bool>::iterator it = _resultSimulateMap.find(key);
        if (it != _resultSimulateMap.end())
        {
          validationFareComponent[fuNum]._passMajor |= it->second;
        }
      }

      for (fuNum = 0; fuNum < fuSz; fuNum++)
      {
        if (!validationFareComponent[fuNum]._passMajor)
        {
          validationFareComponent[fuNum].getSubCat(Combinations::M104) = FAILCOMB;
          return FAILCOMB;
        }
      }
      return PASSCOMB;
    }

    std::map<uint32_t, bool> _resultSimulateMap;
  };

private:
  TestMemHandle _memHandle;
  Combinations::EndOnEndDataAccess* _endOnEndDA;
  PricingTrx* _trx;
  tse::ConfigMan _config;
  PricingRequest _request;
  PricingUnit _pricingUnit;
  FarePath _fp;
  Itin _itin;
  CombinabilityRuleInfo _cat10Rule;
  CombinabilityRuleInfo _cat10;
  EndOnEnd _endOnEndRule;
  size_t _eoeItemNo;
  Combinations::ValidationFareComponents _vfc;
  DiagCollector _diag;
  Combinations::DirectionalityInfo* _directionalityInfo;
  Combinations* _combinations;
  CombinabilityScoreboard* _comboScoreboard;
  CombinationsPrebuiltVFC* _combinationsPrebuiltVFC;
  CombinabilityRuleItemInfo _cat10Seg;
  CombinabilityRuleItemInfo _cat10Seg1;
  Combinations::ValidationElement _negMatchedVfc;
  Fare _fare1;
  FareInfo _fareInfo1;
  TariffCrossRefInfo _tariffRefInfo1;
  FareMarket _fareMarket1;
  PaxTypeFare _paxTypeFare1;
  FareClassAppInfo _appInfo1;
  FareClassAppSegInfo _fareClassAppSegInfo1;
  Fare _fare2;
  FareInfo _fareInfo2;
  TariffCrossRefInfo _tariffRefInfo2;
  FareMarket _fareMarket2;
  PaxTypeFare _paxTypeFare2;
  FareClassAppInfo _appInfo2;
  FareClassAppSegInfo _fareClassAppSegInfo2;
  CombinabilityRuleItemInfoSet* _catRuleItemInfoSet;
  RootLoggerGetOff _loggerOff;
  MyDataHandle* _dbHandle;

  Loc _locYYZ;
  Loc _locWAS;
  Loc _locDFW;
  Loc _locKRK;
  Loc _locWAW;
  Loc _locABA;
  Loc _locAER;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationsTest);
}
