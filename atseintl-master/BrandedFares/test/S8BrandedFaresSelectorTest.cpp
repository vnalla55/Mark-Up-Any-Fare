//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelectorTest.cpp
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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


#include "test/include/CppUnitHelperMacros.h"

#include "BrandedFares/test/S8BrandedFaresSelectorInitializer.h"
#include "BrandedFares/BrandedFaresSelector.h"

using namespace std;
namespace tse
{
class TestDiagnosticInitializer
{
public:
  TestDiagnosticInitializer(PricingTrx&, Diag889Collector*&)
  {
  }
};

class BrandedFaresSelectorTest : public CppUnit::TestFixture, S8BrandedFaresSelectorInitializer<BrandedFaresSelector>
{
  CPPUNIT_TEST_SUITE(BrandedFaresSelectorTest);
  CPPUNIT_TEST(testValidatePass);
  CPPUNIT_TEST(testValidatePass_FareDisplayInfoPopulated);
  CPPUNIT_TEST(testValidatePass_SecondaryT189);
  CPPUNIT_TEST(testPrintNoFaresFound);
  CPPUNIT_TEST(testPrintDataNotFound);
  CPPUNIT_TEST(testFareMarketPrintDataNotFound);
  CPPUNIT_TEST(testPrintFareMarketNotMatched);
  CPPUNIT_TEST(testPrintT189NotExist);
  CPPUNIT_TEST(testPrintBrandProgramNotFound);
  CPPUNIT_TEST(testFareMarketPrintBrandProgramNotFound);
  CPPUNIT_TEST(testPrintBrandNotFound);
  CPPUNIT_TEST(testPrintT189NotFound);
  CPPUNIT_TEST(testPrintT189Banner);
  CPPUNIT_TEST(testPrintSeparator);
  CPPUNIT_TEST(testPrintTravelInfo);
  CPPUNIT_TEST(testPrintSeqNoAndStatus);
  CPPUNIT_TEST(testDisplayT189Secondary);
  CPPUNIT_TEST(testPrintDetailInfo);
  CPPUNIT_TEST(testPrintDetailBrandProgram);
  CPPUNIT_TEST(testPrintBrandProgram);
  CPPUNIT_TEST(testPrintBrandT189Item);
  CPPUNIT_TEST(testPrintBrand);
  CPPUNIT_TEST(testMatchFareMarketForDiagParam);
  CPPUNIT_TEST(testPrintCompleteBrandProgram);
  CPPUNIT_TEST(testPrintCompleteBrand);
  CPPUNIT_TEST(testPrintT189SecondaryDetail);
  CPPUNIT_TEST(testPrintT189Secondary);
  CPPUNIT_TEST(testPrintFareNotValid);
  CPPUNIT_TEST(testIsT189exist);
  CPPUNIT_TEST(testPrintCarrierNotMatched);
  CPPUNIT_TEST(testFareQuoteFareMarketNotMatched);
  CPPUNIT_TEST(testFareQuoteT189NotExist);
  CPPUNIT_TEST(testFareQuoteFareClassNotMatched);
  CPPUNIT_TEST(testFareQuoteCarrierNotMatched);
  CPPUNIT_TEST(testFareQuoteBrandedMarketMapEmpty);
  CPPUNIT_TEST(testProcessPaxTypeFare);
  CPPUNIT_TEST(testCreateBrandProgramMap);
  CPPUNIT_TEST(testPrintBrandSizeAndCurrentBrand);
  CPPUNIT_TEST(testPrintCurrentBrand);
  CPPUNIT_TEST(testMatchGlobalDirection_ProgramHasNoDir_Pass);
  CPPUNIT_TEST(testMatchGlobalDirection_ProgramHasDirZZ_Pass);
  CPPUNIT_TEST(testMatchGlobalDirection_ProgramDir_NotPublished_Fail);
  CPPUNIT_TEST(testMatchGlobalDirection_ProgramDirAT_PtfDirUS_Fail);
  CPPUNIT_TEST(testMatchGlobalDirection_ProgramDirAT_PtfDirZZ_Pass);
  CPPUNIT_TEST(testMatchGlobalDirection_ProgramDirAT_PtfDirAT_Pass);
  CPPUNIT_TEST(testPrintProgramFailGlobalDirection);
  CPPUNIT_TEST(testMatchDirectionality_Fare_BOTH_ProgramOriginLOC_Blank_Pass);
  CPPUNIT_TEST(testMatchDirectionality_Fare_BOTH_ProgramOriginLOC_LOC1_Pass);
  CPPUNIT_TEST(testMatchDirectionality_Fare_BOTH_ProgramOriginLOC_LOC2_Pass);
  CPPUNIT_TEST(testMatchDirectionality_Fare_FROM_ProgramOriginLOC_Blank_Pass);
  CPPUNIT_TEST(testMatchDirectionality_Fare_FROM_ProgramOriginLOC_LOC1_Pass);
  CPPUNIT_TEST(testMatchDirectionality_Fare_FROM_ProgramOriginLOC_LOC2_Fail);
  CPPUNIT_TEST(testMatchDirectionality_Fare_TO_Program_OriginLOC_Blank_Pass);
  CPPUNIT_TEST(testMatchDirectionality_Fare_TO_Program_OriginLOC_LOC1_Fail);
  CPPUNIT_TEST(testMatchDirectionality_Fare_TO_Program_OriginLOC_LOC2_Pass);
  CPPUNIT_TEST(testMatchDirectionalityShopping);
  CPPUNIT_TEST(testPrintProgramFailDirectionality);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    init();
  }

