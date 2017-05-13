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

#include "DataModel/Billing.h"
#include "DataModel/ExcItin.h"
#include "Pricing/FPPQItem.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagCollector.h"

#include "Pricing/FPPQItemValidator.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/FarePathFactoryFailedPricingUnits.h"
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
class FPPQItemValidatorTest : public CppUnit::TestFixture
{
  class FactoriesConfigStub : public FactoriesConfig
  {
  };
  CPPUNIT_TEST_SUITE(FPPQItemValidatorTest);

  CPPUNIT_TEST(testCheckJourneyReturnFalseWhenRexPricingNotNewItinPhase);
  CPPUNIT_TEST(testCheckJourneyReturnFalseWhenNotRexPricingNotWpnc);
  CPPUNIT_TEST(testCheckJourneyReturnFalseWhenJourneyNotApplied);
  CPPUNIT_TEST(testCheckJourneyReturnFalseWhenJourneyNotActivated);
  CPPUNIT_TEST(testCheckJourneyReturnTrueWhenJourneyActivated);
  CPPUNIT_TEST(testCheckJourneyReturnFalseWhenJourneyNotActivatedMip);
  CPPUNIT_TEST(testCheckJourneyReturnTrueWhenJourneyActivatedMip);
  CPPUNIT_TEST(testCheckJourneyReturnTrueWhenJourneyActivatedRex);

  CPPUNIT_TEST(testValidForRebookWhenNoSegmentStatusInformation);
  CPPUNIT_TEST(testValidForRebookWhenLocalFareMarketPassForWP);
  CPPUNIT_TEST(testValidForRebookWhenLocalFareMarketFailForWP);
  CPPUNIT_TEST(testResetToRebook);

  CPPUNIT_TEST_SUITE_END();

  void createValidator(PricingTrx* trx)
  {
    _baseData->trx() = trx;
    FarePathFactoryFailedPricingUnits* failedPU =
        _memHandle.create<FarePathFactoryFailedPricingUnits>();
    _validator = _memHandle.create<FPPQItemValidator>(*_baseData, _allPUF, *failedPU, diag);
  }

public:
  FPPQItemValidatorTest() {}
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _baseData = _memHandle.create<PaxFPFBaseData>(_factoriesConfig);
    _trx = PricingMockDataBuilder::getPricingTrx();

    _ptf11 = _memHandle.create<PaxTypeFare>();
    _ptf12 = _memHandle.create<PaxTypeFare>();
    _ptf21 = _memHandle.create<PaxTypeFare>();
    _ptf22 = _memHandle.create<PaxTypeFare>();

    _fppqItem = _memHandle.create<FPPQItem>();
    _pupqItem1 = _memHandle.create<PUPQItem>();
    _pupqItem2 = _memHandle.create<PUPQItem>();

    _pu1 = _memHandle.create<PricingUnit>();
    _pu2 = _memHandle.create<PricingUnit>();

    _fu11 = _memHandle.create<FareUsage>();
    _fu12 = _memHandle.create<FareUsage>();
    _fu21 = _memHandle.create<FareUsage>();
    _fu22 = _memHandle.create<FareUsage>();

    _pupqItem1->pricingUnit() = _pu1;
    _pupqItem2->pricingUnit() = _pu2;
    _fppqItem->pupqItemVect().push_back(_pupqItem1);
    _fppqItem->pupqItemVect().push_back(_pupqItem2);
    _fppqItem->puIndices().push_back(0);
    _fppqItem->puIndices().push_back(0);

    _farePath = _memHandle.create<FarePath>();
    _fppqItem->farePath() = _farePath;

    _farePath->pricingUnit().push_back(_pu1);
    _farePath->pricingUnit().push_back(_pu2);
    _fu11->paxTypeFare() = _ptf11;
    _fu12->paxTypeFare() = _ptf12;
    _fu21->paxTypeFare() = _ptf21;
    _fu22->paxTypeFare() = _ptf22;

