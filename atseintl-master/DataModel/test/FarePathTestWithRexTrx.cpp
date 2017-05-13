#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ProcessTagInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Common/Money.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/NUCCurrencyConverter.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/ExcItin.h"
#include "test/include/TestConfigInitializer.h"
#include <string>
#include <boost/assign/std/vector.hpp>

namespace tse
{
using namespace boost::assign;

class FarePathTestWithRexTrx : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FarePathTestWithRexTrx);
  CPPUNIT_TEST(testGetNonrefundableAmount1);
  CPPUNIT_TEST(testGetNonrefundableAmount2);
  CPPUNIT_TEST(testGetNonrefundableMessage1);
  CPPUNIT_TEST(testGetNonrefundableMessage2);
  CPPUNIT_TEST(testUpdateTktEndorsement_Blank);
  CPPUNIT_TEST(testUpdateTktEndorsement_X);
  CPPUNIT_TEST(testUpdateTktEndorsement_W);
  CPPUNIT_TEST(testUpdateTktEndorsement_Y);

  CPPUNIT_TEST(testResidualPenaltyIndicator_carrierTrx);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_Blank_nonRefOnly);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_Blank_RefOnly);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_Blank_Mixed_RefAmountGreater);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_Blank_Mixed_nonRefAmountGreater);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_Blank_Mixed_AmountsEqual);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_N_nonRefOnly);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_N_RefOnly);
  CPPUNIT_TEST(testResidualPenaltyIndicator_subscriberTrx_N_Mixed);

  CPPUNIT_TEST(testSetMultipleTourCodeWarningRegularPricing);
  CPPUNIT_TEST(testSetMultipleTourCodeWarningCmdPricing);
  CPPUNIT_TEST(testSetUseSecondRoeDate);
  CPPUNIT_TEST_SUITE_END();

  FarePath* _farePath;
  RexPricingTrx* _trx;
  NUCCurrencyConverter* _conv;
  ProcessTagPermutation* _perm;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<MockRexPricingTrx>();
    _perm = _memHandle.create<MockProcessTagPermutation>();
    _farePath = _memHandle.create<FarePath>();
    _conv = _memHandle.create<MockNUCCurrencyConverter>();

    _farePath->itin() = _trx->newItin().front();
    _farePath->setLowestFee31Perm(_perm);
    _trx->processTagPermutations().push_back(_perm);
  }

  void tearDown() { _memHandle.clear(); }

  void testGetNonrefundableAmount1()
  {
    PricingUnitSettings pu1, pu2;
    pu1.addFareUsage(_memHandle(new MockFareUsage("EU", "USD", 500.00))); // LAX-DFW
    pu2.addFareUsage(_memHandle(new MockFareUsage("BR", "USD", 3000.00))); // DFW-LON
    pu2.addFareUsage(_memHandle(new MockFareUsage("BR", "USD", 3000.00))); // LON-CHI
    pu1.addFareUsage(_memHandle(new MockFareUsage("EU", "USD", 500.00))); // CHI-LAX

    _farePath->pricingUnit() += &pu1.pricingUnit(), &pu2.pricingUnit();

    MoneyAmount amt = _farePath->getNonrefundableAmountInNUC(*_trx);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, amt, EPSILON);
  }

  void testGetNonrefundableAmount2()
  {
    PricingUnitSettings pu1, pu2;
    pu1.addFareUsage(_memHandle(new MockFareUsage("XPN", "NUC", 100.00, true))); // LAX-DFW
    pu2.addFareUsage(_memHandle(new MockFareUsage("BR", "NUC", 3000.00, false))); // DFW-LON
    pu2.addFareUsage(_memHandle(new MockFareUsage("BR", "NUC", 3000.00, false))); // LON-NYC
    pu1.addFareUsage(_memHandle(new MockFareUsage("ER", "NUC", 400.00, true))); // NYC-LAX

    _farePath->pricingUnit() += &pu1.pricingUnit(), &pu2.pricingUnit();
    _farePath->baseFareCurrency() = "USD";
    _farePath->calculationCurrency() = "NUC";

    MoneyAmount amt = _farePath->getNonrefundableAmountInNUC(*_trx);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, amt, EPSILON);
  }

  void testGetNonrefundableMessage1()
  {
    _farePath->_nonrefundableAmount = Money(0.0, "USD");
    _farePath->baseFareCurrency() = "USD";
    std::string msg = _farePath->getNonrefundableMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), msg);
  }

  void testGetNonrefundableMessage2()
  {
    FarePath* old = _farePath;
    MockFarePath fPath;
    _farePath = &fPath;

    _farePath->_nonrefundableAmount = Money(100.0, "USD");
    _farePath->baseFareCurrency() = "USD";
    std::string msg = _farePath->getNonrefundableMessage();
    CPPUNIT_ASSERT_EQUAL(std::string("USD100.00 NONREFUNDABLE"), msg);

    _farePath = old;
  }

  FarePath*
  createFarePathWithPermutationAndEndorsement(Indicator endorsement, bool noEndorsement = false)
  {
    MockFarePath* fp = _memHandle.create<MockFarePath>();
    ProcessTagPermutation* ptp = _memHandle.create<ProcessTagPermutation>();

    ProcessTagInfo* pti = _memHandle.create<ProcessTagInfo>();
    ptp->processTags().push_back(pti);

    VoluntaryChangesInfo* vci = _memHandle.create<VoluntaryChangesInfo>();
    vci->endorsement() = endorsement;

    VoluntaryChangesInfoW* vciw = _memHandle.create<VoluntaryChangesInfoW>();
    vciw->orig() = vci;

    *(pti->record3()) = *vciw;
    fp->setLowestFee31Perm(ptp);

    if (!noEndorsement)
    {
      TicketEndorseItem eItem;
      eItem.priorityCode = 1;
      eItem.endorsementTxt = "testing";
      fp->tktEndorsement().push_back(eItem);
    }

    return fp;
  }

  void testUpdateTktEndorsement_ZeroAmount()
  {
    FarePath* old = _farePath;
    _farePath = createFarePathWithPermutationAndEndorsement(' ', true);

    _farePath->_nonrefundableAmount = Money(0.0, "USD");
    _farePath->updateTktEndorsement();
    CPPUNIT_ASSERT(_farePath->tktEndorsement().empty());

    _farePath = old;
  }

  void testUpdateTktEndorsement_Blank()
  {
    FarePath* old = _farePath;
    _farePath = createFarePathWithPermutationAndEndorsement(' ');

    _farePath->_nonrefundableAmount = Money(100.0, "USD");
    _farePath->updateTktEndorsement();
    CPPUNIT_ASSERT_EQUAL(size_t(2), _farePath->tktEndorsement().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), _farePath->tktEndorsement().back().priorityCode);
    CPPUNIT_ASSERT_EQUAL(std::string("USD100.00 NONREFUNDABLE"),
                         _farePath->tktEndorsement().back().endorsementTxt);

    _farePath = old;
  }

  void testUpdateTktEndorsement_X()
  {
    FarePath* old = _farePath;
    _farePath = createFarePathWithPermutationAndEndorsement('X');

    _farePath->_nonrefundableAmount = Money(100.0, "USD");
    _farePath->updateTktEndorsement();
    CPPUNIT_ASSERT_EQUAL(size_t(1), _farePath->tktEndorsement().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), _farePath->tktEndorsement().back().priorityCode);
    CPPUNIT_ASSERT_EQUAL(std::string("USD100.00 NONREFUNDABLE"),
                         _farePath->tktEndorsement().back().endorsementTxt);

    _farePath = old;
  }

  void testUpdateTktEndorsement_Y()
  {
    FarePath* old = _farePath;
    _farePath = createFarePathWithPermutationAndEndorsement('Y');

    _farePath->_nonrefundableAmount = Money(100.0, "USD");
    _farePath->updateTktEndorsement();
    CPPUNIT_ASSERT_EQUAL(size_t(1), _farePath->tktEndorsement().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), _farePath->tktEndorsement().back().priorityCode);
    CPPUNIT_ASSERT_EQUAL(std::string("testing"), _farePath->tktEndorsement().back().endorsementTxt);

    _farePath = old;
  }

  void testUpdateTktEndorsement_W()
  {
    FarePath* old = _farePath;
    _farePath = createFarePathWithPermutationAndEndorsement('X');

    _farePath->_nonrefundableAmount = Money(100.0, "USD");
    _farePath->updateTktEndorsement();
    CPPUNIT_ASSERT_EQUAL(size_t(1), _farePath->tktEndorsement().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), _farePath->tktEndorsement().front().priorityCode);
    CPPUNIT_ASSERT_EQUAL(std::string("USD100.00 NONREFUNDABLE"),
                         _farePath->tktEndorsement().front().endorsementTxt);

    _farePath = old;
  }

  void testResidualPenaltyIndicator_carrierTrx()
  {
    ((MockRexPricingTrx*)_trx)->agent_.tvlAgencyPCC().clear();
    _trx->secondaryExcReqType().clear();

    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_S;

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_S,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_Blank_nonRefOnly()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_BLANK;

    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin();
    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin();

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_I,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_Blank_RefOnly()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_BLANK;

    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin();
    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin();

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_S,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_Blank_Mixed_RefAmountGreater()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_BLANK;

    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin(200);
    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin(400);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_S,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_Blank_Mixed_nonRefAmountGreater()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_BLANK;

    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin(400);
    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin(200);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_I,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_Blank_Mixed_AmountsEqual()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_BLANK;

    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin(350);
    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin(350);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_I,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_N_nonRefOnly()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_N;

    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin();
    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin();

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_I,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_N_RefOnly()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_N;

    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin();
    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin();

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_S,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testResidualPenaltyIndicator_subscriberTrx_N_Mixed()
  {
    ((MockProcessTagPermutation*)_perm)->vci_.residualInd() = ProcessTagPermutation::RESIDUAL_N;

    ((MockRexPricingTrx*)_trx)->addRefundablePricingUnitToExcItin();
    ((MockRexPricingTrx*)_trx)->addNonrefundablePricingUnitToExcItin();

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_N,
                         _farePath->residualPenaltyIndicator(*_trx));
  }

  void testSetMultipleTourCodeWarningRegularPricing()
  {
    FarePath farePath;

    farePath.setMultipleTourCodeWarning(PASS, false);
    CPPUNIT_ASSERT_EQUAL(false, farePath.multipleTourCodeWarning());

    farePath.setMultipleTourCodeWarning(FAIL, false);
    CPPUNIT_ASSERT_EQUAL(false, farePath.multipleTourCodeWarning());
  }

  void testSetMultipleTourCodeWarningCmdPricing()
  {
    FarePath farePath;

    farePath.setMultipleTourCodeWarning(PASS, true);
    CPPUNIT_ASSERT_EQUAL(false, farePath.multipleTourCodeWarning());

    farePath.setMultipleTourCodeWarning(FAIL, true);
    CPPUNIT_ASSERT_EQUAL(true, farePath.multipleTourCodeWarning());
  }

  void testSetUseSecondRoeDate()
  {
    FarePath farePath;
    CPPUNIT_ASSERT(!farePath.useSecondRoeDate());
    farePath.useSecondRoeDate() = true;
    CPPUNIT_ASSERT(farePath.useSecondRoeDate());
  }

