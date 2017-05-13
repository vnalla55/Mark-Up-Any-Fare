#include <algorithm>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "Rules/TicketingEndorsement.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

using namespace boost::assign;
using namespace std;

namespace tse
{

namespace
{

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  char getVendorType(const VendorCode& vendor)
  {
    VendorCode ddfVendorSMFA = "SMFA";
    VendorCode ddfVendorSMFC = "SMFC";
    if (vendor == ddfVendorSMFA || vendor == ddfVendorSMFC)
      return 'T';
    else
      return 'P';
  }
};
}

class TicketingEndorsementTest : public ::testing::TestWithParam<const char*>
{
public:
  TicketingEndorsementTest()
    : _one(1), _two(2), _three(3), _chickenString("CHICKEN"), _rabbitString("RABBIT")
  {
  }
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();

    TestConfigInitializer::setValue("OPTIMUS_ENDORSEMENT_CAT18_ENABLED", "Y", "PRICING_SVC", true);

    _trx = _memHandle(new PricingTrx());
    _trx->setRequest(_memHandle(new PricingRequest()));
    _trx->getRequest()->ticketingAgent() = _memHandle(new Agent());
    _ticketEndorse = _memHandle(new TicketingEndorsement());
    _fu = _memHandle(new FareUsage());
    _ptf = _memHandle(new PaxTypeFare());
    _itin = _memHandle(new Itin());
    _fp = _memHandle(new FarePath());
    _fp->itin() = _itin;

