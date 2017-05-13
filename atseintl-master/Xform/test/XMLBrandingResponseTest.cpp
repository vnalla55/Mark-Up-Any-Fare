//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"
#include "Xform/XMLBrandingResponse.h"

#include <string>
#include <vector>
#include <set>
#include <sstream>

using namespace std;

namespace tse
{

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class XmlBrandingResponseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XmlBrandingResponseTest);
  CPPUNIT_TEST(emptyResponseTest);
  CPPUNIT_TEST(regularUsageTest);
  CPPUNIT_TEST(diagnosticTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<BrandingTrx>();
    _xbr = _memHandle.insert(new XMLBrandingResponse(*_trx));
    _memHandle.create<TestConfigInitializer>();
    Itin *itin = _memHandle.create<Itin>();
    itin->itinNum() = 0;
    _trx->itin().push_back(itin);

    itin = _memHandle.create<Itin>();
    itin->itinNum() = 3;
    _trx->itin().push_back(itin);
  }

  void tearDown() { _memHandle.clear(); }

  void emptyResponseTest()
  {
    _trx->itin().clear();
    BrandingResponseType empty;
    string out;
    _xbr->buildXmlBrandingResponse(out, empty);
    CPPUNIT_ASSERT_EQUAL(XML_INFO + string("<ShoppingResponse Q0S=\"0\"/>\n"), out);
  }

  void regularUsageTest()
  {
    _xbr->setToken("my token");
    BrandingResponseType tree;
    tree[_trx->itin()[0]]["AB"].insert("123");
    tree[_trx->itin()[0]]["AB"].insert("456");
    tree[_trx->itin()[1]]["UU"].insert("999");
    tree[_trx->itin()[1]]["NN"].insert("111");
    string out;
    _xbr->buildXmlBrandingResponse(out, tree);

    const string suffix = "<ShoppingResponse STK=\"my token\" Q0S=\"2\">\n"
                          "  <ITN NUM=\"0\">\n"
                          "    <GRI SB2=\"AB\">\n"
                          "      <PRG SC0=\"123\"/>\n"
                          "      <PRG SC0=\"456\"/>\n"
                          "    </GRI>\n"
                          "  </ITN>\n"
                          "  <ITN NUM=\"3\">\n"
                          "    <GRI SB2=\"NN\">\n"
                          "      <PRG SC0=\"111\"/>\n"
                          "    </GRI>\n"
                          "    <GRI SB2=\"UU\">\n"
                          "      <PRG SC0=\"999\"/>\n"
                          "    </GRI>\n"
                          "  </ITN>\n"
                          "</ShoppingResponse>\n";

    CPPUNIT_ASSERT_EQUAL(XML_INFO + suffix, out);
  }

  void diagnosticTest()
  {
    _xbr->setToken("my token");
    BrandingResponseType tree;
    tree[_trx->itin()[0]]["AB"].insert("123");
    tree[_trx->itin()[0]]["AB"].insert("456");
    tree[_trx->itin()[1]]["UU"].insert("999");
    tree[_trx->itin()[1]]["NN"].insert("111");

    const string diagText = "DiAg";

    _xbr->setDiagOutput(218, diagText);

    string out;
    _xbr->buildXmlBrandingResponse(out, tree);

    const string suffix = "<ShoppingResponse STK=\"my token\" Q0S=\"2\">\n"
                          "  <DIA Q0A=\"218\">\n"
                          "<![CDATA[DiAg]]>  </DIA>\n"
                          "  <ITN NUM=\"0\">\n"
                          "    <GRI SB2=\"AB\">\n"
                          "      <PRG SC0=\"123\"/>\n"
                          "      <PRG SC0=\"456\"/>\n"
                          "    </GRI>\n"
                          "  </ITN>\n"
                          "  <ITN NUM=\"3\">\n"
                          "    <GRI SB2=\"NN\">\n"
                          "      <PRG SC0=\"111\"/>\n"
                          "    </GRI>\n"
                          "    <GRI SB2=\"UU\">\n"
                          "      <PRG SC0=\"999\"/>\n"
                          "    </GRI>\n"
                          "  </ITN>\n"
                          "</ShoppingResponse>\n";

    CPPUNIT_ASSERT_EQUAL(XML_INFO + suffix, out);
  }

private:
  TestMemHandle _memHandle;
  BrandingTrx* _trx;
  XMLBrandingResponse* _xbr;

  static const string XML_INFO;
};

const string XmlBrandingResponseTest::XML_INFO = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

CPPUNIT_TEST_SUITE_REGISTRATION(XmlBrandingResponseTest);

} // namespace tse
