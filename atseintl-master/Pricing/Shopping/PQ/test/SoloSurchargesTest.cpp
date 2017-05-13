#include "test/include/CppUnitHelperMacros.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Pricing/Shopping/PQ/SoloSurcharges.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestItinFactory.h"
#include "test/testdata/TestTravelSegFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestArunkSegFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{

class MockTravelSeg : public TravelSeg
{
public:
  MockTravelSeg* clone(DataHandle& dh) const
  {
    TSE_ASSERT(false);
    return 0;
  }
};

class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare() : _yqyr(1), _surcharges(1), _nucFareAmount(0) {}

  MoneyAmount& nucFareAmount() { return _nucFareAmount; }
  const MoneyAmount nucFareAmount() const { return _nucFareAmount; }

  MoneyAmount _yqyr;
  MoneyAmount _surcharges;
  MoneyAmount _nucFareAmount;
};

namespace
{
class MockSoloSurcharges : public SoloSurcharges
{
public:
  MockSoloSurcharges(ShoppingTrx& trx,
                     bool surchargesDetailsEnabled,
                     size_t numFaresForCAT12Calculation)
    : SoloSurcharges(writeConfig(trx, surchargesDetailsEnabled, numFaresForCAT12Calculation)),
      _surchargesCounter(0),
      _yqyrCounter(0),
      _valid(true)
  {
  }

  static ShoppingTrx&
  writeConfig(ShoppingTrx& trx, bool surchargesDetailsEnabled, size_t numFaresForCAT12Calculation)
  {
    TestConfigInitializer::setValue("SOLO_SURCHARGES", "Y", "SHOPPING_DIVERSITY", true);
    TestConfigInitializer::setValue(
        "NUM_FARES_FOR_CAT12_CALCULATION", numFaresForCAT12Calculation, "SHOPPING_DIVERSITY", true);
    TestConfigInitializer::setValue(
        "SHOPPING_NUMBER_OF_FARES_TO_PROCESS_THRESHOLD", 10, "FARESV_SVC", true);
    trx.diagnostic().diagnosticType() = (surchargesDetailsEnabled ? Diagnostic923 : DiagnosticNone);
    return trx;
  }

  virtual PaxTypeFareVec getApplicableFares(FareMarket& fareMarket) override
  {
    const PaxTypeFareVec& allFares = fareMarket.allPaxTypeFare();
    const size_t count = std::min(_numFaresForCAT12Calculation, allFares.size());
    return PaxTypeFareVec(allFares.begin(), allFares.begin() + count);
  }

  bool isValid(PaxTypeFare* paxTypeFare) override { return true; }

  bool preparePTFForValidation(const uint32_t carrierKey, PaxTypeFare& ptf) override
  {
    return true;
  }

  bool calculateSurcharges(FarePath* farePath, PricingUnit* pricingUnit) override
  {
    ++_surchargesCounter;
    return _valid;
  }

  TaxResponse* calculateYQYR(const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                             FarePathWrapper& farePathWrapper,
                             FareMarket* fareMarket,
                             const PaxTypeFare* paxTypeFare,
                             const uint16_t sopId) override
  {
    ++_yqyrCounter;
    return _trx.dataHandle().create<TaxResponse>();
  }

  MoneyAmount getSurcharges(FarePath* farePath) override
  {
    return dynamic_cast<MockPaxTypeFare*>(
               farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare())->_surcharges;
  }

  MoneyAmount
  getYQYR(FarePath& farePath, PaxTypeFare& paxTypeFare, TaxResponse& taxResponse) override
  {
    return dynamic_cast<MockPaxTypeFare&>(paxTypeFare)._yqyr;
  }

  ScopedFMGlobalDirSetter*
  createScopedFMGlobalDirSetter(FareMarket* fareMarket, const SOPUsage& sopUsage) override
  {
    return 0;
  }

  int _surchargesCounter;
  int _yqyrCounter;
  bool _valid;
};
}

class SoloSurchargesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloSurchargesTest);

  CPPUNIT_TEST(testMultiValid);
  CPPUNIT_TEST(testNotValid);
  CPPUNIT_TEST(testMultiSurchargesComputation_1);
  CPPUNIT_TEST(testMultiSurchargesComputation_2);
  CPPUNIT_TEST(testMultiSurchargesComputation_3);
  CPPUNIT_TEST(testMultiSurchargesComputation_sort);
  CPPUNIT_TEST(testSopUsgaeItinCopy);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = createTrx(_memHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    _trx->journeyItin() = _memHandle.create<Itin>();
    _trx->journeyItin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml"));
    _trx->journeyItin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml"));
  }

  void tearDown()
  {
    _trx->legs().clear();
    _trx->fareMarket().clear();
    _trx->paxType().clear();
    _memHandle.clear();
  }

  void testNotValid()
  {
    MockSoloSurcharges soloSurcharges(*_trx, false, 10);
    soloSurcharges._valid = false;
    soloSurcharges.process();

    for (ShoppingTrx::Leg& leg : _trx->legs())
    {
      for (ShoppingTrx::SchedulingOption& sop : leg.sop())
      {
        if (!sop.itin())
          continue;
        for (FareMarket* fareMarket : sop.itin()->fareMarket())
        {
          for (PaxTypeBucket& cortege : fareMarket->paxTypeCortege())
            CPPUNIT_ASSERT(cortege.cxrPrecalculatedTaxes().empty());
        }
      }
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), soloSurcharges.surchargesDetailsMap().size());

    for (size_t ptfIdx = 0; ptfIdx < _ptf.size(); ++ptfIdx)
    {
      CPPUNIT_ASSERT_EQUAL(MoneyAmount(ptfIdx + 1), _ptf[ptfIdx]->nucFareAmount());
    }
  }

  void testMultiValid()
  {
    MockSoloSurcharges soloSurcharges(*_trx, true, 3);
    soloSurcharges.process();
    CPPUNIT_ASSERT_EQUAL(3, soloSurcharges._surchargesCounter);
    CPPUNIT_ASSERT_EQUAL(1, soloSurcharges._yqyrCounter);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), soloSurcharges.surchargesDetailsMap().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4),
                         soloSurcharges.surchargesDetailsMap().begin()->second.size());
  }

  void testMultiSurchargesComputation(size_t numFaresForCAT12Calculation)
  {
    MockSoloSurcharges soloSurcharges(*_trx, true, numFaresForCAT12Calculation);

    soloSurcharges.process();

    for (ShoppingTrx::Leg& leg : _trx->legs())
    {
      for (ShoppingTrx::SchedulingOption& sop : leg.sop())
      {
        if (!sop.itin())
          continue;
        for (FareMarket* fareMarket : sop.itin()->fareMarket())
        {
          for (PaxTypeBucket& cortege : fareMarket->paxTypeCortege())
            CPPUNIT_ASSERT(cortege.cxrPrecalculatedTaxes().empty());
        }
      }
    }

    CPPUNIT_ASSERT_EQUAL(numFaresForCAT12Calculation,
                         static_cast<size_t>(soloSurcharges._surchargesCounter));
    CPPUNIT_ASSERT_EQUAL(1, soloSurcharges._yqyrCounter);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), soloSurcharges.surchargesDetailsMap().size());

    for (size_t ptfIdx = 0; ptfIdx < numFaresForCAT12Calculation; ++ptfIdx)
    {
      CPPUNIT_ASSERT_EQUAL(MoneyAmount(ptfIdx + 1 + _ptf[0]->_yqyr + _ptf[ptfIdx]->_surcharges),
                           _ptf[ptfIdx]->nucFareAmount());
    }

    for (size_t ptfIdx = numFaresForCAT12Calculation; ptfIdx < 3; ++ptfIdx)
    {
      CPPUNIT_ASSERT_EQUAL(MoneyAmount(ptfIdx + 1 + _ptf[0]->_yqyr +
                                       _ptf[numFaresForCAT12Calculation - 1]->_surcharges),
                           _ptf[ptfIdx]->nucFareAmount());
    }
  }

  void testMultiSurchargesComputation_1() { testMultiSurchargesComputation(1); }

  void testMultiSurchargesComputation_2() { testMultiSurchargesComputation(2); }

  void testMultiSurchargesComputation_3() { testMultiSurchargesComputation(3); }

  void testMultiSurchargesComputation_sort()
  {
    _ptf[0]->_surcharges = 1000;
    _ptf[1]->_surcharges = 0;
    _ptf[2]->_surcharges = 100000;
    _ptf[2]->nucFareAmount() = 0;

    FareMarket* fm = _ptf[0]->fareMarket();
    MockSoloSurcharges soloSurcharges(*_trx, true, 2);

    soloSurcharges.process();

    CPPUNIT_ASSERT(fm->allPaxTypeFare()[0] == _ptf[1]);
    CPPUNIT_ASSERT(fm->allPaxTypeFare()[1] == _ptf[2]);
    CPPUNIT_ASSERT(fm->allPaxTypeFare()[2] == _ptf[0]);
  }

  void testSopUsgaeItinCopy()
  {
    MockSoloSurcharges soloSurcharges(*_trx, true, 10);

    applicableSOPs[0][0][0][0].itin_->calculationCurrency() = "";
    applicableSOPs[1][0][0][0].itin_->calculationCurrency() = "";

    _trx->journeyItin()->calculationCurrency() = CurrencyCode("ABC");
    soloSurcharges.process();

    CPPUNIT_ASSERT_EQUAL(CurrencyCode(""), applicableSOPs[0][0][0][0].itin_->calculationCurrency());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode(""), applicableSOPs[1][0][0][0].itin_->calculationCurrency());
  }