    _fare = _memHandle(new Fare());
    _fareInfo = _memHandle(new FareInfo());
  }

  void TearDown()
  {
    _memHandle.clear();
  }

  void createAgent()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
  }

  void enableAbacus()
  {
    TrxUtil::enableAbacus();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
  }

  void enableInfini()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1F";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "INFI";
  }

  void enableAxess()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1J";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "AXES";
  }

  void enableSabre()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1S";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "SABR";
  }

  FareUsage* buildBasicFareUsage(VendorCode vc)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();

    fareInfo->_vendor = vc;

    fare->setFareInfo(fareInfo);
    ptf->setFare(fare);
    fu->paxTypeFare() = ptf;
    return fu;
  }

  PricingUnit* buildPricingUnit(FareUsage* fu)
  {
    PricingUnit* pu = _memHandle(new PricingUnit());
    pu->fareUsage().push_back(fu);

    return pu;
  }

  void addPuToFarePath(PricingUnit* pu)
  {
    _fp->pricingUnit().push_back(pu);
  }

  void initializeTicketEndorseItems()
  {
    for (int i = 1; i < 4; ++i)
    {
      _ts = _memHandle(new AirSeg());
      _ts->pnrSegment() = i;
      _ts->segmentOrder() = i;
      _itin->travelSeg().push_back(_ts);
    }
    _fm = _memHandle(new FareMarket());
    _fm->travelSeg().push_back(_ts);
    _ptf->fareMarket() = _fm;

    _tei = _memHandle(new TicketEndorsementsInfo());
    TicketEndorsementsInfo::dummyData(*_tei);

    _te1 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te1->priorityCode = 1;
    _te1->tktLocInd = TicketingEndorsement::TKTLOCIND_FOP_1;
    _te1->endorsementTxt = "NONREF";

    _te2 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te2->priorityCode = 2;
    _te2->tktLocInd = TicketingEndorsement::TKTLOCIND_FOP_3;
    _te2->endorsementTxt = "APEX";

    _te3 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te3->priorityCode = 3;
    _te3->tktLocInd = TicketingEndorsement::TKTLOCIND_FOP_5;
    _te3->endorsementTxt = "SPEX";

    _te4 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te4->priorityCode = 6;
    _te4->tktLocInd = TicketingEndorsement::TKTLOCIND_2;
    _te4->endorsementTxt = "VALID LH ONLY";

    _te5 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te5->priorityCode = 5;
    _te5->tktLocInd = TicketingEndorsement::TKTLOCIND_4;
    _te5->endorsementTxt = "CHANGE OF RES RESTRICTED-";

    _te6 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te6->priorityCode = 4;
    _te6->tktLocInd = TicketingEndorsement::TKTLOCIND_6;
    _te6->endorsementTxt = "NON REFUNDABLE";

    _te7 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te7->priorityCode = 7;
    _te7->tktLocInd = TicketingEndorsement::TKTLOCIND_2;
    _te7->endorsementTxt = "LITWO";

    _te8 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te8->priorityCode = 8;
    _te8->tktLocInd = TicketingEndorsement::TKTLOCIND_4;
    _te8->endorsementTxt = "OJCZYZNO";

    _te9 = _memHandle(new TicketEndorseItem(*_tei, *_ptf));
    _te9->priorityCode = 9;
    _te9->tktLocInd = TicketingEndorsement::TKTLOCIND_6;
    _te9->endorsementTxt = "MOJA";
  }

  void initializeTicketingEndorsement(Indicator tktLocInd, const string& endorsementTxt)
  {
    _ticketEndorsementsInfo = _memHandle(new TicketEndorsementsInfo());
    _ticketEndorsementsInfo->unavailTag() = 'Z';
    _ticketEndorsementsInfo->tktLocInd() = tktLocInd;
    _ticketEndorsementsInfo->endorsementTxt() = endorsementTxt;
    _ticketEndorse->initialize(_ticketEndorsementsInfo);
    EXPECT_EQ(PASS, _ticketEndorse->process(*_trx, *_fu, *_ptf));
  }

  std::vector<TicketEndorseItem>::const_iterator
  eraseNotWantedMsgsTktEndWrapper(std::vector<TicketEndorseItem>& endorsements, bool isDFF)
  {
    return _ticketEndorse->eraseNotWantedMsgsTktEnd(endorsements, isDFF);
  }

  void prepLines(std::vector<TicketEndorseLine*>& lines)
  {
    TicketEndorseLine* first = _trx->dataHandle().create<TicketEndorseLine>();
    first->endorseMessage = "adin";
    first->priorityCode = 400;
    first->segmentOrders.insert(3);

    TicketEndorseLine* second = _trx->dataHandle().create<TicketEndorseLine>();
    second->endorseMessage = "dos";
    second->priorityCode = 100;
    second->segmentOrders.insert(2);

    TicketEndorseLine* third = _trx->dataHandle().create<TicketEndorseLine>();
    third->endorseMessage = "drei";
    third->priorityCode = 200;
    third->segmentOrders.insert(1);

    lines += first, second, third;
  }

  void prepLinesTheSame(std::vector<TicketEndorseLine*>& lines)
  {
    TicketEndorseLine* first = _trx->dataHandle().create<TicketEndorseLine>();
    first->endorseMessage = "blabla";
    first->priorityCode = 400;
    first->carrier = "LH";
    first->segmentOrders.insert(1);

    TicketEndorseLine* second = _trx->dataHandle().create<TicketEndorseLine>();
    second->endorseMessage = "blabla";
    second->priorityCode = 400;
    second->carrier = "UA";
    second->segmentOrders.insert(2);

    TicketEndorseLine* third = _trx->dataHandle().create<TicketEndorseLine>();
    third->endorseMessage = "blabla";
    third->priorityCode = 400;
    third->carrier = "UA";
    third->segmentOrders.insert(3);

    lines += first, second, third;
  }

