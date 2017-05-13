#include <iostream>
#include <string>

#include "Xform/test/TaxInfoXmlBuilderTest.h"
#include "Xform/TaxInfoXmlBuilder.h"
#include "DataModel/TaxInfoRequest.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "Taxes/TaxInfo/TaxInfoBuilderPFC.h"
#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "Taxes/TaxInfo/TaxInfoDriver.h"

using namespace tse;

class TaxInfoTester
{
  TaxRequest request;

public:
  TaxInfoTester(DateTime dt = DateTime::localTime())
  {
    _trx.setRequest(&request);
    _trx.taxInfoRequest() = &_taxInfoRequest;
    _trx.taxInfoRequest()->overrideDate() = dt;
  }

  void addRequestItem(TaxCode taxCode, std::vector<LocCode> airports)
  {
    TaxInfoItem item;

    item.taxCode() = taxCode;

    std::copy(airports.begin(), airports.end(), back_inserter(item.airports()));

    _trx.taxInfoRequest()->taxItems().push_back(item);
  }

  void execute()
  {

    TaxInfoDriver driver(&_trx, false);
    driver.buildTaxInfoResponse();

    std::vector<TaxInfoResponse>::iterator it = _trx.taxInfoResponseItems().begin();
    std::vector<TaxInfoResponse>::iterator itEnd = _trx.taxInfoResponseItems().end();
    for (; it != itEnd; it++)
    {
      _xmlResponse += _xmlBuilder.build(*it);
    }
  }
  TaxTrx* trx() { return &_trx; }

  std::string& response() { return _xmlResponse; }

  std::string _xmlResponse;

  TaxInfoXmlBuilder _xmlBuilder;
  TaxTrx _trx;
  TaxInfoRequest _taxInfoRequest;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxInfoXmlBuilderTest);

void
TaxInfoXmlBuilderTest::testConstructor()
{
  try
  {
    TaxTrx trx;
    TaxInfoDriver driver(&trx, false);
  }
  catch (...)
  {
    // If any error occured at all, then fail.
    CPPUNIT_ASSERT(false);
  }
}

void
TaxInfoXmlBuilderTest::testBuildMisingTaxCodes()
{
  TaxInfoTester test;
  test.execute();

  std::string expect = "<TAX S18=\"MISSING TAX CODE\" />";

  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildPfcForDFW()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");

  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  /*
  std::string expect = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >"
                       "<APT A01=\"DFW\" C6A=\"4.50\" C41=\"USD\" D06=\"2002-07-01\"
  D05=\"2017-03-01\" />"
                       "</TAX>";
 */
  std::string expect1 = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "</TAX>";

  // CPPUNIT_ASSERT( test.response() == expect);

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildPfcForKRK()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("KRK");

  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >"
                       "<APT A01=\"KRK\" S04=\"PFC NOT APPLICABLE\" />"
                       "</TAX>";
  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildPfcForNYC()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("NYC");

  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >"
                       "<APT A01=\"NYC\" S04=\"INVALID AIRPORT CODE\" />"
                       "</TAX>";

  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildPfcForDFWJFK()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");
  airports.push_back("JFK");

  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  /*
  std::string expect = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >"
                       "<APT A01=\"DFW\" C6A=\"4.50\" C41=\"USD\" D06=\"2002-07-01\"
  D05=\"2017-03-01\" />"
                       "<APT A01=\"JFK\" C6A=\"4.50\" C41=\"USD\" D06=\"2006-12-07\"
  D05=\"2011-03-01\" />"
                       "</TAX>";

   CPPUNIT_ASSERT(test.response() == expect);
   */

  std::string expect1 = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "<APT A01=\"JFK\"";
  std::string expect4 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildPfcForDFWKRKJFKNYC()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");
  airports.push_back("KRK");
  airports.push_back("JFK");
  airports.push_back("NYC");

  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  /*
  std::string expect = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >"
                       "<APT A01=\"DFW\" C6A=\"4.50\" C41=\"USD\" D06=\"2002-07-01\"
  D05=\"2017-03-01\" />"
                       "<APT A01=\"KRK\" S04=\"PFC NOT APPLICABLE\" />"
                       "<APT A01=\"JFK\" C6A=\"4.50\" C41=\"USD\" D06=\"2006-12-07\"
  D05=\"2011-03-01\" />"
                       "<APT A01=\"NYC\" S04=\"INVALID AIRPORT CODE\" />"
                       "</TAX>";

  CPPUNIT_ASSERT(test.response() == expect);
  */

  std::string expect1 = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "<APT A01=\"KRK\"";
  std::string expect4 = "<APT A01=\"JFK\"";
  std::string expect5 = "<APT A01=\"NYC\"";
  std::string expect6 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect5) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect6) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildPfcNoAirports()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;

  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"XF\" S18=\"MISSING AIRPORT CODES\" />";

  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildZpForDFW()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"ZP\" S18=\"MISSING AIRPORT CODES\" />";

  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildZpForKRK()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("KRK");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"ZP\" S18=\"MISSING AIRPORT CODES\" />";
  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildZpForNYC()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("NYC");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"ZP\" S18=\"MISSING AIRPORT CODES\" />";

  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildZpForNYCJFK()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("NYC");
  airports.push_back("JFK");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >"
                       "<APT A01=\"NYC\" S04=\"INVALID AIRPORT CODE\" />"
                       "<APT A01=\"JFK\" PYA=\"T\" PYB=\"F\" />"
                       "</TAX>";

  CPPUNIT_ASSERT_EQUAL(expect, test.response());
}

