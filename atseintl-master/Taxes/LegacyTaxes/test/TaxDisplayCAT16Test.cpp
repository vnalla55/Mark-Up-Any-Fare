// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "Taxes/LegacyTaxes/Category16.h"

#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TaxRequest.h"
#include "Common/TseCodeTypes.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <memory>

using namespace tx_test;

namespace tse
{
class TaxDisplayCAT16Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT16Test);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testDisplay);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testCreation()
  {
    std::unique_ptr<Category16> category16(new Category16);
    CPPUNIT_ASSERT(0 != category16);
  }

  void testDisplay()
  {
    TaxCodeGenText* taxCodeGenText0 = new TaxCodeGenText();

    TaxCodeGenText* taxCodeGenText1 = new TaxCodeGenText();
    taxCodeGenText1->messageDisplayCat() = 'I';
    taxCodeGenText1->txtMsgs().push_back("APPLIES ON REISSUES.");

    TaxCodeGenText* taxCodeGenText2 = new TaxCodeGenText();
    taxCodeGenText2->messageDisplayCat() = 'J';
    taxCodeGenText2->txtMsgs().push_back("APPLIES ON REISSUES ONLY IN POLAND.");
    taxCodeGenText2->txtMsgs().push_back("APPLIES ON REISSUES ONLY IN AUSTRALIA.");

    TaxCodeGenText* taxCodeGenText3 = new TaxCodeGenText();
    taxCodeGenText3->messageDisplayCat() = 'I';
    taxCodeGenText3->txtMsgs().push_back("APPLIES ON REISSUES ONLY IN AUSTRALIA.");
    taxCodeGenText3->txtMsgs().push_back("APPLIES ON REISSUES ONLY IN USA.");

    TaxCodeReg* taxCodeReg0 = _memHandle.create<TaxCodeReg>();
    taxCodeReg0->taxCodeGenTexts().push_back(taxCodeGenText0);

    TaxCodeReg* taxCodeReg1 = _memHandle.create<TaxCodeReg>();
    taxCodeReg1->taxCodeGenTexts().push_back(taxCodeGenText1);

    TaxCodeReg* taxCodeReg2 = _memHandle.create<TaxCodeReg>();
    taxCodeReg2->taxCodeGenTexts().push_back(taxCodeGenText2);
    taxCodeReg2->taxCodeGenTexts().push_back(taxCodeGenText3);

    IOContainer io;
    io.push_back(buildIORecord(taxCodeReg0, "     NO REISSUE RESTRICTIONS APPLY.\n"));
    io.push_back(buildIORecord(taxCodeReg1, "** APPLIES ON REISSUES.\n"));
    io.push_back(buildIORecord(taxCodeReg2,
                               "** APPLIES ON REISSUES ONLY IN AUSTRALIA.\n"
                               "   APPLIES ON REISSUES ONLY IN USA.\n"));

    TaxDisplayTestBuilder<Category16> test;
    test.execute(io);
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT16Test);
}