protected:
  TestMemHandle _memHandle;

  const vector<TicketEndorseItem>::size_type _one;
  const vector<TicketEndorseItem>::size_type _two;
  const vector<TicketEndorseItem>::size_type _three;
  const string _chickenString;
  const string _rabbitString;

  PricingTrx* _trx;
  PaxTypeFare* _ptf;
  FareMarket* _fm;
  TravelSeg* _ts;
  Fare* _fare;
  FareInfo* _fareInfo;
  TicketEndorsementsInfo* _ticketEndorsementsInfo;
  TicketingEndorsement* _ticketEndorse;
  FareUsage* _fu;
  FarePath* _fp;
  Itin* _itin;

  TicketEndorsementsInfo* _tei;
  TicketEndorseItem* _te1;
  TicketEndorseItem* _te2;
  TicketEndorseItem* _te3;
  TicketEndorseItem* _te4;
  TicketEndorseItem* _te5;
  TicketEndorseItem* _te6;
  TicketEndorseItem* _te7;
  TicketEndorseItem* _te8;
  TicketEndorseItem* _te9;

  bool collectEndorsementTest(std::string vendor1,
                              std::string vendor2,
                              std::vector<TicketEndorseItem>& endos1,
                              std::vector<TicketEndorseItem>& endos2,
                              std::vector<std::string>& expected)
  {
    FareUsage* fu1 = buildBasicFareUsage(vendor1);
    for (TicketEndorseItem& i : endos1)
      fu1->tktEndorsement() += i;
    FareUsage* fu2 = buildBasicFareUsage(vendor2);
    for (TicketEndorseItem& i : endos2)
      fu2->tktEndorsement() += i;
    PricingUnit* pu = buildPricingUnit(fu1);
    pu->fareUsage() += fu2;
    _fp->pricingUnit() += pu;

    _trx->getRequest()->validatingCarrier() = "LH";

    EndorseCutter endoCut;
    TicketingEndorsement::TicketEndoLines result;
    _ticketEndorse->collectEndorsements(*_trx, *_fp, result, endoCut);
    _ticketEndorse->sortLinesByPrio(*_trx, *_fp, result);

    if (expected.size() != result.size())
      return false;

    return std::equal(expected.begin(),
                      expected.end(),
                      result.begin(),
                      boost::bind(&TicketEndorseLine::endorseMessage, _2) == _1);
  }
};

