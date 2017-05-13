#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include "Diagnostic/Diag689Collector.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/RexBaseRequest.h"
#include "RexPricing/RexAdvResTktValidator.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{
using namespace std;
using boost::assign::list_of;

class RexAdvResTktValidatorStub : public RexAdvResTktValidator
{
public:
  RexAdvResTktValidatorStub(RexPricingTrx& trx,
                            Itin& newItin,
                            FarePath& newFarePath,
                            FarePath& excFarePath,
                            RepriceSolutionValidator::AdvResOverrideCache& _advResOverrideCache,
                            Diag689Collector*& dc,
                            const GenericRexMapper& grm)
    : RexAdvResTktValidator(trx, newItin, newFarePath, excFarePath, _advResOverrideCache, dc, grm),
      _ruleControllerReturnValueIndex(0),
      _ruleControllerReturnValues(),
      _advResOverrides()
  {
  }

  const AdvResOverride& getAdvResOverride(uint16_t index) { return _advResOverrides[index]; }

  size_t getAdvResOverridesSize() { return _advResOverrides.size(); }

  void setUpRuleControllerReturnValues(const vector<bool>& values)
  {
    _ruleControllerReturnValues.clear();
    _ruleControllerReturnValues.insert(
        _ruleControllerReturnValues.begin(), values.begin(), values.end());
  }

private:
  virtual bool performRuleControllerValidation(FareUsage& fu, PricingUnit& pu)
  {
    _advResOverrides.push_back(*pu.volChangesAdvResOverride());

    if (_ruleControllerReturnValueIndex < _ruleControllerReturnValues.size())
      return _ruleControllerReturnValues[_ruleControllerReturnValueIndex++];
    return true;
  }

  uint16_t _ruleControllerReturnValueIndex;
  vector<bool> _ruleControllerReturnValues;
  vector<AdvResOverride> _advResOverrides;
};

class RexAdvResTktValidatorAdvPurTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexAdvResTktValidatorAdvPurTest);

  CPPUNIT_TEST(testIgnoreTktBeforeDeparture1PriorOfDeparture);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture1NoPriorOfDeparture);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture2PriorOfDeparture);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture2NoPriorOfDeparture);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture3PriorOfDeparture);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture3NoPriorOfDeparture);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture1Simultaneous);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture1NoSimultaneous);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture2Simultaneous);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture2DomesticSimultaneous);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture2NoSimultaneous);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture3Simultaneous);
  CPPUNIT_TEST(testIgnoreTktBeforeDeparture3NoSimultaneous);
  CPPUNIT_TEST(testPermIndepFromDate1);
  CPPUNIT_TEST(testPermIndepFromDate2);
  CPPUNIT_TEST(testPermIndepFromDate3);
  CPPUNIT_TEST(testFromDateForNotMapped1);
  CPPUNIT_TEST(testFromDateForNotMapped2);
  CPPUNIT_TEST(testFromDateForNotMapped3);
  CPPUNIT_TEST(testAdvResOverrideFromDateP1);
  CPPUNIT_TEST(testAdvResOverrideFromDateP2);
  CPPUNIT_TEST(testAdvResOverrideFromDateP3);
  CPPUNIT_TEST(testAdvResOverrideFromDateC1);
  CPPUNIT_TEST(testAdvResOverrideFromDateC2);
  CPPUNIT_TEST(testAdvResOverrideFromDateC3);
  CPPUNIT_TEST(testAdvResOverrideFromDate1);
  CPPUNIT_TEST(testAdvResOverrideFromDate2);
  CPPUNIT_TEST(testAdvResOverrideFromDate3);
  CPPUNIT_TEST(testAdvResOverrideFromDateO1);
  CPPUNIT_TEST(testAdvResOverrideFromDateO2);
  CPPUNIT_TEST(testAdvResOverrideFromDateO3);
  CPPUNIT_TEST(testAdvResOverrideToDateJ1);
  CPPUNIT_TEST(testAdvResOverrideToDateJ2);
  CPPUNIT_TEST(testAdvResOverrideToDateJ3);
  CPPUNIT_TEST(testAdvResOverrideToDate1);
  CPPUNIT_TEST(testAdvResOverrideToDate2);
  CPPUNIT_TEST(testAdvResOverrideToDate3);
  CPPUNIT_TEST(testAdvResOverrideToDateF1);
  CPPUNIT_TEST(testAdvResOverrideToDateF2);
  CPPUNIT_TEST(testAdvResOverrideToDateF3);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction1);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction2);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction3);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction1Simultaneus);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction2Simultaneus);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction3Simultaneus);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction1FromInd);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction2FromInd);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktAfterResRestriction3FromInd);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktBeforeDeparture1);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktBeforeDeparture2);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktBeforeDeparture3);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktBeforeDeparture1Y);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktBeforeDeparture2Y);
  CPPUNIT_TEST(testAdvResOverrideIgnoreTktBeforeDeparture3Y);
  CPPUNIT_TEST(testNoT988Conf1);
  CPPUNIT_TEST(testNoT988Conf2);
  CPPUNIT_TEST(testNoT988Conf3);

  CPPUNIT_TEST_SUITE_END();

