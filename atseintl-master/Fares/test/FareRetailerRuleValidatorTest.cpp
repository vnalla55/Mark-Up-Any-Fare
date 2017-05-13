#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusPsgTypeInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "Fares/FareRetailerRuleValidator.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{

class MockFareRetailerRuleValidator : public FareRetailerRuleValidator
{
public:

  MockFareRetailerRuleValidator(PricingTrx& trx) :
    FareRetailerRuleValidator(trx)
  {
    _frrli1.sourcePcc() = "PCC1";
    _frrli2.sourcePcc() = "PCC2";
  }

  virtual ~MockFareRetailerRuleValidator() {}

  void populateCustomerSecurityHandshakeInfo()
  {
    _cshiVFull.push_back(&_c1);
  }

  const std::vector<CustomerSecurityHandshakeInfo*>&
  getCustomerSecurityHandshake(const Code<8> productCD, const PseudoCityCode& pcc) const
  {
    if ((pcc == "PCC1") || (pcc == "PCC2"))
      return _cshiVFull;

    return _cshiVEmpty;
  }

  const FareRetailerRuleLookupInfo*
  getFareRetailerRuleLookup(Indicator applicationType,
                            const PseudoCityCode& sourcePcc,
                            const PseudoCityCode& pcc) const
  {
    if (pcc == "PCC1")
      return &_frrli1;

    if (pcc == "PCC2")
      return &_frrli2;

    return 0;
  }

  const std::vector<FareFocusFareClassInfo*>&
  getFareFocusFareClass(uint64_t fareClassItemNo) const
  {
    std::vector<FareClassCodeC> fc1, fc2;
    fc1.push_back("QQQ?");

    _fffci.fareClass() = fc1;

    _fffciv_good.clear();
    _fffciv_bad.clear();

    _fffciv_good.push_back(&_fffci);
    _fffciv_bad.push_back(0);
    _fffciv_bad.push_back(&_fffci);

    if (fareClassItemNo == 111)
      return _fffciv_good;

    if (fareClassItemNo == 999)
      return _fffciv_bad;

    return _fffciv_empty;
  }

  const FareFocusPsgTypeInfo*
  getFareFocusPsgType(uint64_t psgTypeItemNo) const
  {

    std::vector<PaxTypeCode>& paxTypeCodeV = _fffci_good.psgType();

    paxTypeCodeV.push_back(ADULT);
    paxTypeCodeV.push_back(CHILD);

    std::vector<PaxTypeCode>& paxTypeCodeVempty = _fffci_empty.psgType();

    paxTypeCodeVempty.clear();

    std::vector<PaxTypeCode>& paxTypeCodeVForWCA = _fffci_wildCardA.psgType();

    paxTypeCodeVForWCA.push_back("*A");
    paxTypeCodeVForWCA.push_back(CHILD);

    std::vector<PaxTypeCode>& paxTypeCodeVForWCC = _fffci_wildCardC.psgType();

    paxTypeCodeVForWCC.push_back("*C");
    paxTypeCodeVForWCC.push_back(ADULT);

    std::vector<PaxTypeCode>& paxTypeCodeVForWCI = _fffci_wildCardI.psgType();

    paxTypeCodeVForWCI.push_back("*I");
    paxTypeCodeVForWCI.push_back(ADULT);

    if (psgTypeItemNo == 111)
      return &_fffci_good;
    else if (psgTypeItemNo == 999)
      return nullptr;
    else if (psgTypeItemNo == 888)
      return &_fffci_wildCardA;
    else if (psgTypeItemNo == 777)
      return &_fffci_wildCardC;
    else if (psgTypeItemNo == 666)
      return &_fffci_wildCardI;
    else
      return &_fffci_empty;
  }

private:

  std::vector<CustomerSecurityHandshakeInfo*> _cshiVEmpty, _cshiVFull;
  FareRetailerRuleLookupInfo _frrli1, _frrli2;
  CustomerSecurityHandshakeInfo _c1;
  mutable std::vector<FareFocusFareClassInfo*> _fffciv_good, _fffciv_bad, _fffciv_empty;
  mutable FareFocusFareClassInfo _fffci;
  mutable FareFocusPsgTypeInfo _fffci_good, _fffci_empty;
  mutable FareFocusPsgTypeInfo _fffci_wildCardA, _fffci_wildCardC, _fffci_wildCardI;
};

class FareRetailerRuleValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareRetailerRuleValidatorTest);

  CPPUNIT_TEST(testGetFRRLookupAllSources);
  CPPUNIT_TEST(testGetFRRLookupSources);
  CPPUNIT_TEST(testMergeRules);
  CPPUNIT_TEST(testMergeFrrls);
  CPPUNIT_TEST(testMatchGeo);
  CPPUNIT_TEST(testValidGeoType);
  CPPUNIT_TEST(testMatchLocation);
  CPPUNIT_TEST(testMatchFareClass);
  CPPUNIT_TEST(testMatchExcludeFareClass);
  CPPUNIT_TEST(testMatchPassengerTypeCode);
  CPPUNIT_TEST(testMatchPublicPrivateInd);
  CPPUNIT_TEST(testMatchRetailerCode);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
public:

void setUp()
{
  _memHandle.create<TestConfigInitializer>();
}

void tearDown()
{
  _memHandle.clear();
}

void testGetFRRLookupAllSources()
{
  PricingTrx trx;
  FareMarket fm;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  agent.tvlAgencyPCC() = "PCC1";
  agent.mainTvlAgencyPCC() = "PCC1";

  MockFareRetailerRuleValidator frrv1(trx);
  frrv1.populateCustomerSecurityHandshakeInfo();

  std::vector<const FareRetailerRuleLookupInfo*> frrlV;
  frrv1.getFRRLookupAllSources(frrlV, 0, "RN", 'N', true);

  CPPUNIT_ASSERT(frrlV.size() == 1);

  agent.mainTvlAgencyPCC() = "PCC2";
  frrlV.clear();

  MockFareRetailerRuleValidator frrv2(trx);
  frrv2.populateCustomerSecurityHandshakeInfo();

  frrv2.getFRRLookupAllSources(frrlV, 0, "RN", 'N', true);

  CPPUNIT_ASSERT(frrlV.size() == 2);
}

void testMatchPassengerTypeCode()
{
  PricingTrx trx;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  PaxTypeFare ptf;

  const FareClassAppSegInfo* fcasi =  ptf.fareClassAppSegInfo();
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = "";
  MockFareRetailerRuleValidator frrv(trx);

  // fareClassItemNo = 111. Should match "ADT"
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(111, ptf));

  // fareClassItemNo = 111. Should match "CNN"
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = CHILD;
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(111, ptf));

  // fareClassItemNo = 111. "SRC" Should NOT match "ADT"
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = SRC;
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(111, ptf));

  // fareClassItemNo = 111. Should NOT match "INF"
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = INFANT;
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(111, ptf));

  // fareClassItemNo = 0. Should match return true
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(0, ptf));
  // fareClassItemNo = 999. Should NOT match, 999 return nullptr
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(999, ptf));

  // fareClassItemNo = 888. "" should match *A
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = "";
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(888, ptf));

  // fareClassItemNo = 888. "ADT" should match *A
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = ADULT;
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(888, ptf));

  // fareClassItemNo = 888. "NEG" should match *A
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = NEG;
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(888, ptf));

  // fareClassItemNo = 888. "INF" should NOT match because the vector doesn't have "INF"
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = INFANT;
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(888, ptf));

  // fareClassItemNo = 777. "CNN" should match *C
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = CHILD;
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(777, ptf));

  // fareClassItemNo = 777. "INF" should NOT match because the vector doesn't have "INF"
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = INFANT;
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(777, ptf));

  // fareClassItemNo = 666. "INF" should match *I
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = INFANT;
  CPPUNIT_ASSERT(frrv.matchPassengerTypeCode(666, ptf));

  // fareClassItemNo = 666. "CNN" should NOT match because the vector doesn't have "CNN"
  const_cast<FareClassAppSegInfo*>(fcasi)->_paxType = CHILD;
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(666, ptf));

  // fareClassItemNo = 444. Should NOT match
  CPPUNIT_ASSERT(!frrv.matchPassengerTypeCode(444, ptf));
}
void testMatchPublicPrivateInd()
 {
    PricingTrx trx;
    PricingRequest request;
    Agent agent;

    request.ticketingAgent() = &agent;
    trx.setRequest(&request);

    FareRetailerRuleValidator frrv(trx);

    Indicator publicPrivateIndRule = ' ';
    TariffCategory publicIndFare = RuleConst::PRIVATE_TARIFF;
    CPPUNIT_ASSERT(frrv.matchPublicPrivateInd(publicPrivateIndRule, publicIndFare));
    publicPrivateIndRule = '*';
    CPPUNIT_ASSERT(!frrv.matchPublicPrivateInd(publicPrivateIndRule, publicIndFare));

    // making IndRule as "P" or "V" and checking Indicator fare to public tariff
    publicPrivateIndRule = 'P';
    publicIndFare = RuleConst::PUBLIC_TARIFF;
    CPPUNIT_ASSERT(frrv.matchPublicPrivateInd(publicPrivateIndRule, publicIndFare));
    publicPrivateIndRule = '*';
    CPPUNIT_ASSERT(!frrv.matchPublicPrivateInd(publicPrivateIndRule, publicIndFare));


    // making IndRule as "P" or "V" and checking Indicator fare to Private Tariff
    publicPrivateIndRule = 'V';
    publicIndFare = RuleConst::PRIVATE_TARIFF;
    CPPUNIT_ASSERT(frrv.matchPublicPrivateInd(publicPrivateIndRule, publicIndFare));
    publicPrivateIndRule = '*';
    CPPUNIT_ASSERT(!frrv.matchPublicPrivateInd(publicPrivateIndRule, publicIndFare));
    // cout<< " END of matchPublicPrivateInd" << endl;

 }

