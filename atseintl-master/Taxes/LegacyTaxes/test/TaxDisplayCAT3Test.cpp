// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/Category3.h"
#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

namespace tx_test
{
template <>
std::string
TaxDisplayTestBuilder<tse::Category3>::expectedDisplay(IORecord& ior) const
{
  if (std::get<1>(ior) != "" || std::get<2>(ior) != "")
    return ("* TICKETS SOLD " + std::get<1>(ior) + "AND ISSUED " + std::get<2>(ior) + ".\n");
  else
    return "     TICKETS SOLD AND ISSUED ANYWHERE.\n";
}
} // tx_test

namespace tse
{
class TaxDisplayCAT3Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT3Test);
  CPPUNIT_TEST(catInstanceTest);
  CPPUNIT_TEST(taxDisplayTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void catInstanceTest()
  {
    std::unique_ptr<Category3> ptr(new Category3);
    CPPUNIT_ASSERT(ptr);
  }

  void taxDisplayTest()
  {
    using namespace tx_test;
    IOContainer io;

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_01.xml",
                               "IN GERMANY ",
                               "IN UNITED KINGDOM"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_02.xml",
                               "IN GERMANY ",
                               "ANYWHERE EXCEPT UNITED KINGDOM"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_03.xml",
                          "ANYWHERE EXCEPT GERMANY ",
                               "IN UNITED KINGDOM"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_04.xml",
                               "ANYWHERE EXCEPT GERMANY ",
                          "ANYWHERE EXCEPT UNITED KINGDOM"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_05.xml", "IN JFK ", "IN NRT"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_06.xml",
                               "IN JFK ",
                               "ANYWHERE EXCEPT NRT"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_07.xml",
                               "ANYWHERE EXCEPT JFK ",
                               "IN NRT"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_08.xml", "", "ANYWHERE EXCEPT JFK"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_09.xml",
                          "IN BRAZIL EXCEPT "
                          "AJU, AQA, ARU, ATM, BAT, BAU, BEL, BFH, BGX, BHZ, BPS, BSB, BVB, CAC, "
                          "CAW, CCM, CFB, CGB, CGH, "
                          "CGR, CKS, CMG, CNF, CPQ, CPV, CWB, CXJ, CZS, DIQ, FEN, FLN, FOR, FRC, "
                          "GIG, GRU, GYN, IGU, IMP, "
                          "IOS, IPN, PIN, JDF, JOI, JPA, JPR, LAJ, LDB, LEC, MAB, MAO, MCP, ROO, "
                          "GVR, MCZ, MEA, MEU, MGF, "
                          "MII, MOC, MVS, NAT, NVT, PAV, PET, PLU, PMG, PMW, PNZ, POA, PPB, PPY, "
                          "PVH, QDV, RAO, RBR, REC, "
                          "RIA, RIO, RVD, SAO, SDU, SJK, SJP, SLZ, SSA, STM, TBT, TFF, THE, TMT, "
                          "UBA, UDI, UNA, URG, VAL, VCP, VIX, XAP ",
                          "IN HUX, MID, MTT, OAX, TAP, VSA"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat3_12.xml","IN GREECE EXCEPT CFU, PVK, ZTH ",
    // "ANYWHERE EXCEPT GREECE EXCEPT AOK, RHO"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_13.xml", "IN GERMANY ", "IN NRT"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_14.xml",
                               "ANYWHERE EXCEPT GERMANY ",
                               "IN NRT"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_15.xml",
                               "IN GERMANY ",
                               "ANYWHERE EXCEPT NRT"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_18.xml",
                               "ANYWHERE ",
                               "ANYWHERE EXCEPT THF, TXL"));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_10.xml",
                          "",
                          "ANYWHERE EXCEPT ANDORRA, ALBANIA, ARMENIA, AUSTRIA, AZERBAIJAN, "
                          "BOSNIA AND HERZEGOVINA, BELGIUM, "
                          "BULGARIA, BELARUS, SWITZERLAND, SERBIA AND MONTENEGRO, CYPRUS, CZECH "
                          "REPUBLIC, GERMANY, DENMARK, "
                          "ALGERIA, ESTONIA, SPAIN, FINLAND, FAROE ISLANDS, FRANCE, UNITED "
                          "KINGDOM, GEORGIA, GIBRALTAR, GREECE, "
                          "HONG KONG SAR CHINA, CROATIA, HUNGARY, IRELAND, ICELAND, ITALY, KOREA "
                          "REP OF/SOUTH, LIECHTENSTEIN, "
                          "LITHUANIA, LUXEMBOURG, LATVIA, MOROCCO, MONACO, MOLDOVA, MACEDONIA, "
                          "MALTA, NETHERLANDS, NORWAY, POLAND, "
                          "PORTUGAL, ROMANIA, RUSSIA, SWEDEN, SLOVENIA, SLOVAKIA, SAN MARINO, "
                          "TUNISIA, TURKEY, UKRAINE"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat3_11.xml","", "ANYWHERE"));
    // io.push_back(IORecord(xmlDir+"TaxCodeReg_DispCat3_19.xml", "ANYWHERE EXCEPT FIJI, SAMOA,
    // TONGA ","ANYWHERE EXCEPT AUSTRALIA, COCOS ISLANDS, NORFOLK ISLAND"));

    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_16.xml", "", ""));
    io.push_back(buildIORecord(xmlDir + "TaxCodeReg_DispCat3_17.xml", "", ""));

    tx_test::TaxDisplayTestBuilder<Category3> test;
    test.execute(io);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT3Test);
}