public:
  typedef DateTime DT;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle(new RexPricingTrx);
    _trx->exchangeItin().push_back(_memHandle(new ExcItin));
    _trx->exchangeItin().front()->geoTravelType() = GeoTravelType::International;
    _trx->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;

    RexBaseRequest* request = _memHandle.create<RexBaseRequest>();
    request->ticketingAgent() = _memHandle.create<Agent>();
    _trx->setRequest(request);

    _allRepricePTFs = _memHandle(new std::vector<const PaxTypeFare*>);

    _newItin = _memHandle.create<Itin>();
    _trx->newItin().push_back(_newItin);
    _trx->itin().push_back(_newItin);
    _excFarePath = _memHandle.create<FarePath>();
    _newFarePath = _memHandle.create<FarePath>();
    _newItin->farePath().push_back(_newFarePath);

    _currentPU = 0;
    _currentFU = 0;
    _fareCompNumber = 0;
  }

  void tearDown()
  {
    _memHandle.clear();
    _currentPU = 0;
    _currentFU = 0;
    _fareCompNumber = 0;
  }

  void setApplyReissueExchange(bool value)
  {
    if (value)
      _trx->setRexPrimaryProcessType('A');
    else
      _trx->setRexPrimaryProcessType(' ');
  }

  void setPreviousExchangeDT(DateTime dateTime) { _trx->previousExchangeDT() = dateTime; }

  void setOriginalTktIssueDT(DateTime dateTime) { _trx->setOriginalTktIssueDT() = dateTime; }

  void setCurrentTicketingDT(DateTime dateTime) { _trx->currentTicketingDT() = dateTime; }

  void setUpPermutation(const vector<string>& indicators)
  {
    _permutation = _memHandle.create<ProcessTagPermutation>();
    vector<string>::const_iterator it = indicators.begin();
    uint16_t fareCompNumber = 0;
    for (; it != indicators.end(); ++it)
    {
      ProcessTagInfo* pti = _memHandle.create<ProcessTagInfo>();
      setUpPti(pti, *it, fareCompNumber);
      _permutation->processTags().push_back(pti);
      ++fareCompNumber;
    }

    if (_rartv)
      _rartv->_permutation = _permutation;
  }

  void setUpPermutationWithoutT988(uint16_t size)
  {
    _permutation = _memHandle.create<ProcessTagPermutation>();
    for (uint16_t index = 0; index < size; ++index)
    {
      ProcessTagInfo* pti = _memHandle.create<ProcessTagInfo>();
      pti->fareCompInfo() = _trx->exchangeItin().front()->fareComponent()[index];
      _permutation->processTags().push_back(pti);
    }

    if (_rartv)
      _rartv->_permutation = _permutation;
  }

  void setUpPti(ProcessTagInfo* pti, const string& indicators, const uint16_t fareCompNumber)
  {
    ReissueSequence* rs = _memHandle.create<ReissueSequence>();
    pti->reissueSequence()->orig() = rs;
    pti->fareCompInfo() = _trx->exchangeItin().front()->fareComponent()[fareCompNumber];
    string::const_iterator it = indicators.begin();
    rs->fromAdvResInd() = *(it++); // values: 'P','C',' ','O'
    rs->toAdvResInd() = *(it++); // values: 'J',' ','F'
    rs->ticketResvInd() = *(it++); // values: ' ','X','Y'
  }

  PricingUnit* addPU()
  {
    closePU();

    _currentPU = _memHandle.create<PricingUnit>();
    _newFarePath->pricingUnit().push_back(_currentPU);

    return _currentPU;
  }

  void closePU()
  {
    if (_currentPU)
    {
      closeFU();
      _currentPU = 0;
    }
  }

  FareUsage* addFU(bool inbound,
                   FareMarket::FareRetrievalFlags retrievalFlag = FareMarket::RetrievCurrent)
  {
    closeFU();

    _currentFU = _memHandle.create<FareUsage>();
    _currentFU->inbound() = inbound;
    _currentPU->fareUsage().push_back(_currentFU);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    _currentFU->paxTypeFare() = ptf;
    {
      //      _allRepricePTFs->push_back(ptf);
    }

    FareMarket::RetrievalInfo* retrievalInfo = _memHandle.create<FareMarket::RetrievalInfo>();
    retrievalInfo->_flag = retrievalFlag;
    ptf->retrievalInfo() = retrievalInfo;

    FareMarket* fm = _memHandle.create<FareMarket>();
    //	fm->origin() = _memHandle.create<Loc>();
    //	fm->destination() = _memHandle.create<Loc>();
    ptf->fareMarket() = fm;

    return _currentFU;
  }

  void closeFU()
  {
    if (_currentFU)
    {
      _currentFU->paxTypeFare()->fareMarket()->boardMultiCity() =
          _currentFU->travelSeg().front()->boardMultiCity();
      _currentFU->paxTypeFare()->fareMarket()->offMultiCity() =
          _currentFU->travelSeg().back()->offMultiCity();
      _currentFU = 0;
    }
  }

  TravelSeg* addTS(LocCode src,
                   LocCode dst,
                   DateTime bookingDT,
                   DateTime departureDT,
                   TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED)
  {
    TravelSeg* ts = _memHandle.create<AirSeg>();
    ts->boardMultiCity() = src;
    ts->offMultiCity() = dst;
    ts->bookingDT() = bookingDT;
    ts->departureDT() = departureDT;
    ts->changeStatus() = changeStatus;

    _currentFU->travelSeg().push_back(ts);
    _currentPU->travelSeg().push_back(ts);
    _newItin->travelSeg().push_back(ts);

    return ts;
  }

  FareCompInfo* addFCI(LocCode src, LocCode dst)
  {
    FareCompInfo* fci = _memHandle.create<FareCompInfo>();
    _trx->exchangeItin().front()->fareComponent().push_back(fci);

    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->origin() = _memHandle.create<Loc>();
    fm->destination() = _memHandle.create<Loc>();

    fci->fareMarket() = fm;

    fm->boardMultiCity() = src;
    fm->offMultiCity() = dst;

    fci->fareCompNumber() = ++_fareCompNumber;

    return fci;
  }

  void createRexAdvResTktValidator(std::vector<uint16_t> qam)
  {
    GenericRexMapper* grm = _memHandle(new GenericRexMapper(*_trx, _allRepricePTFs));
    grm->_quickAccessMap = qam;
    //	grm->map();

    _rartv = _memHandle.insert(new RexAdvResTktValidatorStub(
        *_trx, *_newItin, *_newFarePath, *_excFarePath, _cache, _dc, *grm));
  }

  void createRexAdvResTktValidator(std::vector<uint16_t> qam, const vector<bool>& values)
  {
    createRexAdvResTktValidator(qam);
    _rartv->setUpRuleControllerReturnValues(values);
  }

  void setUpConfiguration1()
  {
    setOriginalTktIssueDT(DT(2012, 11, 1));
    setCurrentTicketingDT(DT(2012, 11, 13));

    // NEW ITIN
    addPU();
    addFU(false);
    addTS("KRK", "MOV", DT(2012, 11, 14), DT(2012, 11, 14));
    addFU(true);
    addTS("MOV", "KYI", DT(2012, 11, 16), DT(2012, 11, 16));
    addTS("KYI", "KRK", DT(2012, 11, 17), DT(2012, 11, 18));
    addPU();
    addFU(false);
    addTS("KRK", "LON", DT(2012, 11, 20), DT(2012, 11, 20));
    addTS("LON", "DFW", DT(2012, 11, 22), DT(2012, 11, 22));
    closeFU();
    closePU();

    // OLD ITIN
    addFCI("MOV", "KRK");
    addFCI("KRK", "DFW");
    addFCI("DFW", "MOV");

    createRexAdvResTktValidator(list_of(0)(1)(2));
  }

  void setUpConfiguration2()
  {
    setApplyReissueExchange(true);
    setPreviousExchangeDT(DT(2012, 11, 5));
    setOriginalTktIssueDT(DT(2012, 11, 1));
    setCurrentTicketingDT(DT(2012, 11, 17));

    // NEW ITIN
    addPU();
    addFU(false);
    addTS("KRK", "MOV", DT(2012, 11, 14), DT(2012, 11, 14), TravelSeg::UNCHANGED);
    addFU(true, FareMarket::RetrievHistorical);
    addTS("MOV", "KYI", DT(2012, 11, 16, 23, 31, 0), DT(2012, 11, 16));
    addTS("KYI", "KRK", DT(2012, 11, 17), DT(2012, 11, 18), TravelSeg::UNCHANGED);
    addPU();
    addFU(false, FareMarket::RetrievHistorical);
    addTS("KRK", "LON", DT(2012, 11, 20), DT(2012, 11, 20), TravelSeg::UNCHANGED);
    addTS("LON", "DFW", DT(2012, 11, 22), DT(2012, 11, 22), TravelSeg::UNCHANGED);
    closeFU();
    closePU();

    // OLD ITIN
    addFCI("MOV", "KRK");
    addFCI("KRK", "DFW");
    addFCI("DFW", "MOV");

    createRexAdvResTktValidator(list_of(0)(0)(2), list_of(true)(true)(false));
  }

  void setUpConfiguration3()
  {
    setApplyReissueExchange(true);
    setPreviousExchangeDT(DT(2012, 11, 5));
    setOriginalTktIssueDT(DT(2012, 11, 1));
    setCurrentTicketingDT(DT(2012, 11, 16));

    // NEW ITIN
    addPU();
    addFU(false, FareMarket::RetrievHistorical);
    addTS("KRK", "MOV", DT(2012, 11, 14), DT(2012, 11, 14), TravelSeg::UNCHANGED);
    addFU(true);
    addTS("MOV", "KYI", DT(2012, 11, 16), DT(2012, 11, 16));
    addTS("KYI", "KRK", DT(2012, 11, 17), DT(2012, 11, 18), TravelSeg::UNCHANGED);
    addPU();
    addFU(false);
    addTS("KRK", "LON", DT(2012, 11, 20), DT(2012, 11, 20));
    addTS("LON", "DFW", DT(2012, 11, 22), DT(2012, 11, 22));
    closeFU();
    closePU();

    // OLD ITIN
    addFCI("MOV", "KRK");
    addFCI("KRK", "DFW");
    addFCI("DFW", "MOV");

    createRexAdvResTktValidator(list_of(0)(0)(2), list_of(true)(true)(false));
  }

  void testIgnoreTktBeforeDeparture1PriorOfDeparture()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("P Y")("P Y")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture1NoPriorOfDeparture()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture2PriorOfDeparture()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("P  ")("P Y")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() == 0);
  }

  void testIgnoreTktBeforeDeparture2NoPriorOfDeparture()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture3PriorOfDeparture()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("P Y")("P  ")("P Y"));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture3NoPriorOfDeparture()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture1Simultaneous()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("P X")("P X")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture1NoSimultaneous()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture2Simultaneous()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("P  ")("P X")("P X "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() == 0);
  }

  void testIgnoreTktBeforeDeparture2DomesticSimultaneous()
  {
    _newItin->geoTravelType() = GeoTravelType::Domestic;
    setUpConfiguration2();
    setUpPermutation(list_of("P  ")("P X")("P X "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture2NoSimultaneous()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture3Simultaneous()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("P  ")("P X")("P X"));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testIgnoreTktBeforeDeparture3NoSimultaneous()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(_rartv->getAdvResOverridesSize() > 0);
  }

  void testPermIndepFromDate1()
  {
    setUpConfiguration1();
    _rartv->permutationIndependentSetUp();

    CPPUNIT_ASSERT_EQUAL(DT(2012, 11, 1), *_rartv->_permIndepFromDateAndLoc.first);
  }

  void testPermIndepFromDate2()
  {
    setUpConfiguration2();
    _rartv->permutationIndependentSetUp();

    CPPUNIT_ASSERT_EQUAL(DT(2012, 11, 5), *_rartv->_permIndepFromDateAndLoc.first);
  }

  void testPermIndepFromDate3()
  {
    setUpConfiguration3();
    _rartv->permutationIndependentSetUp();

    CPPUNIT_ASSERT_EQUAL(DT(2012, 11, 5), *_rartv->_permIndepFromDateAndLoc.first);
  }

  void verifyFromDateForNotMapped(const vector<DT>& dates)
  {
    uint16_t index = 0;
    for (const PricingUnit* pu : _newFarePath->pricingUnit())
    {
      CPPUNIT_ASSERT_EQUAL(dates[index], *_rartv->_fromDateForNotMapped[pu]);
      ++index;
    }
  }

  void testFromDateForNotMapped1()
  {
    setUpConfiguration1();
    _rartv->permutationIndependentSetUp();

    verifyFromDateForNotMapped(list_of(DT(2012, 11, 13))(DT(2012, 11, 13)));
  }

  void testFromDateForNotMapped2()
  {
    setUpConfiguration2();
    _rartv->permutationIndependentSetUp();

    verifyFromDateForNotMapped(list_of(DT(2012, 11, 17))(DT(2012, 11, 5)));
  }

  void testFromDateForNotMapped3()
  {
    setUpConfiguration3();
    _rartv->permutationIndependentSetUp();

    verifyFromDateForNotMapped(list_of(DT(2012, 11, 16))(DT(2012, 11, 16)));
  }

  void verifyAdvResOverrideFromDate(const vector<DT>& dates)
  {
    CPPUNIT_ASSERT_EQUAL(dates.size(), _rartv->getAdvResOverridesSize());

    for (uint16_t index = 0; index < dates.size(); ++index)
    {
      CPPUNIT_ASSERT_EQUAL(dates[index], _rartv->getAdvResOverride(index).fromDate());
    }
  }

  void testAdvResOverrideFromDateP1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 13))(DT(2012, 11, 1))(DT(2012, 11, 1)));
  }

  void testAdvResOverrideFromDateP2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 17))(DT(2012, 11, 17))(DT(2012, 11, 5)));
  }

  void testAdvResOverrideFromDateP3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 16))(DT(2012, 11, 16))(DT(2012, 11, 5)));
  }

  void testAdvResOverrideFromDateC1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("C  ")("C  ")("C  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 13))(DT(2012, 11, 13))(DT(2012, 11, 13)));
  }

  void testAdvResOverrideFromDateC2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("C  ")("C  ")("C  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 17))(DT(2012, 11, 17))(DT(2012, 11, 5)));
  }

  void testAdvResOverrideFromDateC3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("C  ")("C  ")("C  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 16))(DT(2012, 11, 16))(DT(2012, 11, 16)));
  }

  void testAdvResOverrideFromDate1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 13))(DT(2012, 11, 13))(DT(2012, 11, 13)));
  }

  void testAdvResOverrideFromDate2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 17))(DT(2012, 11, 17))(DT(2012, 11, 17)));
  }

  void testAdvResOverrideFromDate3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 16))(DT(2012, 11, 16))(DT(2012, 11, 16)));
  }

  void testAdvResOverrideFromDateO1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("O  ")("O  ")("O  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 13))(DT(2012, 11, 1))(DT(2012, 11, 1)));
  }

  void testAdvResOverrideFromDateO2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("O  ")("O  ")("O  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 17))(DT(2012, 11, 17))(DT(2012, 11, 5)));
  }

  void testAdvResOverrideFromDateO3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("O  ")("O  ")("O  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideFromDate(list_of(DT(2012, 11, 16))(DT(2012, 11, 16))(DT(2012, 11, 5)));
  }

  void verifyAdvResOverrideToDate(const vector<DT>& dates)
  {
    CPPUNIT_ASSERT_EQUAL(dates.size(), _rartv->getAdvResOverridesSize());

    for (uint16_t index = 0; index < dates.size(); ++index)
    {
      CPPUNIT_ASSERT_EQUAL(dates[index], _rartv->getAdvResOverride(index).toDate());
    }
  }

  void testAdvResOverrideToDateJ1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of(" J ")(" J ")(" J "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 14)));
  }

  void testAdvResOverrideToDateJ2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of(" J ")(" J ")(" J "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 14)));
  }

  void testAdvResOverrideToDateJ3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of(" J ")(" J ")(" J "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 14)));
  }

  void testAdvResOverrideToDate1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 20)));
  }

  void testAdvResOverrideToDate2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 20)));
  }

  void testAdvResOverrideToDate3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 20)));
  }

  void testAdvResOverrideToDateF1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of(" F ")(" F ")(" F "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 16))(DT(2012, 11, 20)));
  }

  void testAdvResOverrideToDateF2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of(" F ")(" F ")(" F "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 20)));
  }

  void testAdvResOverrideToDateF3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of(" F ")(" F ")(" F "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideToDate(list_of(DT(2012, 11, 14))(DT(2012, 11, 14))(DT(2012, 11, 20)));
  }

  void verifyAdvResOverrideIgnoreTktAfterResRestriction(const vector<bool>& values)
  {
    CPPUNIT_ASSERT_EQUAL(values.size(), _rartv->getAdvResOverridesSize());

    for (uint16_t index = 0; index < values.size(); ++index)
    {
      CPPUNIT_ASSERT_EQUAL(values[index],
                           _rartv->getAdvResOverride(index).ignoreTktAfterResRestriction());
    }
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(false)(false));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(false)(false));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(false)(false));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction1Simultaneus()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("  X")("  X")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(true)(true));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction2Simultaneus()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("   ")("  X")("  X"));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(vector<bool>());
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction3Simultaneus()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("   ")("  X")("  X"));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(false)(true));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction1FromInd()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("P  ")("C  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(true)(false));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction2FromInd()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("C  ")("C  ")("C  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(false)(true));
  }

  void testAdvResOverrideIgnoreTktAfterResRestriction3FromInd()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("P  ")("P  ")("P  "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktAfterResRestriction(list_of(false)(false)(true));
  }

  void verifyAdvResOverrideIgnoreTktBeforeDeparture(const vector<bool>& values)
  {
    CPPUNIT_ASSERT_EQUAL(values.size(), _rartv->getAdvResOverridesSize());

    for (uint16_t index = 0; index < values.size(); ++index)
    {
      CPPUNIT_ASSERT_EQUAL(values[index],
                           _rartv->getAdvResOverride(index).ignoreTktDeforeDeptRestriction());
    }
  }

  void testAdvResOverrideIgnoreTktBeforeDeparture1()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktBeforeDeparture(list_of(false)(false)(false));
  }

  void testAdvResOverrideIgnoreTktBeforeDeparture2()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktBeforeDeparture(list_of(false)(false)(false));
  }

  void testAdvResOverrideIgnoreTktBeforeDeparture3()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("   ")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktBeforeDeparture(list_of(false)(false)(false));
  }

  void testAdvResOverrideIgnoreTktBeforeDeparture1Y()
  {
    setUpConfiguration1();
    setUpPermutation(list_of("  Y")("   ")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktBeforeDeparture(list_of(false)(true)(true));
  }

  void testAdvResOverrideIgnoreTktBeforeDeparture2Y()
  {
    setUpConfiguration2();
    setUpPermutation(list_of("   ")("  Y")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktBeforeDeparture(vector<bool>());
  }

  void testAdvResOverrideIgnoreTktBeforeDeparture3Y()
  {
    setUpConfiguration3();
    setUpPermutation(list_of("   ")("  Y")("   "));

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    verifyAdvResOverrideIgnoreTktBeforeDeparture(vector<bool>());
  }

  void testNoT988Conf1()
  {
    setUpConfiguration1();
    setUpPermutationWithoutT988(3);

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(3 == _rartv->getAdvResOverridesSize());
  }

  void testNoT988Conf2()
  {
    setUpConfiguration2();
    setUpPermutationWithoutT988(3);

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(3 == _rartv->getAdvResOverridesSize());
  }

  void testNoT988Conf3()
  {
    setUpConfiguration3();
    setUpPermutationWithoutT988(3);

    _rartv->permutationIndependentSetUp();
    _rartv->advPurchaseValidation();

    CPPUNIT_ASSERT(3 == _rartv->getAdvResOverridesSize());
  }

private:
  TestMemHandle _memHandle;
  RexPricingTrx* _trx;
  Itin* _newItin;
  FarePath* _newFarePath;
  FarePath* _excFarePath;
  std::vector<const PaxTypeFare*>* _allRepricePTFs;
  RexAdvResTktValidatorStub* _rartv;
  ProcessTagPermutation* _permutation;
  map<tse::AdvResOverride, bool, tse::AdvResOverride> _cache;
  Diag689Collector* _dc;

  PricingUnit* _currentPU;
  FareUsage* _currentFU;
  uint16_t _fareCompNumber;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexAdvResTktValidatorAdvPurTest);
}
