//-------------------------------------------------------------------
//
//  File:        TagWarEngineTest.cpp
//  Created:     August 22, 2007
//  Authors:     Daniel Rolka
//
//  Updates:
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "RexPricing/PermutationGenerator.h"
#include "RexPricing/test/TagWarEngineTest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"

using namespace tse;

void
TagWarEngineTest::testMarkWithFareType_Tag1Tag1()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_THE_FARES);
  AddProcessTag(perm, KEEP_THE_FARES);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag2Tag2()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, GUARANTEED_AIR_FARE);
  AddProcessTag(perm, GUARANTEED_AIR_FARE);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == HISTORICAL);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == HISTORICAL);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == HISTORICAL);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == HISTORICAL);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag1Tag2()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_THE_FARES);
  AddProcessTag(perm, GUARANTEED_AIR_FARE);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag1Tag3()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_THE_FARES);
  AddProcessTag(perm, KEEP_FARES_FOR_TRAVELED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag2Tag3()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, GUARANTEED_AIR_FARE);
  AddProcessTag(perm, KEEP_FARES_FOR_TRAVELED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag3Tag3()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_FARES_FOR_TRAVELED_FC);
  AddProcessTag(perm, KEEP_FARES_FOR_TRAVELED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag1Tag4()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_THE_FARES);
  AddProcessTag(perm, KEEP_FARES_FOR_UNCHANGED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag2Tag4()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, GUARANTEED_AIR_FARE);
  AddProcessTag(perm, KEEP_FARES_FOR_UNCHANGED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag3Tag4()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_FARES_FOR_UNCHANGED_FC);
  AddProcessTag(perm, KEEP_FARES_FOR_TRAVELED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag4Tag4()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_FARES_FOR_UNCHANGED_FC);
  AddProcessTag(perm, KEEP_FARES_FOR_UNCHANGED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag5Tag5()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, NO_GUARANTEED_FARES);
  AddProcessTag(perm, NO_GUARANTEED_FARES);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == CURRENT);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag5Tag6()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, NO_GUARANTEED_FARES);
  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == CURRENT);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag6Tag6_TravelCommenced()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);
  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, true) == TRAVEL_COMMENCEMENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, true) == TRAVEL_COMMENCEMENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, true) == TRAVEL_COMMENCEMENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, true) == TRAVEL_COMMENCEMENT);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag6Tag6_TravelNotCommenced()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);
  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == CURRENT);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag7Tag7()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, REISSUE_DOWN_TO_LOWER_FARE);
  AddProcessTag(perm, REISSUE_DOWN_TO_LOWER_FARE);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == CURRENT);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag9Tag9()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, HISTORICAL_FARES_FOR_TRAVELED_FC);
  AddProcessTag(perm, HISTORICAL_FARES_FOR_TRAVELED_FC);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == HISTORICAL);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag9Tag10()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, HISTORICAL_FARES_FOR_TRAVELED_FC);
  AddProcessTag(perm, KEEP_FOR_UNCH_CURRENT_FOR_CHNG);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag10Tag10()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_FOR_UNCH_CURRENT_FOR_CHNG);
  AddProcessTag(perm, KEEP_FOR_UNCH_CURRENT_FOR_CHNG);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag10Tag11()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_FOR_UNCH_CURRENT_FOR_CHNG);
  AddProcessTag(perm, KEEP_UP_TO_FIRST_CHNG_THEN_HIST);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testMarkWithFareType_Tag11Tag11()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_UP_TO_FIRST_CHNG_THEN_HIST);
  AddProcessTag(perm, KEEP_UP_TO_FIRST_CHNG_THEN_HIST);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == KEEP);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == HISTORICAL);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == HISTORICAL);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == KEEP);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testTagsNotSet()
{
  ProcessTagPermutation perm;

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == UNKNOWN_FA);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == UNKNOWN_FA);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == UNKNOWN_FA);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == UNKNOWN_FA);
}

void
TagWarEngineTest::testMarkWithFareType_Tag1Tag2Tag3Tag4Tag5Tag6Tag9Tag10Tag11()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm, KEEP_THE_FARES);
  AddProcessTag(perm, GUARANTEED_AIR_FARE);
  AddProcessTag(perm, KEEP_FARES_FOR_TRAVELED_FC);
  AddProcessTag(perm, KEEP_FARES_FOR_UNCHANGED_FC);
  AddProcessTag(perm, NO_GUARANTEED_FARES);
  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);
  AddProcessTag(perm, HISTORICAL_FARES_FOR_TRAVELED_FC);
  AddProcessTag(perm, KEEP_FOR_UNCH_CURRENT_FOR_CHNG);
  AddProcessTag(perm, KEEP_UP_TO_FIRST_CHNG_THEN_HIST);

  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UU, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UC, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, UN, false) == CURRENT);
  CPPUNIT_ASSERT(TagWarEngine::getFA(perm, FL, false) == CURRENT);

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testZeroT988First()
{
  ProcessTagPermutation perm;
  AddProcessTag(perm);
  AddProcessTag(perm, GUARANTEED_AIR_FARE);

  CPPUNIT_ASSERT_EQUAL(HISTORICAL, TagWarEngine::getFA(perm, UU, false));

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testZeroT988Between()
{
  ProcessTagPermutation perm;

  AddProcessTag(perm, NO_GUARANTEED_FARES);
  AddProcessTag(perm);
  AddProcessTag(perm, NO_GUARANTEED_FARES);

  CPPUNIT_ASSERT_EQUAL(CURRENT, TagWarEngine::getFA(perm, UU, false));

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::testZeroT988Last()
{
  ProcessTagPermutation perm;

  AddProcessTag(perm, TRAVEL_COMENCEMENT_AIR_FARES);
  AddProcessTag(perm);

  CPPUNIT_ASSERT_EQUAL(CURRENT, TagWarEngine::getFA(perm, UU, false));

  ClearProcessTagPermutationData(perm);
}

void
TagWarEngineTest::AddProcessTag(ProcessTagPermutation& perm, int pt)
{
  ProcessTagInfo* pInfo = new ProcessTagInfo();
  if (pt)
  {
    ReissueSequence* seq = new ReissueSequence();
    pInfo->reissueSequence()->orig() = seq;
    seq->processingInd() = pt;
  }
  perm.processTags().push_back(pInfo);
}

void
TagWarEngineTest::ClearProcessTagPermutationData(ProcessTagPermutation& perm)
{
  const std::vector<ProcessTagInfo*>& ptV = perm.processTags();
  std::vector<ProcessTagInfo*>::const_iterator i;

  for (i = ptV.begin(); i != ptV.end(); i++)
  {
    delete (*i)->reissueSequence()->orig();
    delete *i;
  }
}
/////////////////////////////////////////////

void
TagWarEngineTest::setUp()
{
  CppUnit::TestFixture::setUp();
}

void
TagWarEngineTest::tearDown()
{
  ;
}
