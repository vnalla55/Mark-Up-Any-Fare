//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/Record2Types.h"
#include "DBAccess/test/SerializationTestCombinabilityRuleInfo.h"

namespace tse
{

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestCombinabilityRuleInfo);

void
SerializationTestCombinabilityRuleInfo::testDummyData(const CombinabilityRuleInfo& obj)
{
  CPPUNIT_ASSERT(obj.createDate() == DateTime(2010, 04, 01, 13, 45, 30, 10));
  CPPUNIT_ASSERT(obj.vendorCode() == "ABCD");
  CPPUNIT_ASSERT(obj.tariffNumber() == 1);
  CPPUNIT_ASSERT(obj.carrierCode() == "EFG");
  CPPUNIT_ASSERT(obj.ruleNumber() == "HIJK");
  CPPUNIT_ASSERT(obj.categoryNumber() == 2);
  CPPUNIT_ASSERT(obj.sequenceNumber() == 3);
  CPPUNIT_ASSERT(obj.loc1().loc() == "LMNOPQRS");
  CPPUNIT_ASSERT(obj.loc1().locType() == 'T');
  CPPUNIT_ASSERT(obj.loc2().loc() == "UVWXYZab");
  CPPUNIT_ASSERT(obj.loc2().locType() == 'c');
  CPPUNIT_ASSERT(obj.applInd() == 'd');
  CPPUNIT_ASSERT(obj.hasCatStopovers() == false);
  CPPUNIT_ASSERT(obj.hasCatTransfers() == true);

  CPPUNIT_ASSERT(obj.expireDate() == DateTime(2010, 04, 01, 13, 45, 30, 10));
  CPPUNIT_ASSERT(obj.createDate() == DateTime(2010, 04, 01, 13, 45, 30, 10));
  CPPUNIT_ASSERT(obj.effDate() == DateTime(2010, 04, 01, 13, 45, 30, 10));
  CPPUNIT_ASSERT(obj.discDate() == DateTime(2010, 04, 01, 13, 45, 30, 10));
  CPPUNIT_ASSERT(obj.segCnt() == 1);
  CPPUNIT_ASSERT(obj.jointCarrierTblItemNo() == 2);
  CPPUNIT_ASSERT(obj.samepointstblItemNo() == 3);
  CPPUNIT_ASSERT(obj.dojGeneralRuleTariff() == 4);
  CPPUNIT_ASSERT(obj.ct2GeneralRuleTariff() == 5);
  CPPUNIT_ASSERT(obj.ct2plusGeneralRuleTariff() == 6);
  CPPUNIT_ASSERT(obj.eoeGeneralRuleTariff() == 7);
  CPPUNIT_ASSERT(obj.arbGeneralRuleTariff() == 8);
  CPPUNIT_ASSERT(obj.validityInd() == 'A');
  CPPUNIT_ASSERT(obj.inhibit() == 'B');

  CPPUNIT_ASSERT(obj.locKey1().loc() == "ABCDEFGH");
  CPPUNIT_ASSERT(obj.locKey1().locType() == 'I');
  CPPUNIT_ASSERT(obj.locKey2().loc() == "ABCDEFGH");
  CPPUNIT_ASSERT(obj.locKey2().locType() == 'I');

  CPPUNIT_ASSERT(obj.fareClass() == "aaaaaaaa");
  CPPUNIT_ASSERT(obj.owrt() == 'C');
  CPPUNIT_ASSERT(obj.routingAppl() == 'D');
  CPPUNIT_ASSERT(obj.routing() == "EFGH");
  CPPUNIT_ASSERT(obj.footNote1() == "IJ");
  CPPUNIT_ASSERT(obj.footNote2() == "KL");
  CPPUNIT_ASSERT(obj.fareType() == "MNOPQRST");
  CPPUNIT_ASSERT(obj.seasonType() == 'U');
  CPPUNIT_ASSERT(obj.dowType() == 'V');
  CPPUNIT_ASSERT(obj.batchci() == "bbbbbbbb");
  CPPUNIT_ASSERT(obj.sojInd() == 'W');
  CPPUNIT_ASSERT(obj.sojorigIndestInd() == 'X');
  CPPUNIT_ASSERT(obj.dojInd() == 'Y');
  CPPUNIT_ASSERT(obj.dojCarrierRestInd() == 'Z');
  CPPUNIT_ASSERT(obj.dojTariffRuleRestInd() == 'a');
  CPPUNIT_ASSERT(obj.dojFareClassTypeRestInd() == 'b');
  CPPUNIT_ASSERT(obj.dojGeneralRule() == "cdef");
  CPPUNIT_ASSERT(obj.dojGeneralRuleAppl() == 'g');
  CPPUNIT_ASSERT(obj.ct2Ind() == 'h');
  CPPUNIT_ASSERT(obj.ct2CarrierRestInd() == 'i');
  CPPUNIT_ASSERT(obj.ct2TariffRuleRestInd() == 'j');
  CPPUNIT_ASSERT(obj.ct2FareClassTypeRestInd() == 'k');
  CPPUNIT_ASSERT(obj.ct2GeneralRule() == "lmno");
  CPPUNIT_ASSERT(obj.ct2GeneralRuleAppl() == 'p');
  CPPUNIT_ASSERT(obj.ct2plusInd() == 'q');
  CPPUNIT_ASSERT(obj.ct2plusCarrierRestInd() == 'r');
  CPPUNIT_ASSERT(obj.ct2plusTariffRuleRestInd() == 's');
  CPPUNIT_ASSERT(obj.ct2plusFareClassTypeRestInd() == 't');
  CPPUNIT_ASSERT(obj.ct2plusGeneralRule() == "uvwx");
  CPPUNIT_ASSERT(obj.ct2plusGeneralRuleAppl() == 'y');
  CPPUNIT_ASSERT(obj.eoeInd() == 'z');
  CPPUNIT_ASSERT(obj.eoeCarrierRestInd() == '1');
  CPPUNIT_ASSERT(obj.eoeTariffRuleRestInd() == '2');
  CPPUNIT_ASSERT(obj.eoeFareClassTypeRestInd() == '3');
  CPPUNIT_ASSERT(obj.eoeGeneralRule() == "4567");
  CPPUNIT_ASSERT(obj.eoeGeneralRuleAppl() == '8');
  CPPUNIT_ASSERT(obj.arbInd() == '9');
  CPPUNIT_ASSERT(obj.arbCarrierRestInd() == '0');
  CPPUNIT_ASSERT(obj.arbTariffRuleRestInd() == 'A');
  CPPUNIT_ASSERT(obj.arbFareClassTypeRestInd() == 'B');
  CPPUNIT_ASSERT(obj.arbGeneralRule() == "CDEF");
  CPPUNIT_ASSERT(obj.arbGeneralRuleAppl() == 'G');
  CPPUNIT_ASSERT(obj.versioninheritedInd() == 'H');
  CPPUNIT_ASSERT(obj.versionDisplayInd() == 'I');
  CPPUNIT_ASSERT(obj.dojSameCarrierInd() == 'J');
  CPPUNIT_ASSERT(obj.dojSameRuleTariffInd() == 'K');
  CPPUNIT_ASSERT(obj.dojSameFareInd() == 'L');
  CPPUNIT_ASSERT(obj.ct2SameCarrierInd() == 'M');
  CPPUNIT_ASSERT(obj.ct2SameRuleTariffInd() == 'N');
  CPPUNIT_ASSERT(obj.ct2SameFareInd() == 'O');
  CPPUNIT_ASSERT(obj.ct2plusSameCarrierInd() == 'P');
  CPPUNIT_ASSERT(obj.ct2plusSameRuleTariffInd() == 'Q');
  CPPUNIT_ASSERT(obj.ct2plusSameFareInd() == 'R');
  CPPUNIT_ASSERT(obj.eoeSameCarrierInd() == 'S');
  CPPUNIT_ASSERT(obj.eoeSameRuleTariffInd() == 'T');
  CPPUNIT_ASSERT(obj.eoeSameFareInd() == 'U');
  CPPUNIT_ASSERT(obj.categoryRuleItemInfoSet().size() == 2);

  for (const auto* infoset: obj.categoryRuleItemInfoSet())
  {
    CPPUNIT_ASSERT(infoset->size() == 2);
    for (const auto& item: *infoset)
    {
      CPPUNIT_ASSERT(item.itemcat() == 101);
      CPPUNIT_ASSERT(item.orderNo() == 102);
      CPPUNIT_ASSERT(item.relationalInd() == CategoryRuleItemInfo::AND);
      CPPUNIT_ASSERT(item.inOutInd() == 'A');
      CPPUNIT_ASSERT(item.directionality() == 'B');
      CPPUNIT_ASSERT(item.itemNo() == 104);
      CPPUNIT_ASSERT(item.textonlyInd() == 'Z');
      CPPUNIT_ASSERT(item.eoervalueInd() == 'Y');
      CPPUNIT_ASSERT(item.eoeallsegInd() == 'X');
      CPPUNIT_ASSERT(item.sameCarrierInd() == 'W');
      CPPUNIT_ASSERT(item.sameRuleTariffInd() == 'V');
      CPPUNIT_ASSERT(item.sameFareInd() == 'U');
    }
  }
}

void
SerializationTestCombinabilityRuleInfo::testDummyDataValues()
{
  Flattenizable::Archive archive;
  CombinabilityRuleInfo reassembled;
  CombinabilityRuleInfo obj;

  archive.setTrace(FLATTENIZE_TRACE);
  CombinabilityRuleInfo::dummyData(obj);

  // std::cout << "Checking for correct population of dummy data..." << std::endl ;
  testDummyData(obj);

  // std::cout << "Saving to archive..." << std::endl ;
  std::string ignore;
  FLATTENIZE_SAVE(archive, obj, 0, ignore, ignore);

  // std::cout << "Restoring from archive..." << std::endl ;
  FLATTENIZE_RESTORE(archive, reassembled, NULL, 0);

  // std::cout << "Checking that dummy data was restored properly..." << std::endl ;
  testDummyData(reassembled);
}
} // tse