private:
  class MockFarePath : public FarePath
  {
  public:
    std::string getNonrefundableMessage() const
    {
      std::ostringstream os;
      if (_nonrefundableAmount.value() > EPSILON)
      {
        os << std::setw(3) << _nonrefundableAmount.code();
        os << std::left << std::fixed << std::setprecision(2);
        os << _nonrefundableAmount.value() << " NONREFUNDABLE";
      }
      return os.str();
    }
  };

  class MockFareUsage : public FareUsage
  {
  public:
    MockFareUsage(const FareType& ft,
                  const CurrencyCode& curr,
                  MoneyAmount amt,
                  bool nonRefundable = false)
    {
      internal.fi.currency() = curr;
      internal.fi.fareAmount() = amt;
      internal.ptf.nucFareAmount() = amt;

      internal.fcai._fareType = ft;

      _paxTypeFare = &internal.ptf;
      _nonRefundable = nonRefundable;
    }

  protected:
    struct Internal
    {
      Internal()
      {
        f.setFareInfo(&fi);
        ptf.setFare(&f);
        ptf.fareClassAppInfo() = &fcai;
      }
      FareInfo fi;
      FareClassAppInfo fcai;
      PaxTypeFare ptf;
      Fare f;
    } internal;
  };

  class PricingUnitSettings
  {
    PricingUnit _pu;

  public:
    PricingUnit& pricingUnit() { return _pu; }
    PricingUnitSettings() { _pu.setTotalPuNucAmount(0.0); }
    PricingUnitSettings(FareUsage* fu)
    {
      _pu.setTotalPuNucAmount(0.0);
      addFareUsage(fu);
    }

    void addFareUsage(FareUsage* fu)
    {
      _pu.fareUsage() += fu;
      _pu.setTotalPuNucAmount(
          _pu.getTotalPuNucAmount() +
          convertToNUC(fu->paxTypeFare()->currency(), fu->paxTypeFare()->fareAmount()));
    }

    void calculateTotalNucAmount()
    {
      _pu.setTotalPuNucAmount(0.0);
      std::vector<FareUsage*>::iterator i = _pu.fareUsage().begin();
      for (; i != _pu.fareUsage().end(); ++i)
        _pu.setTotalPuNucAmount(
            _pu.getTotalPuNucAmount() +
            convertToNUC((*i)->paxTypeFare()->currency(), (*i)->paxTypeFare()->fareAmount()));
    }

  private:
    MoneyAmount convertToNUC(const CurrencyCode& curr, MoneyAmount amt)
    {
      MoneyAmount factor = 1.0;
      if (curr == "USD")
        factor = 1.0;
      if (curr == "CAD")
        factor = 0.98;
      if (curr == "CHF")
        factor = 0.88;
      if (curr == "PLN")
        factor = 0.40;
      return factor * amt;
    }
  };

  class MockProcessTagPermutation : public ProcessTagPermutation
  {
  public:
    MockProcessTagPermutation()
    {
      pti_.record3()->orig() = &vci_;
      processTags() += &pti_;
    }

    VoluntaryChangesInfo vci_;
    ProcessTagInfo pti_;
  };

  class MockNUCCurrencyConverter : public NUCCurrencyConverter
  {
  public:
    bool convert(CurrencyConversionRequest& request,
                 CurrencyCollectionResults* results,
                 CurrencyConversionCache* cache = 0)
    {
      MoneyAmount factor = 1.00;
      if (request.target().code() == "USD")
        factor = 1.00;
      if (request.target().code() == "PLN")
        factor = 2.448550;
      if (request.target().code() == "CAD")
        factor = 1.010750;
      if (request.target().code() == "CHF")
        factor = 1.125200;
      request.target().value() = factor * request.source().value();
      return true;
    }
  };

  class MockRexPricingTrx : public RexPricingTrx
  {
    TestMemHandle _memHandle;

  public:
    MockRexPricingTrx()
    {
      _request = &request_;
      request_.ticketingAgent() = &agent_;
      agent_.tvlAgencyPCC() = "KUKU";
      _secondaryExcReqType = "FE";
      _exchangeItin += &excItin_;
      excItin_.farePath() += &excFarePath_;
      newItin().push_back(&itin_);
    }

    void addNonrefundablePricingUnitToExcItin(double amount = 100.0)
    {
      PricingUnitSettings* pu = _memHandle(
          new PricingUnitSettings(_memHandle(new MockFareUsage("XPN", "NUC", amount, true))));
      excFarePath_.pricingUnit() += &pu->pricingUnit();
    }

    void addRefundablePricingUnitToExcItin(double amount = 100.0)
    {
      PricingUnitSettings* pu =
          _memHandle(new PricingUnitSettings(_memHandle(new MockFareUsage("EB", "NUC", amount))));
      excFarePath_.pricingUnit() += &pu->pricingUnit();
    }
    ~MockRexPricingTrx() { _memHandle.clear(); }
    PricingRequest request_;
    Agent agent_;
    ExcItin excItin_;
    Itin itin_;
    FarePath excFarePath_;
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(FarePathTestWithRexTrx);
}
