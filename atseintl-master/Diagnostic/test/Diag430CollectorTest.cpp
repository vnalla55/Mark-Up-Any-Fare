//-----------------------------------------------------------------------------
//
//  File:     Diag430CollectorTest.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2004
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include <time.h>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag430Collector.h"
#include "Common/Config/ConfigMan.h"
#include <unistd.h>

using namespace std;

namespace tse
{
std::string FNAME_BASE = "bceData.tst";

class Diag430CollectorTest : public CppUnit::TestFixture
{

  class DiagConfigInitializer : public TestConfigInitializer
  {
  public:
    tse::ConfigMan* config() { return &_config; }
  };

  CPPUNIT_TEST_SUITE(Diag430CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamingOperatorBCESequence);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  tse::ConfigMan* _configMan;

public:
  void setUp()
  {
    DiagConfigInitializer* dci = _memHandle.create<DiagConfigInitializer>();
    _configMan = dci->config();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag430Collector diag(diagroot);
      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  /* public */
  void testStreamingOperatorBCESequence()
  {

    Diagnostic diagroot(Diagnostic430);
    diagroot.activate();
    Diag430Collector diag(diagroot);
    diag.enable(Diagnostic430);
    CPPUNIT_ASSERT(diag.isActive());
    BookingCodeExceptionSequence bceSequence;
    loadBCE(bceSequence);
    diag << bceSequence;
    CPPUNIT_ASSERT(true);
  }

  // Can't use the BookingCodeExceptionSequenceList because it is const -- and this is a test that
  // pushes things onto the List.
  void loadBCE(BookingCodeExceptionSequence& bceSequence)
  {
    long ldummy;
    int intdummy;
    char chardummy;
    std::string strdummy;
    int seqCount = 1;
    bool readSeqItemTrue = true;
    //  TSEDate tdate=0;
    //  TSEDateTime tdtime=0;
    TSICode tsidummy;

    _configMan->read(FNAME_BASE);

    // Figure out how to read sequences until there are no more.
    std::string sequenceGroup = "SEQUENCE";
    std::string sequenceName;
    // while(readSeqItemTrue)
    //{
    sequenceName = sequenceGroup + (char)((seqCount) + '0');

    // first check to see if another sequence is there.
    readSeqItemTrue = _configMan->getValue("itemNo", ldummy, sequenceName);
    if (!readSeqItemTrue)
    {
      return;
    }
    // BookingCodeExceptionSequence* pSeq = new BookingCodeExceptionSequence();
    // bceSequenceList.push_back(pSeq);
    bceSequence.itemNo() = (uint32_t)ldummy;
    // pSeq->expireDate() = tdate;

    _configMan->getValue("primeInd", chardummy, sequenceName);
    bceSequence.primeInd() = chardummy;

    _configMan->getValue("tableType", chardummy, sequenceName);
    bceSequence.tableType() = chardummy;

    _configMan->getValue("seqNo", ldummy, sequenceName);
    bceSequence.seqNo() = (uint32_t)ldummy;

    _configMan->getValue(" Construct/Spec'd", chardummy, sequenceName);
    bceSequence.constructSpecified() = chardummy;

    _configMan->getValue("ifTag", chardummy, sequenceName);
    bceSequence.ifTag() = chardummy;

    _configMan->getValue("segCnt", intdummy, sequenceName);
    bceSequence.segCnt() = (uint16_t)intdummy;

    if (bceSequence.segCnt() > 0)
    {
      std::string segmentGroup = "SEGMENT";
      std::string segmentName;
      BookingCodeExceptionSegmentVector& segmentVec = bceSequence.segmentVector();

      //    Loop thru and get the segment Data
      for (int i = 0; i < bceSequence.segCnt(); i++)
      {
        segmentName = segmentGroup + (char)(seqCount + '0') + '-' + (char)((i + 1) + '0');

        BookingCodeExceptionSegment* pSegment = new BookingCodeExceptionSegment();

        segmentVec.push_back(pSegment);

        _configMan->getValue("segNo", intdummy, segmentName);
        pSegment->segNo() = (uint16_t)intdummy;

        _configMan->getValue("viaCarrier", strdummy, segmentName);
        pSegment->viaCarrier() = strdummy;

        _configMan->getValue("primarySecondary", chardummy, segmentName);
        pSegment->primarySecondary() = chardummy;

        _configMan->getValue("fltRangeAppl", chardummy, segmentName);
        pSegment->fltRangeAppl() = chardummy;

        _configMan->getValue("flight1", ldummy, segmentName);
        pSegment->flight1() = (uint32_t)ldummy;

        _configMan->getValue("flight2", ldummy, segmentName);
        pSegment->flight2() = (uint32_t)ldummy;

        _configMan->getValue("equipType", strdummy, segmentName);
        pSegment->equipType() = strdummy;

        _configMan->getValue("tvlPortion", strdummy, segmentName);
        pSegment->tvlPortion() = strdummy;

        _configMan->getValue("tsi", tsidummy, segmentName);
        pSegment->tsi() = tsidummy;

        _configMan->getValue("directionInd", chardummy, segmentName);
        pSegment->directionInd() = chardummy;

        _configMan->getValue("loc1Type", chardummy, segmentName);

        if (chardummy == '0')
        {
          chardummy = '\0';
        }
        pSegment->loc1Type() = chardummy;

        _configMan->getValue("loc1", strdummy, segmentName);
        pSegment->loc1() = (LocCode&)strdummy;

        _configMan->getValue("loc2Type", chardummy, segmentName);

        if (chardummy == '0')
        {
          chardummy = '\0';
        }
        pSegment->loc2Type() = chardummy;

        _configMan->getValue("loc2", strdummy, segmentName);
        pSegment->loc2() = (LocCode&)strdummy;

        _configMan->getValue("posTsi", tsidummy, segmentName);
        pSegment->posTsi() = tsidummy;

        _configMan->getValue("posLocType", chardummy, segmentName);

        if (chardummy == '0')
        {
          chardummy = '\0';
        }

        pSegment->posLocType() = chardummy;

        _configMan->getValue("posLoc", strdummy, segmentName);
        pSegment->posLoc() = (LocCode&)strdummy;

        _configMan->getValue("soldInOut", chardummy, segmentName);
        pSegment->soldInOutInd() = chardummy;

        _configMan->getValue("daysOfWeek", strdummy, segmentName);
        pSegment->daysOfWeek() = strdummy;

        _configMan->getValue("fareclassType", chardummy, segmentName);
        pSegment->fareclassType() = chardummy;

        _configMan->getValue("fareclass", strdummy, segmentName);
        pSegment->fareclass() = strdummy;

        _configMan->getValue("restriction", chardummy, segmentName);
        pSegment->restrictionTag() = chardummy;

        _configMan->getValue("bkcode1", strdummy, segmentName);
        pSegment->bookingCode1() = strdummy;

        _configMan->getValue("bkcode2", strdummy, segmentName);
        pSegment->bookingCode2() = strdummy;
      } // end of segment loop
    } // end segment count > 0
    seqCount++;
    //} // end while readSequence true
  } // end loadBCE
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag430CollectorTest);
}
