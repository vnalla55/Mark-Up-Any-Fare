#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag880Collector.h"
#include "ServiceFees/MerchCarrierStrategy.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
struct SubCodeInitializerFake : public ServiceFeesGroup::SubCodeInitializer
{
  SubCodeInitializerFake(PricingTrx& trx,
                         FarePath* farePath,
                         TravelSeg* first,
                         TravelSeg* last,
                         MerchCarrierStrategy& merchStrategy)
    : SubCodeInitializer(trx, farePath, first, last, merchStrategy)
  {
  }

  void getSubCode(std::vector<SubCodeInfo*>& subCodes,
                  const CarrierCode& carrier,
                  const ServiceTypeCode& srvTypeCode,
                  const ServiceGroup& groupCode,
                  const DateTime& travelDate) const
  {
    subCodes = _sciv;
  }

  OCFees* newOCFee() const
  {
    static int i = 0;
    if (i++ % 2 == 0)
      return const_cast<OCFees*>(&_ocFee1);
    else
      return const_cast<OCFees*>(&_ocFee2);
  }

  void setSubCodeInfo()
  {
    _sciv.push_back(&_sci1);
    _sciv.push_back(&_sci2);
  }

private:
  std::vector<SubCodeInfo*> _sciv;
  SubCodeInfo _sci1;
  SubCodeInfo _sci2;
  OCFees _ocFee1;
  OCFees _ocFee2;
};

class ServiceFeesGroupMock : public ServiceFeesGroup
{
public:
  ServiceFeesGroupMock() : ServiceFeesGroup() {}

  // overrides to external services
  bool getFeeRounding(const CurrencyCode& currencyCode,
                      RoundingFactor& roundingFactor,
                      CurrencyNoDec& roundingNoDec,
                      RoundingRule& roundingRule) const
  {
    return false;
  }

  void convertCurrency(Money& target, const Money& source) const
  {
    if (target.code()[0] == source.code()[0])
      target.value() = source.value();
    else if (target.code()[0] > source.code()[0])
      target.value() = source.value() / 2;
    else
      target.value() = source.value() * 2;
  }
};
}

class ServiceFeesGroupTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ServiceFeesGroupTest);

  CPPUNIT_TEST(testBracketOperator);

  CPPUNIT_TEST(testCopyUnique_PassSubCode);
  CPPUNIT_TEST(testCopyUnique_PassFltTktMerchInd);
  CPPUNIT_TEST(testCopyUnique_Fail);

  CPPUNIT_TEST(testIsSubCodeProcessedWhenFound);
  CPPUNIT_TEST(testIsSubCodeProcessedWhenSubCodeNotFound);
  CPPUNIT_TEST(testIsSubCodeProcessedWhenTravelSegNotFound);

  CPPUNIT_TEST(testFoundSolutionWhenPass);
  CPPUNIT_TEST(testFoundSolutionWhenFailOnAmount);
  CPPUNIT_TEST(testFoundSolutionWhenFailOnSkippedSegs);

  CPPUNIT_TEST(testGetAmountSumWhenAllFound);
  CPPUNIT_TEST(testGetAmountSumWhenNotAllFound);
  CPPUNIT_TEST(testGetAmountSumWithConversion);
  CPPUNIT_TEST(testGetAmountSumWithoutConversion);

  CPPUNIT_TEST(testGetMktCxrWhen4SameMktCxrs);
  CPPUNIT_TEST(testGetMktCxrWhen3DifferentMktCxrs);

  CPPUNIT_TEST(testCountCarrierOccurrencesWhen4SameCxrs);
  CPPUNIT_TEST(testCountCarrierOccurrencesWhen3DifferentCxrs);

  CPPUNIT_TEST(testChooseSolutionWhenCheaperFee);
  CPPUNIT_TEST(testChooseSolutionWhenSameFeeAndMostCarriers);
  CPPUNIT_TEST(testChooseSolutionWhenNotCheaperFee);

  CPPUNIT_TEST(testFindSolutionForSubCodeWhenCheapestWins);
  CPPUNIT_TEST(testFindSolutionForSubCodeWhenWinsForMostCarrierOccurrence);
  CPPUNIT_TEST(testFindSolutionForSubCodeWhenCheapestWins_WithDiag);
  CPPUNIT_TEST(testFindSolutionForSubCodeWhenWinsForMostCarrierOccurrence_WithDiag);

  CPPUNIT_TEST(testDetermineOutputCurrency_TheSame);
  CPPUNIT_TEST(testDetermineOutputCurrency_Different);

  CPPUNIT_TEST(testRemoveInvalidOCFeesWhenSolutionFound);
  CPPUNIT_TEST(testPrintSolutionsWhenHardMatch);
  CPPUNIT_TEST(testPrintSolutionsWhenSoftMatch);
  CPPUNIT_TEST(testPrintSolutionsWhenFail);
  CPPUNIT_TEST(testFindSolutionForSubCodeForAncillaryPricingWhenHardPass);
  CPPUNIT_TEST(testFindSolutionForSubCodeForAncillaryPricingWhenSoftPass);
  CPPUNIT_TEST(testFindSolutionForSubCodeForAncillaryPricingWhenFail);
  CPPUNIT_TEST(testFindSolutionForSubCodeForAncillaryPricingWhenDiagExist);
  CPPUNIT_TEST(testRemoveSoftMatches);

  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMerchPrefMapEmpty);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenSingleMarketDrivenFlight);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenNoMarketDrivenFlights);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheBeginning);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMarketDrivenFlightInTheMiddle);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenArunkOnTheBeginning);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenArunkOnTheEnd);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenArunkOnTheLastEnd);

  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS);
  CPPUNIT_TEST(testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA);

  CPPUNIT_TEST(
      testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_for_M70_request);
  CPPUNIT_TEST(
      testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS_for_M70_request);
  CPPUNIT_TEST(
      testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA_for_M70_request);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingTrx* _pricingTrx;
  Billing* _billing;
  FarePath* _farePath;
  ServiceFeesGroup::SubCodeInitializer* _initializer;
  ServiceFeesGroup* _srvFeesGroup;
  ServiceFeesGroup* _srvFeesGroup1;
  ServiceTypeCode* _srvTypeCode;
  AirSeg* _first;
  AirSeg* _last;
  SubCodeInfo* _subCode1, *_subCode2;
  std::vector<SubCodeInfo*> _subCodes;
  std::vector<ServiceFeesGroup::TvlSegPair*> _routes;
  ServiceFeesGroup::KeyItemMap _keyItemMap;
  TseServer* _server;
  std::vector<std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> >* _unitsOfTravel;
  std::vector<TravelSeg*>* _tvlSeg;
  AirSeg* _seg1;
  ArunkSeg* _arunkSeg;
  AirSeg* _seg2;
  AirSeg* _seg3;
  AirSeg* _seg4;

