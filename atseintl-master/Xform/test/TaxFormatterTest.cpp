// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Xform/TaxFormatter.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestMemHandle.h"

#include "Common/XMLConstruct.h"

#include <vector>

namespace tse
{
class TaxFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxFormatterTest);
  CPPUNIT_TEST(testFormatTaxInformation);
  CPPUNIT_TEST(testFormatTaxExempt);
  CPPUNIT_TEST_SUITE_END();

public:

  void
  testFormatTaxInformation() {
    XMLConstruct construct;
    std::vector<std::string> taxIdExempted;
    TaxFormatter formatter(construct, false, false, taxIdExempted);

    OCFees::TaxItem taxItem;
    taxItem.setTaxCode("AAA");
    taxItem.setTaxAmount(100.25);
    taxItem.setNumberOfDec(2);
    taxItem.setTaxType('P');
    formatter.formatTaxInformation(taxItem);


    const std::string expectedXML = "<TAX BC0=\"AAA\" C6B=\"100.25\" A06=\"P\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, construct.getXMLData());
  }

  void
  testFormatTaxExempt() {
    XMLConstruct construct;
    std::vector<std::string> taxIdExempted;
    TaxFormatter formatter(construct, false, false, taxIdExempted);

    OCFees::TaxItem taxItem;
    taxItem.setTaxCode("AAA");
    taxItem.setTaxAmount(100.25);
    taxItem.setNumberOfDec(2);
    taxItem.setTaxType('F');
    formatter.formatTaxExempt(taxItem);


    const std::string expectedXML = "<TBE BC0=\"AAA\" C6B=\"100.25\" A06=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, construct.getXMLData());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxFormatterTest);

}
