#include "test/include/CppUnitHelperMacros.h"
#include "Xform/RebuildXMLUtil.h"

namespace tse
{
class RebuildXMLUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RebuildXMLUtilTest);
  CPPUNIT_TEST(testGetAttributeValue);
  CPPUNIT_TEST(testCopyTo);
  CPPUNIT_TEST(testCopyContentOf);
  CPPUNIT_TEST(testRebuildDTLTkt1);
  CPPUNIT_TEST(testRebuildDTLTkt2);
  CPPUNIT_TEST(testRebuildDTLNoMT);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetAttributeValue()
  {
    char originalXML[] = "AB1=\"ONE\" AB2=\"TWO\" AB3=\"THREE\" ";
    std::string value;

    RebuildXMLUtil::getAttributeValue(originalXML, "AB2", value);
    const std::string expectedResult = "TWO";
    CPPUNIT_ASSERT_EQUAL(expectedResult, value);

    RebuildXMLUtil::getAttributeValue(originalXML, "AB4", value);
    CPPUNIT_ASSERT(value.empty());
  }
  void testCopyTo()
  {
    char originalXML[] = "<BLA AB1=\"TEST\" AB2=\"TWO\">BLABLA</BLA>"
                         "<ABC AB1=\"TEST\" AB2=\"ONE\">BAD</ABC>"
                         "<ABC AB1=\"TEST\" AB2=\"TWO\">GOOD</ABC>"
                         "<ABC AB1=\"TEST\" AB2=\"THREE\">UGLY</ABC>";

    RebuildXMLUtil rebuildXMLUtil(originalXML);
    std::string tagName = "<ABC";
    CPPUNIT_ASSERT(rebuildXMLUtil.copyTo(tagName) );

    std::string expectedResult = "<BLA AB1=\"TEST\" AB2=\"TWO\">BLABLA</BLA>";
    CPPUNIT_ASSERT_EQUAL(expectedResult, rebuildXMLUtil.outputXML() );
  }
  void testCopyContentOf()
  {
    char originalXML[] = "<BLA AB1=\"TEST\" AB2=\"TWO\">BLABLA</BLA>"
                         "<ABC AB1=\"TEST\" AB2=\"ONE\">BAD</ABC>"
                         "<ABC AB1=\"TEST\" AB2=\"TWO\">GOOD</ABC>"
                         "<ABC AB1=\"TEST\" AB2=\"THREE\">UGLY</ABC>";
    std::map<std::string, std::string> matchAttributes;
    matchAttributes[std::string("AB1")] = "TEST";
    matchAttributes[std::string("AB2")] = "NONE";
    std::string tagName = "<ABC";

    RebuildXMLUtil rebuildXMLUtil(originalXML);

    bool result = rebuildXMLUtil.copyContentOf(tagName, 0, matchAttributes, true);
    CPPUNIT_ASSERT(!result);

    matchAttributes.clear();
    matchAttributes[std::string("AB1")] = "TEST";
    matchAttributes[std::string("AB2")] = "TWO";
    result = rebuildXMLUtil.copyContentOf(tagName, 0, matchAttributes, true);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(std::string("GOOD"), rebuildXMLUtil.outputXML() );

    std::string leftToRead = "<ABC AB1=\"TEST\" AB2=\"THREE\">UGLY</ABC>";
    CPPUNIT_ASSERT_EQUAL(std::string(rebuildXMLUtil._currentPtr), leftToRead);
  }
  void testRebuildDTLTkt1()
  {
    char originalXML[] = "<DTL TKN=\"1\">"
                         "<PricingResponse>"
                         "<AGI A01=\"80K2\"/>"
                         "<BIL A01=\"ATP\"/>"
                         "<EXT>"
                         "<PCR TKN=\"1\">"
                         "<Q00>1</Q00>"
                         "</PCR>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"1\">BLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"HELP\"/>"
                         "</EXT>"
                         "<EXT>"
                         "<PCR TKN=\"2\">"
                         "<Q00>2</Q00>"
                         "</PCR>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"2\">BLABLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"COMING\"/>"
                         "</EXT>"
                         "</PricingResponse>"
                         "</DTL>";

    char expectedResult[] = "<DTL TKN=\"1\">"
                         "<PricingResponse>"
                         "<AGI A01=\"80K2\"/>"
                         "<BIL A01=\"ATP\"/>"
                         "<PCR TKN=\"1\">"
                         "<Q00>1</Q00>"
                         "</PCR>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"1\">BLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"HELP\"/>"
                         "</PricingResponse>"
                         "</DTL>";

    RebuildXMLUtil rebuildXMLUtil(originalXML);

    std::string tktNum;
    rebuildXMLUtil.getAttributeValue(originalXML, "TKN", tktNum);

    CPPUNIT_ASSERT(!tktNum.empty());
    
    std::map<std::string, std::string> matchAttributes;
    matchAttributes[std::string("TKN")] = tktNum;
    std::string tagEXT("<EXT");
    rebuildXMLUtil.copyTo(tagEXT);
    rebuildXMLUtil.copyContentOf(tagEXT, "<PCR", matchAttributes, true);
    std::string emptyStr = "";
    rebuildXMLUtil.exclusiveCopyTo(emptyStr, tagEXT);
    CPPUNIT_ASSERT_EQUAL(std::string(expectedResult), rebuildXMLUtil.outputXML() );
  }
  void testRebuildDTLTkt2()
  {
    char originalXML[] = "<DTL TKN=\"2\">"
                         "<PricingResponse>"
                         "<AGI A01=\"80K2\"/>"
                         "<BIL A01=\"ATP\"/>"
                         "<EXT>"
                         "<PCR TKN=\"1\">"
                         "<Q00>1</Q00>"
                         "</PCR>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"1\">BLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"HELP\"/>"
                         "</EXT>"
                         "<EXT>"
                         "<PCR TKN=\"2\">"
                         "<Q00>2</Q00>"
                         "</PCR>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"2\">BLABLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"COMING\"/>"
                         "</EXT>"
                         "</PricingResponse>"
                         "</DTL>";

    char expectedResult[] = "<DTL TKN=\"2\">"
                         "<PricingResponse>"
                         "<AGI A01=\"80K2\"/>"
                         "<BIL A01=\"ATP\"/>"
                         "<PCR TKN=\"2\">"
                         "<Q00>2</Q00>"
                         "</PCR>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"2\">BLABLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"COMING\"/>"
                         "</PricingResponse>"
                         "</DTL>";

    RebuildXMLUtil rebuildXMLUtil(originalXML);

    std::string tktNum;
    rebuildXMLUtil.getAttributeValue(originalXML, "TKN", tktNum);

    CPPUNIT_ASSERT(!tktNum.empty());
    
    std::map<std::string, std::string> matchAttributes;
    matchAttributes[std::string("TKN")] = tktNum;
    std::string tagEXT("<EXT");
    rebuildXMLUtil.copyTo(tagEXT);
    rebuildXMLUtil.copyContentOf(tagEXT, "<PCR", matchAttributes, true);

    std::string emptyStr = "";
    rebuildXMLUtil.exclusiveCopyTo(emptyStr, tagEXT);
    CPPUNIT_ASSERT_EQUAL(std::string(expectedResult), rebuildXMLUtil.outputXML() );
  }
  void testRebuildDTLNoMT()
  {
    char originalXML[] = "<DTL>"
                         "<PricingResponse>"
                         "<AGI A01=\"80K2\"/>"
                         "<BIL A01=\"ATP\"/>"
                         "<SUM>"
                         "<PXI=\"ADT\" AB2=\"1\">BLABLA</PXI>"
                         "</SUM>"
                         "<MSG S01=\"HELP\"/>"
                         "</PricingResponse>"
                         "</DTL>";

    RebuildXMLUtil rebuildXMLUtil(originalXML);

    std::string tktNum;
    rebuildXMLUtil.getAttributeValue(originalXML, "TKN", tktNum);
    CPPUNIT_ASSERT(tktNum.empty());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(RebuildXMLUtilTest);
}
