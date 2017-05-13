#include "Rules/test/ToursApplicationTest.h"

#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Tours.h"
#include "Rules/RuleConst.h"
#include "Rules/ToursApplication.h"

#include "test/include/TestConfigInitializer.h"

#include <vector>

namespace tse
{

namespace
{
class ToursApplicationMock : public ToursApplication
{
public:
  using ToursApplication::validate;

  ToursApplicationMock(Record3ReturnTypes result = PASS) : ToursApplication(0), _result(result)
  {
    Agent* agent = _memHandle.create<Agent>();
    PricingRequest* pricingRequest = _memHandle.create<PricingRequest>();
    pricingRequest->ticketingAgent() = agent;

    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(pricingRequest);
  }

  Record3ReturnTypes validate(const vector<FareUsage*>& fareUsages)
  {
    _fareUsages = fareUsages;
    return _result;
  }

  const vector<FareUsage*> validatedFareUsages() { return _fareUsages; }

private:
  Record3ReturnTypes _result;
  vector<FareUsage*> _fareUsages;
  TestMemHandle _memHandle;
};
}

void
ToursApplicationTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  Agent* agent = _memHandle.create<Agent>();
  PricingRequest* pricingRequest = _memHandle.create<PricingRequest>();
  pricingRequest->ticketingAgent() = agent;

  _pricingTrx = _memHandle.create<PricingTrx>();
  _pricingTrx->setRequest(pricingRequest);
  _pricingTrx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
  _pricingTrx->getRequest()->ticketingDT() = DateTime(2025, 06, 13);

  _pricingTrx->setOptions(_memHandle.create<PricingOptions>());

  _toursApplication = _memHandle.insert(new ToursApplication(_pricingTrx));
}

void
ToursApplicationTest::tearDown()
{
  _memHandle.clear();
}

void
ToursApplicationTest::addTourCode(PaxTypeFare& fare, const string& tourCode)
{
  Tours* tours = _memHandle.create<Tours>();
  tours->tourNo() = tourCode;

  PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
  ruleData->ruleItemInfo() = tours;

  PaxTypeFare::PaxTypeFareAllRuleData* data =
      _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
  data->fareRuleData = ruleData;

  (*fare.paxTypeFareRuleDataMap())[RuleConst::TOURS_RULE] = data;
}

FareUsage*
ToursApplicationTest::createFareUsage(const string& tourCode, const CarrierCode& carrier)
{
  PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
  addTourCode(*paxTypeFare, tourCode);
  paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();

  FareInfo* fareInfo = _memHandle.create<FareInfo>();
  fareInfo->carrier() = carrier;

  Fare* fare = _memHandle.create<Fare>();
  fare->setFareInfo(fareInfo);

  paxTypeFare->setFare(fare);

  FareUsage* fareUsage = _memHandle.create<FareUsage>();
  fareUsage->paxTypeFare() = paxTypeFare;
  return fareUsage;
}

void
ToursApplicationTest::testValidateEmptyFareUsageVec()
{
  vector<FareUsage*> emptyVec;
  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(emptyVec));
}