/*
void testMatchFareType()
-{
-    PricingTrx trx;
-    PricingRequest request;
-    Agent agent;
-
-    request.ticketingAgent() = &agent;
-    trx.setRequest(&request);
-
-    FareRetailerRuleValidator frrv(trx);
-
-    fallback::value::fallbackmatchGenericFareType.set(true);
-
-    FareTypeAbbrevC fareTypeRule;
-    FareType fareTypeFare = "XYZ";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fareTypeRule = "*X"; // one of family class X
-    fareTypeFare = "XEX";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fareTypeRule = "*B";  // one of the family class B
-    fareTypeFare = "BR";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fareTypeRule = "*Y";  // one of the family class Y
-    fareTypeFare = "EU";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    // false cases
-
-    fareTypeRule = "ABC";
-    fareTypeFare = "SIP";
-    CPPUNIT_ASSERT(!frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fareTypeRule = "*EW";
-    fareTypeFare = "ZIP";
-    CPPUNIT_ASSERT(!frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fallback::value::fallbackmatchGenericFareType.set(false);
-
-    fareTypeRule = "*BJ";
-    fareTypeFare = "JR";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fareTypeRule = "*B";  // one of the family
-    fareTypeFare = "BX";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-    fareTypeRule = "*EW";  // one of the family class Y
-    fareTypeFare = "SIP";
-    CPPUNIT_ASSERT(frrv.matchFareType(fareTypeRule, fareTypeFare));
-
-}

*/

void testMatchFareClass()
{
  PricingTrx trx;
  FareMarket fm;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  PricingOptions option;
  trx.setOptions(&option);

  PaxTypeFare ptf;

  Fare fare;
  FareInfo fareInfo;
  fareInfo.fareClass() = "QQQ3";

  FareMarket fm1;
  fare.initialize(Fare::FS_Domestic, &fareInfo, fm1);
  ptf.setFare(&fare);

  MockFareRetailerRuleValidator frrv(trx);

  // fareClassItemNo == 0
  CPPUNIT_ASSERT(frrv.matchFareClass(0, ptf));

  // fareClassItemNo = 111. Should match "QQQ?"
  CPPUNIT_ASSERT(frrv.matchFareClass(111, ptf));

  fareInfo.fareClass() = "QQS3";
  // now no match
  CPPUNIT_ASSERT(!frrv.matchFareClass(111, ptf));

  fareInfo.fareClass() = "QQQ3";
  CPPUNIT_ASSERT(frrv.matchFareClass(111, ptf));

  // item no 999 has NULL fareClass. Should not match
  CPPUNIT_ASSERT(!frrv.matchFareClass(999, ptf));

  // item no 888 has empty class. Should not match
  CPPUNIT_ASSERT(!frrv.matchFareClass(888, ptf));
}