public:
  void setUp()
  {
    _server = _memHandle.create<MockTseServer>();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    _pricingTrx->getOptions()->currencyOverride() = "KKK";
    _billing = _memHandle.create<Billing>();
    _pricingTrx->billing() = _billing;
    _farePath = _memHandle.create<FarePath>();
    _farePath->paxType() = _memHandle.create<PaxType>();
    _initializer = _memHandle.insert(new SubCodeInitializerFake(
        *_pricingTrx, _farePath, NULL, NULL, *(_memHandle.create<MultipleSegmentStrategy>())));
    _srvFeesGroup = _memHandle.create<ServiceFeesGroupMock>();
    _srvFeesGroup1 = _memHandle.create<ServiceFeesGroup>();
    _srvFeesGroup->initialize(_pricingTrx);
    _srvFeesGroup1->initialize(_pricingTrx);
    _srvTypeCode = _memHandle.create<ServiceTypeCode>();
    _subCode1 = _memHandle.create<SubCodeInfo>();
    _subCode2 = _memHandle.create<SubCodeInfo>();
    _first = _memHandle.create<AirSeg>();
    _last = _memHandle.create<AirSeg>();
    _subCodes.push_back(_subCode1);
    _tvlSeg = _memHandle.create<std::vector<TravelSeg*> >();
    _unitsOfTravel = _memHandle.create<
        std::vector<std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> > >();
    _seg1 = _memHandle.create<AirSeg>();
    _arunkSeg = _memHandle.create<ArunkSeg>();
    _seg2 = _memHandle.create<AirSeg>();
    _seg3 = _memHandle.create<AirSeg>();
    _seg4 = _memHandle.create<AirSeg>();
  }

  void tearDown()
  {
    _memHandle.clear();
    _subCodes.clear();
    _routes.clear();
    _keyItemMap.clear();
  }

  void initializeSubCodes() { ((SubCodeInitializerFake*)_initializer)->setSubCodeInfo(); }

  void createKeyItemMap(const CarrierCode& cxr1,
                        const CarrierCode& cxr2,
                        bool addToMap = true,
                        MoneyAmount feeAmount = 0.0,
                        CurrencyCode cur = "")
  {
    OCFees* ocFees = _memHandle.create<OCFees>();
    AirSeg* seg1 = _memHandle.create<AirSeg>();
    AirSeg* seg2 = _memHandle.create<AirSeg>();
    seg1->setMarketingCarrierCode(cxr1);
    seg2->setMarketingCarrierCode(cxr2);
    if (addToMap)
      _keyItemMap[std::make_pair(seg1, seg2)] = std::make_pair(ocFees, true);
    std::vector<TravelSeg*>* tvlSeg = _memHandle.create<std::vector<TravelSeg*> >();
    tvlSeg->push_back(seg1);
    tvlSeg->push_back(seg2);
    ServiceFeesGroup::TvlSegPair* route =
        _memHandle.insert(new ServiceFeesGroup::TvlSegPair(tvlSeg->begin(), tvlSeg->end()));
    _routes.push_back(route);
    ocFees->feeAmount() = feeAmount;
    ocFees->feeCurrency() = cur;
  }

  ServiceFeesGroup::TvlSegPair*
  addFareMarketToSolution(std::vector<ServiceFeesGroup::TvlSegPair*>& solution,
                          CarrierCode carrierCode)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->setMarketingCarrierCode(carrierCode);
    std::vector<TravelSeg*>* tvlSeg = _memHandle.create<std::vector<TravelSeg*> >();
    tvlSeg->push_back(seg);
    ServiceFeesGroup::TvlSegPair* route =
        _memHandle.insert(new ServiceFeesGroup::TvlSegPair(tvlSeg->begin(), tvlSeg->end()));
    solution.push_back(route);
    return route;
  }

  void addFeeToSubCodeMap(ServiceFeesGroup::KeyItemMap& item,
                          ServiceSubTypeCode subCode,
                          ServiceFeesGroup::TvlSegPair& route,
                          MoneyAmount feeAmount)
  {
    OCFees* ocFee = _memHandle.create<OCFees>();
    ocFee->subCodeInfo() = _subCode1;
    ocFee->feeAmount() = feeAmount;
    ocFee->feeCurrency() = "NUC";
    if (subCode == "A01" || subCode == "A02")
    {
      OptionalServicesInfo* optFee = _memHandle.create<OptionalServicesInfo>();
      ocFee->optFee() = optFee;

      if (subCode == "A02")
        ocFee->softMatchS7Status().set(OCFees::S7_RULE_SOFT);
    }

    item.insert(std::make_pair(std::make_pair(*route.first, *(route.second - 1)),
                               std::make_pair(ocFee, true)));
  }

  void create2Solutions(MoneyAmount feeAmount,
                        ServiceSubTypeCode subCode,
                        ServiceFeesGroup::KeyItemMap& key,
                        TseUtil::SolutionSet& solutions)
  {
    std::vector<ServiceFeesGroup::TvlSegPair*>* solution1 =
        _memHandle.create<std::vector<ServiceFeesGroup::TvlSegPair*> >();
    ServiceFeesGroup::TvlSegPair* route1 = addFareMarketToSolution(*solution1, "AA");
    ServiceFeesGroup::TvlSegPair* route2 = addFareMarketToSolution(*solution1, "AA");
    ServiceFeesGroup::TvlSegPair* route3 = addFareMarketToSolution(*solution1, "AA");
    solutions.insert(TseUtil::Solution(0, *solution1));

    addFeeToSubCodeMap(key, subCode, *route1, 5.0);
    addFeeToSubCodeMap(key, subCode, *route2, 6.0);
    addFeeToSubCodeMap(key, subCode, *route3, 7.0);

    std::vector<ServiceFeesGroup::TvlSegPair*>* solution2 =
        _memHandle.create<std::vector<ServiceFeesGroup::TvlSegPair*> >();
    solution2->push_back(route1);
    ServiceFeesGroup::TvlSegPair* route4 = addFareMarketToSolution(*solution2, "UA");
    solutions.insert(TseUtil::Solution(0, *solution2));

    addFeeToSubCodeMap(key, subCode, *route4, feeAmount);
  }

  Diag880Collector* createDiagnostic()
  {
    Diag880Collector* diag =
        _memHandle.insert(new Diag880Collector(*_memHandle.create<Diagnostic>()));
    diag->activate();
    return diag;
  }

  // TESTS
  void testBracketOperator()
  {
    initializeSubCodes();

    _initializer->operator()(_srvFeesGroup, "AA", *_srvTypeCode, ServiceGroup(), DateTime());

    std::map<const FarePath*, std::vector<OCFees*> >::const_iterator srvFeesGrpI =
        _srvFeesGroup->ocFeesMap().find(_farePath);

    CPPUNIT_ASSERT(srvFeesGrpI != _srvFeesGroup->ocFeesMap().end());
    CPPUNIT_ASSERT_EQUAL(size_t(2), srvFeesGrpI->second.size());
  }

  void testCopyUnique_PassSubCode()
  {
    _subCode2->serviceSubTypeCode() = "ABC";
    MultipleSegmentStrategy::copyUnique(&_subCodes, _subCode2);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _subCodes.size());
  }

  void testCopyUnique_PassFltTktMerchInd()
  {
    _subCode2->fltTktMerchInd() = 'X';
    MultipleSegmentStrategy::copyUnique(&_subCodes, _subCode2);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _subCodes.size());
  }

  void testCopyUnique_Fail()
  {
    MultipleSegmentStrategy::copyUnique(&_subCodes, _subCode2);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _subCodes.size());
  }

  void testIsSubCodeProcessedWhenFound()
  {

    _srvFeesGroup
        ->_subCodeMap[0][std::make_tuple(_farePath, "05A", 'P')][std::make_pair(_first, _last)];
    CPPUNIT_ASSERT(_srvFeesGroup->isSubCodePassed(0, _farePath, _first, _last, "05A", 'P'));
  }

  void testIsSubCodeProcessedWhenSubCodeNotFound()
  {
    _srvFeesGroup
        ->_subCodeMap[0][std::make_tuple(_farePath, "05A", 'P')][std::make_pair(_first, _last)];
    CPPUNIT_ASSERT(!_srvFeesGroup->isSubCodePassed(0, _farePath, _first, _last, "06A", 'P'));
    _srvFeesGroup
        ->_subCodeMap[0][std::make_tuple(_farePath, "05A", 'P')][std::make_pair(_first, _last)];
    CPPUNIT_ASSERT(!_srvFeesGroup->isSubCodePassed(0, _farePath, _first, _last, "05A", 'F'));
  }

  void testIsSubCodeProcessedWhenTravelSegNotFound()
  {
    _srvFeesGroup
        ->_subCodeMap[0][std::make_tuple(_farePath, "06A", 'P')][std::make_pair(_first, _first)];
    CPPUNIT_ASSERT(!_srvFeesGroup->isSubCodePassed(0, _farePath, _first, _last, "06A", 'P'));
  }

  void testFoundSolutionWhenPass()
  {
    uint16_t skippedSegs = 0;

    CPPUNIT_ASSERT(_srvFeesGroup->foundSolution(skippedSegs, 1, 10.0));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), skippedSegs);
  }

  void testFoundSolutionWhenFailOnAmount()
  {
    uint16_t skippedSegs = 0;

    CPPUNIT_ASSERT(!_srvFeesGroup->foundSolution(skippedSegs, 1, -1.0));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), skippedSegs);
  }

  void testFoundSolutionWhenFailOnSkippedSegs()
  {
    uint16_t skippedSegs = 1;

    CPPUNIT_ASSERT(!_srvFeesGroup->foundSolution(skippedSegs, 1, 10.0));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), skippedSegs);
  }

  void testGetAmountSumWhenAllFound()
  {
    Money feeAmountSum("");
    createKeyItemMap("", "", true, 6.5);
    createKeyItemMap("", "", true, 3.2);

    CPPUNIT_ASSERT(_srvFeesGroup->getAmountSum(feeAmountSum, _routes, _keyItemMap));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.7, feeAmountSum.value(), EPSILON);
  }

  void testGetAmountSumWhenNotAllFound()
  {
    Money feeAmountSum("");
    createKeyItemMap("", "", true, 6.5);
    createKeyItemMap("", "", false, 3.2);

    CPPUNIT_ASSERT(!_srvFeesGroup->getAmountSum(feeAmountSum, _routes, _keyItemMap));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, feeAmountSum.value(), EPSILON);
  }

  void testGetAmountSumWithConversion()
  {
    Money feeAmountSum("BBB");
    createKeyItemMap("", "", true, 10.0, "AAA");
    createKeyItemMap("", "", true, 10.0, "CCC");

    CPPUNIT_ASSERT(_srvFeesGroup->getAmountSum(feeAmountSum, _routes, _keyItemMap));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0, feeAmountSum.value(), EPSILON);
  }

  void testGetAmountSumWithoutConversion()
  {
    Money feeAmountSum("BBB");
    createKeyItemMap("", "", true, 10.0, "BBB");
    createKeyItemMap("", "", true, 10.0, "BBB");

    CPPUNIT_ASSERT(_srvFeesGroup->getAmountSum(feeAmountSum, _routes, _keyItemMap));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, feeAmountSum.value(), EPSILON);
  }

  void testGetMktCxrWhen4SameMktCxrs()
  {
    std::multiset<CarrierCode> marketingCxrs;
    createKeyItemMap("AA", "AA");
    createKeyItemMap("AA", "AA");

    _srvFeesGroup->getMktCxr(marketingCxrs, _routes, _keyItemMap);

    CPPUNIT_ASSERT_EQUAL(size_t(4), marketingCxrs.size());
  }

  void testGetMktCxrWhen3DifferentMktCxrs()
  {
    std::multiset<CarrierCode> marketingCxrs;
    createKeyItemMap("AA", "UA");
    createKeyItemMap("NW", "AA");

    _srvFeesGroup->getMktCxr(marketingCxrs, _routes, _keyItemMap);

    CPPUNIT_ASSERT_EQUAL(size_t(4), marketingCxrs.size());
  }

  void testCountCarrierOccurrencesWhen4SameCxrs()
  {
    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounter;
    std::multiset<CarrierCode> marketingCxrs;
    marketingCxrs.insert("AA");
    marketingCxrs.insert("AA");
    marketingCxrs.insert("AA");
    marketingCxrs.insert("AA");

    _srvFeesGroup->countCarrierOccurrences(cxrCounter, marketingCxrs);

    CPPUNIT_ASSERT_EQUAL(size_t(1), cxrCounter.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), *cxrCounter.begin());
  }

  void testCountCarrierOccurrencesWhen3DifferentCxrs()
  {
    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounter;
    std::multiset<CarrierCode> marketingCxrs;
    marketingCxrs.insert("AA");
    marketingCxrs.insert("AA");
    marketingCxrs.insert("NW");
    marketingCxrs.insert("UA");

    _srvFeesGroup->countCarrierOccurrences(cxrCounter, marketingCxrs);

    CPPUNIT_ASSERT_EQUAL(size_t(3), cxrCounter.size());
    std::multiset<uint16_t, std::greater<uint16_t> >::const_iterator pos = cxrCounter.begin();
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), *pos++);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), *pos++);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), *pos++);
  }

  void testChooseSolutionWhenCheaperFee()
  {
    MoneyAmount minFeeAmountSum = 10.0;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution = 0;
    std::multiset<uint16_t, std::greater<uint16_t> > winningCxrCounter;
    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounter;
    const std::vector<ServiceFeesGroup::TvlSegPair*> solution;

    _srvFeesGroup->chooseSolution(
        minFeeAmountSum, winningSolution, winningCxrCounter, cxrCounter, 9.9, solution);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.9, minFeeAmountSum, EPSILON);
    CPPUNIT_ASSERT_EQUAL(&solution, winningSolution);
  }

  void testChooseSolutionWhenSameFeeAndMostCarriers()
  {
    MoneyAmount minFeeAmountSum = 10.0;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution = 0;
    std::multiset<uint16_t, std::greater<uint16_t> > winningCxrCounter;
    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounter;
    const std::vector<ServiceFeesGroup::TvlSegPair*> solution;

    winningCxrCounter.insert(3);
    winningCxrCounter.insert(2);
    winningCxrCounter.insert(1);
    winningCxrCounter.insert(1);
    cxrCounter.insert(3);
    cxrCounter.insert(2);
    cxrCounter.insert(2);

    std::multiset<uint16_t, std::greater<uint16_t> > winningCxrCounterBefore = winningCxrCounter;
    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounterBefore = cxrCounter;

    _srvFeesGroup->chooseSolution(
        minFeeAmountSum, winningSolution, winningCxrCounter, cxrCounter, 10.0, solution);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, minFeeAmountSum, EPSILON);
    CPPUNIT_ASSERT_EQUAL(&solution, winningSolution);
    CPPUNIT_ASSERT_EQUAL(winningCxrCounter.size(), cxrCounterBefore.size());
    CPPUNIT_ASSERT_EQUAL(cxrCounter.size(), winningCxrCounterBefore.size());
  }

  void testChooseSolutionWhenNotCheaperFee()
  {
    MoneyAmount minFeeAmountSum = 10.0;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution = 0;
    std::multiset<uint16_t, std::greater<uint16_t> > winningCxrCounter;
    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounter;
    const std::vector<ServiceFeesGroup::TvlSegPair*> solution;

    winningCxrCounter.insert(2);
    cxrCounter.insert(1);
    cxrCounter.insert(1);

    _srvFeesGroup->chooseSolution(
        minFeeAmountSum, winningSolution, winningCxrCounter, cxrCounter, 10.00, solution);
    CPPUNIT_ASSERT(!winningSolution);
    CPPUNIT_ASSERT_EQUAL(size_t(1), winningCxrCounter.size());
  }

  void testFindSolutionForSubCodeWhenCheapestWins()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "05A", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = 0;
    DateTime tktDate;
    ServiceSubTypeCode subCode = "05A";

    create2Solutions(14.0, subCode, subCodeItem.second, solutions);

    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        _srvFeesGroup->findSolutionForSubCode(
            subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, 0);
    CPPUNIT_ASSERT_EQUAL(&(solutions.get<0>().rbegin()->_routes), winningSolution);
  }

  void testFindSolutionForSubCodeWhenWinsForMostCarrierOccurrence()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "05A", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = 0;
    DateTime tktDate;
    ServiceSubTypeCode subCode = "05A";

    create2Solutions(13.0, subCode, subCodeItem.second, solutions);

    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        _srvFeesGroup->findSolutionForSubCode(
            subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, 0);
    CPPUNIT_ASSERT_EQUAL(&(solutions.get<0>().rbegin()->_routes), winningSolution);
  }

  void testFindSolutionForSubCodeWhenCheapestWins_WithDiag()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "05A", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = createDiagnostic();
    DateTime tktDate;
    ServiceSubTypeCode subCode = "05A";

    create2Solutions(12.0, subCode, subCodeItem.second, solutions);

    _srvFeesGroup->findSolutionForSubCode(
        subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, 0);
    CPPUNIT_ASSERT(
        diag880->str().find(
            "TOTAL: NUC17.00\nSTATUS: PASS - LOWEST FEE/MOST FLIGHT SEGMENTS SELECTED") !=
        std::string::npos);
  }

  void testFindSolutionForSubCodeWhenWinsForMostCarrierOccurrence_WithDiag()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "05A", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = createDiagnostic();
    DateTime tktDate;
    ServiceSubTypeCode subCode = "05A";

    create2Solutions(13.0, subCode, subCodeItem.second, solutions);

    uint16_t multiTicketNbr = 0;
    _srvFeesGroup->findSolutionForSubCode(
        subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, multiTicketNbr);
    CPPUNIT_ASSERT(
        diag880->str().find(
            "TOTAL: NUC18.00\nSTATUS: PASS - LOWEST FEE/MOST FLIGHT SEGMENTS SELECTED") !=
        std::string::npos);
  }

  void testDetermineOutputCurrency_TheSame()
  {
    createKeyItemMap("", "", true, 1.0, "AAA");
    createKeyItemMap("", "", true, 1.0, "AAA");
    TseUtil::SolutionSet solution;
    solution.insert(TseUtil::Solution(0, _routes));

    CPPUNIT_ASSERT_EQUAL(CurrencyCode("AAA"),
                         _srvFeesGroup->determineOutputCurrency(solution, _keyItemMap));
  }

  void testDetermineOutputCurrency_Different()
  {
    createKeyItemMap("", "", true, 1.0, "AAA");
    createKeyItemMap("", "", true, 1.0, "BBB");
    TseUtil::SolutionSet solution;
    solution.insert(TseUtil::Solution(0, _routes));

    CPPUNIT_ASSERT_EQUAL(CurrencyCode("KKK"),
                         _srvFeesGroup->determineOutputCurrency(solution, _keyItemMap));
  }

  OCFees* addOCFeeToMap(std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution,
                        std::vector<TravelSeg*>::const_iterator tvlSegI,
                        std::vector<TravelSeg*>::const_iterator tvlSegIE,
                        FarePath& farePath,
                        ServiceSubTypeCode srvSubTypeCode)
  {
    OCFees* ocfee = _memHandle.create<OCFees>();
    SubCodeInfo* subCodeInfo = _memHandle.create<SubCodeInfo>();
    ocfee->subCodeInfo() = subCodeInfo;

    ocfee->travelStart() = *tvlSegI;
    ocfee->travelEnd() = *(tvlSegIE - 1);
    subCodeInfo->serviceSubTypeCode() = srvSubTypeCode;
    subCodeInfo->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    _srvFeesGroup->_ocFeesMap[&farePath].push_back(ocfee);

    if (winningSolution)
    {
      ServiceFeesGroup::TvlSegPair* tvlSegPair =
          _memHandle.insert(new ServiceFeesGroup::TvlSegPair(tvlSegI, tvlSegIE));
      winningSolution->push_back(tvlSegPair);
    }
    return ocfee;
  }

  void testRemoveInvalidOCFeesWhenSolutionFound()
  {
    std::vector<ServiceFeesGroup::TvlSegPair*> winningSolution;
    FarePath farePath;
    ServiceSubTypeCode srvSubTypeCode = "0A6";
    ServiceSubTypeCode otherSrvSubTypeCode = "0A5";
    std::vector<TravelSeg*> tvlSegs;
    AirSeg seg1, seg2, seg3, seg4;

    tvlSegs.push_back(&seg1);
    seg1.segmentOrder() = 1;
    tvlSegs.push_back(&seg2);
    seg2.segmentOrder() = 2;
    tvlSegs.push_back(&seg3);
    seg3.segmentOrder() = 3;
    tvlSegs.push_back(&seg4);
    seg4.segmentOrder() = 4;
    std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();

    OCFees* ocfee1 =
        addOCFeeToMap(&winningSolution, tvlSegI, tvlSegI + 1, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI, tvlSegI + 2, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI, tvlSegI + 3, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI, tvlSegI + 4, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI + 1, tvlSegI + 2, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI + 1, tvlSegI + 3, farePath, srvSubTypeCode);
    OCFees* ocfee24 =
        addOCFeeToMap(&winningSolution, tvlSegI + 1, tvlSegI + 4, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI + 2, tvlSegI + 3, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI + 2, tvlSegI + 4, farePath, srvSubTypeCode);
    addOCFeeToMap(0, tvlSegI + 3, tvlSegI + 4, farePath, srvSubTypeCode);
    OCFees* otherOCfee =
        addOCFeeToMap(&winningSolution, tvlSegI, tvlSegI + 1, farePath, otherSrvSubTypeCode);

    _srvFeesGroup->removeInvalidOCFees(
        &winningSolution, &farePath, srvSubTypeCode, 'F', &seg1, &seg4, false);

    std::vector<OCFees*> ocFees = _srvFeesGroup->_ocFeesMap[&farePath];

    CPPUNIT_ASSERT_EQUAL(size_t(3), ocFees.size());
    CPPUNIT_ASSERT_EQUAL(ocfee1, ocFees[0]);
    CPPUNIT_ASSERT_EQUAL(ocfee24, ocFees[1]);
    CPPUNIT_ASSERT_EQUAL(otherOCfee, ocFees[2]);
  }

  void testPrintSolutionsWhenHardMatch()
  {
    Diag880Collector* diag880 = createDiagnostic();
    uint16_t solNo = 1;
    ServiceFeesGroup::KeyItemMap item;
    TseUtil::SolutionSet solutions;
    ServiceSubTypeCode subCode = "A01";

    create2Solutions(0.0, subCode, item, solutions);

    const std::vector<ServiceFeesGroup::TvlSegPair*>& routes = solutions.get<0>().begin()->_routes;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution = &routes;

    _srvFeesGroup->printSolutions(diag880, solNo, routes, item, winningSolution);
    CPPUNIT_ASSERT_EQUAL(
        std::string("1. -AA-  -UA-  \n \n"
                    "-AA-\n"
                    "HARD MATCH FOUND\n \n"
                    "-UA-\n"
                    "HARD MATCH FOUND\n \n"
                    "STATUS: PASS\n"
                    "***************************************************************\n"),
        diag880->str());
  }

  void testPrintSolutionsWhenSoftMatch()
  {
    Diag880Collector* diag880 = createDiagnostic();
    uint16_t solNo = 1;
    ServiceFeesGroup::KeyItemMap item;
    TseUtil::SolutionSet solutions;
    ServiceSubTypeCode subCode = "A02";

    create2Solutions(0.0, subCode, item, solutions);

    const std::vector<ServiceFeesGroup::TvlSegPair*>& routes = solutions.get<0>().begin()->_routes;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution = 0;

    _srvFeesGroup->printSolutions(diag880, solNo, routes, item, winningSolution);
    CPPUNIT_ASSERT_EQUAL(
        std::string("1. -AA-  -UA-  \n \n"
                    "-AA-\n"
                    "SOFT MATCH FOUND\n \n"
                    "-UA-\n"
                    "SOFT MATCH FOUND\n \n"
                    "STATUS: FAIL\n"
                    "***************************************************************\n"),
        diag880->str());
  }

  void testPrintSolutionsWhenFail()
  {
    Diag880Collector* diag880 = createDiagnostic();
    uint16_t solNo = 1;
    ServiceFeesGroup::KeyItemMap item;
    TseUtil::SolutionSet solutions;
    ServiceSubTypeCode subCode = "A03";

    create2Solutions(0.0, subCode, item, solutions);

    const std::vector<ServiceFeesGroup::TvlSegPair*>& routes = solutions.get<0>().begin()->_routes;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution = 0;

    _srvFeesGroup->printSolutions(diag880, solNo, routes, item, winningSolution);
    CPPUNIT_ASSERT_EQUAL(
        std::string("1. -AA-  -UA-  \n \n"
                    "-AA-\n"
                    "NOT FOUND\n \n"
                    "-UA-\n"
                    "NOT FOUND\n \n"
                    "STATUS: FAIL\n"
                    "***************************************************************\n"),
        diag880->str());
  }

  void testFindSolutionForSubCodeForAncillaryPricingWhenHardPass()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "A01", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = 0;
    DateTime tktDate;
    ServiceSubTypeCode subCode = "A01";

    create2Solutions(0.0, subCode, subCodeItem.second, solutions);

    uint16_t multiTicketNbr = 0;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        _srvFeesGroup->findSolutionForSubCodeForAncillaryPricing(
            subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, multiTicketNbr);
    CPPUNIT_ASSERT_EQUAL(&(solutions.get<0>().begin()->_routes), winningSolution);
  }

  void testFindSolutionForSubCodeForAncillaryPricingWhenSoftPass()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "A02", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = 0;
    DateTime tktDate;
    ServiceSubTypeCode subCode = "A02";

    create2Solutions(0.0, subCode, subCodeItem.second, solutions);

    uint16_t multiTicketNbr = 0;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        _srvFeesGroup->findSolutionForSubCodeForAncillaryPricing(
            subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, multiTicketNbr);
    CPPUNIT_ASSERT_EQUAL(&(solutions.get<0>().rbegin()->_routes), winningSolution);
  }

  void testFindSolutionForSubCodeForAncillaryPricingWhenFail()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "A03", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = 0;
    DateTime tktDate;
    ServiceSubTypeCode subCode = "A03";

    create2Solutions(0.0, subCode, subCodeItem.second, solutions);

    uint16_t multiTicketNbr = 0;
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        _srvFeesGroup->findSolutionForSubCodeForAncillaryPricing(
            subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, multiTicketNbr);
    CPPUNIT_ASSERT_EQUAL(&(solutions.get<0>().rbegin()->_routes), winningSolution);
  }

  void testFindSolutionForSubCodeForAncillaryPricingWhenDiagExist()
  {
    std::pair<const ServiceFeesGroup::SubCodeMapKey, ServiceFeesGroup::KeyItemMap> subCodeItem =
        std::make_pair(std::make_tuple(_farePath, "A01", 'F'), _keyItemMap);
    TseUtil::SolutionSet solutions;
    std::vector<TravelSeg*>::const_iterator firstSegI;
    std::vector<TravelSeg*>::const_iterator endSegI;
    Diag880Collector* diag880 = createDiagnostic();
    DateTime tktDate;
    ServiceSubTypeCode subCode = "A01";

    create2Solutions(0.0, subCode, subCodeItem.second, solutions);

    uint16_t multiTicketNbr = 0;
    _srvFeesGroup->findSolutionForSubCodeForAncillaryPricing(
        subCodeItem, solutions, firstSegI, endSegI, diag880, tktDate, multiTicketNbr);
    CPPUNIT_ASSERT_EQUAL(
        std::string("*********************** SLICE AND DICE FOR ********************\n"
                    "GROUP:     SUBCODE: A01    REQUESTED PAXTYPE: \n"
                    "----------------------- PORTION OF TRAVEL ---------------------\n\n"
                    "------------------------ FLIGHT SEGMENTS ----------------------\n"
                    "----------------------- SLICE-DICE MATRIX ---------------------\n"
                    "---------------------- ALL SECTORS COVERED --------------------\n"
                    "1.       \n"
                    "2.          \n"
                    "***************************************************************\n"
                    "1. -AA-  -UA-  \n \n"
                    "-AA-\n"
                    "HARD MATCH FOUND\n \n"
                    "-UA-\n"
                    "HARD MATCH FOUND\n \n"
                    "STATUS: PASS\n"
                    "***************************************************************\n"),
        diag880->str());
  }

  std::vector<OCFees*> prepareRemoveSoftMatchesCase(
      std::vector<std::pair<const TravelSeg*, const TravelSeg*> >& winningSolutionSeg,
      FarePath& farePath,
      ServiceSubTypeCode& srvSubTypeCode,
      AirSeg& firstSeg,
      AirSeg& lastSeg)
  {
    ServiceSubTypeCode otherSrvSubTypeCode = "0B6";
    std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        _memHandle.create<std::vector<ServiceFeesGroup::TvlSegPair*> >();
    std::vector<TravelSeg*>* tvlSegs = _memHandle.create<std::vector<TravelSeg*> >();
    AirSeg* seg2 = _memHandle.create<AirSeg>();
    AirSeg* seg3 = _memHandle.create<AirSeg>();
    tvlSegs->push_back(&firstSeg);
    firstSeg.segmentOrder() = 1;
    tvlSegs->push_back(seg2);
    seg2->segmentOrder() = 2;
    tvlSegs->push_back(seg3);
    seg3->segmentOrder() = 3;
    tvlSegs->push_back(&lastSeg);
    lastSeg.segmentOrder() = 4;

    std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs->begin();

    std::vector<OCFees*> ocFees;
    ocFees.push_back(
        addOCFeeToMap(winningSolution, tvlSegI, tvlSegI + 1, farePath, srvSubTypeCode));
    ocFees.push_back(
        addOCFeeToMap(winningSolution, tvlSegI + 1, tvlSegI + 4, farePath, srvSubTypeCode));
    ocFees.push_back(
        addOCFeeToMap(winningSolution, tvlSegI, tvlSegI + 1, farePath, otherSrvSubTypeCode));

    std::vector<ServiceFeesGroup::TvlSegPair*>::const_iterator winningSolutionIt =
        winningSolution->begin();
    for (; winningSolutionIt != winningSolution->end(); ++winningSolutionIt)
      winningSolutionSeg.push_back(
          std::make_pair(*(*winningSolutionIt)->first, *((*winningSolutionIt)->second - 1)));

    // ocfee1->softMatchS7Status().set(OCFees::S7_CABIN_SOFT);
    const OptionalServicesInfo* optFee1 = _memHandle.create<OptionalServicesInfo>();
    ocFees[0]->optFee() = optFee1;
    ocFees[0]->addSeg(_pricingTrx->dataHandle());
    const OptionalServicesInfo* optFee2 = _memHandle.create<OptionalServicesInfo>();
    ocFees[0]->optFee() = optFee2;

    ocFees[1]->optFee() = optFee1;
    ocFees[1]->addSeg(_pricingTrx->dataHandle());
    ocFees[1]->optFee() = optFee2;
    ocFees[1]->addSeg(_pricingTrx->dataHandle());
    const OptionalServicesInfo* optFee3 = _memHandle.create<OptionalServicesInfo>();
    ocFees[1]->optFee() = optFee3;

    return ocFees;
  }

  void testRemoveSoftMatches()
  {
    FarePath farePath;
    ServiceSubTypeCode srvSubTypeCode = "0A6";
    std::vector<std::pair<const TravelSeg*, const TravelSeg*> > winningSolutionSeg;
    AirSeg firstSeg, lastSeg;
    std::vector<OCFees*> ocFees = prepareRemoveSoftMatchesCase(
        winningSolutionSeg, farePath, srvSubTypeCode, firstSeg, lastSeg);

    OCFees* ocFee1 = ocFees[0];
    ocFee1->segments()[0]->_softMatchS7Status.set(OCFees::S7_CABIN_SOFT);
    OCFees::OCFeesSeg* ocFee1HardMatch = ocFee1->segments()[1];

    OCFees* ocFee24 = ocFees[1];
    ocFee24->segments()[0]->_softMatchS7Status.set(OCFees::S7_CABIN_SOFT);
    ocFee24->segments()[1]->_softMatchS7Status.set(OCFees::S7_CABIN_SOFT);
    OCFees::OCFeesSeg* ocFee24HardMatch = ocFee24->segments()[2];

    _srvFeesGroup->removeSoftMatches(
        winningSolutionSeg, &farePath, srvSubTypeCode, 'F', &firstSeg, &lastSeg);

    std::vector<OCFees*> resOCFees = _srvFeesGroup->_ocFeesMap[&farePath];

    CPPUNIT_ASSERT_EQUAL(size_t(3), resOCFees.size());
    CPPUNIT_ASSERT_EQUAL(ocFee1, resOCFees[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), ocFee1->segCount());
    CPPUNIT_ASSERT_EQUAL(ocFee1HardMatch->_optFee, resOCFees[0]->optFee());
    CPPUNIT_ASSERT_EQUAL(ocFee24, resOCFees[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), ocFee24->segCount());
    CPPUNIT_ASSERT_EQUAL(ocFee24HardMatch->_optFee, resOCFees[1]->optFee());
    CPPUNIT_ASSERT_EQUAL(ocFees[2], resOCFees[2]);
  }

  void testCollectUnitsOfTravelWhenMerchPrefMapEmpty()
  {
    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());
    CPPUNIT_ASSERT(_unitsOfTravel->empty());
  }

  void addMarketDrivenSegment(TravelSeg* seg, bool isArunk = false)
  {
    _tvlSeg->push_back(seg);
    if (!isArunk)
    {
      static_cast<AirSeg*>(seg)->setMarketingCarrierCode("US");
      static_cast<AirSeg*>(seg)->setOperatingCarrierCode("US");
    }
    MerchCarrierPreferenceInfo* rec = 0;
    _srvFeesGroup->_merchCxrPref.insert(std::make_pair("US", rec));
  }

  void testCollectUnitsOfTravelWhenSingleMarketDrivenFlight()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;

    addMarketDrivenSegment(_seg1);

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(1), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));
  }

  void testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;

    addMarketDrivenSegment(_seg1);
    addMarketDrivenSegment(_seg2);
    addMarketDrivenSegment(_seg3);

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(3), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[2]));
  }

  void testCollectUnitsOfTravelWhenNoMarketDrivenFlights()
  {
    addMarketDrivenSegment(_seg1);
    _seg1->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg3);
    _seg3->setMarketingCarrierCode("AA");

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT(_unitsOfTravel->empty());
  }

  void testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheBeginning()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg3);
    _seg3->setMarketingCarrierCode("AA");
    _seg3->setOperatingCarrierCode("AA");

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(2), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 3 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));
  }

  void testCollectUnitsOfTravelWhenMarketDrivenFlightInTheMiddle()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    _seg1->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    _seg2->setOperatingCarrierCode("AA");
    addMarketDrivenSegment(_seg3);
    addMarketDrivenSegment(_seg4);
    _seg4->setMarketingCarrierCode("AA");

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(3), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 3 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 3 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[2]));
  }

  void testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    _seg1->setMarketingCarrierCode("AA");
    _seg1->setOperatingCarrierCode("AA");
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg3);

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(2), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));
  }

  void testCollectUnitsOfTravelWhenArunkOnTheBeginning()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_arunkSeg, true);
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg3);

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(2), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));
  }

  void testCollectUnitsOfTravelWhenArunkOnTheEnd()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    _seg1->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_arunkSeg, true);
    addMarketDrivenSegment(_seg3);

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(2), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));
  }

  void testCollectUnitsOfTravelWhenArunkOnTheLastEnd()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_arunkSeg, true);

    _srvFeesGroup->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(2), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));
  }

  void testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA()
  {
    commonTest_CollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA();
  }

  void testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS()
  {
    commonTest_CollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS();
  }

  void testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA()
  {
    commonTest_CollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA();
  }

  void testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_for_M70_request()
  {
    commonTest_CollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA();
  }

  void testCollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS_for_M70_request()
  {
    commonTest_CollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS();
  }

  void testCollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA_for_M70_request()
  {
    commonTest_CollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA();
  }

  void commonTest_CollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    addMarketDrivenSegment(_seg2);
    addMarketDrivenSegment(_seg3);
    _srvFeesGroup1->groupCode() = "SA";
    _pricingTrx->billing()->requestPath() = "PPSS";
    _srvFeesGroup1->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(3), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[2]));
  }

  void commonTest_CollectUnitsOfTravelWhenMultipleMarketDrivenFlights_GroupCode_SA_PACS()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;

    addMarketDrivenSegment(_seg1);
    addMarketDrivenSegment(_seg2);
    addMarketDrivenSegment(_seg3);
    _srvFeesGroup1->groupCode() = "SA";
    _pricingTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    MerchCarrierPreferenceInfo* rec = 0;
    _srvFeesGroup1->_merchCxrPref.insert(std::make_pair("US", rec));
    _srvFeesGroup1->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(3), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[2]));
  }

  void commonTest_CollectUnitsOfTravelWhenMarketDrivenFlightOnTheEnd_Group_Code_SA()
  {
    static const uint16_t BEGIN_OF_TVL_PORTION = 0;
    static const uint16_t END_OF_TVL_PORTION = 1;
    static const uint16_t MARKET_DRIVEN_STRATEGY = 2;
    static constexpr bool MARKET_DRIVEN = true;
    static constexpr bool MULTIPLE_SEGMENT = false;

    addMarketDrivenSegment(_seg1);
    _seg1->setMarketingCarrierCode("AA");
    _seg1->setOperatingCarrierCode("AA");
    addMarketDrivenSegment(_seg2);
    _seg2->setMarketingCarrierCode("AA");
    addMarketDrivenSegment(_seg3);
    _srvFeesGroup1->groupCode() = "SA";
    _pricingTrx->billing()->requestPath() = "PPSS";
    MerchCarrierPreferenceInfo* rec = 0;
    _srvFeesGroup1->_merchCxrPref.insert(std::make_pair("US", rec));
    _srvFeesGroup1->collectUnitsOfTravel(*_unitsOfTravel, _tvlSeg->begin(), _tvlSeg->end());

    CPPUNIT_ASSERT_EQUAL(size_t(3), _unitsOfTravel->size());
    CPPUNIT_ASSERT(_tvlSeg->begin() == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[0]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[0]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 1 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[1]));
    CPPUNIT_ASSERT_EQUAL(MULTIPLE_SEGMENT, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[1]));

    CPPUNIT_ASSERT(_tvlSeg->begin() + 2 == std::get<BEGIN_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT(_tvlSeg->end() == std::get<END_OF_TVL_PORTION>((*_unitsOfTravel)[2]));
    CPPUNIT_ASSERT_EQUAL(MARKET_DRIVEN, std::get<MARKET_DRIVEN_STRATEGY>((*_unitsOfTravel)[2]));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceFeesGroupTest);
}