protected:
  ShoppingTrx* createTrx(TestMemHandle& dataHandle, const std::string& filename)
  {
    ShoppingTrx* trx = TestShoppingTrxFactory::create(filename);
    _ptf.resize(5);

    PaxType* paxType = dataHandle.create<PaxType>();
    trx->paxType().push_back(paxType);

    ShoppingTrx::Leg* pLeg1 = dataHandle.create<ShoppingTrx::Leg>();
    ShoppingTrx::Leg* pLeg2 = dataHandle.create<ShoppingTrx::Leg>();
    Itin* itin1 = dataHandle.create<Itin>();
    Itin* itin2 = dataHandle.create<Itin>();

    for (size_t i = 0; i < _ptf.size(); ++i)
    {
      _ptf[i] = dataHandle.create<MockPaxTypeFare>();
      _ptf[i]->setFare(dataHandle.create<Fare>());
      FareInfo* fareInfo = dataHandle.create<FareInfo>();
      fareInfo->directionality() = BOTH;
      _ptf[i]->fare()->setFareInfo(fareInfo);
      _ptf[i]->actualPaxType() = paxType;
    }

    FareMarket* fm1 = dataHandle.create<FareMarket>();
    FareMarket* fm2 = dataHandle.create<FareMarket>();
    FareMarket* fm3 = dataHandle.create<FareMarket>();

    trx->fareMarket().push_back(fm1);
    trx->fareMarket().push_back(fm2);
    trx->fareMarket().push_back(fm3);

    for (size_t i = 0; i < _ptf.size(); ++i)
    {
      _ptf[i]->setComponentValidationForCarrier(0, false, 0);
      _ptf[i]->setFlightBitmapSize(1);
      _ptf[i]->nucFareAmount() = i + 1;
      _ptf[i]->_yqyr = 10 * (i + 1);
      _ptf[i]->_surcharges = 100 * (i + 1);
    }

    _ptf[0]->fareMarket() = fm1;
    fm1->allPaxTypeFare().push_back(_ptf[0]);
    _ptf[1]->fareMarket() = fm1;
    fm1->allPaxTypeFare().push_back(_ptf[1]);
    _ptf[2]->fareMarket() = fm1;
    fm1->allPaxTypeFare().push_back(_ptf[2]);
    _ptf[3]->fareMarket() = fm2;
    fm2->allPaxTypeFare().push_back(_ptf[3]);
    _ptf[4]->fareMarket() = fm3;
    fm3->allPaxTypeFare().push_back(_ptf[4]);

    fm1->legIndex() = 0;
    fm2->legIndex() = 0;
    fm3->legIndex() = 1;

    itin1->fareMarket().push_back(fm1);
    itin1->fareMarket().push_back(fm2);
    itin1->travelSeg().push_back(dataHandle.create<MockTravelSeg>());
    itin2->fareMarket().push_back(fm3);
    itin2->travelSeg().push_back(dataHandle.create<MockTravelSeg>());

    pLeg1->sop().push_back(ShoppingTrx::SchedulingOption(itin1, 0));
    pLeg2->sop().push_back(ShoppingTrx::SchedulingOption(itin2, 1));

    trx->legs().push_back(*pLeg1);
    trx->legs().push_back(*pLeg2);

    fm1->paxTypeCortege().resize(1);
    fm1->paxTypeCortege().front().requestedPaxType() = paxType;
    fm2->paxTypeCortege().resize(1);
    fm2->paxTypeCortege().front().requestedPaxType() = paxType;
    fm3->paxTypeCortege().resize(1);
    fm3->paxTypeCortege().front().requestedPaxType() = paxType;

    applicableSOPs.push_back(dataHandle.create<ApplicableSOP>());
    applicableSOPs.push_back(dataHandle.create<ApplicableSOP>());

    applicableSOPs[0][0][0].push_back(SOPUsage());
    applicableSOPs[1][0][0].push_back(SOPUsage());

    applicableSOPs[0][0][0][0].applicable_ = true;
    applicableSOPs[0][0][0][0].origSopId_ = 0;
    applicableSOPs[0][0][0][0].itin_ = itin1;
    applicableSOPs[0][0][0][0].startSegment_ = applicableSOPs[0][0][0][0].endSegment_ = 0;
    applicableSOPs[1][0][0][0].applicable_ = false;
    applicableSOPs[1][0][0][0].origSopId_ = 0;
    applicableSOPs[1][0][0][0].itin_ = itin2;
    applicableSOPs[1][0][0][0].startSegment_ = applicableSOPs[1][0][0][0].endSegment_ = 0;

    fm1->getApplicableSOPs() = applicableSOPs[0];
    fm3->getApplicableSOPs() = applicableSOPs[1];

    trx->journeyItin() = dataHandle.create<Itin>();

    trx->getRequest()->ticketingAgent() = trx->dataHandle().create<Agent>();

    return trx;
  }

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  std::vector<MockPaxTypeFare*> _ptf;
  std::vector<ApplicableSOP*> applicableSOPs;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SoloSurchargesTest);
}