void testMatchExcludeFareClass()
{
  PricingTrx trx;
  FareMarket fm;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  PricingOptions option;
  trx.setOptions(&option);

  PaxTypeFare ptf;

  Fare fare;
  FareInfo fareInfo;
  fareInfo.fareClass() = "QQQ3";

  FareMarket fm1;
  fare.initialize(Fare::FS_Domestic, &fareInfo, fm1);
  ptf.setFare(&fare);

  MockFareRetailerRuleValidator frrv(trx);

  // fareClassItemNo == 0
  CPPUNIT_ASSERT(!frrv.matchFareClass(0, ptf, true));

  // fareClassItemNo = 111. Should match "QQQ?"
  CPPUNIT_ASSERT(frrv.matchFareClass(111, ptf, true));

  fareInfo.fareClass() = "QQS3";
  // now no match
  CPPUNIT_ASSERT(!frrv.matchFareClass(111, ptf, true));

  fareInfo.fareClass() = "QQQ3";
  CPPUNIT_ASSERT(frrv.matchFareClass(111, ptf, true));

  // item no 999 has NULL fareClass. Should match
  CPPUNIT_ASSERT(frrv.matchFareClass(999, ptf, true));

  // item no 888 has empty class. Should not match
  CPPUNIT_ASSERT(!frrv.matchFareClass(888, ptf, true));
}

void testGetFRRLookupSources()
{
  PricingTrx trx;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  MockFareRetailerRuleValidator frrv(trx);

  std::vector<const FareRetailerRuleLookupInfo*> frrlV;
  PseudoCityCode pcc = "PCC1";
  frrv.getFRRLookupSources(frrlV, pcc, "RN", 'N');

  CPPUNIT_ASSERT(frrlV.empty());

  frrv.populateCustomerSecurityHandshakeInfo();

  frrv.getFRRLookupSources(frrlV, pcc, "RN", 'N');
  CPPUNIT_ASSERT(frrlV.size());

  pcc = "ABCD";
  frrlV.clear();
  frrv.getFRRLookupSources(frrlV, pcc, "RN", 'N');
  CPPUNIT_ASSERT(frrlV.empty());
}

void testMergeRules()
{
  PricingTrx trx;
  FareMarket fm;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  FareRetailerRuleValidator frrv(trx);

  FareRetailerRuleLookupInfo f1, f2;

  f1.applicationType() = 'N';
  f1.sourcePcc() = "ABCD";
  f1.pcc() = "ZXCV";

  FareRetailerRuleLookupInfo* result = frrv.mergeRules(&f1, &f2);
  CPPUNIT_ASSERT(result->applicationType() == 'N');
  CPPUNIT_ASSERT(result->sourcePcc() == "ABCD");
  CPPUNIT_ASSERT(result->pcc() == "ZXCV");
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds().empty());

  FareRetailerRuleLookupId frrl1(5, 10);
  f1.fareRetailerRuleLookupIds().push_back(frrl1);
  result = frrv.mergeRules(&f1, &f2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds().size() == 1);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 5);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._ruleSeqNo == 10);

  f1.fareRetailerRuleLookupIds().clear();
  f2.fareRetailerRuleLookupIds().push_back(frrl1);
  result = frrv.mergeRules(&f1, &f2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds().size() == 1);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 5);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._ruleSeqNo == 10);

  FareRetailerRuleLookupId frrl2(3, 11);
  f1.fareRetailerRuleLookupIds().push_back(frrl2);
  result = frrv.mergeRules(&f1, &f2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds().size() == 2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 5);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._ruleSeqNo == 10);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[1]._fareRetailerRuleId == 3);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[1]._ruleSeqNo == 11);

  f1.fareRetailerRuleLookupIds().clear();
  f1.fareRetailerRuleLookupIds().push_back(frrl1);
  result = frrv.mergeRules(&f1, &f2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds().size() == 1);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 5);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._ruleSeqNo == 10);

  f1.fareRetailerRuleLookupIds().clear();
  f2.fareRetailerRuleLookupIds().clear();

  f1.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(9, 3));
  f1.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(4, 6));
  f1.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(6, 9));
  f2.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(1, 1));
  f2.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(2, 2));
  f2.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(4, 6));

  result = frrv.mergeRules(&f1, &f2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds().size() == 5);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._ruleSeqNo == 1);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 1);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[1]._ruleSeqNo == 2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[1]._fareRetailerRuleId == 2);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[2]._ruleSeqNo == 3);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[2]._fareRetailerRuleId == 9);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[3]._ruleSeqNo == 6);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[3]._fareRetailerRuleId == 4);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[4]._ruleSeqNo == 9);
  CPPUNIT_ASSERT(result->fareRetailerRuleLookupIds()[4]._fareRetailerRuleId == 6);
}