    _pu1->fareUsage().push_back(_fu11);
    _pu1->fareUsage().push_back(_fu12);
    _pu2->fareUsage().push_back(_fu21);
    _pu2->fareUsage().push_back(_fu22);
    createValidator(_trx);
  }
  void tearDown()
  {
    _memHandle.clear();
    delete _trx;
    _trx = nullptr;
  }
  void prepareTrx()
  {
    PricingOptions* options = _memHandle.create<PricingOptions>();
    options->applyJourneyLogic() = true;
    options->journeyActivatedForPricing() = true;
    options->journeyActivatedForShopping() = true;
    _trx->setOptions(options);
    PricingRequest* req = _memHandle.create<PricingRequest>();
    req->lowFareRequested() = YES;
    _trx->setRequest(req);
    Billing* billing = _memHandle.create<Billing>();
    billing->actionCode() = "WPNC";
    _trx->billing() = billing;
    createValidator(_trx);
  }
  void testCheckJourneyReturnFalseWhenRexPricingNotNewItinPhase()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    createValidator(&rexTrx);
    CPPUNIT_ASSERT(!_validator->checkJourney());
  }

  void testCheckJourneyReturnFalseWhenNotRexPricingNotWpnc()
  {
    prepareTrx();
    _trx->getRequest()->lowFareRequested() = NO;
    CPPUNIT_ASSERT(!_validator->checkJourney());
  }

  void testCheckJourneyReturnFalseWhenJourneyNotApplied()
  {
    prepareTrx();
    _trx->getOptions()->applyJourneyLogic() = false;
    CPPUNIT_ASSERT(!_validator->checkJourney());
  }

  void testCheckJourneyReturnFalseWhenJourneyNotActivated()
  {
    prepareTrx();
    _trx->getOptions()->journeyActivatedForPricing() = false;
    CPPUNIT_ASSERT(!_validator->checkJourney());
  }

  void testCheckJourneyReturnTrueWhenJourneyActivated()
  {
    prepareTrx();
    CPPUNIT_ASSERT(_validator->checkJourney());
  }

  void testCheckJourneyReturnFalseWhenJourneyNotActivatedMip()
  {
    prepareTrx();
    _trx->setTrxType(PricingTrx::MIP_TRX);
    _trx->getOptions()->journeyActivatedForShopping() = false;
    CPPUNIT_ASSERT(!_validator->checkJourney());
  }

  void testCheckJourneyReturnTrueWhenJourneyActivatedMip()
  {
    prepareTrx();
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(_validator->checkJourney());
  }

  void testCheckJourneyReturnTrueWhenJourneyActivatedRex()
  {
    prepareTrx();
    RexPricingTrx rexTrx;
    rexTrx.setOptions(_trx->getOptions());
    rexTrx.setRequest(_trx->getRequest());
    rexTrx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    createValidator(&rexTrx);
    CPPUNIT_ASSERT(_validator->checkJourney());
  }

  void testValidForRebookWhenNoSegmentStatusInformation()
  {
    _ptf11->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;

    CPPUNIT_ASSERT(!_validator->isSameFarePathValidForRebook(*_farePath));
  }

  void testValidForRebookWhenLocalFareMarketPassForWP()
  {
    PaxTypeFare::SegmentStatus ss1;
    PaxTypeFare::SegmentStatus ss2;
    ss2._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION, true);
    _ptf11->segmentStatus().push_back(ss1);
    _ptf11->segmentStatus().push_back(ss2);
    _ptf11->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;

    CPPUNIT_ASSERT(_validator->isSameFarePathValidForRebook(*_farePath));

    _ptf11->segmentStatus().clear();
    _ptf22->segmentStatus().push_back(ss1);
    _ptf22->segmentStatus().push_back(ss2);
    _ptf22->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    ss2._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION, false);
    ss1._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION, true);

    CPPUNIT_ASSERT(_validator->isSameFarePathValidForRebook(*_farePath));
  }

  void testValidForRebookWhenLocalFareMarketFailForWP()
  {
    PaxTypeFare::SegmentStatus ss1;
    PaxTypeFare::SegmentStatus ss2;
    PaxTypeFare::SegmentStatus ss3;
    PaxTypeFare::SegmentStatus ss4;
    _ptf11->segmentStatus().push_back(ss1);
    _ptf12->segmentStatus().push_back(ss2);
    _ptf21->segmentStatus().push_back(ss3);
    _ptf22->segmentStatus().push_back(ss4);
    _ptf11->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _ptf12->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _ptf21->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _ptf22->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;

    CPPUNIT_ASSERT(!_validator->isSameFarePathValidForRebook(*_farePath));
  }

  void testResetToRebook()
  {
    PaxTypeFare::SegmentStatus ss;
    ss._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REX_WP_LOCALMARKET_VALIDATION, true);

    _farePath->rebookClassesExists() = false;
    _ptf11->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _ptf12->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _ptf12->segmentStatus().push_back(ss);
    _ptf21->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _ptf22->bkgCodeTypeForRex() = PaxTypeFare::BKSS_PASS;
    _pupqItem1->rebookClassesExists() = false;
    _pupqItem2->rebookClassesExists() = false;

    _validator->resetToRebook(*_farePath, *_fppqItem);

    CPPUNIT_ASSERT(_farePath->rebookClassesExists() == true);
    CPPUNIT_ASSERT(_ptf11->bkgCodeTypeForRex() == PaxTypeFare::BKSS_PASS);
    CPPUNIT_ASSERT(_ptf12->bkgCodeTypeForRex() == PaxTypeFare::BKSS_REBOOKED);
    CPPUNIT_ASSERT(_ptf21->bkgCodeTypeForRex() == PaxTypeFare::BKSS_PASS);
    CPPUNIT_ASSERT(_ptf22->bkgCodeTypeForRex() == PaxTypeFare::BKSS_PASS);
    CPPUNIT_ASSERT(_pupqItem1->rebookClassesExists());
    CPPUNIT_ASSERT(!_pupqItem2->rebookClassesExists());
  }
  FPPQItemValidator* _validator;
  PricingTrx* _trx;
  PaxFPFBaseData* _baseData;
  DiagCollector diag;

  std::vector<PricingUnitFactory*> _allPUF;
  FareUsage* _fu11;
  FareUsage* _fu12;
  FareUsage* _fu21;
  FareUsage* _fu22;
  TestMemHandle _memHandle;
  PUPQItem* _pupqItem1;
  PUPQItem* _pupqItem2;
  PricingUnit* _pu1;
  PricingUnit* _pu2;

  PaxTypeFare* _ptf11;
  PaxTypeFare* _ptf12;
  PaxTypeFare* _ptf21;
  PaxTypeFare* _ptf22;
  FarePath* _farePath;
  FPPQItem* _fppqItem;

  FactoriesConfigStub _factoriesConfig;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FPPQItemValidatorTest);
}
