#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "FreeBagService/BaggageAncillaryChargesValidator.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"


namespace tse
{

class BaggageAncillaryChargesValidatorDisclosureFinderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAncillaryChargesValidatorDisclosureFinderTest);

  CPPUNIT_TEST(testNoAllowance);
  CPPUNIT_TEST(testOneInAllowance);
  CPPUNIT_TEST(testTwoInAllowance);

  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->getOptions()->serviceGroupsVec().push_back(*_memHandle.create<RequestedOcFeeGroup>());
    _trx->getOptions()->serviceGroupsVec().front().setRequestedInformation(RequestedOcFeeGroup::DisclosureData);
    _trx->getOptions()->serviceGroupsVec().front().groupCode() = "BG";
    _trx->getOptions()->serviceGroupsVec().front().addAncillaryServiceType('A');
    _trx->getOptions()->serviceGroupsVec().front().addAncillaryServiceType('B');
    _trx->getOptions()->serviceGroupsVec().front().addAncillaryServiceType('C');
    _trx->getOptions()->serviceGroupsVec().front().addAncillaryServiceType('E');
    _bagTvl = _memHandle.create<BaggageTravel>();
    _bagTvl->_allowance = _memHandle.create<OCFees>();
    _baggageCharge = _memHandle.create<BaggageCharge>();
    _baggageCharge->subCodeInfo() = S5Builder(&_memHandle).withFltTktMerchIndAndDescription(BAGGAGE_CHARGE, "1", "").build();
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  BaggageTravel* _bagTvl;
  BaggageCharge* _baggageCharge;


  void testNoAllowance()
  {
    _bagTvl->_allowance->optFee() = &S7Builder(&_memHandle).withBaggagePcs(0).buildRef();
    BaggageAncillaryChargesValidator::DisclosureFinder disclosureFinder(*_trx, *_bagTvl);
    OptionalServicesInfo* optSrv(&S7Builder(&_memHandle).withBaggageOccurrence(1, -1).buildRef());

    disclosureFinder.processBaggageCharge(*optSrv, *_baggageCharge);

    CPPUNIT_ASSERT(_baggageCharge->matched1stBag());
    CPPUNIT_ASSERT(_baggageCharge->matched2ndBag());
  }

  void testOneInAllowance()
  {
    _bagTvl->_allowance->optFee() = &S7Builder(&_memHandle).withBaggagePcs(1).buildRef();
    BaggageAncillaryChargesValidator::DisclosureFinder disclosureFinder(*_trx, *_bagTvl);
    OptionalServicesInfo* optSrv(&S7Builder(&_memHandle).withBaggageOccurrence(1, -1).buildRef());

    disclosureFinder.processBaggageCharge(*optSrv, *_baggageCharge);

    CPPUNIT_ASSERT(!_baggageCharge->matched1stBag());
    CPPUNIT_ASSERT(_baggageCharge->matched2ndBag());
  }

  void testTwoInAllowance()
  {
    _bagTvl->_allowance->optFee() = &S7Builder(&_memHandle).withBaggagePcs(2).buildRef();
    BaggageAncillaryChargesValidator::DisclosureFinder disclosureFinder(*_trx, *_bagTvl);
    OptionalServicesInfo* optSrv(&S7Builder(&_memHandle).withBaggageOccurrence(1, -1).buildRef());

    disclosureFinder.processBaggageCharge(*optSrv, *_baggageCharge);

    CPPUNIT_ASSERT(!_baggageCharge->matched1stBag());
    CPPUNIT_ASSERT(!_baggageCharge->matched2ndBag());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAncillaryChargesValidatorDisclosureFinderTest);
}