TEST_P(TicketingEndorsementTest, testSortAndGlue_No_ENDORSEMENTS)
  {
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    EXPECT_EQ((TicketEndorseLine*) nullptr, line);
  }

  TEST_P(TicketingEndorsementTest, testSortAndGlueDeleteItemsWithTicketLocIndValue3_Vendor_DFF)
  {
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    EXPECT_EQ((TicketEndorseLine*) nullptr, line);
  }

  TEST_F(TicketingEndorsementTest, testSortAndGlueDeleteItemsWithTicketLocIndValue3_Vendor_ATP)
  {
    VendorCode vc = "ATP";
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    EXPECT_EQ((TicketEndorseLine*) nullptr, line);
  }

  TEST_P(TicketingEndorsementTest,
         testSortAndGlueSortsAndGlueTextForItemsWithTicketLocIndValue2_Vendor_DFF)
  {
    createAgent(); // though we create agent , deactivating a particular user
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3, *_te4, *_te5, *_te6;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    string expected = _te6->endorsementTxt + "/" + _te5->endorsementTxt + "/" +
                      _te4->endorsementTxt;
    EXPECT_EQ(expected, line->endorseMessage);
    EXPECT_EQ(4, line->priorityCode);
  }

  TEST_P(TicketingEndorsementTest,
         testSortAndGlueSortsAndGlueTextForItemsWithTicketLocIndValue2_Vendor_DFF_Abacus)
  {
    createAgent();
    enableAbacus();
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3, *_te4,* _te5, *_te6;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    string expected = _te4->endorsementTxt + "/" + _te5->endorsementTxt + "/" +
                      _te6->endorsementTxt;
    EXPECT_EQ(expected, line->endorseMessage);
    EXPECT_EQ(4, line->priorityCode);
  }

  TEST_P(TicketingEndorsementTest,
         testSortAndGlueSortsAndGlueTextForItemsWithTicketLocIndValue2_Vendor_DFF_Axess)
  {
    createAgent();
    enableAxess();
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3, *_te4,* _te5, *_te6;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    string expected = _te4->endorsementTxt + "/" + _te5->endorsementTxt + "/" +
                      _te6->endorsementTxt;
    EXPECT_EQ(expected, line->endorseMessage);
    EXPECT_EQ(4, line->priorityCode);
  }

  TEST_P(TicketingEndorsementTest,
         testSortAndGlueSortsAndGlueTextForItemsWithTicketLocIndValue2_Vendor_Infini)
  {
    createAgent();
    enableInfini();
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3, *_te4,* _te5, *_te6;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    string expected = _te4->endorsementTxt + "/" + _te5->endorsementTxt + "/" +
                      _te6->endorsementTxt;
    EXPECT_EQ(expected, line->endorseMessage);
    EXPECT_EQ(4, line->priorityCode);
  }

  TEST_P(TicketingEndorsementTest,
         testSortAndGlueSortsAndGlueTextForItemsWithTicketLocIndValue2_Vendor_Sabre)
  {
    createAgent();
    enableSabre();
    VendorCode vc = GetParam();
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3, *_te4,* _te5, *_te6;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    string expected = _te4->endorsementTxt + "/" + _te5->endorsementTxt + "/" +
                      _te6->endorsementTxt;
    EXPECT_EQ(expected, line->endorseMessage);
    EXPECT_EQ(4, line->priorityCode);
  }

  TEST_F(TicketingEndorsementTest,
         testSortAndGlueSortsAndGlueTextForItemsWithTicketLocIndValue2_Vendor_ATP)
  {
    VendorCode vc = "ATP";
    FareUsage* fu = buildBasicFareUsage(vc);
    initializeTicketEndorseItems();
    fu->tktEndorsement() += *_te1, *_te2, *_te3, *_te4,* _te5, *_te6;
    TicketEndorseLine* line = _ticketEndorse->sortAndGlue(*_trx, *_itin, *fu,
                                                          EndorseCutter());
    string expected = _te6->endorsementTxt + "/" + _te5->endorsementTxt + "/" +
                      _te4->endorsementTxt;
    EXPECT_EQ(expected, line->endorseMessage);
    EXPECT_EQ(4, line->priorityCode);
  }

  TEST_F(TicketingEndorsementTest, testAddOneItem)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(_chickenString, _fu->tktEndorsement()[0].endorsementTxt);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameItems)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(_chickenString, _fu->tktEndorsement()[0].endorsementTxt);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoDifferentItems)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _rabbitString);

    EXPECT_EQ(_two, _fu->tktEndorsement().size());
    EXPECT_EQ(_chickenString, _fu->tktEndorsement()[0].endorsementTxt);
    EXPECT_EQ(_rabbitString, _fu->tktEndorsement()[1].endorsementTxt);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtDifferentTktLocIndItems)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _chickenString);

    EXPECT_EQ(_two, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_1, _fu->tktEndorsement()[0].tktLocInd);
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_2, _fu->tktEndorsement()[1].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTktLocIndDifferentTxtItems)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _rabbitString);

    EXPECT_EQ(_two, _fu->tktEndorsement().size());
    EXPECT_EQ(_chickenString, _fu->tktEndorsement()[0].endorsementTxt);
    EXPECT_EQ(_rabbitString, _fu->tktEndorsement()[1].endorsementTxt);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems26)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_6, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_2, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems62)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_6, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_6, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems51)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_5, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_5, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems15)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_5, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_1, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems31)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_3, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_1, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems35)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_3, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_5, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_5, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems13)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_3, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_1, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddTwoSameTxtItems53)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_5, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_3, _chickenString);

    EXPECT_EQ(_one, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_5, _fu->tktEndorsement()[0].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, testAddThreeSameTxtItems126)
  {
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_FOP_1, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_2, _chickenString);
    initializeTicketingEndorsement(TicketingEndorsement::TKTLOCIND_6, _chickenString);

    EXPECT_EQ(_two, _fu->tktEndorsement().size());
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_FOP_1, _fu->tktEndorsement()[0].tktLocInd);
    EXPECT_EQ(TicketingEndorsement::TKTLOCIND_2, _fu->tktEndorsement()[1].tktLocInd);
  }

  TEST_F(TicketingEndorsementTest, test_eraseNotWantedMsgsTktEnd)
  {
    std::vector<TicketEndorseItem> teVec;

    initializeTicketEndorseItems();
    teVec.push_back(*_te1);
    teVec.push_back(*_te2);
    teVec.push_back(*_te3);
    teVec.push_back(*_te4);

    vector<TicketEndorseItem>::const_iterator newend = eraseNotWantedMsgsTktEndWrapper(teVec, true);

    size_t validMsgSize = newend - teVec.begin();
    EXPECT_EQ(_one, validMsgSize);
  }

  TEST_F(TicketingEndorsementTest, test_Process_Fail)
  {
    EXPECT_EQ(FAIL, _ticketEndorse->process(*_trx, *_fu, *_ptf));
  }

  TEST_F(TicketingEndorsementTest, test_Process_UNAVAILTAG_IS_X_Fail)
  {
    _ticketEndorsementsInfo = _memHandle(new TicketEndorsementsInfo());
    _ticketEndorsementsInfo->unavailTag() = 'X';
    _ticketEndorsementsInfo->tktLocInd() = TicketingEndorsement::TKTLOCIND_FOP_1;
    _ticketEndorsementsInfo->endorsementTxt() = "TEST";
    _ticketEndorse->initialize(_ticketEndorsementsInfo);
    EXPECT_EQ(FAIL, _ticketEndorse->process(*_trx, *_fu, *_ptf));
  }

  TEST_F(TicketingEndorsementTest, test_Process_UNAVAILTAG_IS_Y_SKIP)
  {
    _ticketEndorsementsInfo = _memHandle(new TicketEndorsementsInfo());
    _ticketEndorsementsInfo->unavailTag() = 'Y';
    _ticketEndorsementsInfo->tktLocInd() = TicketingEndorsement::TKTLOCIND_FOP_1;
    _ticketEndorsementsInfo->endorsementTxt() = "TEST";
    _ticketEndorse->initialize(_ticketEndorsementsInfo);
    EXPECT_EQ(SKIP, _ticketEndorse->process(*_trx, *_fu, *_ptf));
  }

  TEST_F(TicketingEndorsementTest, test_collectEndo_ticketing_DFF_DFF)
  {
    initializeTicketEndorseItems();
    std::vector<std::string> expected;
    expected += _te4->endorsementTxt + "/" + _te6->endorsementTxt + "/" + _te5->endorsementTxt;
    expected += _te7->endorsementTxt + "/" + _te8->endorsementTxt + "/" + _te9->endorsementTxt;

    std::vector<TicketEndorseItem> first;
    first += *_te7, *_te8, *_te9;

    std::vector<TicketEndorseItem> second;
    second += *_te4, *_te6, *_te5;

    createAgent();
    enableAxess();
    EXPECT_TRUE(collectEndorsementTest("SMFA", "SMFC", first, second, expected));
  }

  TEST_F(TicketingEndorsementTest, test_collectEndo_ticketing_DFF_DFF_noOrderChangeNeeded)
  {
    initializeTicketEndorseItems();
    std::vector<std::string> expected;
    expected += _te6->endorsementTxt + "/" + _te8->endorsementTxt + "/" + _te9->endorsementTxt;
    expected += _te7->endorsementTxt + "/" + _te4->endorsementTxt + "/" + _te5->endorsementTxt;

    std::vector<TicketEndorseItem> first;
    first += *_te7, *_te4, *_te5;

    std::vector<TicketEndorseItem> second;
    second += *_te6, *_te8, *_te9;

    createAgent();
    enableAxess();
    EXPECT_TRUE(collectEndorsementTest("SMFA", "SMFC", first, second, expected));
  }

  TEST_F(TicketingEndorsementTest, test_collectEndo_ticketing_ATP_DFF)
  {
    initializeTicketEndorseItems();
    std::vector<std::string> expected;
    expected += _te4->endorsementTxt + "/" + _te6->endorsementTxt + "/" + _te5->endorsementTxt;
    expected += _te7->endorsementTxt + "/" + _te8->endorsementTxt + "/" + _te9->endorsementTxt;

    std::vector<TicketEndorseItem> first;
    first += *_te9, *_te8, *_te7;

    std::vector<TicketEndorseItem> second;
    second += *_te4, *_te6, *_te5;

    createAgent();
    enableAxess();
    EXPECT_TRUE(collectEndorsementTest("ATP", "SMFC", first, second, expected));
  }

  TEST_F(TicketingEndorsementTest, test_collectEndo_ticketing_ATP_ATP)
  {
    initializeTicketEndorseItems();
    std::vector<std::string> expected;
    expected += _te6->endorsementTxt + "/" + _te5->endorsementTxt + "/" + _te4->endorsementTxt;
    expected += _te7->endorsementTxt + "/" + _te8->endorsementTxt + "/" + _te9->endorsementTxt;

    std::vector<TicketEndorseItem> first;
    first += *_te9, *_te8, *_te7;

    std::vector<TicketEndorseItem> second;
    second += *_te4, *_te6, *_te5;

    createAgent();
    enableAxess();
    EXPECT_TRUE(collectEndorsementTest("ATP", "ATP", first, second, expected));
  }

  TEST_F(TicketingEndorsementTest, test_collectEndo_ticketing_FOP)
  {
    initializeTicketEndorseItems();
    std::vector<std::string> expected;
    expected += _te6->endorsementTxt + "/" + _te5->endorsementTxt;
    expected += _te7->endorsementTxt + "/" + _te8->endorsementTxt;

    std::vector<TicketEndorseItem> first;
    first += *_te1, *_te8, *_te7;

    std::vector<TicketEndorseItem> second;
    second += *_te1, *_te6, *_te5;

    createAgent();
    enableAxess();
    EXPECT_TRUE(collectEndorsementTest("ATP", "ATP", first, second, expected));
  }

  TEST_F(TicketingEndorsementTest, test_collectEndo_ticketing_Abacus)
  {
    initializeTicketEndorseItems();
    std::vector<std::string> expected;
    expected += _te4->endorsementTxt + "/" + _te5->endorsementTxt + "/" + _te6->endorsementTxt;
    expected += _te9->endorsementTxt + "/" + _te7->endorsementTxt + "/" + _te8->endorsementTxt;

    std::vector<TicketEndorseItem> first;
    first += *_te4, *_te5, *_te6;

    std::vector<TicketEndorseItem> second;
    second += *_te9, *_te7, *_te8;

    createAgent();
    enableAbacus();
    EXPECT_TRUE(collectEndorsementTest("ATP", "SMFA", first, second, expected));
  }

  TEST_F(TicketingEndorsementTest, test_endorse_cutter)
  {
    initializeTicketEndorseItems();
    std::vector<TicketEndorseItem> input;
    input += *_te1, *_te2, *_te3, *_te4, *_te5, *_te6, *_te7, *_te8, *_te9;

    std::string expected =
        _te1->endorsementTxt + "/" + _te2->endorsementTxt + "/" + _te3->endorsementTxt + "/" +
        _te4->endorsementTxt + "/" + _te5->endorsementTxt + "/" + _te6->endorsementTxt + "/" +
        _te7->endorsementTxt + "/" + _te8->endorsementTxt + "/" + _te9->endorsementTxt;

    TicketEndorseLine output;

    EndorseCutter endoCut;
    endoCut(input, &output);

    EXPECT_EQ(output.endorseMessage, expected);
    EXPECT_EQ(output.priorityCode, 1);
  }

  TEST_F(TicketingEndorsementTest, test_line_ordering_by_prio)
  {
    TicketingEndorsement::TicketEndoLines lines;
    prepLines(lines);

    TicketingEndorsement::TicketEndoLines expected;
    expected += lines[1], lines[2], lines[0];

    _trx->getRequest()->validatingCarrier() = "LH";

    _ticketEndorse->sortLinesByPrio(*_trx, *_fp, lines);

    ASSERT_EQ(lines.size(), expected.size());

    std::vector<TicketEndorseLine*>::const_iterator line = lines.begin();
    std::vector<TicketEndorseLine*>::const_iterator lineExpected = expected.begin();
    while (line != lines.end())
    {
      EXPECT_EQ((**line).endorseMessage, (**lineExpected).endorseMessage);
      EXPECT_EQ((**line).priorityCode, (**lineExpected).priorityCode);
      EXPECT_EQ((*(**line).segmentOrders.begin()), (*(**lineExpected).segmentOrders.begin()));

      ++line;
      ++lineExpected;
    }
  }

  TEST_F(TicketingEndorsementTest, test_line_ordering_by_prio_the_same_txt)
  {
    TicketingEndorsement::TicketEndoLines lines;
    prepLinesTheSame(lines);

    TicketEndorseLine expected(*lines.back());

    _trx->getRequest()->validatingCarrier() = "UA";

    _ticketEndorse->sortLinesByPrio(*_trx, *_fp, lines);

    EXPECT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]->endorseMessage, expected.endorseMessage);
    EXPECT_EQ(lines[0]->priorityCode, expected.priorityCode);
    EXPECT_EQ(lines[0]->carrier, expected.carrier);
  }

  TEST_F(TicketingEndorsementTest, test_line_ordering_by_pnr)
  {
    TicketingEndorsement::TicketEndoLines lines;
    prepLines(lines);

    TicketingEndorsement::TicketEndoLines expected;
    expected += lines[2], lines[1], lines[0];

    _trx->getRequest()->validatingCarrier() = "LH";

    _ticketEndorse->sortLinesByPnr(*_trx, lines);

    ASSERT_EQ(lines.size(), expected.size());

    std::vector<TicketEndorseLine*>::const_iterator line = lines.begin();
    std::vector<TicketEndorseLine*>::const_iterator lineExpected = expected.begin();
    while (line != lines.end())
    {
      EXPECT_EQ((**line).endorseMessage, (**lineExpected).endorseMessage);
      EXPECT_EQ((**line).priorityCode, (**lineExpected).priorityCode);
      EXPECT_EQ((*(**line).segmentOrders.begin()), (*(**lineExpected).segmentOrders.begin()));

      ++line;
      ++lineExpected;
    }
  }

  TEST_F(TicketingEndorsementTest, test_line_ordering_by_pnr_the_same_txt)
  {
    TicketingEndorsement::TicketEndoLines lines;
    prepLinesTheSame(lines);

    TicketEndorseLine expected(*lines.front());

    _trx->getRequest()->validatingCarrier() = "LH";

    _ticketEndorse->sortLinesByPnr(*_trx, lines);

    EXPECT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0]->endorseMessage, expected.endorseMessage);
    EXPECT_EQ(lines[0]->priorityCode, expected.priorityCode);
    EXPECT_EQ(lines[0]->carrier, expected.carrier);
  }

  INSTANTIATE_TEST_CASE_P(SmfaSmfcInstance,
                          TicketingEndorsementTest,
                          ::testing::Values("SMFA", "SMFC"));
}
