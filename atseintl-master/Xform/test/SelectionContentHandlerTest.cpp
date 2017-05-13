//----------------------------------------------------------------------------
//
//  File:  SelectionContentHandlerTest.cpp
//  Description: Unit test for SelectionContentHandler
//  Created:  July 30 2008
//  Authors:  Okan Okcu
//
//  Copyright Sabre 2008
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

#include "Xform/SelectionContentHandler.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class SelectionContentHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SelectionContentHandlerTest);
  CPPUNIT_TEST(testTrueParseP77RecordQuoteAttribute);
  CPPUNIT_TEST(testFalseParseP77RecordQuoteAttribute);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { xercesc::XMLPlatformUtils::Initialize(); }

  const char* buildSelectionRequestElement(const char* p77AttributeValue)
  {
    std::string s = "<SelectionRequest><REQ A70=\"WPA*\" S81=\"18\" PBV=\"F\" P77=\"" +
                    std::string(p77AttributeValue) + "\"/></SelectionRequest>";
    return strdup(s.c_str());
  }

  void testTrueParseP77RecordQuoteAttribute()
  {
    const char* p77AttributeValue = buildSelectionRequestElement("T");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("TRUE");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("t");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("true");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("ttt");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(_sdh.recordQuote());
  }

  void testFalseParseP77RecordQuoteAttribute()
  {
    const char* p77AttributeValue = buildSelectionRequestElement("F");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(!_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("FALSE");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(!_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("f");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(!_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("false");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(!_sdh.recordQuote());

    p77AttributeValue = buildSelectionRequestElement("fff");
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(!_sdh.recordQuote());

    p77AttributeValue = "<SelectionRequest>Dummy Text</SelectionRequest>";
    _sdh.parse(p77AttributeValue);
    CPPUNIT_ASSERT(!_sdh.recordQuote());
  }

private:
  SelectionContentHandler _sdh;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SelectionContentHandlerTest);
}