void testMergeFrrls()
{
  PricingTrx trx;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  FareRetailerRuleValidator frrv(trx);

  FareRetailerRuleLookupInfo f1, f2, f3, f4, f5, f6;
  std::vector<const FareRetailerRuleLookupInfo*> frrlV1, frrlV2;

  f1.sourcePcc() = "AAA1";
  f1.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(9, 9));

  frrlV2.push_back(&f1);

  frrv.mergeFrrls(frrlV1, frrlV2);
  CPPUNIT_ASSERT(frrlV1.size() == 1);
  CPPUNIT_ASSERT(f1.fareRetailerRuleLookupIds().size() == 1);
  CPPUNIT_ASSERT(f1.fareRetailerRuleLookupIds()[0]._ruleSeqNo == 9);
  CPPUNIT_ASSERT(f1.fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 9);

  f2.sourcePcc() = "AAA2";
  f3.sourcePcc() = "AAA3";

  f4.sourcePcc() = "BBB1";
  f5.sourcePcc() = "BBB2";
  f6.sourcePcc() = "BBB3";

  frrlV1.clear();
  frrlV2.clear();

  frrlV1.push_back(&f1);
  frrlV1.push_back(&f2);
  frrlV1.push_back(&f3);

  frrlV2.push_back(&f4);
  frrlV2.push_back(&f5);
  frrlV2.push_back(&f6);

  frrv.mergeFrrls(frrlV1, frrlV2);
  CPPUNIT_ASSERT(frrlV1.size() == 6);

  std::set<std::string> sourcePccList;
  for (auto f : frrlV1)
    sourcePccList.insert(f->sourcePcc());

  CPPUNIT_ASSERT(sourcePccList.count("AAA1"));
  CPPUNIT_ASSERT(sourcePccList.count("AAA2"));
  CPPUNIT_ASSERT(sourcePccList.count("AAA3"));
  CPPUNIT_ASSERT(sourcePccList.count("BBB1"));
  CPPUNIT_ASSERT(sourcePccList.count("BBB2"));
  CPPUNIT_ASSERT(sourcePccList.count("BBB3"));

  frrlV1.clear();
  frrlV2.clear();
  sourcePccList.clear();

  f6.sourcePcc() = "AAA1";
  f6.fareRetailerRuleLookupIds().push_back(FareRetailerRuleLookupId(8, 8));
  frrlV1.push_back(&f1);
  frrlV1.push_back(&f2);

  frrlV2.push_back(&f5);
  frrlV2.push_back(&f6);
  frrv.mergeFrrls(frrlV1, frrlV2);
  CPPUNIT_ASSERT(frrlV1.size() == 3);

  // AAA1 is common, should be merged.
  for (auto f : frrlV1)
  {
    sourcePccList.insert(f->sourcePcc());
    if (f->sourcePcc() == "AAA1")
    {
      CPPUNIT_ASSERT(f->fareRetailerRuleLookupIds().size() == 2);
      CPPUNIT_ASSERT(f->fareRetailerRuleLookupIds()[0]._ruleSeqNo == 8);
      CPPUNIT_ASSERT(f->fareRetailerRuleLookupIds()[0]._fareRetailerRuleId == 8);
      CPPUNIT_ASSERT(f->fareRetailerRuleLookupIds()[1]._ruleSeqNo == 9);
      CPPUNIT_ASSERT(f->fareRetailerRuleLookupIds()[1]._fareRetailerRuleId == 9);
    }
  }

  CPPUNIT_ASSERT(sourcePccList.count("AAA1"));
  CPPUNIT_ASSERT(sourcePccList.count("AAA2"));
  CPPUNIT_ASSERT(sourcePccList.count("BBB2"));
}

