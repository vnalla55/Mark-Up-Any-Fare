#include <string>
#include <time.h>
#include <iostream>
#include <vector>
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Diagnostic/Diag804Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>

using namespace std;
namespace tse
{
class Diag804CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag804CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorTaxItemOut);
  CPPUNIT_TEST_SUITE_END();

public:
  // void testConstructor()
  //{
  //   try
  //   {
  //       Diagnostic* diagroot = NULL;
  //       Diag804Collector diag( diagroot );
  //       string str = diag.str();
  //       CPPUNIT_ASSERT_EQUAL( string(""), str );
  //   }
  //   catch( ... )
  //   {
  //      // If any error occured at all, then fail.
  //      CPPUNIT_ASSERT( false );
  //   }
  //}
  //
  // void testInactiveStreamingOperator()
  //{
  //    Diagnostic* diagroot = new Diagnostic( LegacyTaxDiagnostic24, "");
  //    diagroot->activate();
  //
  //    Diag804Collector diag( diagroot );
  //
  //    TaxItem taxItem;
  //
  //    // First, we test being inactive. No changes are expected.
  //
  //    diag << taxItem;
  //    string str = diag.str();
  //    CPPUNIT_ASSERT_EQUAL( string(""), str );
  //
  //}
  //
  // void testStreamingOperator()
  //{
  //    Diagnostic* diagroot = new Diagnostic( LegacyTaxDiagnostic24, "");
  //    diagroot->activate();
  //
  //    Diag804Collector diag( diagroot );
  //
  //    // Now, enable it and do the same streaming operation.
  //    diag.enable(LegacyTaxDiagnostic24 );
  //
  //    CPPUNIT_ASSERT( diag.isActive() );
  //
  //    TaxItem taxItem;
  //    diag << taxItem;
  //
  //    string str = diag.str();
  //    string expected;
  //
  //    cout << str;
  //
  //    expected +="\n";
  //    expected +="***************************************************************\n";
  //    expected +="** JRP'24 WPQ/*24 - Tax Diagnostics  0 Items  Psgr 0 \n";
  //    expected +="***************************************************************\n";
  //    expected +="\n";
  //    expected +="                  FARE             TAX AMT   PUBLISHED\n";
  //    expected +="TAX  MARKET               PERCENT           AMT        \n";
  //
  //    CPPUNIT_ASSERT_EQUAL( expected, str );
  //}

  void testStreamingOperatorTaxItemOut()
  {
    PricingTrx trx;
    trx.setValidatingCxrGsaApplicable(false);
    trx.setTrxType(PricingTrx::PRICING_TRX);
    PricingRequest request;
    trx.setRequest(&request);
    request.owPricingRTTaxProcess() = false;
    Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
    diagroot->activate();

    Diag804Collector diag(*diagroot);
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(LegacyTaxDiagnostic24);

    CPPUNIT_ASSERT(diag.isActive());

    TaxResponse taxResponse;

    //     uint32_t name = taxItem.initializeTaxItemOut( );
    string expected;

    // std::vector<TaxItemOut *>::iterator i;
    //   bool only1row = false;
    //  uint32_t taxOutCount = 0;
    //  FareAmount fareAmount = 0;
    //  uint16_t taxOutSize = 1;
    //
    //   diag << "\n***************************************************************\n"
    //    << "** JRP'24 WPQ/*24 - Tax Diagnostics  " << taxOutSize
    //    << " Items  Psgr " << taxItem.paxIndex() << " "
    //    << taxItem.paxTypeCode() << "\n"
    //    << "***************************************************************\n\n"
    //    << "                  FARE             TAX AMT   PUBLISHED\n"
    //    << "TAX  MARKET       " << taxItem.currencyCode()
    //    << "        PERCENT    " << taxItem.currencyCode()
    //    << "       AMT        \n";
    //
    //  for ( i = taxItem.taxItemOutVector().begin(); i != taxItem.taxItemOutVector().end();
    //        i++ )
    //  {
    //    fareAmount = (*i)->taxAmt();
    //
    //    if ((*i)->taxThruBoard()  == (*i)->taxLocalBoard()  ||
    //        (*i)->taxThruOff()  == (*i)->taxLocalOff() )
    //    {
    //      only1row = true;
    //      fareAmount = (*i)->taxableFare();
    //    }
    //
    //    diag << (*i)->taxCode() << " "
    //       << (*i)->taxThruBoard()
    //       << (*i)->taxThruOff() << " "
    //       << fareAmount  ;
    //
    //    if (! only1row)
    //    {
    //      diag << "\n";      // Put everything else on the next line
    //                       // Start of new line
    //
    //      diag << "    " << (*i)->taxLocalBoard()
    //         << (*i)->taxLocalOff()
    //         << " "
    //         << (*i)->taxableFare() ;
    //    }
    //
    //    if ((*i)->taxType() == "P")
    //    {
    //      diag << "    " << (*i)->taxAmt()
    //         << "  " << (*i)->taxTotal();
    //    }
    //    else
    //    {
    //      diag << "          " << (*i)->taxTotal()
    //         << "  " << (*i)->taxAmt()
    //         << " " << (*i)->taxCurrency();
    //    }
    //
    //
    //  }         // End TaxItem For Loop

    /*

     for ( i = taxItem.taxItemOutVector().begin(); i != taxItem.taxItemOutVector().end();
     i++ )
     {


     cout  << (*i)->taxCode() << " "
     << (*i)->taxLocalBoard()
     << (*i)->taxLocalOff() << " "
     << (*i)->taxAmt();

     }
     */
    //     if (1 == 1)
    //     {
    //        expected += "Push failed \n\n" ;
    //     }
    //     else
    //     {
    //     	expected += "Push OK  \n\n" ;
    //     }

    diag << taxResponse;

    string str = diag.str();
    //  string str = "\n\nOh NO";

    // cout << str;

    expected += "\n";
    // expected +="***************************************************************\n";
    // expected +="** JRP'24 WPQ/*24 - Tax Diagnostics  0 Items  Psgr 0 \n";
    // expected +="***************************************************************\n";
    // expected +="\n";
    // expected +="                  FARE             TAX AMT   PUBLISHED\n";
    // expected +="TAX  MARKET               PERCENT           AMT        \n";

    expected += "********************* TAX LOGIC ANALYSIS *********************\n";
    expected += "                  FARE AMT               TAX AMT   PUBLISHED\n";
    expected += "TAX  MARKET                  PERCENTAGE            AMOUNT\n";
    expected += "--------------------------------------------------------------\n";
    expected += " T A X E S   N O T   A P P L I C A B L E \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag804CollectorTest);
}