  void tearDown() { _memHandle.clear(); }

  void createDiagnostic(bool isDDInfo)
  {
   _s8BrandedFaresSelector->_diag889 =
       _memHandle.insert(new Diag889Collector(*_memHandle.create<Diagnostic>()));
   _s8BrandedFaresSelector->_diag889->activate();

   if (isDDInfo)
     _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DISPLAY_DETAIL, "INFO"));
  }

  void testValidatePass()
  {
    _fdS8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(!_ptf1->getBrandStatusVec().empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ptf1->getBrandStatusVec().size());
  }

  void testValidatePass_FareDisplayInfoPopulated()
  {
    FareDisplayInfo fdInfo1;
    _ptf1->fareDisplayInfo() = &fdInfo1;
    _fdS8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(!_ptf1->getBrandStatusVec().empty());

    std::pair<ProgramCode, BrandCode> newS8Brand = _ptf1->fareDisplayInfo()->programBrand();

    CPPUNIT_ASSERT_EQUAL(ProgramCode("us"), newS8Brand.first);
    CPPUNIT_ASSERT_EQUAL(BrandCode("app"), newS8Brand.second);
  }

  void testValidatePass_SecondaryT189()
  {
    _svcFeesFareIdInfo->bookingCode1() = "FN";
    _brand1->fareIDdataSecondaryT189() = *_secSvcFeesFareIdInfoVector;
    _fdS8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(!_ptf1->getBrandStatusVec().empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ptf1->getBrandStatusVec().size());
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::BS_SOFT_PASS, _ptf1->getBrandStatusVec().front().first);
  }


  void testPrintNoFaresFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printNoFaresFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("NO FARES FOUND FOR MARKET : SFO-DFW\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintDataNotFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printDataNotFound();
    CPPUNIT_ASSERT_EQUAL(string("BRAND DATA NOT FOUND\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testFareMarketPrintDataNotFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printDataNotFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW BRAND DATA NOT FOUND\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintFareMarketNotMatched()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printFareMarketNotMatched(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW NOT MATCHED\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintT189NotExist()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printT189NotExist(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW NO FAREID T189 DATA EXIST\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintBrandProgramNotFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printBrandProgramNotFound();
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM : NOT FOUND\n"), _s8BrandedFaresSelector->_diag889->str());
  }

  void testFareMarketPrintBrandProgramNotFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printBrandProgramNotFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW PROGRAM : NOT FOUND\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintBrandNotFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printBrandNotFound();
    CPPUNIT_ASSERT_EQUAL(string("BRAND   : NOT FOUND P T189 ITEMNO : NOT FOUND\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintT189NotFound()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printT189NotFound(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("BRAND   : APP  P T189 ITEMNO : NOT FOUND\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintT189Banner()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printT189Banner();
    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES T189 ANALYSIS ***************\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintSeparator()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printSeparator();
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintTravelInfo()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printTravelInfo(_fm1);
    CPPUNIT_ASSERT_EQUAL(string("PORTION OF TRAVEL  :  -    GOV CARRIER  : AA \n"
                                "***********************************************************\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintSeqNoAndStatus()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printSeqNoAndStatus(_svcFeesFareIdInfo, PASS_T189);
    CPPUNIT_ASSERT_EQUAL(string("P   8888        199      "),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testDisplayT189Secondary()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.displayT189Secondary(_svcFeesFareIdInfo);
    CPPUNIT_ASSERT_EQUAL(string("S T189 ITEM NO :    8888 SEQ NO  :     199\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintDetailInfo()
  {
    createDiagnostic(true);
    StatusT189 status = PASS_T189;
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printDetailInfo(_svcFeesFareIdInfo, status);
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"
                                " VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\nFARE INFORMATION\n"
                                " APPL  : \n"
                                " OW/RT : 1 - ONE WAY MAY BE DOUBLED\n"
                                " RULETARIFF IND : PUB\n"
                                " RULE TARIFF : -1    RULE NUMBER : \n"
                                " FARE CLASS  : GOGO    \n"
                                " FARE TYPE : BUR \n"
                                " PAX TYPE : \n"
                                " ROUTING : 99999\n"
                                " PRIME RBD1 : \n"
                                " PRIME RBD2 : A \n"
                                "\n FARE RANGE AMOUNT\n"
                                " MIN 1 :      50.00      MIN 2 :     20.000AUD\n"
                                " MAX 1 :     255.00      MAX 2 :    777.000AUD\n\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintDetailBrandProgram()
  {
    createDiagnostic(true);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printSeqNoAndStatus(_svcFeesFareIdInfo, PASS_T189);
    CPPUNIT_ASSERT_EQUAL(string("P   8888        199      "),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintBrandProgram()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printBrandProgram(_bProgram1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM : US         \n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintBrandT189Item()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printBrandT189Item(_svcFeesFareIdInfo, _brand1);
    CPPUNIT_ASSERT_EQUAL(string("BRAND   : APP        \n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintBrand()
  {
    createDiagnostic(true);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printBrand(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"
                                "BRAND CODE : APP\n"
                                "BRAND NAME : APPLE                           TIER : 99     \n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testMatchFareMarketForDiagParam()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "LAXJFK"));
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchFareMarketForDiagParam(*_fm1) == false);
  }

  void testPrintCompleteBrandProgram()
  {
    createDiagnostic(true);BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printCompleteBrandProgram(_bProgram1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM CODE : US         \n"
                                "PROGRAM NAME : DOMESTIC US                    \n"
                                "PROGRAM DESC : \n"
                                "VENDOR : \n"
                                "PSG TYPE : ADT \n"
                                "DIRECTION : \n"
                                "GLOBAL DIRECTION : \n"
                                "ORIGINLOC : \n"
                                "SEQ NO  : \n"
                                "EFFECTIVE DATE : N/A\n"
                                "DISCONTINUE DATE : N/A\n"
                                "ACCOUNT CODE : \n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintCompleteBrand()
  {
    createDiagnostic(true);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printCompleteBrand(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"
                                "BRAND CODE : APP\n"
                                "BRAND NAME : APPLE                          \n"
                                "TIER NUMBER : 99     \n"
                                "PRIMARYFAREIDTABLE : \n"
                                "SECONDARYFAREIDTABLE : \n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintT189SecondaryDetail()
  {
    createDiagnostic(true);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printT189Secondary(_svcFeesFareIdInfo, true);
    CPPUNIT_ASSERT_EQUAL(string(" VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\nFARE INFORMATION\n"
                                " PRIME RBD1 : \n"
                                " PRIME RBD2 : A \n"
                                " STATUS : SOFTPASS SECOND RBD\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintT189Secondary()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printT189Secondary(_svcFeesFareIdInfo, true);
    CPPUNIT_ASSERT_EQUAL(string("S   8888        199      SOFTPASS SECOND RBD\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintFareNotValid()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printFareNotValid();
    CPPUNIT_ASSERT_EQUAL(string("FARE NOT VALID FOR PRICING - SEE DIAG 499/DDALLFARES\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testIsT189exist()
  {
    int marketID = 1;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->isT189exist(marketID) == true);
  }

  void testPrintCarrierNotMatched()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printCarrierNotMatched(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW CARRIER NOT MATCHED\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testFareQuoteFareMarketNotMatched()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "LAXJFK"));
    _s8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(_ptf1->getBrandStatusVec().empty());
  }

  void testFareQuoteT189NotExist()
  {
    _fm1->marketIDVec().push_back(2);
    _s8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(_ptf1->getBrandStatusVec().empty());
  }

  void testFareQuoteFareClassNotMatched()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "ABCD"));
    _s8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(_ptf1->getBrandStatusVec().empty());
  }

  void testFareQuoteCarrierNotMatched()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("CX", "DL"));
    _s8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(_ptf1->getBrandStatusVec().empty());
  }

  void testFareQuoteBrandedMarketMapEmpty()
  {
    _trx->brandedMarketMap().clear();
    _s8BrandedFaresSelector->validate(_paxTypeFare);
    CPPUNIT_ASSERT(_ptf1->getBrandStatusVec().empty());
  }

  void testProcessPaxTypeFare()
  {
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->processPaxTypeFare(_ptf1, diagnostics));
  }

  void testCreateBrandProgramMap()
  {
    std::map<BrandProgram*, std::vector<BrandInfo*>, BrandProgramComparator> brandProgramMap;
    _s8BrandedFaresSelector->createBrandProgramMap(*_fm1, brandProgramMap);
    CPPUNIT_ASSERT(!brandProgramMap.empty());
    CPPUNIT_ASSERT_EQUAL(2, (int)brandProgramMap.size());
  }

  void testPrintBrandSizeAndCurrentBrand()
  {
    createDiagnostic(true);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printCurrentBrand(_brand1, 2);
    CPPUNIT_ASSERT_EQUAL(string("NUMBER OF BRANDS : 2 CURRENT BRAND CODE : APP        \n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testPrintCurrentBrand()
  {
    createDiagnostic(false);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printCurrentBrand(_brand1, 1);
    CPPUNIT_ASSERT_EQUAL(string("CURRENT BRAND CODE : APP         "),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testMatchGlobalDirection_ProgramHasNoDir_Pass()
  {
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchGlobalDirection(*_bProgram1, *_ptf1));
  }

  void testMatchGlobalDirection_ProgramHasDirZZ_Pass()
  {
    _bProgram1->globalDirection() = GlobalDirection::ZZ;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchGlobalDirection(*_bProgram1, *_ptf1));
  }

  void testMatchGlobalDirection_ProgramDir_NotPublished_Fail()
  {
    _bProgram1->globalDirection() = static_cast<GlobalDirection>(55);

    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchGlobalDirection(*_bProgram1, *_ptf1));
  }

  void testMatchGlobalDirection_ProgramDirAT_PtfDirUS_Fail()
  {
    _bProgram1->globalDirection() = GlobalDirection::AT;

    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchGlobalDirection(*_bProgram1, *_ptf1));
  }

  void testMatchGlobalDirection_ProgramDirAT_PtfDirZZ_Pass()
  {
    _bProgram1->globalDirection() = GlobalDirection::AT;

    FareInfo fareInfo;
    fareInfo.globalDirection() = GlobalDirection::ZZ;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);

    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchGlobalDirection(*_bProgram1, *_ptf1));
  }

  void testMatchGlobalDirection_ProgramDirAT_PtfDirAT_Pass()
  {
    _bProgram1->globalDirection() = GlobalDirection::AT;

    FareInfo fareInfo;
    fareInfo.globalDirection() = GlobalDirection::AT;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);

    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchGlobalDirection(*_bProgram1, *_ptf1));
  }

  void testPrintProgramFailGlobalDirection()
  {
    createDiagnostic(true);
    _bProgram1->globalDirection() = GlobalDirection::AT;
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printProgramFailGlobalDirection(*_bProgram1, *_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM GLOBAL DIR : AT  DOES NOT MATCH FARE GLOBAL DIR : US\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

  void testMatchDirectionality_Fare_BOTH_ProgramOriginLOC_Blank_Pass()
  {
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_BOTH_ProgramOriginLOC_LOC1_Pass()
  {
    _bProgram1->originLoc() = _fm1->origin()->loc(); //"SFO";
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_BOTH_ProgramOriginLOC_LOC2_Pass()
  {
    _bProgram1->originLoc() = _fm1->destination()->loc(); //"DFW";
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_FROM_ProgramOriginLOC_Blank_Pass()
  {
    _bProgram1->originLoc() = "";
    FareInfo fareInfo;
    fareInfo.directionality() = FROM;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_FROM_ProgramOriginLOC_LOC1_Pass()
  {
    _bProgram1->originLoc() = _fm1->origin()->loc(); //"SFO";

    FareInfo fareInfo;
    fareInfo.directionality() = FROM;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_FROM_ProgramOriginLOC_LOC2_Fail()
  {
    _bProgram1->originLoc() = _fm1->destination()->loc(); //"DFW";

    FareInfo fareInfo;
    fareInfo.directionality() = FROM;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    Direction direction;
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_TO_Program_OriginLOC_Blank_Pass()
  {
    _bProgram1->originLoc() = "";
    FareInfo fareInfo;
    fareInfo.directionality() = TO;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_TO_Program_OriginLOC_LOC1_Fail()
  {
    _bProgram1->originLoc() = _fm1->origin()->loc(); //"SFO";

    FareInfo fareInfo;
    fareInfo.directionality() = TO;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    Direction direction;
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionality_Fare_TO_Program_OriginLOC_LOC2_Pass()
  {
    _bProgram1->originLoc() = _fm1->destination()->loc(); //"DFW";

    FareInfo fareInfo;
    fareInfo.directionality() = TO;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    Direction direction;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionality(*_bProgram1, *_ptf1, direction));
    CPPUNIT_ASSERT_EQUAL(Direction::BOTHWAYS, direction);
  }

  void testMatchDirectionalityShopping()
  {
    FareInfo fareInfo;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);

    fareInfo.directionality() = TO;
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionalityShopping("SYD", *_ptf1));
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionalityShopping("MEL", *_ptf1));
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionalityShopping("AKL", *_ptf1));
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionalityShopping("LAX", *_ptf1));
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionalityShopping("ZQN", *_ptf1));
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionalityShopping("ROT", *_ptf1));

    fareInfo.directionality() = FROM;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionalityShopping("SYD", *_ptf1));
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionalityShopping("MEL", *_ptf1));
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionalityShopping("AKL", *_ptf1));
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionalityShopping("LAX", *_ptf1));
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->matchDirectionalityShopping("ZQN", *_ptf1));
    CPPUNIT_ASSERT(!_s8BrandedFaresSelector->matchDirectionalityShopping("ROT", *_ptf1));
  }

  void testPrintProgramFailDirectionality()
  {
    createDiagnostic(true);
    _bProgram1->originLoc() = _fm1->destination()->loc(); //"DFW";
    _bProgram1->direction() = "4";
    FareInfo fareInfo;
    fareInfo.directionality() = FROM;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    BrandedFareDiagnostics diagnostics(*_trx, _s8BrandedFaresSelector->_diag889);
    diagnostics.printProgramFailDirectionality(*_bProgram1, *_ptf1, Direction::BOTHWAYS);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR :           DOES NOT MATCH FARE DIR : FROM\n"),
                         _s8BrandedFaresSelector->_diag889->str());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFaresSelectorTest);
}
