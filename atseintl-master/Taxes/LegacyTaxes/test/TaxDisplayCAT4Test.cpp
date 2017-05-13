#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/Category4.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

#include "test/include/CppUnitHelperMacros.h"

#include <ctime>
#include <iostream>
#include <string>

using namespace std;

namespace tse
{
std::string zonedesc0001906 =
    "AJU, AQA, ARU, ATM, BAT, BAU, BEL, BFH, BGX, BHZ, BPS, BSB, BVB, CAC, CAW, CCM, CFB, CGB, "
    "CGH, CGR, CKS, CMG, CNF, CPQ, CPV, CWB, CXJ, CZS, DIQ, FEN, FLN, FOR, FRC, GIG, GRU, GYN, "
    "IGU, IMP, IOS, IPN, PIN, JDF, JOI, JPA, JPR, LAJ, LDB, LEC, MAB, MAO, MCP, ROO, GVR, MCZ, "
    "MEA, MEU, MGF, MII, MOC, MVS, NAT, NVT, PAV, PET, PLU, PMG, PMW, PNZ, POA, PPB, PPY, PVH, "
    "QDV, RAO, RBR, REC, RIA, RIO, RVD, SAO, SDU, SJK, SJP, SLZ, SSA, STM, TBT, TFF, THE, TMT, "
    "UBA, UDI, UNA, URG, VAL, VCP, VIX, XAP";
std::string zonedesc0004789 = "HUX, MID, MTT, OAX, TAP, VSA";

class TaxDisplayCAT4Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT4Test);
  CPPUNIT_TEST(testBuildCAT4FromTo);
  CPPUNIT_TEST(testBuildCAT4Between);
  CPPUNIT_TEST(testBuildCAT4Within);
  CPPUNIT_TEST(testBuildCAT4WithinWholly);
  CPPUNIT_TEST(testBuildCAT4Origination);
  CPPUNIT_TEST(testBuildCAT4Enplanement);
  CPPUNIT_TEST(testBuildCAT4Deplanement);
  CPPUNIT_TEST(testBuildCAT4Destination);
  CPPUNIT_TEST(testBuildCAT4Termination);
  CPPUNIT_TEST(testBuildCAT4MiscInfo);
  CPPUNIT_TEST(testBuildCAT4originItin);
  CPPUNIT_TEST_SUITE_END();