void
TaxInfoXmlBuilderTest::testBuildZpForDFWJFK()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");
  airports.push_back("JFK");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  /*
  std::string expect = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >"
                       "<APT A01=\"DFW\" C6A=\"3.60\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
                       "<APT A01=\"JFK\" PYA=\"T\" PYB=\"F\" />"
                       "</TAX>";

  CPPUNIT_ASSERT(test.response() == expect);
  */

  std::string expect1 = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "<APT A01=\"JFK\" PYA=\"T\" PYB=\"F\" />";
  std::string expect4 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildZpForABIDFW()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("ABI");
  airports.push_back("DFW");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();
  /*
  std::string expect = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >"
                       "<APT A01=\"ABI\" C6A=\"0.00\" C41=\"USD\" PYA=\"T\" PYB=\"T\" />"
                       "<APT A01=\"DFW\" PYA=\"T\" PYB=\"F\" />"
                       "</TAX>";

  CPPUNIT_ASSERT(test.response() == expect);
  */

  std::string expect1 = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >";
  std::string expect2 = "<APT A01=\"ABI\"";
  std::string expect3 = "<APT A01=\"DFW\" PYA=\"T\" PYB=\"F\" />";
  std::string expect4 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildZpForDFWKRKJFKNYC()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");
  airports.push_back("KRK");
  airports.push_back("JFK");
  airports.push_back("NYC");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  /*
  std::string expect = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >"
                        "<APT A01=\"DFW\" C6A=\"0.00\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
                        "<APT A01=\"KRK\" C6A=\"0.00\" C41=\"USD\" PYA=\"F\" PYB=\"F\" />"
                        "<APT A01=\"JFK\" C6A=\"0.00\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
            "<APT A01=\"NYC\" S04=\"INVALID AIRPORT CODE\" />"
            "</TAX>";

  CPPUNIT_ASSERT(test.response() == expect);
  */

  std::string expect1 = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "<APT A01=\"KRK\"";
  std::string expect4 = "<APT A01=\"JFK\"";
  std::string expect5 = "<APT A01=\"NYC\" S04=\"INVALID AIRPORT CODE\" />";
  std::string expect6 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect5) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect6) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildZpForDFWORDDENLAX()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");
  airports.push_back("ORD");
  airports.push_back("DEN");
  airports.push_back("LAX");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();
  /*
  std::string expect = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >"
                       "<APT A01=\"DFW\" C6A=\"3.60\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
                       "<APT A01=\"ORD\" C6A=\"3.60\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
                       "<APT A01=\"DEN\" C6A=\"3.60\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
               "<APT A01=\"LAX\" PYA=\"T\" PYB=\"F\" />"
               "</TAX>";



  CPPUNIT_ASSERT(test.response() == expect);
  */

  std::string expect1 = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "<APT A01=\"ORD\"";
  std::string expect4 = "<APT A01=\"DEN\"";
  std::string expect5 = "<APT A01=\"LAX\" PYA=\"T\" PYB=\"F\" />";
  std::string expect6 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect5) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect6) != std::string::npos);
}

void
TaxInfoXmlBuilderTest::testBuildZpNoAirports()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.execute();

  std::string expect = "<TAX BC0=\"ZP\" S18=\"MISSING AIRPORT CODES\" />";

  CPPUNIT_ASSERT(test.response() == expect);
}

void
TaxInfoXmlBuilderTest::testBuildZpPfc()
{
  TaxInfoTester test;

  std::vector<LocCode> airports;
  airports.push_back("DFW");
  airports.push_back("JFK");

  test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
  test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
  test.execute();

  /*
  std::string expect = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >"
                       "<APT A01=\"DFW\" C6A=\"3.60\" C41=\"USD\" PYA=\"T\" PYB=\"F\" />"
               "<APT A01=\"JFK\" PYA=\"T\" PYB=\"F\" />"
               "</TAX>"
               "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >"
                       "<APT A01=\"DFW\" C6A=\"4.50\" C41=\"USD\" D06=\"2002-07-01\"
  D05=\"2017-03-01\" />"
                       "<APT A01=\"JFK\" C6A=\"4.50\" C41=\"USD\" D06=\"2006-12-07\"
  D05=\"2011-03-01\" />"
                       "</TAX>";

  CPPUNIT_ASSERT(test.response() == expect);
  */

  std::string expect1 = "<TAX BC0=\"ZP\" A06=\"F\" A40=\"US\" S04=\"SEGMENT TAX\" >";
  std::string expect2 = "<APT A01=\"DFW\"";
  std::string expect3 = "<APT A01=\"JFK\" PYA=\"T\" PYB=\"F\" />";
  std::string expect4 = "</TAX>";
  std::string expect5 = "<TAX BC0=\"XF\" A06=\"F\" A40=\"US\" S04=\"PASSENGER FACILITY CHARGE\" >";
  std::string expect6 = "<APT A01=\"DFW\"";
  std::string expect7 = "<APT A01=\"JFK\"";
  std::string expect8 = "</TAX>";

  CPPUNIT_ASSERT(test.response().find(expect1) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect2) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect3) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect4) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect5) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect6) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect7) != std::string::npos);
  CPPUNIT_ASSERT(test.response().find(expect8) != std::string::npos);
}
