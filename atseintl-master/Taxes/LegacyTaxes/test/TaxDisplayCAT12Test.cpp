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
#include "Taxes/LegacyTaxes/Category12.h"

#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TaxRequest.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include "test/include/TestMemHandle.h"

using namespace tx_test;

namespace tse
{
class TaxDisplayCAT12Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT12Test);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testDisplay);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }
  void testCreation()
  {
    Category12* category12 = _memHandle.create<Category12>();

    CPPUNIT_ASSERT(0 != category12);
  }

  void testDisplay()
  {

    TaxRestrictionTktDesignator* tktDsg1 = new TaxRestrictionTktDesignator;
    tktDsg1->carrier() = "TZ";
    tktDsg1->tktDesignator() = "CORP";

    TaxRestrictionTktDesignator* tktDsg1_2 = new TaxRestrictionTktDesignator;
    tktDsg1_2->carrier() = "TZ";
    tktDsg1_2->tktDesignator() = "CORP";

    TaxRestrictionTktDesignator* tktDsg2 = new TaxRestrictionTktDesignator;
    tktDsg2->carrier() = "US";
    tktDsg2->tktDesignator() = "CFP00";

    TaxRestrictionTktDesignator* tktDsg3 = new TaxRestrictionTktDesignator;
    tktDsg3->carrier() = "TZ";
    tktDsg3->tktDesignator() = "CFP00";

    TaxRestrictionTktDesignator* tktDsg4 = new TaxRestrictionTktDesignator;
    tktDsg4->carrier() = "N7";
    tktDsg4->tktDesignator() = "CPTVL";

    TaxRestrictionTktDesignator* tktDsg5 = new TaxRestrictionTktDesignator;
    tktDsg5->carrier() = "US";
    tktDsg5->tktDesignator() = "CSGFSP";

    TaxRestrictionTktDesignator* tktDsg6 = new TaxRestrictionTktDesignator;
    tktDsg6->carrier() = "HP";
    tktDsg6->tktDesignator() = "INRTFC";

    TaxRestrictionTktDesignator* tktDsg7 = new TaxRestrictionTktDesignator;
    tktDsg7->carrier() = "YX";
    tktDsg7->tktDesignator() = "NDSEO";

    TaxRestrictionTktDesignator* tktDsg8 = new TaxRestrictionTktDesignator;
    tktDsg8->carrier() = "AA";
    tktDsg8->tktDesignator() = "PROALP";

    TaxRestrictionTktDesignator* tktDsg9 = new TaxRestrictionTktDesignator;
    tktDsg9->carrier() = "AA";
    tktDsg9->tktDesignator() = "PROALP";

    TaxRestrictionTktDesignator* tktDsg10 = new TaxRestrictionTktDesignator;
    tktDsg10->carrier() = "US";
    tktDsg10->tktDesignator() = "APPLA";

    TaxCodeReg* taxCodeReg1 = _memHandle.create<TaxCodeReg>();
    taxCodeReg1->taxRestrTktDsgs().push_back(tktDsg1);

    TaxCodeReg* taxCodeReg = _memHandle.create<TaxCodeReg>();
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg1_2);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg2);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg3);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg4);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg5);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg6);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg7);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg8);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg9);
    taxCodeReg->taxRestrTktDsgs().push_back(tktDsg10);

    IOContainer io;
    io.push_back(buildIORecord(taxCodeReg1,
                               "* TICKET DESIGNATOR -\n"
                               "  ON CARRIER TZ - CORP.\n"));
    io.push_back(buildIORecord(taxCodeReg,
                               "* TICKET DESIGNATOR -\n"
                               "  ON CARRIER AA - PROALP\n"
                               "  ON CARRIER HP - INRTFC\n"
                               "  ON CARRIER N7 - CPTVL\n"
                               "  ON CARRIER TZ - CFP00, CORP\n"
                               "  ON CARRIER US - APPLA, CFP00, CSGFSP\n"
                               "  ON CARRIER YX - NDSEO.\n"));

    TaxDisplayTestBuilder<Category12> test;
    test.execute(io);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT12Test);
}
