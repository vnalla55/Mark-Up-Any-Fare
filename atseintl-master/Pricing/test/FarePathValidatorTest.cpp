//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/ClassOfService.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagCollector.h"

#include "Pricing/FarePathValidator.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/PUPath.h"
#include "Pricing/test/FactoriesConfigStub.h"
#include "Pricing/test/PricingMockDataBuilder.h"
#include "Pricing/BCMixedClassValidator.h"
#include "test/include/CppUnitHelperMacros.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FarePathValidatorTest : public CppUnit::TestFixture
{
  class FactoriesConfigStub : public FactoriesConfig
  {
  };
  CPPUNIT_TEST_SUITE(FarePathValidatorTest);

  CPPUNIT_TEST(testExchangeReissueForQrexWhenItsNotRequestedNonCat31);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenItsNotRequestedCat31);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenItsNotRequestedExcIS);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedNonCat31Reissue);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedCat31Reissue);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedExcISReissue);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedNonCat31Exchange);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedCat31Exchange);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedExcISExchange);
  CPPUNIT_TEST(testExchangeReissueForQrexWhenITSRequestedForAFExchange);

  CPPUNIT_TEST(testCheckReissueExchangeIndForRebookFPNoInventoryChangeAndExchange);
  CPPUNIT_TEST(
      testCheckReissueExchangeIndForRebookFPRebookInventoryChangeBkgNotAsInExchItinAndExchange);
  CPPUNIT_TEST(
      testCheckReissueExchangeIndForRebookFPRebookInventoryChangeBkgAndRebookAsInExchItinAndExchange);
  CPPUNIT_TEST(
      testCheckReissueExchangeIndForRebookFPRebookInventoryChangeBkgNotAsInExchItinAndReissue);
  CPPUNIT_TEST(
      testCheckReissueExchangeIndForRebookFPRebookStatusUnchangedBkgEqualToInExchItinAndReissue);
  CPPUNIT_TEST(
      testCheckReissueExchangeIndForRebookFPRebookStatusUnchangedBkgNotAsInExchItinAndReissue);

  CPPUNIT_TEST(testHardPassOnEachLegCheckIBF);
  CPPUNIT_TEST(testAtLeastOneHardPass);

  CPPUNIT_TEST(testisValidFPathForValidatingCxr_NoValidatingCxr);
  CPPUNIT_TEST(testisValidFPathForValidatingCxr_ValidCxrList);
  CPPUNIT_TEST(testisValidFPathForValidatingCxr_ShouldFail);

  CPPUNIT_TEST(testisTag1Tag1_Tag1Tag1);
  CPPUNIT_TEST(testisTag1Tag1_Tag1_NotTag1);
  CPPUNIT_TEST(testisTag1Tag1_NotTag1_Tag1);
  CPPUNIT_TEST(testisTag3Tag3_Tag3Tag3);
  CPPUNIT_TEST(testisTag3Tag3_Tag3_NotTag3);
  CPPUNIT_TEST(testisTag3Tag3_NotTag3_Tag3);
  CPPUNIT_TEST(testisAnyTag2_Tag1Tag1);
  CPPUNIT_TEST(testisAnyTag2_Tag1Tag2);
  CPPUNIT_TEST(testisAnyTag2_Tag1Tag3);
  CPPUNIT_TEST(testisAnyTag2_Tag2Tag1);
  CPPUNIT_TEST(testisAnyTag2_Tag2Tag2);
  CPPUNIT_TEST(testisAnyTag2_Tag2Tag3);
  CPPUNIT_TEST(testisAnyTag2_Tag3Tag1);
  CPPUNIT_TEST(testisAnyTag2_Tag3Tag2);
  CPPUNIT_TEST(testisAnyTag2_Tag3Tag3);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_Default);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_BothPuNo_UpperCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_BothPuNo_LowerCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_BothPuYes_UpperCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_BothPuYes_LowerCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_FirstPuYes_UpperCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_FirstPuYes_LowerCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_SecondPuYes_UpperCase);
  CPPUNIT_TEST(testcheckTag1Tag3CarrierPreference_SecondPuYes_LowerCase);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_NoOneWay);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag2);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag1_International);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag1_ForeignDomestic);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag1_UnknownTravelType);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag1_Domestic);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag1_Transborder);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag3Tag3);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag3_SameFareType);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag3Tag1_SameFareType);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag3_CombinationNotAllowedViaCarrierPreference);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag3Tag1_CombinationNotAllowedViaCarrierPreference);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag1Tag3_CombinationAllowedViaCarrierPreference);
  CPPUNIT_TEST(testcheckTag1Tag3FareInABAItin_Tag3Tag1_CombinationAllowedViaCarrierPreference);

  CPPUNIT_TEST(testRecalculatePriority);

  CPPUNIT_TEST(testFailMixedDateForRefund_AllHist);
  CPPUNIT_TEST(testFailMixedDateForRefund_AllCommence);
  CPPUNIT_TEST(testFailMixedDateForRefund_Mixed1);
  CPPUNIT_TEST(testFailMixedDateForRefund_Mixed2);
  CPPUNIT_TEST(testFailMixedDateForRefund_Mixed3);
  CPPUNIT_TEST(testFailMixedDateForRefund_Mixed4);
  CPPUNIT_TEST(testFailMixedDateForRefund_Mixed5);

  CPPUNIT_TEST_SUITE_END();

  void setsUpForRexRebookExchangeReissue(RexPricingTrx& trx)
  {
    putTravelSegsInFareUsages();
    _farePath->rebookClassesExists() = true;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    ExcItin* excItin = static_cast<ExcItin*>(_trx->itin()[0]);
    trx.exchangeItin().push_back(excItin);
    createValidator(&trx);
    _farePath->itin() = trx.newItin().front();
  }

  void putTravelSegsInFareUsages()
  {
    _itin = PricingMockDataBuilder::addItin(*_trx);

    _trx->itin().push_back(_itin);
    _fu1->segmentStatus().resize(1);
    _fu1->segmentStatus()[0]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    _fu1->segmentStatus()[0]._bkgCodeReBook = "Q";
    Loc* loc1 = PricingMockDataBuilder::getLoc(*_trx, "DFW");
    Loc* loc2 = PricingMockDataBuilder::getLoc(*_trx, "NYC");
    TravelSeg* tSeg1 = PricingMockDataBuilder::addTravelSeg(*_trx, *_itin, "AA", loc1, loc2, 1);
    tSeg1->setBookingCode("Y");
    _fu1->travelSeg().push_back(tSeg1);
  }

  void createValidator(PricingTrx* trx)
  {
    _baseData->trx() = trx;
    _validator = _memHandle.create<FarePathValidator>(*_baseData);
  }

  void preparePUPath()
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    _fu1->paxTypeFare() = ptf;
    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    _fu2->paxTypeFare() = ptf2;

    Itin* itin = addItin();
    _trx->itin().push_back(itin);
    _farePath->itin() = itin;

    PUPath* puPath = PricingMockDataBuilder::getPUPath(*_trx);
    PricingMockDataBuilder::addRTPUToMainTrip(
        *_trx, *puPath, itin->fareMarket()[0], itin->fareMarket()[3]);
    PricingMockDataBuilder::addRTPUToMainTrip(
        *_trx, *puPath, itin->fareMarket()[1], itin->fareMarket()[2]);
    puPath->eoePUAvailable().push_back(true);
    puPath->eoePUAvailable().push_back(true);
    createValidator(_trx);
    _validator->_puPath = puPath;
  }
  Itin* addItin()
  {
    Itin* itin = PricingMockDataBuilder::addItin(*_trx);

    // DFW-AA-NYC-BA-LON-BA-NYC-AA-DFW
    Loc* loc1 = PricingMockDataBuilder::getLoc(*_trx, "DFW");
    Loc* loc2 = PricingMockDataBuilder::getLoc(*_trx, "NYC");
    Loc* loc3 = PricingMockDataBuilder::getLoc(*_trx, "LON");

    TravelSeg* tSeg1 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "AA", loc1, loc2, 1);
    TravelSeg* tSeg2 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "BA", loc2, loc3, 2);

    TravelSeg* tSeg3 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "BA", loc3, loc2, 3);
    TravelSeg* tSeg4 = PricingMockDataBuilder::addTravelSeg(*_trx, *itin, "AA", loc2, loc1, 4);

    FareMarket* fm1 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "AA", loc1, loc2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fm1);

    FareMarket* fm2 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "BA", loc2, loc3);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fm2);

    FareMarket* fm3 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "BA", loc3, loc2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fm3);

    FareMarket* fm4 = PricingMockDataBuilder::addFareMarket(*_trx, *itin, "AA", loc2, loc1);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg1, *fm1);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg2, *fm2);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg3, *fm3);
    PricingMockDataBuilder::addTraveSegToFareMarket(tSeg4, *fm4);

    return itin;
  }

