#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "ServiceFees/OptionalFeeConcurValidator.h"
#include "ServiceFees/SliceAndDice.h"
#include "gmock/gmock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <utility>

namespace tse
{
class MockSliceAndDice : public SliceAndDice
{
public:
  MockSliceAndDice(PricingTrx& trx,
                   const ItinBoolMap& isInternational,
                   const ItinBoolMap& isRoundTrip,
                   bool& stopMatchProcess,
                   const Ts2ss& ts2ss,
                   const std::map<const CarrierCode, std::vector<ServiceGroup*> >& cXrGrp,
                   const std::vector<ServiceGroupInfo*>& allGroupCodes,
                   const bool& shoppingOCF,
                   const bool& needFirstMatchOnly,
                   int16_t& numberOfOcFeesForItin,
                   bool& timeOut,
                   boost::mutex& mutex) : SliceAndDice(trx, isInternational, isRoundTrip, stopMatchProcess, ts2ss,
                                                       cXrGrp, allGroupCodes, shoppingOCF, needFirstMatchOnly,
                                                       numberOfOcFeesForItin, timeOut, mutex) {}
  virtual ~MockSliceAndDice() {}

  MOCK_CONST_METHOD2(processServiceFeesGroup99, void(ServiceFeesGroup*, int));

};

class SliceAndDiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SliceAndDiceTest);
  CPPUNIT_TEST(testProcessServiceFeesGroup99);
  CPPUNIT_TEST(testProcessServiceFeesGroupNot99);
  CPPUNIT_TEST(testProcessServiceFeesGroup99NotAsRq);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("MAX_PSS_OUTPUT", 0, "OUTPUT_LIMITS");
    _trx.setRequest(_memHandle.create<PricingRequest>());
    _trx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx.billing() = _memHandle.create<Billing>();
    _trx.billing()->partitionID() = RuleConst::SABRE1B;
    _trx.billing()->aaaCity() = "AA";
    _falseVal = false;
    _numberOfOcFeesForItin = 100;
    _sliceDice = _memHandle.create<MockSliceAndDice>(_trx,
                    ItinBoolMap(),
                    ItinBoolMap(),
                    _falseVal,
                    Ts2ss(),
                    std::map<const CarrierCode, std::vector<ServiceGroup*> >(),
                    std::vector<ServiceGroupInfo*>(),
                    _falseVal,
                    _falseVal,
                    _numberOfOcFeesForItin,
                    _falseVal,
                    _mutex);
    _s6Validator = _memHandle.create<OptionalFeeConcurValidator>(_trx, nullptr);
    _serviceFeesGroup = _memHandle.create<ServiceFeesGroup>();
    _serviceFeesGroup->groupCode() = "99";
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testProcessServiceFeesGroup99()
  {
    EXPECT_CALL(*_sliceDice, processServiceFeesGroup99(testing::_, testing::_)).Times(1);
    _sliceDice->processSingleServiceFeesGroup(0, _serviceFeesGroup, false, *_s6Validator);
  }

  void testProcessServiceFeesGroupNot99()
  {
    _serviceFeesGroup->groupCode() = "BG";

    EXPECT_CALL(*_sliceDice, processServiceFeesGroup99(testing::_, testing::_)).Times(0);
    _sliceDice->processSingleServiceFeesGroup(0, _serviceFeesGroup, false, *_s6Validator);
  }

  void testProcessServiceFeesGroup99NotAsRq()
  {
    _trx.billing()->aaaCity() = "1234";

    EXPECT_CALL(*_sliceDice, processServiceFeesGroup99(testing::_, testing::_)).Times(0);
    _sliceDice->processSingleServiceFeesGroup(0, _serviceFeesGroup, false, *_s6Validator);
  }


  TestMemHandle _memHandle;
  PricingTrx _trx;
  boost::mutex _mutex;
  bool _falseVal;
  int16_t _numberOfOcFeesForItin;
  MockSliceAndDice* _sliceDice;
  OptionalFeeConcurValidator* _s6Validator;
  ServiceFeesGroup* _serviceFeesGroup;

};

CPPUNIT_TEST_SUITE_REGISTRATION(SliceAndDiceTest);
}