public:
  void setLoc1Loc2Type1Type2(tse::LocCode loc1,
                             tse::LocCode loc2,
                             tse::Indicator ind1,
                             tse::Indicator ind2,
                             TaxCodeReg& taxCodeReg)
  {
    taxCodeReg.loc1() = loc1;
    taxCodeReg.loc2() = loc2;
    taxCodeReg.loc1Type() = ind1;
    taxCodeReg.loc2Type() = ind2;
  }

  void testBuildCAT4FromTo()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);

    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM GERMANY TO UNITED KINGDOM.\n"),
                         category4.subCat1());

    // change trip type and check
    taxCodeReg.tripType() = TripTypesValidator::TAX_BETWEEN;
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL BETWEEN GERMANY AND UNITED KINGDOM.\n"),
                         category4.subCat1());

    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM ANYWHERE EXCEPT GERMANY TO UNITED KINGDOM.\n"),
                         category4.subCat1());

    taxCodeReg.tripType() = TripTypesValidator::TAX_BETWEEN;
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL BETWEEN GERMANY AND UNITED KINGDOM.\n"),
                         category4.subCat1());

    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM GERMANY TO ANYWHERE EXCEPT UNITED KINGDOM.\n"),
                         category4.subCat1());

    taxCodeReg.tripType() = TripTypesValidator::TAX_BETWEEN;
    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL BETWEEN GERMANY AND UNITED KINGDOM.\n"),
                         category4.subCat1());

    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* TRAVEL FROM ANYWHERE EXCEPT GERMANY TO ANYWHERE EXCEPT UNITED KINGDOM.\n"),
        category4.subCat1());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM JFK TO LHR.\n"), category4.subCat1());

    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM ANYWHERE EXCEPT JFK TO LHR.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM JFK TO ANYWHERE EXCEPT LHR.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM ANYWHERE EXCEPT JFK TO ANYWHERE EXCEPT LHR.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM BRAZIL EXCEPT " + zonedesc0001906 + " TO " +
                                     zonedesc0004789 + ".\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM ANYWHERE EXCEPT BRAZIL EXCEPT " +
                                     zonedesc0001906 + " TO " + zonedesc0004789 + ".\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM BRAZIL EXCEPT " + zonedesc0001906 +
                                     " TO ANYWHERE EXCEPT " + zonedesc0004789 + ".\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL FROM BRAZIL EXCEPT " + zonedesc0001906 +
                                     " TO ANYWHERE EXCEPT " + zonedesc0004789 + ".\n"),
                         category4.subCat1());
  }

  //--------------------------------------------------------------------------
  void testBuildCAT4Between()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    taxCodeReg.tripType() = TripTypesValidator::TAX_BETWEEN;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL BETWEEN JFK AND LHR.\n"), category4.subCat1());
  }
  //--------------------------------------------------------------------------

  void testBuildCAT4Within()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.tripType() = TripTypesValidator::TAX_WITHIN_SPEC;
    taxCodeReg.originLocType() = tse::LocType(' ');
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WITHIN SPECIFIED LOCATION GERMANY.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WITHIN SPECIFIED LOCATION GERMANY.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WITHIN SPECIFIED LOCATION GERMANY.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WITHIN SPECIFIED LOCATION GERMANY.\n"),
                         category4.subCat1());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WITHIN SPECIFIED LOCATION JFK.\n"),
                         category4.subCat1());
  }

  void testBuildCAT4WithinWholly()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.tripType() = TripTypesValidator::TAX_WITHIN_WHOLLY;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WHOLLY WITHIN GERMANY.\n"), category4.subCat1());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("PL", "", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TRAVEL WHOLLY WITHIN POLAND.\n"), category4.subCat1());
  }

  void testBuildCAT4Origination()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc1Appl() = LocRestrictionValidator::TAX_ORIGIN;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM ANYWHERE EXCEPT GERMANY.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM ANYWHERE EXCEPT GERMANY.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM GERMANY TO ANYWHERE EXCEPT UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM ANYWHERE EXCEPT GERMANY.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM JFK TO LHR.\n"), category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM ANYWHERE EXCEPT JFK.\n"), category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM JFK TO ANYWHERE EXCEPT LHR.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM ANYWHERE EXCEPT JFK.\n"), category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM BRAZIL EXCEPT " + zonedesc0001906 + " TO " +
                                     zonedesc0004789 + ".\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* ORIGIN FROM ANYWHERE EXCEPT BRAZIL EXCEPT " + zonedesc0001906 + ".\n"),
        category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ORIGIN FROM BRAZIL EXCEPT " + zonedesc0001906 +
                                     " TO ANYWHERE EXCEPT " + zonedesc0004789 + ".\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* ORIGIN FROM ANYWHERE EXCEPT BRAZIL EXCEPT " + zonedesc0001906 + ".\n"),
        category4.subCat2());
  }

  void testBuildCAT4Enplanement()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------
    taxCodeReg.originLocType() = tse::LocType(' ');

    taxCodeReg.loc1Appl() = LocRestrictionValidator::TAX_ENPLANEMENT;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM GERMANY TO UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM ANYWHERE EXCEPT GERMANY.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* ENPLANEMENT FROM GERMANY TO ANYWHERE EXCEPT UNITED KINGDOM.\n"),
        category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM ANYWHERE EXCEPT GERMANY.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM JFK TO LHR.\n"), category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM ANYWHERE EXCEPT JFK.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM JFK TO ANYWHERE EXCEPT LHR.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("JFK", "LHR", 'C', 'C', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM ANYWHERE EXCEPT JFK.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = NO;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM BRAZIL EXCEPT " + zonedesc0001906 +
                                     " TO " + zonedesc0004789 + ".\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* ENPLANEMENT FROM ANYWHERE EXCEPT BRAZIL EXCEPT " + zonedesc0001906 + ".\n"),
        category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ENPLANEMENT FROM BRAZIL EXCEPT " + zonedesc0001906 +
                                     " TO ANYWHERE EXCEPT " + zonedesc0004789 + ".\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("0001906", "0004789", 'Z', 'Z', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* ENPLANEMENT FROM ANYWHERE EXCEPT BRAZIL EXCEPT " + zonedesc0001906 + ".\n"),
        category4.subCat2());
  }
  void testBuildCAT4Deplanement()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* DEPLANEMENT LOCATION UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* DEPLANEMENT LOCATION UNITED KINGDOM.\n"),
                         category4.subCat2());
  }

  void testBuildCAT4Destination()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_DESTINATION;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* DESTINATION LOCATION UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* DESTINATION LOCATION UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* DESTINATION LOCATION ANYWHERE EXCEPT UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* DESTINATION LOCATION ANYWHERE EXCEPT UNITED KINGDOM.\n"),
                         category4.subCat2());
  }

  void testBuildCAT4Termination()
  {
    TaxTrx trx;
    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_TERMINATION;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TERMINATION LOCATION UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TERMINATION LOCATION UNITED KINGDOM.\n"),
                         category4.subCat2());

    taxCodeReg.loc1ExclInd() = NO;
    taxCodeReg.loc2ExclInd() = YES;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TERMINATION LOCATION ANYWHERE EXCEPT UNITED KINGDOM.\n"),
                         category4.subCat2());
  }
  void testBuildCAT4MiscInfo()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.travelType() = 'D';
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ON DOMESTIC TRAVEL.\n"), category4.subCat3());

    taxCodeReg.travelType() = 'I';
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ON INTERNATIONAL TRAVEL.\n"), category4.subCat3());

    taxCodeReg.itineraryType() = 'R';
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ON ROUND TRIP TRAVEL.\n"), category4.subCat4());

    taxCodeReg.itineraryType() = 'O';
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ON ONE WAY TRAVEL.\n"), category4.subCat4());

    taxCodeReg.itineraryType() = 'J';
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* ON OPEN JAW TRAVEL.\n"), category4.subCat4());
  }

  void testBuildCAT4originItin()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    Category4 category4;
    //-------------------------------------------------------

    taxCodeReg.originLocType() = tse::LocType(' ');

    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_TERMINATION;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    taxCodeReg.originLocType() = tse::LocType('N');
    taxCodeReg.originLoc() = tse::LocCode("PL");
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(
        std::string("* TERMINATION LOCATION UNITED KINGDOM.\n* ITINERARY ORIGINATING IN POLAND.\n"),
        category4.subCat2());

    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_TERMINATION;
    setLoc1Loc2Type1Type2("DE", "GB", 'N', 'N', taxCodeReg);
    taxCodeReg.originLocExclInd() = YES;
    taxCodeReg.originLoc() = tse::LocCode("0001906");
    taxCodeReg.originLocType() = tse::LocType('Z');
    taxDisplayItem.taxCodeReg() = &taxCodeReg;
    category4.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TERMINATION LOCATION UNITED KINGDOM.\n* ITINERARY "
                                     "ORIGINATING ANYWHERE EXCEPT IN BRAZIL EXCEPT " +
                                     zonedesc0001906 + ".\n"),
                         category4.subCat2());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT4Test);
}