void testMatchGeo()
{
  PricingTrx trx;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  FareRetailerRuleValidator frrv(trx);

  FareRetailerRuleInfo frri;
  PaxTypeFare ptf;

  // both locs null
  CPPUNIT_ASSERT(frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));

  // invalid loc type
  frri.loc1().locType() = LOCTYPE_AIRPORT;
  frri.loc1().loc() = "DFW";
  CPPUNIT_ASSERT(!frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));

  frri.loc1().locType() = LOCTYPE_CITY;
  frri.loc2().locType() = LOCTYPE_AIRPORT;
  frri.loc2().loc() = "NYC";
  CPPUNIT_ASSERT(!frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));

  frri.loc2().locType() = LOCTYPE_CITY;

  Fare fare;
  FareInfo fareInfo;
  fareInfo.market1() = "DFW";
  fareInfo.market2() = "NYC";

  // Loc2 null
  frri.loc1().locType() = LOCTYPE_NATION;
  frri.loc1().loc() = "US";
  frri.loc2().locType() = ' ';
  frri.loc2().loc() = "";

  FareMarket fm1;
  fare.initialize(Fare::FS_Domestic, &fareInfo, fm1);

  ptf.setFare(&fare);

  CPPUNIT_ASSERT(frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));

  // Loc1 null
  frri.loc2().locType() = LOCTYPE_NATION;
  frri.loc2().loc() = "US";
  frri.loc1().locType() = ' ';
  frri.loc1().loc() = "";

  CPPUNIT_ASSERT(frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));

  // Both non-NULL
  frri.loc2().locType() = LOCTYPE_NATION;
  frri.loc2().loc() = "US";
  frri.loc1().locType() = LOCTYPE_NATION;
  frri.loc1().loc() = "US";

  CPPUNIT_ASSERT(frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));

  // No match
  frri.loc1().locType() = LOCTYPE_NATION;
  frri.loc1().loc() = "FR";
  frri.loc2().locType() = ' ';
  frri.loc2().loc() = "";

  CPPUNIT_ASSERT(!frrv.matchGeo(frri.loc1(), frri.loc2(), ptf, false));
}

void testValidGeoType()
{
  PricingTrx trx;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  FareRetailerRuleValidator frrv(trx);

  CPPUNIT_ASSERT(frrv.validGeoType(LOCTYPE_AREA));
  CPPUNIT_ASSERT(frrv.validGeoType(LOCTYPE_NATION));
  CPPUNIT_ASSERT(frrv.validGeoType(LOCTYPE_ZONE));
  CPPUNIT_ASSERT(frrv.validGeoType(LOCTYPE_CITY));
  CPPUNIT_ASSERT(frrv.validGeoType(GROUP_LOCATION));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_SUBAREA));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_AIRPORT));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_STATE));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_FMSZONE));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_NONE));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_PCC));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_PCC_ARC));
  CPPUNIT_ASSERT(!frrv.validGeoType(LOCTYPE_USER));
}

void testMatchLocation()
{
  PricingTrx trx;
  PricingRequest request;
  Agent agent;

  request.ticketingAgent() = &agent;
  trx.setRequest(&request);

  FareRetailerRuleValidator frrv(trx);

  // Null location should always return true
  LocKey loc;
  loc.locType() = ' ';
  loc.loc() = "";

  LocCode market = "DFW";

  PaxTypeFare ptf;

  Fare fare;
  FareInfo fareInfo;
  fareInfo.market1() = "DFW";
  fareInfo.market2() = "NYC";
  FareMarket fm1;
  fare.initialize(Fare::FS_Domestic, &fareInfo, fm1);
  ptf.setFare(&fare);

  CPPUNIT_ASSERT(frrv.matchLocation(loc, market, ptf));

  loc.locType() = LOCTYPE_NATION;
  loc.loc() = "US";
  CPPUNIT_ASSERT(frrv.matchLocation(loc, market, ptf));

  loc.loc() = "FR";
  CPPUNIT_ASSERT(!frrv.matchLocation(loc, market, ptf));
}

void testMatchRetailerCode()
{
  PricingTrx trx;
  PricingRequest request;

  trx.setRequest(&request);

  FareRetailerRuleValidator frrv(trx);
  FareRetailerRuleInfo frri;

  request.setPRM(false);

  int count = request.setRCQValues("ABC123,DFE456,MMSZZA,ZDF9876");
  CPPUNIT_ASSERT(count == 4);
  CPPUNIT_ASSERT(request.rcqValues().size() == 4);
  CPPUNIT_ASSERT(request.rcqValues()[0] == "ABC123");
  CPPUNIT_ASSERT(request.rcqValues()[1] == "DFE456");
  CPPUNIT_ASSERT(request.rcqValues()[2] == "MMSZZA");
  CPPUNIT_ASSERT(request.rcqValues()[3] == "ZDF9876");

  CPPUNIT_ASSERT(frrv.matchRetailerCode("ABC123"));
  CPPUNIT_ASSERT(frrv.matchRetailerCode("DFE456"));
  CPPUNIT_ASSERT(frrv.matchRetailerCode("MMSZZA"));
  CPPUNIT_ASSERT(frrv.matchRetailerCode("ZDF9876"));
  CPPUNIT_ASSERT(!frrv.matchRetailerCode("ABC1234"));
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(FareRetailerRuleValidatorTest);

}
