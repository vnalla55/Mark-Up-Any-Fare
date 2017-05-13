//----------------------------------------------------------------------------
//  File:        Diag23Collector.cpp
//  Authors:
//  Created:     July 8004
//
//  Description: Diagnostic 200 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2004, 2009
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

#include <time.h>
#include <cppunit/TestFixture.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/MiscFareTag.h"
#include <iostream>
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareMarket.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag223Collector.h"
#include "DataModel/Trx.h"
#include <unistd.h>

using namespace std;
namespace tse
{
class Diag223CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag223CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamingOperatorFare);
  CPPUNIT_TEST(testStreamingOperatorFareMarket);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag223Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorFare()
  //---------------------------------------------------------------------
  void testStreamingOperatorFare()
  {

    Diagnostic diagroot(Diagnostic223);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag223Collector diag(diagroot);

    diag.enable(Diagnostic223);
    CPPUNIT_ASSERT(diag.isActive());

    PaxTypeFare mf;
    PaxType paxtype;
    FareMarket faremarket;
    Fare fare;
    MiscFareTag miscFareTag;
    FareClassAppInfo fareClassAppInfo;
    FareClassAppSegInfo fareClassAppSegInfo;
    string expected;

    mf.initialize(&fare, &paxtype, &faremarket);
    mf.miscFareTag() = &miscFareTag;
    const_cast<FareType&>(mf.fcaFareType()) = string("PCF");
    const_cast<PaxTypeCode&>(mf.fcasPaxType()) = string("ADT");

    miscFareTag.unavailtag() = 'X';
    miscFareTag.constInd() = 'N';
    miscFareTag.prorateInd() = 'R';
    miscFareTag.diffcalcInd() = 'N';
    miscFareTag.refundcalcInd() = 'R';
    miscFareTag.proportionalInd() = 'P';
    miscFareTag.curradjustInd() = 'N';
    miscFareTag.fareClassType1Ind() = 'F';
    miscFareTag.fareClassType1() = "Y26";
    miscFareTag.fareClassType2Ind() = 'T';
    miscFareTag.fareClassType2() = "123";
    miscFareTag.geoTblItemNo() = 2008;
    miscFareTag.itemNo() = 2988540072;

    // ....
    Fare* fare1 = mf.fare();
    FareInfo fareInfo;
    fare1->setFareInfo(&fareInfo);
    fareInfo._fareClass = "Y26";
    fareInfo._ruleNumber = "6059";
    fareInfo._fareTariff = 77;
    fareInfo._fareAmount = 106;
    fareInfo._currency = "USD";
    mf.setFare(fare1);
    mf.fareClassAppInfo() = &fareClassAppInfo;
    fareClassAppInfo._fareType = "PCF";

    mf.fareClassAppSegInfo() = &fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    fare1->nucFareAmount() = 106;

    string expMisc = "ITEM NO - 2988540072\n"
                     "CONSTR -  N ; PRORATE -  R ; DIFF CALC -  N ; UNAVAIL -  X ;\n"
                     "REF CALC: R ; PROPORT -  P ; CURR ADJ -  N ; \n"
                     "GEO TABLE NO - 2008            ;\n"
                     "FARE 1: TYPE - F  ; FARE CLASS FARE/TYPE - Y26         \n"
                     "FARE 2: TYPE - T  ; FARE CLASS FARE/TYPE - 123         \n";

    // 1....
    mf.status().setNull();
    mf.status().set(PaxTypeFare::PTF_Discounted);

    fareInfo._globalDirection = GlobalDirection::AT;
    fareInfo._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo._vendor = Vendor::ATPCO;
    fareInfo._directionality = FROM;

    diag << mf;
    expected = "D AT A 6059 Y26          77  R O   106.00 USD PCF ADT   106.00\n" + expMisc;
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
    diag.flushMsg();

    // 2...
    mf.status().setNull();
    mf.status().set(PaxTypeFare::PTF_FareByRule);

    fareInfo._globalDirection = GlobalDirection::AT;
    fareInfo._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo._vendor = Vendor::ATPCO;
    fareInfo._directionality = FROM;

    diag << mf;
    expected = "B AT A 6059 Y26          77  R O   106.00 USD PCF ADT   106.00\n" + expMisc;
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
    diag.flushMsg();

    // 3...
    mf.status().setNull();
    mf.status().set(PaxTypeFare::PTF_Negotiated);

    fareInfo._globalDirection = GlobalDirection::AT;
    fareInfo._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo._vendor = Vendor::ATPCO;
    fareInfo._directionality = FROM;

    diag << mf;
    expected = "N AT A 6059 Y26          77  R O   106.00 USD PCF ADT   106.00\n" + expMisc;
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
    diag.flushMsg();
  }

  void testStreamingOperatorFareMarket()
  {

    Diagnostic diagroot(Diagnostic223);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag223Collector diag(diagroot);
    diag.enable(Diagnostic223);

    CPPUNIT_ASSERT(diag.isActive());

    string expected;
    FareMarket fm;
    vector<TravelSeg*> travelSegments;
    AirSeg airSeg;

    airSeg.carrier() = "AA";
    airSeg.boardMultiCity() = "DFW";
    airSeg.offMultiCity() = "LHR";
    airSeg.destAirport() = "LHR";
    fm.governingCarrier() = "AA";
    fm.direction() = FMDirection::OUTBOUND;
    travelSegments.push_back(dynamic_cast<TravelSeg*>(&airSeg));
    fm.travelSeg() = travelSegments;

    vector<PaxTypeBucket> paxTypeCortegeVec;

    PaxTypeFare paxTypeFare;
    PaxType paxtype;
    FareMarket faremarket;
    Fare fare;
    FareInfo fareInfo;
    FareClassAppInfo fareClassAppInfo;
    FareClassAppSegInfo fareClassAppSegInfo;
    MiscFareTag miscFareTag;

    fare.setFareInfo(&fareInfo);
    paxTypeFare.paxType() = &paxtype;
    paxTypeFare.fareMarket() = &faremarket;
    paxTypeFare.fareClassAppInfo() = &fareClassAppInfo;
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.fareMarket() = &faremarket;
    paxTypeFare.miscFareTag() = &miscFareTag;

    miscFareTag.unavailtag() = 'X';
    miscFareTag.constInd() = 'N';
    miscFareTag.prorateInd() = 'R';
    miscFareTag.diffcalcInd() = 'N';
    miscFareTag.refundcalcInd() = 'R';
    miscFareTag.proportionalInd() = 'P';
    miscFareTag.curradjustInd() = 'N';
    miscFareTag.fareClassType1Ind() = 'F';
    miscFareTag.fareClassType1() = "Y26";
    miscFareTag.fareClassType2Ind() = 'T';
    miscFareTag.fareClassType2() = "123";
    miscFareTag.geoTblItemNo() = 2008;
    miscFareTag.itemNo() = 1953392997;

    fareInfo._fareClass = "Y26";
    fareInfo._ruleNumber = "6059";
    fareInfo._fareTariff = 77;
    fareInfo._fareAmount = 106;
    fareInfo._currency = "USD";
    fareInfo._globalDirection = GlobalDirection::AT;
    fareInfo._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo._vendor = Vendor::ATPCO;
    fareInfo._directionality = FROM;

    paxTypeFare.setFare(&fare);

    fareClassAppInfo._fareType = "PCF";
    fareClassAppSegInfo._paxType = "ADT";
    fare.nucFareAmount() = 106;
    paxTypeFare.status() = PaxTypeFare::PTF_Discounted;

    string expMisc = "ITEM NO - 1953392997\n"
                     "CONSTR -  N ; PRORATE -  R ; DIFF CALC -  N ; UNAVAIL -  X ;\n"
                     "REF CALC: R ; PROPORT -  P ; CURR ADJ -  N ; \n"
                     "GEO TABLE NO - 2008            ;\n"
                     "FARE 1: TYPE - F  ; FARE CLASS FARE/TYPE - Y26         \n"
                     "FARE 2: TYPE - T  ; FARE CLASS FARE/TYPE - 123         \n\n";

    expected = "D AT A 6059 Y26          77  R O   106.00 USD PCF ADT   106.00\n" + expMisc;

    vector<PaxTypeFare*> paxFareVec;
    paxFareVec.push_back(&paxTypeFare);

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.paxTypeFare() = paxFareVec;

    PaxType pt;
    pt.paxType() = "ADT";
    paxTypeCortege.requestedPaxType() = &pt;

    paxTypeCortegeVec.push_back(paxTypeCortege);

    fm.paxTypeCortege() = paxTypeCortegeVec;
    fm.setGlobalDirection(GlobalDirection::AT);

    diag << fm;
    expected = "\n-AA-LHR    /CXR-AA/ #GI-AT#  .OUT.\n\n"
               "REQUESTED PAXTYPE : ADT\n"
               " INBOUND CURRENCY : USD\n"
               "OUTBOUND CURRENCY : USD\n\n"
               "  GI V RULE   FARE CLS   TRF O O      AMT CUR FAR PAX   CNV   \n"
               "                         NUM R I              TPE TPE   AMT   \n"
               "- -- - ---- ------------ --- - - -------- --- --- --- --------\n" +
               expected;
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag223CollectorTest);
}