public:
  FarePathValidatorTest() {}
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _baseData = _memHandle.create<PaxFPFBaseData>(_factoriesConfig);
    _farePath = _memHandle.create<FarePath>();
    _trx = PricingMockDataBuilder::getPricingTrx();
    _fu1 = _memHandle.create<FareUsage>();
    _fu2 = _memHandle.create<FareUsage>();

    _pu1 = _memHandle.create<PricingUnit>();
    _pu2 = _memHandle.create<PricingUnit>();
    PaxTypeFare* paxTypeFare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* paxTypeFare2 = _memHandle.create<PaxTypeFare>();
    _fu1->paxTypeFare() = paxTypeFare1;
    _fu2->paxTypeFare() = paxTypeFare2;

    PaxTypeFare* paxTypeFare3 = _memHandle.create<PaxTypeFare>();
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = paxTypeFare3;

    _pu1->fareUsage().push_back(_fu1);
    _pu1->fareUsage().push_back(_fu2);
    _pu2->fareUsage().push_back(fareUsage);
    _farePath->pricingUnit().push_back(_pu1);
    _farePath->pricingUnit().push_back(_pu2);
    createValidator(_trx);
  }

  void setUpAllRetrievalInfos(FareMarket::FareRetrievalFlags flag)
  {
    RefundPricingTrx* rtrx = _memHandle.insert(new RefundPricingTrx);
    rtrx->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    createValidator(rtrx);
    _ptf11 = _memHandle.create<PaxTypeFare>();
    _ptf12 = _memHandle.create<PaxTypeFare>();
    _ptf21 = _memHandle.create<PaxTypeFare>();
    _ptf22 = _memHandle.create<PaxTypeFare>();


    FareUsage* fu21 = _memHandle.create<FareUsage>();
    FareUsage* fu22 = _memHandle.create<FareUsage>();

    _pu2->fareUsage().clear();
    _pu2->fareUsage().push_back(fu21);
    _pu2->fareUsage().push_back(fu22);

    _fu1->paxTypeFare() = _ptf11;
    _fu2->paxTypeFare() = _ptf12;
    fu21->paxTypeFare() = _ptf21;
    fu22->paxTypeFare() = _ptf22;

    _ptf11->retrievalInfo() = createRetrievalInfo(flag);
    _ptf12->retrievalInfo() = createRetrievalInfo(flag);
    _ptf21->retrievalInfo() = createRetrievalInfo(flag);
    _ptf22->retrievalInfo() = createRetrievalInfo(flag);
  }

  FareMarket::RetrievalInfo* createRetrievalInfo(FareMarket::FareRetrievalFlags flag)
  {
    FareMarket::RetrievalInfo* info = _memHandle.insert(new FareMarket::RetrievalInfo);
    info->_flag = flag;
    return info;
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _trx;
    _trx = nullptr;
  }

  void testExchangeReissueForQrexWhenItsNotRequestedNonCat31()
  {
    ExchangePricingTrx trx;
    trx.setExcTrxType(PricingTrx::PORT_EXC_TRX);
    createValidator(&trx);
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == BLANK);
  }

  void testExchangeReissueForQrexWhenItsNotRequestedCat31()
  {
    RexPricingTrx trx;
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
    _baseData->trx() = pricingTrx;
    _validator = _memHandle.create<FarePathValidator>(*_baseData);
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == BLANK);
  }

  void testExchangeReissueForQrexWhenItsNotRequestedExcIS()
  {
    ExchangePricingTrx trx;
    trx.setExcTrxType(PricingTrx::EXC_IS_TRX);
    createValidator(&trx);
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == BLANK);
  }

  void testExchangeReissueForQrexWhenITSRequestedNonCat31Reissue()
  {
    putTravelSegsInFareUsages();
    ExchangePricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::PORT_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = REISSUE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == '1');
  }

  void testExchangeReissueForQrexWhenITSRequestedCat31Reissue()
  {
    putTravelSegsInFareUsages();
    RexPricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = REISSUE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == '1');
  }

  void testExchangeReissueForQrexWhenITSRequestedExcISReissue()
  {
    putTravelSegsInFareUsages();
    ExchangePricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::EXC_IS_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = REISSUE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == '1');
  }

  void testExchangeReissueForQrexWhenITSRequestedNonCat31Exchange()
  {
    putTravelSegsInFareUsages();
    ExchangePricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::PORT_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == '2');
  }

  void testExchangeReissueForQrexWhenITSRequestedCat31Exchange()
  {
    putTravelSegsInFareUsages();
    RexPricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == '2');
  }

  void testExchangeReissueForQrexWhenITSRequestedExcISExchange()
  {
    putTravelSegsInFareUsages();
    ExchangePricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::EXC_IS_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == '2');
  }

  void testExchangeReissueForQrexWhenITSRequestedForAFExchange()
  {
    putTravelSegsInFareUsages();
    ExchangePricingTrx trx;
    Itin* newItin = _trx->itin()[0];
    trx.itin().push_back(newItin);
    trx.setExcTrxType(PricingTrx::AF_EXC_TRX);
    trx.setRexPrimaryProcessType('A');
    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    createValidator(&trx);
    _farePath->itin() = newItin;
    _validator->exchangeReissueForQrex(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == BLANK);
  }

  void testCheckReissueExchangeIndForRebookFPNoInventoryChangeAndExchange()
  {
    RexPricingTrx trx;
    setsUpForRexRebookExchangeReissue(trx);

    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    _validator->exchangeReissueForQrex(*_farePath);
    _bcMixedClassValidator = _memHandle.create<BCMixedClassValidator>(trx);

    _bcMixedClassValidator->checkReissueExchangeIndForRebookFP(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == EXCHANGE);
  }

  void testCheckReissueExchangeIndForRebookFPRebookInventoryChangeBkgNotAsInExchItinAndExchange()
  {
    RexPricingTrx trx;
    setsUpForRexRebookExchangeReissue(trx);

    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    trx.newItin().front()->travelSeg()[0]->changeStatus() = TravelSeg::INVENTORYCHANGED;

    _validator->exchangeReissueForQrex(*_farePath);
    _bcMixedClassValidator = _memHandle.create<BCMixedClassValidator>(trx);

    _bcMixedClassValidator->checkReissueExchangeIndForRebookFP(*_farePath);
    CPPUNIT_ASSERT(_farePath->exchangeReissue() == EXCHANGE);
  }

  void
  testCheckReissueExchangeIndForRebookFPRebookInventoryChangeBkgAndRebookAsInExchItinAndExchange()
  {
    RexPricingTrx trx;
    setsUpForRexRebookExchangeReissue(trx);

    trx.newItin().front()->exchangeReissue() = EXCHANGE;
    trx.newItin().front()->travelSeg()[0]->changeStatus() = TravelSeg::INVENTORYCHANGED;
    trx.exchangeItin().front()->travelSeg()[0]->setBookingCode("Q");

    _validator->exchangeReissueForQrex(*_farePath);

    _bcMixedClassValidator = _memHandle.create<BCMixedClassValidator>(trx);
    _bcMixedClassValidator->checkReissueExchangeIndForRebookFP(*_farePath);

    CPPUNIT_ASSERT(_farePath->exchangeReissue() == REISSUE);
  }

  void testCheckReissueExchangeIndForRebookFPRebookInventoryChangeBkgNotAsInExchItinAndReissue()
  {
    RexPricingTrx trx;
    setsUpForRexRebookExchangeReissue(trx);

    trx.newItin().front()->exchangeReissue() = REISSUE;
    trx.newItin().front()->travelSeg()[0]->changeStatus() = TravelSeg::INVENTORYCHANGED;

    _validator->exchangeReissueForQrex(*_farePath);

    _bcMixedClassValidator = _memHandle.create<BCMixedClassValidator>(trx);
    _bcMixedClassValidator->checkReissueExchangeIndForRebookFP(*_farePath);

    CPPUNIT_ASSERT(_farePath->exchangeReissue() == REISSUE);
  }

  void testCheckReissueExchangeIndForRebookFPRebookStatusUnchangedBkgEqualToInExchItinAndReissue()
  {
    RexPricingTrx trx;
    setsUpForRexRebookExchangeReissue(trx);

    trx.newItin().front()->exchangeReissue() = REISSUE;
    trx.newItin().front()->travelSeg()[0]->changeStatus() = TravelSeg::UNCHANGED;
    trx.exchangeItin().front()->travelSeg()[0]->setBookingCode("Q");

    _validator->exchangeReissueForQrex(*_farePath);

    _bcMixedClassValidator = _memHandle.create<BCMixedClassValidator>(trx);
    _bcMixedClassValidator->checkReissueExchangeIndForRebookFP(*_farePath);

    CPPUNIT_ASSERT(_farePath->exchangeReissue() == REISSUE);
  }

  void testCheckReissueExchangeIndForRebookFPRebookStatusUnchangedBkgNotAsInExchItinAndReissue()
  {
    RexPricingTrx trx;
    setsUpForRexRebookExchangeReissue(trx);

    trx.newItin().front()->exchangeReissue() = REISSUE;
    trx.newItin().front()->travelSeg()[0]->changeStatus() = TravelSeg::UNCHANGED;

    _validator->exchangeReissueForQrex(*_farePath);

    _bcMixedClassValidator = _memHandle.create<BCMixedClassValidator>(trx);
    _bcMixedClassValidator->checkReissueExchangeIndForRebookFP(*_farePath);

    CPPUNIT_ASSERT(_farePath->exchangeReissue() == EXCHANGE);
  }

  void testHardPassOnEachLegCheckIBF()
  {
    preparePUPath();
    _validator->_puPath->puPath().front()->fareMarket().front()->brandCode() = NO_BRAND;
    _validator->_puPath->setBrandCode(NO_BRAND);
    CPPUNIT_ASSERT(_validator->hardPassOnEachLegCheckIBF(*_farePath));

    TravelSeg* tSeg1 = _farePath->itin()->travelSeg().front();
    _validator->_itin = _farePath->itin();

    _validator->_puPath->puPath().front()->fareMarket().front()->brandCode() = "PS";
    _validator->_puPath->setBrandCode("PS");
    _farePath->itin()->itinLegs().push_back(std::vector<TravelSeg*>(1, tSeg1));
    CPPUNIT_ASSERT(!_validator->hardPassOnEachLegCheckIBF(*_farePath));

    TravelSeg* tSeg2 = _farePath->itin()->travelSeg().back();
    tSeg2->segmentType() = Air;

    tSeg1->segmentType() = Arunk;
    _farePath->itin()->itinLegs().push_back(std::vector<TravelSeg*>(1, tSeg2));

    CPPUNIT_ASSERT(!_validator->hardPassOnEachLegCheckIBF(*_farePath));

    tSeg2->segmentType() = Arunk;
    CPPUNIT_ASSERT(_validator->hardPassOnEachLegCheckIBF(*_farePath));
  }

  void testAtLeastOneHardPass()
  {
    preparePUPath();
    TravelSeg* tSeg1 = _farePath->itin()->travelSeg().front();

    FareUsage* fu = _farePath->pricingUnit().front()->fareUsage().front();
    fu->travelSeg().push_back(tSeg1);

    FareMarket* fm = _farePath->itin()->fareMarket().front();
    fu->paxTypeFare()->fareMarket() = fm;

    tSeg1->segmentType() = Arunk;

    TravelSegPtrVec tSegs;
    tSegs.push_back(tSeg1);
    std::string brandCode = "PS";

    CPPUNIT_ASSERT(_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    tSeg1->segmentType() = Air;
    brandCode = NO_BRAND;
    CPPUNIT_ASSERT(_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    brandCode = "PS";
    CPPUNIT_ASSERT(!_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    fm->brandProgramIndexVec().push_back(0);
    fu->paxTypeFare()->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));

    BrandProgram bProgram1;
    BrandInfo brand1;
    brand1.brandCode() = "PS";
    _trx->brandProgramVec().push_back(std::make_pair(&bProgram1, &brand1));

    CPPUNIT_ASSERT(_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    fu->paxTypeFare()->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_SOFT_PASS;
    CPPUNIT_ASSERT(!_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    fu->paxTypeFare()->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_FAIL;
    CPPUNIT_ASSERT(!_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    // Check with two FUs
    FareUsage* fu2 = _farePath->pricingUnit().front()->fareUsage().back();
    TravelSeg* tSeg2 = _farePath->itin()->travelSeg().back();
    FareMarket* fm2 = _farePath->itin()->fareMarket().back();

    fu2->travelSeg().push_back(tSeg2);
    fu2->paxTypeFare()->fareMarket() = fm2;

    tSegs.push_back(tSeg2);
    tSeg2->segmentType() = Arunk;

    CPPUNIT_ASSERT(!_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    tSeg2->segmentType() = Air;
    CPPUNIT_ASSERT(!_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));

    fu->paxTypeFare()->getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_HARD_PASS;
    CPPUNIT_ASSERT(_validator->atLeastOneHardPass(tSegs, *_farePath, brandCode));
  }

  void testisValidFPathForValidatingCxr_NoValidatingCxr()
  {
    _trx->setValidatingCxrGsaApplicable(true);
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;

    _pu1->validatingCarriers().clear();
    _pu2->validatingCarriers().clear();
    CPPUNIT_ASSERT(_validator->isValidFPathForValidatingCxr(*_farePath, diag) == true);
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testisValidFPathForValidatingCxr_ValidCxrList()
  {
    _trx->setValidatingCxrGsaApplicable(false);
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;

    CarrierCode set1[] = {"AA", "AB", "DL", "EK", "BA"};
    CarrierCode set2[] = {"AB", "AA", "AF", "EK"};

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 5);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 4);

    _pu1->validatingCarriers() = list1;
    _pu2->validatingCarriers() = list2;

    CPPUNIT_ASSERT(_validator->isValidFPathForValidatingCxr(*_farePath, diag) == true);
  }

  void testisValidFPathForValidatingCxr_ShouldFail()
  {
    _trx->setValidatingCxrGsaApplicable(true);
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;

    CarrierCode set1[] = {"AB", "DL", "EK", "BA"};
    CarrierCode set2[] = {"AL", "AA", "AF", "JJ"};

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 4);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 4);

    _pu1->validatingCarriers() = list1;
    _pu2->validatingCarriers() = list2;
    CPPUNIT_ASSERT(_validator->isValidFPathForValidatingCxr(*_farePath, diag) == false);
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }
  void setFareAndFareInfo(PaxTypeFare* ptf)
  {
    ptf->setFare(_memHandle.create<Fare>());
    ptf->fare()->setFareInfo(_memHandle.create<FareInfo>());
  }

  void setTag1(PaxTypeFare* ptf)
  {
    FareInfo* fareInfo = const_cast<FareInfo*>(ptf->fare()->fareInfo());
    fareInfo->owrt() = ONE_WAY_MAY_BE_DOUBLED;
  }

  void setTag2(PaxTypeFare* ptf)
  {
    FareInfo* fareInfo = const_cast<FareInfo*>(ptf->fare()->fareInfo());
    fareInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
  }

  void setTag3(PaxTypeFare* ptf)
  {
    FareInfo* fareInfo = const_cast<FareInfo*>(ptf->fare()->fareInfo());
    fareInfo->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
  }
  CarrierPreference* createCarrierPreference(Indicator noApplyCombTag1AndTag3 = ' ')
  {
    CarrierPreference* cp = _memHandle.create<CarrierPreference>();
    cp->noApplycombtag1and3() = noApplyCombTag1AndTag3;
    return cp;
  }

  FareMarket* createFareMarketWithCarrierPreference(Indicator noApplyCombTag1AndTag3 = ' ')
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->governingCarrierPref() = createCarrierPreference(noApplyCombTag1AndTag3);
    return fm;
  }

  void testisTag1Tag1_Tag1Tag1()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(_validator->isTag1Tag1(*_pu1, *_pu2));
  }
  void testisTag1Tag1_Tag1_NotTag1()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag2(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag1Tag1(*_pu1, *_pu2));

    setTag1(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag1Tag1(*_pu1, *_pu2));
  }

  void testisTag1Tag1_NotTag1_Tag1()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag2(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag1Tag1(*_pu1, *_pu2));

    setTag3(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag1Tag1(*_pu1, *_pu2));
  }

  void testisTag3Tag3_Tag3Tag3()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(_validator->isTag3Tag3(*_pu1, *_pu2));
  }

  void testisTag3Tag3_Tag3_NotTag3()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag3Tag3(*_pu1, *_pu2));

    setTag3(ptf1);
    setTag2(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag3Tag3(*_pu1, *_pu2));
  }

  void testisTag3Tag3_NotTag3_Tag3()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag3Tag3(*_pu1, *_pu2));

    setTag2(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->isTag3Tag3(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag1Tag1()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag1Tag2()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag2(ptf2);
    CPPUNIT_ASSERT(_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag1Tag3()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag2Tag1()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag2(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag2Tag2()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag2(ptf1);
    setTag2(ptf2);
    CPPUNIT_ASSERT(_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag2Tag3()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag2(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag3Tag1()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag3Tag2()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag2(ptf2);
    CPPUNIT_ASSERT(_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testisAnyTag2_Tag3Tag3()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->isAnyTag2(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_Default()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference();

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference();

    CPPUNIT_ASSERT(_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_BothPuNo_UpperCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('N');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('N');

    CPPUNIT_ASSERT(_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_BothPuNo_LowerCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('n');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('n');

    CPPUNIT_ASSERT(_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_BothPuYes_UpperCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('Y');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('Y');

    CPPUNIT_ASSERT(!_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_BothPuYes_LowerCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('y');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('y');

    CPPUNIT_ASSERT(!_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_FirstPuYes_UpperCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('Y');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('N');

    CPPUNIT_ASSERT(!_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_FirstPuYes_LowerCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('y');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('n');

    CPPUNIT_ASSERT(!_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_SecondPuYes_UpperCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('N');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('Y');

    CPPUNIT_ASSERT(!_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3CarrierPreference_SecondPuYes_LowerCase()
  {
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('n');

    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('y');

    CPPUNIT_ASSERT(!_validator->checkTag1Tag3CarrierPreference(*_pu1, *_pu2));
  }

  void testcheckTag1Tag3FareInABAItin_NoOneWay()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = false;
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag2()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag2(ptf1);
    setTag2(ptf2);
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag1_International()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::International;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse(" FAILED: TAG1-TAG1 COMBINATION\n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag1_ForeignDomestic()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::ForeignDomestic;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse(" FAILED: TAG1-TAG1 COMBINATION\n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag1_UnknownTravelType()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag1_Domestic()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Domestic;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag1_Transborder()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag3Tag3()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    PUPath* puPath = _validator->_puPath;

    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse(" FAILED: TAG3-TAG3 COMBINATION\n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag3_SameFareType()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    _pu1->puFareType() = PricingUnit::SP;
    _pu2->puFareType() = PricingUnit::SP;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag3(ptf2);
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("FAILED: TAG1-TAG3 All NL OR SP \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());

    diag.clear();
    _pu1->puFareType() = PricingUnit::NL;
    _pu2->puFareType() = PricingUnit::NL;
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag3Tag1_SameFareType()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    _pu1->puFareType() = PricingUnit::NL;
    _pu2->puFareType() = PricingUnit::NL;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag1(ptf2);
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("FAILED: TAG1-TAG3 All NL OR SP \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());

    diag.clear();
    _pu1->puFareType() = PricingUnit::SP;
    _pu2->puFareType() = PricingUnit::SP;
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag3_CombinationNotAllowedViaCarrierPreference()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    _pu1->puFareType() = PricingUnit::NL;
    _pu2->puFareType() = PricingUnit::SP;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag3(ptf2);
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('Y');
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('Y');
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("FAILED: TAG1-TAG3 CXR PREF \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());

    diag.clear();
    _pu1->puFareType() = PricingUnit::SP;
    _pu2->puFareType() = PricingUnit::NL;
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag3Tag1_CombinationNotAllowedViaCarrierPreference()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    _pu1->puFareType() = PricingUnit::NL;
    _pu2->puFareType() = PricingUnit::SP;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag1(ptf2);
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('Y');
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('Y');
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("FAILED: TAG1-TAG3 CXR PREF \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());

    diag.clear();
    _pu1->puFareType() = PricingUnit::SP;
    _pu2->puFareType() = PricingUnit::NL;
    CPPUNIT_ASSERT(!_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag1Tag3_CombinationAllowedViaCarrierPreference()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;
    PUPath* puPath = _validator->_puPath;
    ;
    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    _pu1->puFareType() = PricingUnit::NL;
    _pu2->puFareType() = PricingUnit::SP;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag1(ptf1);
    setTag3(ptf2);
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('N');
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('N');
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());

    diag.clear();
    _pu1->puFareType() = PricingUnit::SP;
    _pu2->puFareType() = PricingUnit::NL;
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testcheckTag1Tag3FareInABAItin_Tag3Tag1_CombinationAllowedViaCarrierPreference()
  {
    preparePUPath();
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;
    PUPath* puPath = _validator->_puPath;

    puPath->abaTripWithOWPU() = true;
    _pu1->geoTravelType() = GeoTravelType::Transborder;
    _pu1->puFareType() = PricingUnit::NL;
    _pu2->puFareType() = PricingUnit::SP;
    PaxTypeFare* ptf1 = _pu1->fareUsage().front()->paxTypeFare();
    PaxTypeFare* ptf2 = _pu2->fareUsage().front()->paxTypeFare();
    setFareAndFareInfo(ptf1);
    setFareAndFareInfo(ptf2);
    setTag3(ptf1);
    setTag1(ptf2);
    ptf1->fareMarket() = createFareMarketWithCarrierPreference('N');
    ptf2->fareMarket() = createFareMarketWithCarrierPreference('N');
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    std::string expectedDiagResponse("");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());

    diag.clear();
    _pu1->puFareType() = PricingUnit::SP;
    _pu2->puFareType() = PricingUnit::NL;
    CPPUNIT_ASSERT(_validator->checkTag1Tag3FareInABAItin(*_farePath, diag));
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testRecalculatePriority()
  {
    PUPQItem* pupqItem = _memHandle.create<PUPQItem>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pupqItem->pricingUnit() = pu;
    pupqItem->mutablePriorityStatus().setFarePriority(PRIORITY_LOW);

    FPPQItem* fppqItem = _memHandle.create<FPPQItem>();
    PUPQItem* prevPUPQItem = _memHandle.create<PUPQItem>();
    prevPUPQItem->pricingUnit() = _pu1;
    fppqItem->pupqItemVect().push_back(prevPUPQItem);

    CPPUNIT_ASSERT_EQUAL(DEFAULT_PRIORITY, fppqItem->priorityStatus().farePriority());

    fppqItem->pupqItemVect()[0] = pupqItem;
    _validator->recalculatePriority(*fppqItem);

    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, fppqItem->priorityStatus().farePriority());
    fppqItem->pupqItemVect()[0] = prevPUPQItem;
  }

  void testFailMixedDateForRefund_AllHist()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievHistorical);
    CPPUNIT_ASSERT(!_validator->failMixedDateForRefund(*_farePath));
  }
  void testFailMixedDateForRefund_AllCommence()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(!_validator->failMixedDateForRefund(*_farePath));
  }
  void testFailMixedDateForRefund_Mixed1()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(_validator->failMixedDateForRefund(*_farePath));
  }
  void testFailMixedDateForRefund_Mixed2()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievTvlCommence);
    _ptf22->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    CPPUNIT_ASSERT(_validator->failMixedDateForRefund(*_farePath));
  }
  void testFailMixedDateForRefund_Mixed3()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievHistorical);
    _ptf21->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(_validator->failMixedDateForRefund(*_farePath));
  }
  void testFailMixedDateForRefund_Mixed4()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievTvlCommence);
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    CPPUNIT_ASSERT(_validator->failMixedDateForRefund(*_farePath));
  }
  void testFailMixedDateForRefund_Mixed5()
  {
    setUpAllRetrievalInfos(FareMarket::RetrievTvlCommence);
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    CPPUNIT_ASSERT(_validator->failMixedDateForRefund(*_farePath));
  }
  FarePathValidator* _validator;
  BCMixedClassValidator* _bcMixedClassValidator;
  Itin* _itin;
  PricingTrx* _trx;
  PaxFPFBaseData* _baseData;
  FarePath* _farePath;
  PricingUnit* _pu1;
  PricingUnit* _pu2;

  PaxTypeFare* _ptf11;
  PaxTypeFare* _ptf12;
  PaxTypeFare* _ptf21;
  PaxTypeFare* _ptf22;

  FareUsage* _fu1;
  FareUsage* _fu2;
  TestMemHandle _memHandle;
  FactoriesConfigStub _factoriesConfig;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FarePathValidatorTest);
}
