//----------------------------------------------------------------------------
//
//  File   :  BrandResponseHandlerTest.cpp
//
//  Author :  Mauricio Dantas
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include "BrandingService/BrandResponseHandler.h"
#include "BrandingService/BrandResponseItem.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DBAccess/DiskCache.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace std;
namespace tse
{

class SpecificTestConfigInitializer : public TestConfigInitializer
{
public:
  SpecificTestConfigInitializer()
  {
    DiskCache::initialize(_config);
    _memHandle.create<MockDataManager>();
  }

  ~SpecificTestConfigInitializer() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

class BrandResponseHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandResponseHandlerTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST_SUITE_END();

  //---------------------------------------------------------------------
  // test parse XML response from Brand service
  //---------------------------------------------------------------------
  void testParse()
  {

    TestMemHandle memHandle;
    memHandle.create<SpecificTestConfigInitializer>();

    std::string xmlResponse =
        "<AirlineBrandingResponse VER=\"1.0\"><SRC A20=\"00C7\" "
        "AQ1=\"FQ\"/><BRS><RES><D01>2008-05-20</D01><A01>SYD</A01><A02>MEL</A02><CBD><CBR "
        "B10=\"QF\"><SB1>B</SB1><SB7/><BRD><BRN><SB2>BU</SB2><SB3>Business</SB3><SB4>FULLY "
        "FLEXIBLE-PREMIUM SERVICE</SB4><SB5>D|J</SB5></BRN><BRN><SB2>TB</SB2><SB3>test "
        "brand</SB3><SB4>TESTS SINGLE FARE IN A BRAND</SB4><FBC N21=\"Y\" "
        "SB6=\"BIPOX\"/><SB5/></BRN><BRN><SB2>FS</SB2><SB3>Flexisaver</SB3><SB4>NO CHANGE FEES-NON "
        "REFUNDABLE</SB4><SB5>K</SB5></BRN><BRN><SB2>SS</SB2><SB3>SuperSaver</SB3><SB4>LOW "
        "FARE-RESTRICTIONS-CHANGE FEES APPLY</SB4><FBC N21=\"N\" "
        "SB6=\"LOZPASA\"/><SB5>L|V|S|R|M</SB5></BRN><BRN><SB2>RD</SB2><SB3>Red "
        "edeal</SB3><SB4>Wiktor 2 test red edeal "
        "text</SB4><SB5>O|N</SB5></BRN><BRN><SB2>FF</SB2><SB3>FullyFlexible</SB3><SB4>WIKTOR TEST "
        "NO RESTRICTIONS-FULLY REFUNDABLE</SB4><FBC N21=\"Y\" "
        "SB6=\"Y|HOX\"/><SB5/></BRN></BRD></CBR></CBD></RES></BRS></AirlineBrandingResponse>";

    std::vector<BrandResponseItem*> _brandResponseItemVec;
    PricingTrx _trx;
    BrandResponseHandler _docHandler(_trx, _brandResponseItemVec);

    _docHandler.initialize();

    CPPUNIT_ASSERT(_docHandler.parse(xmlResponse.c_str()) == true);
    return;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandResponseHandlerTest);
}
