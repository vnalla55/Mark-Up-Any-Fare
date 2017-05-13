#include <vector>
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/RefundPermutation.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "RexPricing/CommonSolutionValidator.h"
#include "RexPricing/RefundSolutionValidator.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/FarePath.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/RefundProcessInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareCompInfo.h"
#include "Diagnostic/Diag689Collector.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPenalty.h"
#include "DBAccess/FareTypeTable.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class MyRefundSolutionValidator : public RefundSolutionValidator
{
public:
  MyRefundSolutionValidator(RefundPricingTrx& trx, FarePath& fp) : RefundSolutionValidator(trx, fp)
  {
  }

  std::string getFareBasis(const PaxTypeFare& paxTypeFare) const
  {
    return paxTypeFare.createFareBasis(0);
  }

  std::vector<FareTypeTable*> _fareTypeVec;

protected:
  void printCommonFail(const PaxTypeFare& ptf,
                       const RefundProcessInfo& processInfo,
                       const std::string& message)
  {
  }
};

class FakeRexTrx : public RefundPricingTrx
{
public:
  std::vector<FareTypeTable*>* _fttv;

  FakeRexTrx() { _fttv = new std::vector<FareTypeTable*>; }

  virtual const std::vector<FareTypeTable*>&
  getFareTypeTables(const VendorCode& vendor, uint32_t tblItemNo, const DateTime& applicationDate)
      const
  {
    return *_fttv;
  }

