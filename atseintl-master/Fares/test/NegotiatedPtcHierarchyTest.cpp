//-------------------------------------------------------------------
//
//  File:        NegotiatedPtcHierarchyTest.cpp
//  Created:     Jun 26, 2009
//  Authors:     Slawek Machowicz
//
//  Description: Code moved from NegotiatedFareControllerTest class
//
//  Updates:
//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "Fares/NegotiatedPtcHierarchy.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/NegFareRest.h"

namespace tse
{
namespace // it prevents usage these mocks in other files,
{
class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(const PaxTypeCode& paxTypeCode)
  {
    _paxType.paxType() = paxTypeCode;
    _fareClassAppSegInfo._paxType = paxTypeCode;

    paxType() = &_paxType;
    fareClassAppSegInfo() = &_fareClassAppSegInfo;
  }

protected:
  PaxType _paxType;
  FareClassAppSegInfo _fareClassAppSegInfo;
};
}

class NegotiatedPtcHierarchyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegotiatedPtcHierarchyTest);

  CPPUNIT_TEST(testIsNegGroup);
  CPPUNIT_TEST(testIsNegGroupFail);
  CPPUNIT_TEST(testIsJcbGroup);
  CPPUNIT_TEST(testIsJcbGroupFail);
  CPPUNIT_TEST(testIsPfaGroup);
  CPPUNIT_TEST(testIsPfaGroupFail);

  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegCnn);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegCnnFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegCne);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegCneFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegInf);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegInfFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegIne);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegIneFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupNegFailOtherPaxInFare);

  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbJcb);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbJcbFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbJnn);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbJnnFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbJnf);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbJnfFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupJcbFailOtherPaxInFare);

  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupPfaCbc);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupPfaCbcFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupPfaCbi);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupPfaCbiFail);
  CPPUNIT_TEST(testIfPtcsBelongToTheSameGroupPfaFailOtherPaxInFare);

  CPPUNIT_TEST(testFindLowerHierarchy_Fail_NullNegFareRest);
  CPPUNIT_TEST(testFindLowerHierarchy_Fail_MatchLowerHierarchy);
  CPPUNIT_TEST(testFindLowerHierarchy_Pass_MatchLowerHierarchy);

  CPPUNIT_TEST(testMatchLowerHierarchy_NegGroup_Fail);
  CPPUNIT_TEST(testMatchLowerHierarchy_NegGroup_Pass);
  CPPUNIT_TEST(testMatchLowerHierarchy_JcbGroup_Fail);
  CPPUNIT_TEST(testMatchLowerHierarchy_JcbGroup_Pass);
  CPPUNIT_TEST(testMatchLowerHierarchy_PfaGroup_Fail);
  CPPUNIT_TEST(testMatchLowerHierarchy_PfaGroup_Pass);
  CPPUNIT_TEST(testMatchLowerHierarchy_Fail_NoGroup);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  NegotiatedPtcHierarchyTest()
  {
    // it should be called only once
    NegotiatedPtcHierarchy::loadPtcHierarchy();
  }
  ~NegotiatedPtcHierarchyTest() {}

  void testIsNegGroup()
  {
    // Checks that specified types belong to NEG group
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isNegGroup(CHILD));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isNegGroup(CNE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isNegGroup(INFANT));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isNegGroup(INE));
  }

  void testIsNegGroupFail()
  {
    // Checks that other types don't belong to NEG group
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isNegGroup(NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isNegGroup(JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isNegGroup(CBC));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isNegGroup(ADULT));
  }

  void testIsJcbGroup()
  {
    // Checks that specified types belong to JCB group
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isJcbGroup(JCB));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isJcbGroup(JNN));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isJcbGroup(JNF));
  }

  void testIsJcbGroupFail()
  {
    // Checks that other types don't belong to JCB group
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isJcbGroup(NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isJcbGroup(CHILD));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isJcbGroup(INFANT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isJcbGroup(ADULT));
  }

  void testIsPfaGroup()
  {
    // Checks that specified types belong to PFA group
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isPfaGroup(CBC));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::isPfaGroup(CBI));
  }

  void testIsPfaGroupFail()
  {
    // Checks that other types don't belong to PFA group
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isPfaGroup(NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isPfaGroup(CHILD));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isPfaGroup(INFANT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::isPfaGroup(ADULT));
  }

  void testIfPtcsBelongToTheSameGroupNegCnn()
  {
    // Checks that specified types belong to the same group like CNN
    MockPaxTypeFare child(CHILD);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CHILD));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CNE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, INFANT));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, INE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, NEG));
  }

  void testIfPtcsBelongToTheSameGroupNegCnnFail()
  {
    // Checks that other types don't belong to the group, where CNN belongs to
    MockPaxTypeFare child(CHILD);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CBC));
  }

  void testIfPtcsBelongToTheSameGroupNegCne()
  {
    // Checks that specified types belong to the same group like CNE
    MockPaxTypeFare child(CNE);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CHILD));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CNE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, INFANT));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, INE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, NEG));
  }

  void testIfPtcsBelongToTheSameGroupNegCneFail()
  {
    // Checks that other types don't belong to the group, where CNE belongs to
    MockPaxTypeFare child(CNE);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CBC));
  }

  void testIfPtcsBelongToTheSameGroupNegInf()
  {
    // Checks that specified types belong to the same group like INF
    MockPaxTypeFare inf(INFANT);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, CHILD));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, CNE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, INFANT));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, INE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, NEG));
  }

  void testIfPtcsBelongToTheSameGroupNegInfFail()
  {
    // Checks that other types don't belong to the group, where INF belongs to
    MockPaxTypeFare inf(INFANT);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(inf, CBC));
  }

  void testIfPtcsBelongToTheSameGroupNegIne()
  {
    // Checks that specified types belong to the same group like INE
    MockPaxTypeFare child(INE);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CHILD));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CNE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, INFANT));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, INE));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, NEG));
  }

  void testIfPtcsBelongToTheSameGroupNegIneFail()
  {
    // Checks that other types don't belong to the group, where INE belongs to
    MockPaxTypeFare child(INE);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(child, CBC));
  }

  void testIfPtcsBelongToTheSameGroupNegFailOtherPaxInFare()
  {
    // Checks that pax types from NEG group, can't be matched with invalid PTFs
    MockPaxTypeFare adt(ADULT);
    MockPaxTypeFare neg(NEG);
    MockPaxTypeFare jcb(JCB);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(adt, CHILD));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(neg, CHILD));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, CHILD));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(adt, INFANT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(neg, INFANT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, INFANT));
  }

  void testIfPtcsBelongToTheSameGroupJcbJcb()
  {
    // Checks that specified types belong to the same group like JCB
    MockPaxTypeFare jcb(JCB);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, JCB));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, JNN));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, JNF));
  }

  void testIfPtcsBelongToTheSameGroupJcbJcbFail()
  {
    // Checks that other types don't belong to the group, where JCB belongs to
    MockPaxTypeFare jcb(JCB);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jcb, CBC));
  }

  void testIfPtcsBelongToTheSameGroupJcbJnn()
  {
    // Checks that specified types belong to the same group like JNN
    MockPaxTypeFare jnn(JNN);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnn, JCB));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnn, JNN));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnn, JNF));
  }

  void testIfPtcsBelongToTheSameGroupJcbJnnFail()
  {
    // Checks that other types don't belong to the group, where JNN belongs to
    MockPaxTypeFare jnn(JNN);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnn, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnn, NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnn, CBC));
  }

  void testIfPtcsBelongToTheSameGroupJcbJnf()
  {
    // Checks that specified types belong to the same group like JNF
    MockPaxTypeFare jnf(JNF);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnf, JCB));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnf, JNN));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnf, JNF));
  }

  void testIfPtcsBelongToTheSameGroupJcbJnfFail()
  {
    // Checks that other types don't belong to the group, where JNF belongs to
    MockPaxTypeFare jnf(JNF);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnf, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnf, NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(jnf, CBC));
  }

  void testIfPtcsBelongToTheSameGroupJcbFailOtherPaxInFare()
  {
    // Checks that pax types from JCB group, can't be matched with invalid PTFs
    MockPaxTypeFare adt(ADULT);
    MockPaxTypeFare neg(NEG);
    MockPaxTypeFare pfa(PFA);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(adt, JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(neg, JCB));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(pfa, JNN));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(adt, JNN));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(neg, JNF));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(pfa, JNF));
  }

  void testIfPtcsBelongToTheSameGroupPfaCbc()
  {
    // Checks that specified types belong to the same group like CBC
    MockPaxTypeFare cbc(CBC);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbc, PFA));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbc, CBC));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbc, CBI));
  }

  void testIfPtcsBelongToTheSameGroupPfaCbcFail()
  {
    // Checks that other types don't belong to the group, where CBC belongs to
    MockPaxTypeFare cbc(CBC);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbc, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbc, NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbc, JCB));
  }

  void testIfPtcsBelongToTheSameGroupPfaCbi()
  {
    // Checks that specified types belong to the same group like CBI
    MockPaxTypeFare cbi(CBI);

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbi, PFA));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbi, CBC));
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbi, CBI));
  }

  void testIfPtcsBelongToTheSameGroupPfaCbiFail()
  {
    // Checks that other types don't belong to the group, where CBI belongs to
    MockPaxTypeFare cbi(CBI);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbi, ADULT));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbi, NEG));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(cbi, JCB));
  }

  void testIfPtcsBelongToTheSameGroupPfaFailOtherPaxInFare()
  {
    // Checks that pax types from PFA group, can't be matched with invalid PTFs
    MockPaxTypeFare adt(ADULT);
    MockPaxTypeFare neg(NEG);
    MockPaxTypeFare pfa(PFA);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(adt, CBC));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(neg, CBC));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(pfa, CBC));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(adt, CBI));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(neg, CBI));
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(pfa, CBI));
  }

  void testFindLowerHierarchy_Fail_NullNegFareRest()
  {
    NegPaxTypeFareRuleData ruleData;
    NegFareRest negFareRest;
    PaxTypeFare ptFare;

    // neg fare rest is null
    ruleData.ruleItemInfo() = NULL;

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::findLowerHierarchy(ruleData, negFareRest, ptFare));
  }

  void testFindLowerHierarchy_Fail_MatchLowerHierarchy()
  {
    NegPaxTypeFareRuleData ruleData;
    NegFareRest negFareRest;

    // neg fare rest is not null
    ruleData.ruleItemInfo() = &negFareRest;
    // but ptf is not Cat35 type
    MockPaxTypeFare ptFare(ADULT);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::findLowerHierarchy(ruleData, negFareRest, ptFare));
  }

  void testFindLowerHierarchy_Pass_MatchLowerHierarchy()
  {
    NegPaxTypeFareRuleData ruleData;
    NegFareRest negFareRest1, negFareRest2;

    // neg fare rest is not null
    ruleData.ruleItemInfo() = &negFareRest1;

    // and matchLowerHierarchy returns true
    MockPaxTypeFare ptFare(INFANT);
    negFareRest1.psgType() = NEG;
    negFareRest2.psgType() = INFANT;

    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::findLowerHierarchy(ruleData, negFareRest2, ptFare));
  }

  void testMatchLowerHierarchy_NegGroup_Fail()
  {
    // ptf is NEG type
    MockPaxTypeFare ptFare(INFANT);
    // and both specified types also
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, INFANT, NEG));
  }

  void testMatchLowerHierarchy_NegGroup_Pass()
  {
    // ptf is NEG type
    MockPaxTypeFare ptFare(INFANT);
    // and both specified types also
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, NEG, INFANT));
  }

  void testMatchLowerHierarchy_JcbGroup_Fail()
  {
    // ptf is Jcb type
    MockPaxTypeFare ptFare(JCB);
    // and both specified types also
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, JNN, JCB));
  }

  void testMatchLowerHierarchy_JcbGroup_Pass()
  {
    // ptf is Jcb type
    MockPaxTypeFare ptFare(JCB);
    // and both specified types also
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, JCB, JNN));
  }

  void testMatchLowerHierarchy_PfaGroup_Fail()
  {
    // ptf is Pfa type
    MockPaxTypeFare ptFare(CBC);
    // and both specified types also
    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, CBI, CBC));
  }

  void testMatchLowerHierarchy_PfaGroup_Pass()
  {
    // ptf is Pfa type
    MockPaxTypeFare ptFare(CBC);
    // and both specified types also
    CPPUNIT_ASSERT(NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, CBC, CBI));
  }

  void testMatchLowerHierarchy_Fail_NoGroup()
  {
    // ptf is not cat35 type
    MockPaxTypeFare ptFare(ADULT);

    CPPUNIT_ASSERT(!NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, NEG, INFANT));
  }

}; // class

CPPUNIT_TEST_SUITE_REGISTRATION(NegotiatedPtcHierarchyTest);
} // tse
