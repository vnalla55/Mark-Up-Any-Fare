#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag688Collector.h"
#include "DataModel/RefundPermutation.h"
#include "DBAccess/VoluntaryRefundsInfo.h"

namespace tse
{

using namespace boost::assign;

class Diag688CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag688CollectorTest);

  CPPUNIT_TEST(testPrintRefundProcessInfo);
  CPPUNIT_TEST(testPrintRefundPermutation);
  CPPUNIT_TEST(testPrintRefundPermutations);

  CPPUNIT_TEST_SUITE_END();

protected:
  Diag688Collector* _diag;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _diag = _memH.insert(new Diag688Collector);
    _diag->activate();
  }

  void tearDown() { _memH.clear(); }

  std::string getDiagString()
  {
    _diag->flushMsg();
    return _diag->str();
  }

  RefundProcessInfo* createRefundProcessInfo(const LocCode& board,
                                             const LocCode& off,
                                             int itemNo,
                                             Indicator repriceInd = ' ')
  {
    PaxTypeFare* ptf = _memH.insert(new PaxTypeFare);
    FareMarket* fm = _memH.insert(new FareMarket);
    ptf->fareMarket() = fm;
    fm->fareCompInfo() = _memH.insert(new FareCompInfo);
    fm->fareCompInfo()->fareMarket() = fm;
    VoluntaryRefundsInfo* vri = _memH.insert(new VoluntaryRefundsInfo);

    fm->boardMultiCity() = board;
    fm->offMultiCity() = off;
    vri->itemNo() = itemNo;
    vri->repriceInd() = repriceInd;

    return _memH.insert(new RefundProcessInfo(vri, ptf, fm->fareCompInfo()));
  }

  RefundPermutation*
  createRefundPermutation(int number, RefundProcessInfo* inf1, RefundProcessInfo* inf2)
  {
    RefundPermutation* rp = _memH.insert(new RefundPermutation);
    rp->number() = number;
    rp->processInfos() += inf1, inf2;
    rp->setRepriceIndicator();
    return rp;
  }

  void testPrintRefundProcessInfo()
  {
    RefundProcessInfo* info = createRefundProcessInfo("KRK", "DFW", 13246, 'A');
    _diag->printRefundProcessInfo(*info);
    CPPUNIT_ASSERT_EQUAL(std::string("KRK DFW 13246  A\n"), getDiagString());
  }

  void testPrintRefundPermutation()
  {
    RefundPermutation* rp =
        createRefundPermutation(1,
                                createRefundProcessInfo("KRK", "DFW", 13246, 'A'),
                                createRefundProcessInfo("DFW", "KRK", 64231, 'B'));

    _diag->printRefundPermutation(*rp);

    CPPUNIT_ASSERT_EQUAL(std::string("PERMUTATION 1\n"
                                     "FM      REC3   RI\n"
                                     "KRK DFW 13246  A\n"
                                     "DFW KRK 64231  B\n"
                                     "     RE-PRICE: A\n"),
                         getDiagString());
  }

  void testPrintRefundPermutations()
  {
    std::vector<RefundPermutation*> perm;
    perm += createRefundPermutation(1,
                                    createRefundProcessInfo("KRK", "DFW", 13246, 'A'),
                                    createRefundProcessInfo("DFW", "KRK", 64231, 'B')),
        createRefundPermutation(2,
                                createRefundProcessInfo("DFW", "LAX", 151515, 'B'),
                                createRefundProcessInfo("LAX", "DFW", 515151, 'B'));

    _diag->printRefundPermutations(perm);

    std::string expect("CATEGORY 33 VOL REFUND - DIAG 688\n"
                       "***************************************************************\n"
                       "2 VALID PERMUTATIONS\n"
                       "***************************************************************\n"
                       "PERMUTATION 1\n"
                       "FM      REC3   RI\n"
                       "KRK DFW 13246  A\n"
                       "DFW KRK 64231  B\n"
                       "     RE-PRICE: A\n"
                       "***************************************************************\n"
                       "PERMUTATION 2\n"
                       "FM      REC3   RI\n"
                       "DFW LAX 151515 B\n"
                       "LAX DFW 515151 B\n"
                       "     RE-PRICE: B\n"
                       "***************************************************************\n");
    CPPUNIT_ASSERT_EQUAL(expect, getDiagString());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag688CollectorTest);

} // end of tse
