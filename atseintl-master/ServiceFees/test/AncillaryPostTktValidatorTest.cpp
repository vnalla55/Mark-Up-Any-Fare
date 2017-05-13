#include "test/include/CppUnitHelperMacros.h"
#include "ServiceFees/AncillaryPostTktValidator.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include <vector>
#include <map>
#include "DataModel/AirSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/Diag877Collector.h"
#include "ServiceFees/OCFees.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/AncRequest.h"

namespace tse
{
class AncillaryPostTktValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPostTktValidatorTest);
  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_AdvTktY_Pass);
  CPPUNIT_TEST(testCheckAdvPurchaseTktInd_AdvTktX_Fail);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _farePath = _memHandle.create<FarePath>();
    _paxType = _memHandle.create<PaxType>();
    _itin = _memHandle.create<Itin>();
    _farePath->paxType() = _paxType;
    _farePath->itin() = _itin;
    _trx = _memHandle.create<PricingTrx>();
    _seg1 = _memHandle.create<AirSeg>();
    _ocFees = _memHandle.create<OCFees>();
    _trx->setRequest(_memHandle.create<AncRequest>());
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
    _tvlSegs.push_back(_seg1);
    _endOfJourney = _tvlSegs.end();
    _ocInfo = _memHandle.create<OptionalServicesInfo>();
    OcValidationContext ctx(*_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
    _validator = _memHandle.insert(new AncillaryPostTktValidator(ctx,
                                                                 _tvlSegs.begin(),
                                                                 _tvlSegs.end(),
                                                                 _endOfJourney,
                                                                 _ts2ss,
                                                                 true,
                                                                 true,
                                                                 true,
                                                                 0));
  }
  void tearDown() { _memHandle.clear(); }
  void testCheckAdvPurchaseTktInd_AdvTktY_Pass()
  {
    _ocInfo->advPurchTktIssue() = 'Y';
    CPPUNIT_ASSERT(_validator->checkAdvPurchaseTktInd(*_ocInfo));
  }
  void testCheckAdvPurchaseTktInd_AdvTktX_Fail()
  {
    _ocInfo->advPurchTktIssue() = 'X';
    CPPUNIT_ASSERT(!_validator->checkAdvPurchaseTktInd(*_ocInfo));
  }

private:
  TestMemHandle _memHandle;
  AncillaryPostTktValidator* _validator;
  FarePath* _farePath;
  Itin* _itin;
  PaxType* _paxType;
  AirSeg* _seg1;
  PricingTrx* _trx;
  std::vector<TravelSeg*> _tvlSegs;
  std::vector<TravelSeg*>::const_iterator _endOfJourney;
  Ts2ss _ts2ss;
  OCFees* _ocFees;
  OptionalServicesInfo* _ocInfo;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPostTktValidatorTest);
}