  virtual ~FakeRexTrx() { delete _fttv; }
};
class RefundSolutionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RefundSolutionValidatorTest);

  CPPUNIT_TEST(testValidateSameFareIndicatorWhenBlankInd);
  CPPUNIT_TEST(testValidateSameFareIndicatorWhenInvalidInd);
  CPPUNIT_TEST(testValidateSameFareIndicatorWhenSameFareType);
  CPPUNIT_TEST(testValidateSameFareIndicatorWhenDifferentFareType);
  CPPUNIT_TEST(testValidateSameFareIndicatorWhenSameFareClass);
  CPPUNIT_TEST(testValidateSameFareIndicatorWhenDifferentFareClass);

  CPPUNIT_TEST(testGetRelatedFUsWhenNotMapped);
  CPPUNIT_TEST(testGetRelatedFUsWhenOneMappedToOne);
  CPPUNIT_TEST(testGetRelatedFUsWhenMappedOneOfTwo);
  CPPUNIT_TEST(testGetRelatedFUsWhenTwoMappedToOne);

  CPPUNIT_TEST(testOriginChanged_True);
  CPPUNIT_TEST(testOriginChanged_False);

  CPPUNIT_TEST(testDestinationChanged_True);
  CPPUNIT_TEST(testDestinationChanged_False);

  CPPUNIT_TEST(testCheckFareBreaks_Flown_Blank_ChangedOriginAndDestination);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_Blank_ChangedOriginOnly);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_Blank_ChangedDestinationOnly);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_Blank_Unchanged);
  CPPUNIT_TEST(testCheckFareBreaks_PartiallyFlown_ChangedOriginAndDestination);
  CPPUNIT_TEST(testCheckFareBreaks_PartiallyFlown_ChangedOriginOnly);
  CPPUNIT_TEST(testCheckFareBreaks_PartiallyFlown_ChangedDestinationOnly);
  CPPUNIT_TEST(testCheckFareBreaks_PartiallyFlown_Unchanged);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_X_ChangedOriginAndDestination);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_X_ChangedOriginOnly);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_X_ChangedDestinationOnly);
  CPPUNIT_TEST(testCheckFareBreaks_Flown_X_Unchanged);

  CPPUNIT_TEST(testHasDiagAndFilterPassed);
  CPPUNIT_TEST(testPrintValidationResult);
  CPPUNIT_TEST(testPrintFareBreaksFail);

  CPPUNIT_TEST(testCheckSameRuleTariff_SamePI_R3andPTFtariffs);
  CPPUNIT_TEST(testCheckSameRuleTariff_SamePI_PTF_FCAIandPTF_FCAItariffs);
  CPPUNIT_TEST(testCheckSameRuleTariff_DifferentPI_PTFandPTFtariffs);
  CPPUNIT_TEST(testCheckSameRuleTariff_DifferentPI_FCAIandPTF_FCAItariffs);
  CPPUNIT_TEST(testCheckAnyRuleTariff_BlankBothPublic);
  CPPUNIT_TEST(testCheckAnyRuleTariff_BlankPublicPrivate);
  CPPUNIT_TEST(testCheckAnyRuleTariff_BlankPrivatePublic);
  CPPUNIT_TEST(testCheckAnyRuleTariff_BlankPrivatePrivate);
  CPPUNIT_TEST(testCheckAnyRuleTariff_XBothPublic);
  CPPUNIT_TEST(testCheckAnyRuleTariff_XPublicPrivate);
  CPPUNIT_TEST(testCheckAnyRuleTariff_XPrivatePublic);
  CPPUNIT_TEST(testCheckAnyRuleTariff_XPrivatePrivate);
  CPPUNIT_TEST(testCheckRuleTariff_SITA);
  CPPUNIT_TEST(testCheckRuleTariff_AnyRule);
  CPPUNIT_TEST(testCheckRuleTariff_SameRule);

  CPPUNIT_TEST(testRepriceIndicatorValidation_TheSameFares_Pass);
  CPPUNIT_TEST(testRepriceIndicatorValidation_TheSameFares_Fail);
  CPPUNIT_TEST(testRepriceIndicatorValidation_MixedFares_Fail);

  CPPUNIT_TEST(testCheckRuleWhenNoRule);
  CPPUNIT_TEST(testCheckRuleWhenValidRule);
  CPPUNIT_TEST(testCheckRuleWhenInvalidRule);
  CPPUNIT_TEST(testCheckRuleWhenValidRuleMask);
  CPPUNIT_TEST(testCheckRuleWhenInvalidRuleMask);

  CPPUNIT_TEST(testCache);
  CPPUNIT_TEST(testExecuteValidation_pass);
  CPPUNIT_TEST(testExecuteValidation_fail);
  CPPUNIT_TEST(testExecuteValidation_passCacheFill);
  CPPUNIT_TEST(testExecuteValidation_failCacheFill);
  CPPUNIT_TEST(testExecuteValidation_failCacheUse);
  CPPUNIT_TEST(testExecuteValidation_passCacheUse);

  CPPUNIT_TEST(testCheckAmountWhenNotFullyFlown);
  CPPUNIT_TEST(testCheckAmountWhenOnlyExchangeFlown);
  CPPUNIT_TEST(testCheckAmountWhenOnlyExchangeUnflown);
  CPPUNIT_TEST(testCheckAmountWhenOriginChanged);
  CPPUNIT_TEST(testCheckAmountWhenDestinationChanged);
  CPPUNIT_TEST(testCheckAmountWhenXAndHigherAmount);
  CPPUNIT_TEST(testCheckAmountWhenXAndTheSameAmount);
  CPPUNIT_TEST(testCheckAmountWhenBlankAndHigherAmount);
  CPPUNIT_TEST(testCheckAmountWhenBlankAndTheSameAmount);
  CPPUNIT_TEST(testCheckAmountWhenBlankAndLowerAmount);

  CPPUNIT_TEST(testCheckFareType_noTable);
  CPPUNIT_TEST(testCheckFareType_emptyTable);
  CPPUNIT_TEST(testCheckFareType_foundPermitted);
  CPPUNIT_TEST(testCheckFareType_foundForbidden);
  CPPUNIT_TEST(testCheckFareType_notFoundPermitted);
  CPPUNIT_TEST(testCheckFareType_notFoundForbidden);
  CPPUNIT_TEST(testCheckFareType_foundPermittedMany);
  CPPUNIT_TEST(testCheckFareType_foundForbiddenMany);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  RefundPricingTrx* _refTrx;
  FarePath* _farePath;
  RefundSolutionValidator* _validator;
  VoluntaryRefundsInfo* _record3;
  PaxTypeFare* _excPaxTypeFare;
  FareClassAppInfo* _excFareClassAppInfo;
  FareInfo* _excFareInfo;
  FareCompInfo* _fareCompInfo;
  RefundProcessInfo* _refundProcessInfo;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage1;
  FareUsage* _fareUsage2;
  FareClassAppInfo* _fareClassAppInfo1;
  FareInfo* _fareInfo1;
  ConfigMan* _configMan;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _configMan = _memHandle.create<ConfigMan>();

    _refTrx = _memHandle.create<FakeRexTrx>();
    _farePath = _memHandle.create<FarePath>();

    _refTrx->diagnostic().diagnosticType() = Diagnostic689;
    _refTrx->diagnostic().activate();

    _pricingUnit = _memHandle.create<PricingUnit>();
    _fareUsage1 = _memHandle.create<FareUsage>();
    _fareUsage2 = _memHandle.create<FareUsage>();
    PaxTypeFare* paxTypeFare1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* paxTypeFare2 = _memHandle.create<PaxTypeFare>();
    FareMarket* fareMarket1 = _memHandle.create<FareMarket>();
    FareMarket* fareMarket2 = _memHandle.create<FareMarket>();

    _farePath->pricingUnit().push_back(_pricingUnit);
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _pricingUnit->fareUsage().push_back(_fareUsage2);
    _fareUsage1->paxTypeFare() = paxTypeFare1;
    _fareUsage2->paxTypeFare() = paxTypeFare2;
    paxTypeFare1->fareMarket() = fareMarket1;
    paxTypeFare2->fareMarket() = fareMarket2;

    _validator = _memHandle.insert(new MyRefundSolutionValidator(*_refTrx, *_farePath));
    _record3 = _memHandle.create<VoluntaryRefundsInfo>();
    _excPaxTypeFare = _memHandle.create<PaxTypeFare>();
    _excFareClassAppInfo = _memHandle.create<FareClassAppInfo>();
    FareMarket* excFareMarket = _memHandle.insert(new FareMarket);
    Fare* excFare = _memHandle.create<Fare>();
    _excFareInfo = _memHandle.create<FareInfo>();
    _fareCompInfo = _memHandle.create<FareCompInfo>();
    Fare* fare1 = _memHandle.create<Fare>();
    _fareInfo1 = _memHandle.create<FareInfo>();
    _fareClassAppInfo1 = _memHandle.create<FareClassAppInfo>();
    _refundProcessInfo =
        _memHandle.insert(new RefundProcessInfo(_record3, _excPaxTypeFare, _fareCompInfo));

    fare1->setFareInfo(_fareInfo1);
    paxTypeFare1->setFare(fare1);
    paxTypeFare1->fareClassAppInfo() = _fareClassAppInfo1;
    _excPaxTypeFare->fareMarket() = excFareMarket;
    _excPaxTypeFare->fareClassAppInfo() = _excFareClassAppInfo;
    excFare->setFareInfo(_excFareInfo);
    _excPaxTypeFare->setFare(excFare);
    _fareCompInfo->fareMarket() = _memHandle.insert(new FareMarket);
  }

  void tearDown() { _memHandle.clear(); }

  void testValidateSameFareIndicatorWhenBlankInd()
  {
    _record3->sameFareInd() = RefundSolutionValidator::BLANK;
    _excFareClassAppInfo->_fareType = "BU";
    _fareClassAppInfo1->_fareType = "FR";

    CPPUNIT_ASSERT(_validator->validateSameFareIndicator(*_fareUsage1, *_refundProcessInfo));
  }

  void testValidateSameFareIndicatorWhenInvalidInd()
  {
    _record3->sameFareInd() = 'S';
    _excFareClassAppInfo->_fareType = "BU";
    _fareClassAppInfo1->_fareType = "FR";

    CPPUNIT_ASSERT(!_validator->validateSameFareIndicator(*_fareUsage1, *_refundProcessInfo));
    CPPUNIT_ASSERT_EQUAL(std::string(" FAILED\n *** INVALID SAME FARE INDICATOR *** \n"),
                         _validator->_dc->str());
  }

  void testValidateSameFareIndicatorWhenSameFareType()
  {
    _record3->sameFareInd() = RefundSolutionValidator::SAME_FARE_TYPE;
    _excFareClassAppInfo->_fareType = "BU";
    _fareClassAppInfo1->_fareType = "BU";

    CPPUNIT_ASSERT(_validator->validateSameFareIndicator(*_fareUsage1, *_refundProcessInfo));
  }

  void testValidateSameFareIndicatorWhenDifferentFareType()
  {
    _record3->sameFareInd() = RefundSolutionValidator::SAME_FARE_TYPE;
    _excFareClassAppInfo->_fareType = "BU";
    _fareClassAppInfo1->_fareType = "FR";

    CPPUNIT_ASSERT(!_validator->validateSameFareIndicator(*_fareUsage1, *_refundProcessInfo));
    CPPUNIT_ASSERT_EQUAL(std::string(" EXC FARE TYPE: BU\n"
                                     " REPRICE FARE TYPE: FR\n"),
                         _validator->_dc->str());
  }

  void testValidateSameFareIndicatorWhenSameFareClass()
  {
    _record3->sameFareInd() = RefundSolutionValidator::SAME_FARE_CLASS;
    _excFareInfo->fareClass() = "Y1";
    _fareInfo1->fareClass() = "Y1";

    CPPUNIT_ASSERT(_validator->validateSameFareIndicator(*_fareUsage1, *_refundProcessInfo));
  }

  void testValidateSameFareIndicatorWhenDifferentFareClass()
  {
    _record3->sameFareInd() = RefundSolutionValidator::SAME_FARE_CLASS;
    _excFareInfo->fareClass() = "Y";
    _fareInfo1->fareClass() = "C";

    CPPUNIT_ASSERT(!_validator->validateSameFareIndicator(*_fareUsage1, *_refundProcessInfo));
    CPPUNIT_ASSERT_EQUAL(std::string(" EXC FARE CLASS: Y\n"
                                     " REPRICE FARE CLASS: C\n"),
                         _validator->_dc->str());
  }

  std::vector<FareUsage*> getFareUsageVec() { return std::vector<FareUsage*>(); }

  std::vector<FareUsage*> getFareUsageVec(FareUsage* fu) { return std::vector<FareUsage*>(1, fu); }

  std::vector<FareUsage*> getFareUsageVec(FareUsage* fu1, FareUsage* fu2)
  {
    if (fu1->paxTypeFare()->fareMarket() > fu2->paxTypeFare()->fareMarket())
      std::swap(fu1, fu2);
    FareUsage* fu[] = { fu1, fu2 };
    return std::vector<FareUsage*>(fu, fu + 2);
  }

  void testGetRelatedFUsWhenNotMapped()
  {
    _pricingUnit->fareUsage().clear();
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _fareCompInfo->getMappedFCs().insert(_fareUsage1->paxTypeFare()->fareMarket());
    FareMarket fm2;
    _fareUsage1->paxTypeFare()->fareMarket() = &fm2;

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(), _validator->getRelatedFUs(*_fareCompInfo));

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(), _validator->getRelatedFUs(*_fareCompInfo));
  }

  void testGetRelatedFUsWhenOneMappedToOne()
  {
    _pricingUnit->fareUsage().clear();
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _fareCompInfo->getMappedFCs().insert(_fareUsage1->paxTypeFare()->fareMarket());

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(_fareUsage1), _validator->getRelatedFUs(*_fareCompInfo));

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(_fareUsage1), _validator->getRelatedFUs(*_fareCompInfo));
  }

  void testGetRelatedFUsWhenMappedOneOfTwo()
  {
    FareMarket fm2;
    _pricingUnit->fareUsage().clear();
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _pricingUnit->fareUsage().push_back(_fareUsage2);
    _fareUsage2->paxTypeFare()->fareMarket() = &fm2;

    _fareCompInfo->getMappedFCs().insert(_fareUsage2->paxTypeFare()->fareMarket());

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(_fareUsage2), _validator->getRelatedFUs(*_fareCompInfo));

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(_fareUsage2), _validator->getRelatedFUs(*_fareCompInfo));
  }

  void testGetRelatedFUsWhenTwoMappedToOne()
  {
    _fareCompInfo->getMappedFCs().insert(_fareUsage1->paxTypeFare()->fareMarket());
    _fareCompInfo->getMappedFCs().insert(_fareUsage2->paxTypeFare()->fareMarket());

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(_fareUsage1, _fareUsage2),
                         _validator->getRelatedFUs(*_fareCompInfo));

    CPPUNIT_ASSERT_EQUAL(getFareUsageVec(_fareUsage1, _fareUsage2),
                         _validator->getRelatedFUs(*_fareCompInfo));
  }

  enum ChangeStatus
  {
    UNCHANGED = 0,
    CHANGED
  };

  void setUpOrigin(ChangeStatus status)
  {
    const LocCode CRACOW_CITY = "KRK";
    const LocCode DALLAS_CITY = "DFW";
    _fareUsage1->paxTypeFare()->fareMarket()->boardMultiCity() = CRACOW_CITY;
    _fareCompInfo->fareMarket()->boardMultiCity() = status ? DALLAS_CITY : CRACOW_CITY;
  }

  void setUpDestination(ChangeStatus status)
  {
    const LocCode CRACOW_CITY = "KRK";
    const LocCode DALLAS_CITY = "DFW";
    _fareUsage1->paxTypeFare()->fareMarket()->offMultiCity() = CRACOW_CITY;
    _fareCompInfo->fareMarket()->offMultiCity() = status ? DALLAS_CITY : CRACOW_CITY;
  }

  void testOriginChanged_True()
  {
    setUpOrigin(CHANGED);
    CPPUNIT_ASSERT(_validator->originChanged(*_fareUsage1, *_refundProcessInfo));
  }

  void testOriginChanged_False()
  {
    setUpOrigin(UNCHANGED);
    CPPUNIT_ASSERT(!_validator->originChanged(*_fareUsage1, *_refundProcessInfo));
  }

  void testDestinationChanged_True()
  {
    setUpDestination(CHANGED);
    CPPUNIT_ASSERT(_validator->destinationChanged(*_fareUsage1, *_refundProcessInfo));
  }

  void testDestinationChanged_False()
  {
    setUpDestination(UNCHANGED);
    CPPUNIT_ASSERT(!_validator->destinationChanged(*_fareUsage1, *_refundProcessInfo));
  }
  enum FlownStatus
  {
    FULLY_FLOWN = 0,
    PARTIALLY_FLOWN
  };
  void setUpFlownStatus(const FlownStatus& flownStatus)
  {
    if (flownStatus)
    {
      _fareCompInfo->fareMarket()->travelSeg().push_back(_memHandle.insert(new AirSeg));
      _fareCompInfo->fareMarket()->travelSeg().back()->unflown() = !flownStatus;
    }

    _fareCompInfo->fareMarket()->travelSeg().push_back(_memHandle.insert(new AirSeg));
    _fareCompInfo->fareMarket()->travelSeg().back()->unflown() = flownStatus;
  }

  void testCheckFareBreaks_Flown_Blank_ChangedOriginAndDestination()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(CHANGED);
    setUpDestination(CHANGED);
    _record3->fareBreakpoints() = RefundSolutionValidator::CHANGE_TO_FARE_BREAKS_NOT_ALLOWED;
    CPPUNIT_ASSERT(!_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_Flown_Blank_ChangedOriginOnly()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(CHANGED);
    setUpDestination(UNCHANGED);
    _record3->fareBreakpoints() = RefundSolutionValidator::CHANGE_TO_FARE_BREAKS_NOT_ALLOWED;
    CPPUNIT_ASSERT(!_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_Flown_Blank_ChangedDestinationOnly()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(UNCHANGED);
    setUpDestination(CHANGED);
    _record3->fareBreakpoints() = RefundSolutionValidator::CHANGE_TO_FARE_BREAKS_NOT_ALLOWED;
    CPPUNIT_ASSERT(!_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_Flown_Blank_Unchanged()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(UNCHANGED);
    setUpDestination(UNCHANGED);
    _record3->fareBreakpoints() = RefundSolutionValidator::CHANGE_TO_FARE_BREAKS_NOT_ALLOWED;
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_PartiallyFlown_ChangedOriginAndDestination()
  {
    setUpFlownStatus(PARTIALLY_FLOWN);
    setUpOrigin(CHANGED);
    setUpDestination(CHANGED);
    CPPUNIT_ASSERT(!_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_PartiallyFlown_ChangedOriginOnly()
  {
    setUpFlownStatus(PARTIALLY_FLOWN);
    setUpOrigin(CHANGED);
    setUpDestination(UNCHANGED);
    CPPUNIT_ASSERT(!_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_PartiallyFlown_ChangedDestinationOnly()
  {
    setUpFlownStatus(PARTIALLY_FLOWN);
    setUpOrigin(UNCHANGED);
    setUpDestination(CHANGED);
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_PartiallyFlown_Unchanged()
  {
    setUpFlownStatus(PARTIALLY_FLOWN);
    setUpOrigin(UNCHANGED);
    setUpDestination(UNCHANGED);
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  static const Indicator CHANGE_TO_FARE_BREAKS_ALLOWED = 'X';

  void testCheckFareBreaks_Flown_X_ChangedOriginAndDestination()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(CHANGED);
    setUpDestination(CHANGED);
    _record3->fareBreakpoints() = CHANGE_TO_FARE_BREAKS_ALLOWED;
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_Flown_X_ChangedOriginOnly()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(CHANGED);
    setUpDestination(UNCHANGED);
    _record3->fareBreakpoints() = CHANGE_TO_FARE_BREAKS_ALLOWED;
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_Flown_X_ChangedDestinationOnly()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(UNCHANGED);
    setUpDestination(CHANGED);
    _record3->fareBreakpoints() = CHANGE_TO_FARE_BREAKS_ALLOWED;
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareBreaks_Flown_X_Unchanged()
  {
    setUpFlownStatus(FULLY_FLOWN);
    setUpOrigin(UNCHANGED);
    setUpDestination(UNCHANGED);
    _record3->fareBreakpoints() = CHANGE_TO_FARE_BREAKS_ALLOWED;
    CPPUNIT_ASSERT(_validator->checkFareBreaks(*_fareUsage1, *_refundProcessInfo));
  }

  RefundPermutation* createPermutation()
  {
    RefundPermutation* perm = _memHandle.insert(new RefundPermutation);
    return perm;
  }

  void setUpDiag(RefundSolutionValidator* validator = 0)
  {
    if (!validator)
      validator = _validator;

    validator->_dc = _memHandle.insert(new Diag689Collector);
    validator->_dc->activate();
  }
  void testHasDiagAndFilterPassed()
  {
    setUpDiag();
    CPPUNIT_ASSERT(_validator->hasDiagAndFilterPassed());
  }
  const std::string getDiagString()
  {
    _validator->_dc->flushMsg();
    return _validator->_dc->str();
  }
  void testPrintValidationResult()
  {
    setUpDiag();
    _validator->printValidationResult(1, true);
    CPPUNIT_ASSERT_EQUAL(std::string("PERMUTATION 1: PASSED\n"), getDiagString());
  }
  void testPrintCommonFail()
  {
    RefundSolutionValidator validator(*_refTrx, *_farePath);
    setUpDiag(&validator);
    validator.printCommonFail(*_excPaxTypeFare, *_refundProcessInfo, "HEN");

    validator._dc->flushMsg();

    CPPUNIT_ASSERT_EQUAL(std::string("FAILED\n"
                                     " FAILED: HEN\n"
                                     "   0: -- R3 ITEM 0\n"
                                     "MATCHING TO REPRICE FC -\n"),
                         validator._dc->str());
  }
  void testPrintFareBreaksFail()
  {
    setUpDiag();
    _validator->printFareBreaksFail(*_fareUsage1, *_refundProcessInfo, "HEN");
    CPPUNIT_ASSERT_EQUAL(std::string("HEN FARE BREAK POINT CHANGED\n"), getDiagString());
  }

  PaxTypeFare& setUpSameRuleTariff(const int& processInfoR3ruleTariff,
                                   const int& processInfoPTFruleTariff,
                                   const int& processInfoPTFFCAIruleTariff,
                                   const int& ptfRuleTariff,
                                   const int& ptfFCAIRuleTariff)
  {
    _record3->ruleTariff() = processInfoR3ruleTariff;

    TariffCrossRefInfo* processTagTCRI = _memHandle.insert(new TariffCrossRefInfo);
    processTagTCRI->ruleTariff() = processInfoPTFruleTariff;
    setUpRuleTariffPTF(_excPaxTypeFare, processTagTCRI);
    FareClassAppInfo* processTagFCAI = _memHandle.insert(new FareClassAppInfo);
    processTagFCAI->_ruleTariff = processInfoPTFFCAIruleTariff;
    _excPaxTypeFare->fareClassAppInfo() = processTagFCAI;

    PaxTypeFare* ptf = _memHandle.insert(new PaxTypeFare);
    TariffCrossRefInfo* ptfTCRI = _memHandle.insert(new TariffCrossRefInfo);
    ptfTCRI->ruleTariff() = ptfRuleTariff;
    setUpRuleTariffPTF(ptf, ptfTCRI);
    FareClassAppInfo* ptfFCAI = _memHandle.insert(new FareClassAppInfo);
    ptfFCAI->_ruleTariff = ptfFCAIRuleTariff;
    ptf->fareClassAppInfo() = ptfFCAI;

    return *ptf;
  }
  void setUpRuleTariffPTF(PaxTypeFare* ptf, const TariffCrossRefInfo* tcri)
  {
    Fare* fare = _memHandle.insert(new Fare);
    fare->setTariffCrossRefInfo(tcri);
    ptf->setFare(fare);
  }
  void testCheckSameRuleTariff_SamePI_R3andPTFtariffs()
  {
    CPPUNIT_ASSERT(
        _validator->checkSameRuleTariff(setUpSameRuleTariff(1, 2, 3, 1, 4), *_refundProcessInfo));
  }
  void testCheckSameRuleTariff_SamePI_PTF_FCAIandPTF_FCAItariffs()
  {
    CPPUNIT_ASSERT(
        _validator->checkSameRuleTariff(setUpSameRuleTariff(3, 1, 2, 1, 2), *_refundProcessInfo));
  }
  void testCheckSameRuleTariff_DifferentPI_PTFandPTFtariffs()
  {
    CPPUNIT_ASSERT(
        _validator->checkSameRuleTariff(setUpSameRuleTariff(4, 2, 1, 3, 1), *_refundProcessInfo));
  }
  void testCheckSameRuleTariff_DifferentPI_FCAIandPTF_FCAItariffs()
  {
    CPPUNIT_ASSERT(
        _validator->checkSameRuleTariff(setUpSameRuleTariff(4, 3, 2, 3, 1), *_refundProcessInfo));
  }
  enum RuleTariffCat
  {
    PUBLIC = 0,
    PRIVATE
  };
  PaxTypeFare& setUpAnyRuleTariff(const Indicator processInfoR3ruleTariffInd,
                                  const RuleTariffCat& processInfoPTFruleTariffCat,
                                  const RuleTariffCat& ptfRuleTariffCat)
  {
    _record3->ruleTariffInd() = processInfoR3ruleTariffInd;

    TariffCrossRefInfo* processTagTCRI = _memHandle.insert(new TariffCrossRefInfo);
    processTagTCRI->tariffCat() = processInfoPTFruleTariffCat;
    setUpRuleTariffPTF(_excPaxTypeFare, processTagTCRI);

    PaxTypeFare* ptf = _memHandle.insert(new PaxTypeFare);
    TariffCrossRefInfo* ptfTCRI = _memHandle.insert(new TariffCrossRefInfo);
    ptfTCRI->tariffCat() = ptfRuleTariffCat;
    setUpRuleTariffPTF(ptf, ptfTCRI);

    return *ptf;
  }
  void testCheckAnyRuleTariff_BlankBothPublic()
  {
    CPPUNIT_ASSERT(_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_NO_RESTRICTION, PUBLIC, PUBLIC),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_BlankPublicPrivate()
  {
    CPPUNIT_ASSERT(!_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_NO_RESTRICTION, PUBLIC, PRIVATE),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_BlankPrivatePublic()
  {
    CPPUNIT_ASSERT(_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_NO_RESTRICTION, PRIVATE, PUBLIC),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_BlankPrivatePrivate()
  {
    CPPUNIT_ASSERT(_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_NO_RESTRICTION, PRIVATE, PRIVATE),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_XBothPublic()
  {
    CPPUNIT_ASSERT(_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_EXACT_TARIFF, PUBLIC, PUBLIC),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_XPublicPrivate()
  {
    CPPUNIT_ASSERT(!_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_EXACT_TARIFF, PUBLIC, PRIVATE),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_XPrivatePublic()
  {
    CPPUNIT_ASSERT(!_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_EXACT_TARIFF, PRIVATE, PUBLIC),
        *_refundProcessInfo));
  }
  void testCheckAnyRuleTariff_XPrivatePrivate()
  {
    CPPUNIT_ASSERT(_validator->checkAnyRuleTariff(
        setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_EXACT_TARIFF, PRIVATE, PRIVATE),
        *_refundProcessInfo));
  }
  void testCheckRuleTariff_SITA()
  {
    _fareInfo1->vendor() = SITA_VENDOR_CODE;
    CPPUNIT_ASSERT(!_validator->checkRuleTariff(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckRuleTariff_AnyRule()
  {
    _fareInfo1->vendor() = ATPCO_VENDOR_CODE;
    _record3->ruleTariff() = 0;
    _fareUsage1->paxTypeFare() =
        &setUpAnyRuleTariff(RefundSolutionValidator::RULE_TARIFF_EXACT_TARIFF, PUBLIC, PRIVATE);
    Fare* fare = _fareUsage1->paxTypeFare()->fare();
    fare->setFareInfo(_fareInfo1);
    _fareUsage1->paxTypeFare()->setFare(fare);
    CPPUNIT_ASSERT(!_validator->checkRuleTariff(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckRuleTariff_SameRule()
  {
    _fareInfo1->vendor() = ATPCO_VENDOR_CODE;
    _fareUsage1->paxTypeFare() = &setUpSameRuleTariff(4, 3, 2, 3, 1);
    Fare* fare = _fareUsage1->paxTypeFare()->fare();
    fare->setFareInfo(_fareInfo1);
    _fareUsage1->paxTypeFare()->setFare(fare);
    CPPUNIT_ASSERT(_validator->checkRuleTariff(*_fareUsage1, *_refundProcessInfo));
  }

  typedef FareMarket::RetrievalInfo RetrievalInfo;

  void testRepriceIndicatorValidation_TheSameFares_Pass()
  {
    RefundPermutation perm;
    perm.repriceIndicator() = RefundPermutation::HISTORICAL_TICKET_BASED;

    RetrievalInfo* ri =
        RetrievalInfo::construct(*_refTrx, DateTime(2009, 8, 7), FareMarket::RetrievHistorical);
    _fareUsage1->paxTypeFare()->retrievalInfo() = ri;
    _fareUsage2->paxTypeFare()->retrievalInfo() = ri;

    CPPUNIT_ASSERT(_validator->repriceIndicatorValidation(perm));
  }

  void testRepriceIndicatorValidation_TheSameFares_Fail()
  {
    RefundPermutation perm;
    perm.repriceIndicator() = RefundPermutation::HISTORICAL_TICKET_BASED;

    RetrievalInfo* ri =
        RetrievalInfo::construct(*_refTrx, DateTime(2009, 8, 7), FareMarket::RetrievTvlCommence);
    _fareUsage1->paxTypeFare()->retrievalInfo() = ri;
    _fareUsage2->paxTypeFare()->retrievalInfo() = ri;

    CPPUNIT_ASSERT(!_validator->repriceIndicatorValidation(perm));
  }

  void testRepriceIndicatorValidation_MixedFares_Fail()
  {
    RefundPermutation perm;
    perm.repriceIndicator() = RefundPermutation::HISTORICAL_TICKET_BASED;

    _fareUsage1->paxTypeFare()->retrievalInfo() =
        RetrievalInfo::construct(*_refTrx, DateTime(2009, 8, 7), FareMarket::RetrievHistorical);
    _fareUsage2->paxTypeFare()->retrievalInfo() =
        RetrievalInfo::construct(*_refTrx, DateTime(2009, 8, 7), FareMarket::RetrievTvlCommence);

    CPPUNIT_ASSERT(!_validator->repriceIndicatorValidation(perm));
  }

  void testCheckRuleWhenNoRule()
  {
    _record3->rule() = "";

    CPPUNIT_ASSERT(_validator->checkRule(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckRuleWhenValidRule()
  {
    RuleNumber rule = "2435";
    _record3->rule() = rule;
    _fareInfo1->ruleNumber() = rule;
    Fare* fare = _fareUsage1->paxTypeFare()->fare();
    fare->setFareInfo(_fareInfo1);
    _fareUsage1->paxTypeFare()->setFare(fare);
    CPPUNIT_ASSERT(_validator->checkRule(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckRuleWhenInvalidRule()
  {
    _record3->rule() = "2435";
    _fareInfo1->ruleNumber() = "2445";
    Fare* fare = _fareUsage1->paxTypeFare()->fare();
    fare->setFareInfo(_fareInfo1);
    _fareUsage1->paxTypeFare()->setFare(fare);

    CPPUNIT_ASSERT(!_validator->checkRule(*_fareUsage1, *_refundProcessInfo));
    CPPUNIT_ASSERT_EQUAL(std::string(" FAILED\nFAILED: RULE NUMBER\n"), _validator->_dc->str());
  }

  void testCheckRuleWhenValidRuleMask()
  {
    _record3->rule() = "24**";
    _fareInfo1->ruleNumber() = "2445";
    Fare* fare = _fareUsage1->paxTypeFare()->fare();
    fare->setFareInfo(_fareInfo1);
    _fareUsage1->paxTypeFare()->setFare(fare);

    CPPUNIT_ASSERT(_validator->checkRule(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckRuleWhenInvalidRuleMask()
  {
    _record3->rule() = "24**";
    _fareInfo1->ruleNumber() = "2345";
    Fare* fare = _fareUsage1->paxTypeFare()->fare();
    fare->setFareInfo(_fareInfo1);
    _fareUsage1->paxTypeFare()->setFare(fare);

    CPPUNIT_ASSERT(!_validator->checkRule(*_fareUsage1, *_refundProcessInfo));
    CPPUNIT_ASSERT_EQUAL(std::string(" FAILED\nFAILED: RULE NUMBER\n"), _validator->_dc->str());
  }

  RefundPermutation& setUpExecuteValidation(bool status)
  {
    _pricingUnit->fareUsage().clear();
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _fareCompInfo->getMappedFCs().insert(_fareUsage1->paxTypeFare()->fareMarket());
    _record3->owrt() = status ? ONE_WAY_MAY_BE_DOUBLED : ROUND_TRIP_MAYNOT_BE_HALVED;
    _fareInfo1->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    RefundPermutation* perm = _memHandle.insert(new RefundPermutation);
    perm->processInfos().push_back(_refundProcessInfo);
    return *perm;
  }
  void setUpExecuteValidation_cache(bool status)
  {
    _validator->_dc = 0; // diagnostic().deActivate();
    RefundSolutionValidator::CacheKey key(*_fareCompInfo, *_record3, RefundSolutionValidator::OWRT);
    _validator->_validationCache.insert(std::make_pair(key, status));
  }
  void testCache()
  {
    setUpExecuteValidation_cache(true);
    RefundSolutionValidator::CacheKey key(*_fareCompInfo, *_record3, RefundSolutionValidator::OWRT);
    CPPUNIT_ASSERT(_validator->_validationCache.find(key)->second);
  }
  void testCheckCache()
  {
    setUpExecuteValidation_cache(true);
    RefundSolutionValidator::CacheKey key(*_fareCompInfo, *_record3, RefundSolutionValidator::OWRT);
    CPPUNIT_ASSERT(_validator->_validationCache.find(key)->second);
  }

  void testExecuteValidation_pass()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(
        _validator->executeValidation(setUpExecuteValidation(true), RefundSolutionValidator::OWRT));
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_validator->_validationCache.size()));
  }
  void testExecuteValidation_fail()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(!_validator->executeValidation(setUpExecuteValidation(false),
                                                  RefundSolutionValidator::OWRT));
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_validator->_validationCache.size()));
  }
  void testExecuteValidation_passCacheFill()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(
        _validator->executeValidation(setUpExecuteValidation(true), RefundSolutionValidator::OWRT));
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(_validator->_validationCache.size() == 1);
  }
  void testExecuteValidation_failCacheFill()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(!_validator->executeValidation(setUpExecuteValidation(false),
                                                  RefundSolutionValidator::OWRT));
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_validator->_validationCache.size()));
  }
  void testExecuteValidation_failCacheUse()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_validator->_validationCache.size()));
    setUpExecuteValidation_cache(false);
    CPPUNIT_ASSERT(!_validator->hasDiagAndFilterPassed());
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(!_validator->executeValidation(setUpExecuteValidation(true),
                                                  RefundSolutionValidator::OWRT));
  }
  void testExecuteValidation_passCacheUse()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_validator->_validationCache.size()));
    setUpExecuteValidation_cache(true);
    CPPUNIT_ASSERT(!_validator->hasDiagAndFilterPassed());
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_validator->_validationCache.size()));
    CPPUNIT_ASSERT(_validator->executeValidation(setUpExecuteValidation(false),
                                                 RefundSolutionValidator::OWRT));
  }

  void testCheckAmountWhenNotFullyFlown()
  {
    AirSeg seg1;
    seg1.unflown() = true;
    AirSeg seg2;
    seg2.unflown() = true;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenOnlyExchangeFlown()
  {
    AirSeg seg1;
    seg1.unflown() = true;
    AirSeg seg2;
    seg2.unflown() = false;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenOnlyExchangeUnflown()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    AirSeg seg2;
    seg2.unflown() = true;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenOriginChanged()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    AirSeg seg3;
    seg3.unflown() = false;
    seg3.segmentOrder() = 3;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg3);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg3);

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenDestinationChanged()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    AirSeg seg3;
    seg3.unflown() = false;
    seg3.segmentOrder() = 3;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg1);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg3);

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenXAndHigherAmount()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg1);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg2);
    _record3->fareAmountInd() = 'X';
    _fareUsage1->paxTypeFare()->fare()->nucFareAmount() = 100.01;
    _excPaxTypeFare->fare()->nucFareAmount() = 100.00;

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenXAndTheSameAmount()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg1);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg2);
    _record3->fareAmountInd() = 'X';
    _fareUsage1->paxTypeFare()->fare()->nucFareAmount() = 100.00;
    _excPaxTypeFare->fare()->nucFareAmount() = 100.00;

    CPPUNIT_ASSERT(!_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenBlankAndHigherAmount()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg1);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg2);
    _record3->fareAmountInd() = ' ';
    _fareUsage1->paxTypeFare()->fare()->nucFareAmount() = 100.01;
    _excPaxTypeFare->fare()->nucFareAmount() = 100.00;

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenBlankAndTheSameAmount()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg1);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg2);
    _record3->fareAmountInd() = ' ';
    _fareUsage1->paxTypeFare()->fare()->nucFareAmount() = 100.00;
    _excPaxTypeFare->fare()->nucFareAmount() = 100.00;

    CPPUNIT_ASSERT(_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckAmountWhenBlankAndLowerAmount()
  {
    AirSeg seg1;
    seg1.unflown() = false;
    seg1.segmentOrder() = 1;
    AirSeg seg2;
    seg2.unflown() = false;
    seg2.segmentOrder() = 2;
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg1);
    _excPaxTypeFare->fareMarket()->travelSeg().push_back(&seg2);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg1);
    _fareUsage1->paxTypeFare()->fareMarket()->travelSeg().push_back(&seg2);
    _record3->fareAmountInd() = ' ';
    _fareUsage1->paxTypeFare()->fare()->nucFareAmount() = 100.00;
    _excPaxTypeFare->fare()->nucFareAmount() = 100.01;

    CPPUNIT_ASSERT(!_validator->checkFareAmount(*_fareUsage1, *_refundProcessInfo));
  }

  void testCheckFareType_noTable()
  {
    _record3->fareTypeTblItemNo() = 0;
    CPPUNIT_ASSERT(_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckFareType_emptyTable()
  {
    _record3->fareTypeTblItemNo() = 1;
    CPPUNIT_ASSERT(!_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }

  void addFareTypeToDB(const FareType& dbft, const Indicator& ftAppl)
  {
    _record3->fareTypeTblItemNo() = 1;
    FakeRexTrx& trx = static_cast<FakeRexTrx&>(*_refTrx);
    trx._fttv->push_back(_memHandle(new FareTypeTable));
    trx._fttv->back()->fareType() = dbft;
    trx._fttv->back()->fareTypeAppl() = ftAppl;
  }
  void testCheckFareType_foundPermitted()
  {
    addFareTypeToDB("HEN", ' ');
    _fareClassAppInfo1->_fareType = "HEN";
    CPPUNIT_ASSERT(_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckFareType_foundForbidden()
  {
    addFareTypeToDB("HEN", 'N');
    _fareClassAppInfo1->_fareType = "HEN";
    CPPUNIT_ASSERT(!_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckFareType_notFoundPermitted()
  {
    addFareTypeToDB("HEN", ' ');
    _fareClassAppInfo1->_fareType = "CHICKEN";
    CPPUNIT_ASSERT(!_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckFareType_notFoundForbidden()
  {
    addFareTypeToDB("HEN", 'N');
    _fareClassAppInfo1->_fareType = "CHICKEN";
    CPPUNIT_ASSERT(_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckFareType_foundPermittedMany()
  {
    addFareTypeToDB("CHICKEN", ' ');
    addFareTypeToDB("HEN", ' ');
    addFareTypeToDB("CHICKEN", ' ');
    _fareClassAppInfo1->_fareType = "HEN";
    CPPUNIT_ASSERT(_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
  void testCheckFareType_foundForbiddenMany()
  {
    addFareTypeToDB("CHICKEN", 'N');
    addFareTypeToDB("HEN", 'N');
    addFareTypeToDB("CHICKEN", 'N');
    _fareClassAppInfo1->_fareType = "HEN";
    CPPUNIT_ASSERT(!_validator->checkFareType(*_fareUsage1, *_refundProcessInfo));
  }
};

std::ostream& operator<<(std::ostream& os, const std::vector<FareUsage*>& cont)
{
  std::copy(cont.begin(), cont.end(), std::ostream_iterator<FareUsage*>(os, " "));
  return os;
}

CPPUNIT_TEST_SUITE_REGISTRATION(RefundSolutionValidatorTest);
}
