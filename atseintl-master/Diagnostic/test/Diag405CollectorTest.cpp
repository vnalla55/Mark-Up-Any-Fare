#include <time.h>
#include <iostream>
#include <unistd.h>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag405Collector.h"
#include "Common/Config/ConfigMan.h"

using namespace std;
namespace tse
{
std::string FNAME_BASE_TEST = "bceData.tst";

class Diag405CollectorTest : public CppUnit::TestFixture
{
  class DiagConfigInitializer : public TestConfigInitializer
  {
  public:
    tse::ConfigMan* config() { return &_config; }
  };

  CPPUNIT_TEST_SUITE(Diag405CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorBCESequence);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  tse::ConfigMan* _configMan;

public:
  void setUp()
  {
    DiagConfigInitializer* di = _memHandle.create<DiagConfigInitializer>();
    _configMan = di->config();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag405Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testStreamingOperatorBCESequence()
  {
    Diagnostic diagroot(Diagnostic405);
    diagroot.activate();
    Diag405Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic405);

    CPPUNIT_ASSERT(diag.isActive());
    // Make this not a BookingCodeExceptionSequenceList which is a const
    // -- and we can't use a const in a test environment because we are loading it.
    BookingCodeExceptionSequence bceSequence;
    loadBCE(bceSequence);
    string str = diag.str();

    // cout << str;
  }

public:
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
    TSICode tsidummy;

    _configMan->read(FNAME_BASE_TEST);

    // Figure out how to read sequences until there are no more.
    std::string sequenceGroup = "SEQUENCE";
    std::string sequenceName;

    sequenceName = sequenceGroup + (char)((seqCount) + '0');

    // first check to see if another sequence is there.
    readSeqItemTrue = _configMan->getValue("itemNo", ldummy, sequenceName);
    if (!readSeqItemTrue)
    {
      return;
    }

    bceSequence.itemNo() = (uint32_t)ldummy;

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
CPPUNIT_TEST_SUITE_REGISTRATION(Diag405CollectorTest);
}