void
ToursApplicationTest::testValidateSingleFareUsageEmptyTourCode()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateSingleFareUsageWithTourCode()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TOURCODE", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesNoTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesNoTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithSameTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithSameTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "BB"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithDiffTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "AA"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithDiffTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "BB"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithOneEmptyTourCodeFirstSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithOneEmptyTourCodeFirstDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "BB"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithOneEmptyTourCodeLastSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateTwoFareUsagesWithOneEmptyTourCodeLastDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesNoTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesNoTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));
  fareUsages.push_back(createFareUsage("", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithFirstTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithFirstTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));
  fareUsages.push_back(createFareUsage("", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithMiddleTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithMiddleTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC", "BB"));
  fareUsages.push_back(createFareUsage("", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithLastTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithLastTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));
  fareUsages.push_back(createFareUsage("TC", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoFirstTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoFirstTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC", "BB"));
  fareUsages.push_back(createFareUsage("TC", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoFirstMixTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "AA"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesNoWithFirstMixTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC1", "BB"));
  fareUsages.push_back(createFareUsage("TC2", "CC"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoMiddleTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoMiddleTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));
  fareUsages.push_back(createFareUsage("TC", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoMiddleMixTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "AA"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoMiddleMixTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("", "BB"));
  fareUsages.push_back(createFareUsage("TC2", "CC"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoLastTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoLastTourCodeDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "BB"));
  fareUsages.push_back(createFareUsage("", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoLastMixTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "AA"));
  fareUsages.push_back(createFareUsage("", "AA"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithNoLastMixTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "BB"));
  fareUsages.push_back(createFareUsage("", "CC"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithSameTourCodeSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "AA"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithSameTourCodeDiffCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC", "AA"));
  fareUsages.push_back(createFareUsage("TC", "BB"));
  fareUsages.push_back(createFareUsage("TC", "CC"));

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithDiffTourCodesSameCarrier()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "AA"));
  fareUsages.push_back(createFareUsage("TC3", "AA"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateThreeFareUsagesWithDiffTourCodesDiffCarriers()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "BB"));
  fareUsages.push_back(createFareUsage("TC3", "CC"));

  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidateCanBeSkippedForSoloCarnival()
{
  vector<FareUsage*> fareUsages;
  fareUsages.push_back(createFareUsage("TC1", "AA"));
  fareUsages.push_back(createFareUsage("TC2", "BB"));

  TestConfigInitializer::setValue("SKIP_TOUR_CODES_VALIDATION", "Y", "SOLO_CARNIVAL_OPT");

  _pricingTrx->getOptions()->setCarnivalSumOfLocal(true);
  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));

  _pricingTrx->setTrxType(PricingTrx::IS_TRX);
  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(fareUsages));

  _pricingTrx->getOptions()->setCarnivalSumOfLocal(false);
  CPPUNIT_ASSERT_EQUAL(FAIL, _toursApplication->validate(fareUsages));
}

void
ToursApplicationTest::testValidatePricingUnitNotValid()
{
  PricingUnit* pricingUnit = 0;
  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(pricingUnit));
}

void
ToursApplicationTest::testValidatePricingUnitFail()
{
  PricingUnit pricingUnit;
  ToursApplicationMock toursApplication(FAIL);
  CPPUNIT_ASSERT_EQUAL(FAIL, toursApplication.validate(&pricingUnit));
}

void
ToursApplicationTest::testValidatePricingUnitPass()
{
  PricingUnit pricingUnit;
  ToursApplicationMock toursApplication(PASS);
  CPPUNIT_ASSERT_EQUAL(PASS, toursApplication.validate(&pricingUnit));
}

void
ToursApplicationTest::testValidateFarePathNotValid()
{
  FarePath* farePath = 0;
  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(farePath));
}

void
ToursApplicationTest::testValidateFarePathEmpty()
{
  FarePath farePath;
  ToursApplicationMock toursApplication;
  toursApplication.validate(&farePath);

  CPPUNIT_ASSERT_EQUAL(0, (int)toursApplication.validatedFareUsages().size());
}

void
ToursApplicationTest::testValidateFarePathOnePricingUnitOneFareUsage()
{
  FarePath farePath;
  PricingUnit pricingUnit;
  FareUsage fareUsage;
  fareUsage.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  pricingUnit.fareUsage().push_back(&fareUsage);
  farePath.pricingUnit().push_back(&pricingUnit);

  ToursApplicationMock toursApplication;
  toursApplication.validate(&farePath);
  const vector<FareUsage*>& fareUsages = toursApplication.validatedFareUsages();

  CPPUNIT_ASSERT_EQUAL(1, (int)fareUsages.size());
  CPPUNIT_ASSERT(fareUsages[0] == &fareUsage);
}

void
ToursApplicationTest::testValidateFarePathOnePricingUnitTwoFareUsage()
{
  FarePath farePath;
  PricingUnit pricingUnit;
  FareUsage fareUsage1;
  FareUsage fareUsage2;
  fareUsage1.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  fareUsage2.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  pricingUnit.fareUsage().push_back(&fareUsage1);
  pricingUnit.fareUsage().push_back(&fareUsage2);

  farePath.pricingUnit().push_back(&pricingUnit);

  ToursApplicationMock toursApplication;
  toursApplication.validate(&farePath);
  const vector<FareUsage*>& fareUsages = toursApplication.validatedFareUsages();

  CPPUNIT_ASSERT_EQUAL(2, (int)fareUsages.size());
  CPPUNIT_ASSERT(fareUsages[0] == &fareUsage1);
  CPPUNIT_ASSERT(fareUsages[1] == &fareUsage2);
}

void
ToursApplicationTest::testValidateFarePathTwoPricingUnitOneFareUsage()
{
  FarePath farePath;
  PricingUnit pricingUnit1;
  PricingUnit pricingUnit2;
  FareUsage fareUsage1;
  FareUsage fareUsage2;
  fareUsage1.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  fareUsage2.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  pricingUnit1.fareUsage().push_back(&fareUsage1);
  pricingUnit2.fareUsage().push_back(&fareUsage2);

  farePath.pricingUnit().push_back(&pricingUnit1);
  farePath.pricingUnit().push_back(&pricingUnit2);

  ToursApplicationMock toursApplication;
  toursApplication.validate(&farePath);
  const vector<FareUsage*>& fareUsages = toursApplication.validatedFareUsages();

  CPPUNIT_ASSERT_EQUAL(2, (int)fareUsages.size());
  CPPUNIT_ASSERT(fareUsages[0] == &fareUsage1);
  CPPUNIT_ASSERT(fareUsages[1] == &fareUsage2);
}

void
ToursApplicationTest::testValidateFarePathTwoPricingUnitTwoFareUsage()
{
  FarePath farePath;
  PricingUnit pricingUnit1;
  PricingUnit pricingUnit2;
  FareUsage fareUsage1;
  FareUsage fareUsage2;
  FareUsage fareUsage3;
  FareUsage fareUsage4;

  fareUsage1.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  fareUsage2.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  fareUsage3.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  fareUsage4.paxTypeFare() = _memHandle.create<PaxTypeFare>();
  pricingUnit1.fareUsage().push_back(&fareUsage1);
  pricingUnit1.fareUsage().push_back(&fareUsage2);
  pricingUnit2.fareUsage().push_back(&fareUsage3);
  pricingUnit2.fareUsage().push_back(&fareUsage4);

  farePath.pricingUnit().push_back(&pricingUnit1);
  farePath.pricingUnit().push_back(&pricingUnit2);

  ToursApplicationMock toursApplication;
  toursApplication.validate(&farePath);
  const vector<FareUsage*>& fareUsages = toursApplication.validatedFareUsages();

  CPPUNIT_ASSERT_EQUAL(4, (int)fareUsages.size());
  CPPUNIT_ASSERT(fareUsages[0] == &fareUsage1);
  CPPUNIT_ASSERT(fareUsages[1] == &fareUsage2);
  CPPUNIT_ASSERT(fareUsages[2] == &fareUsage3);
  CPPUNIT_ASSERT(fareUsages[3] == &fareUsage4);
}

void
ToursApplicationTest::testValidatePricingUnitCmdPricingInvalidCombination()
{
  FareUsage* fareUsage1 = createFareUsage("TC1", "AA");
  FareUsage* fareUsage2 = createFareUsage("TC2", "AA");

  PricingUnit pricingUnit;
  pricingUnit.fareUsage().push_back(fareUsage1);
  pricingUnit.fareUsage().push_back(fareUsage2);

  fareUsage1->paxTypeFare()->fareMarket()->fareBasisCode() = "J23";
  fareUsage2->paxTypeFare()->fareMarket()->fareBasisCode() = "J23";

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(&pricingUnit));
}

void
ToursApplicationTest::testValidateFarePathCmdPricingInvalidCombination()
{
  FareUsage* fareUsage1 = createFareUsage("TC1", "AA");
  FareUsage* fareUsage2 = createFareUsage("TC2", "AA");

  PricingUnit pricingUnit;
  pricingUnit.fareUsage().push_back(fareUsage1);
  pricingUnit.fareUsage().push_back(fareUsage2);

  FarePath farePath;
  farePath.pricingUnit().push_back(&pricingUnit);

  fareUsage1->paxTypeFare()->fareMarket()->fareBasisCode() = "J23";
  fareUsage2->paxTypeFare()->fareMarket()->fareBasisCode() = "J23";

  CPPUNIT_ASSERT_EQUAL(PASS, _toursApplication->validate(&farePath));
  CPPUNIT_ASSERT_EQUAL(true, farePath.multipleTourCodeWarning());
}

PaxTypeFare*
ToursApplicationTest::createPaxTypeFare(CarrierCode carrier, CarrierCode governing)
{
  PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
  FareMarket* fareMarket = _memHandle.create<FareMarket>();
  fareMarket->governingCarrier() = governing;
  paxTypeFare->fareMarket() = fareMarket;

  FareInfo* fareInfo = _memHandle.create<FareInfo>();
  fareInfo->carrier() = carrier;

  Fare* fare = _memHandle.create<Fare>();
  fare->setFareInfo(fareInfo);
  paxTypeFare->setFare(fare);

  return paxTypeFare;
}

void
ToursApplicationTest::testGetCarrierIndustryCarrier()
{
  PaxTypeFare* paxTypeFare = createPaxTypeFare("AA", "LH");
  CPPUNIT_ASSERT_EQUAL((string) "AA", (string)_toursApplication->getCarrier(paxTypeFare).c_str());
}

void
ToursApplicationTest::testGetCarrierNoIndustryCarrier()
{
  PaxTypeFare* paxTypeFare = createPaxTypeFare("YY", "LH");
  CPPUNIT_ASSERT_EQUAL((string) "LH", (string)_toursApplication->getCarrier(paxTypeFare).c_str());
}

void
ToursApplicationTest::testIsNegotiatedFareWithTourCodeForNonNegotiatedFare()
{
  PaxTypeFare* paxTypeFare = createPaxTypeFare("AA", "AA");
  CPPUNIT_ASSERT_EQUAL(false, _toursApplication->isNegotiatedFareWithTourCode(paxTypeFare));
}
}
