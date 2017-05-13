#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "Common/MatrixUtils.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace std;

class PricingOrchestrator_RemoveExcessiveAltDateOptionsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingOrchestrator_RemoveExcessiveAltDateOptionsTest);
  CPPUNIT_TEST(testPricingOrchestrator_RemoveAltDateExcessiveOptions_emptyFlightMatrix);
  CPPUNIT_TEST(testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftOne);
  CPPUNIT_TEST(testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftTwo);
  CPPUNIT_TEST(testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftOnePerDate);
  CPPUNIT_TEST(testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftTwoPerDate);
  CPPUNIT_TEST(
      testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftOnePerDateOneLeg);
  CPPUNIT_TEST(
      testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftTwoPerDateOneLeg);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _po = new PricingOrchestratorDerived(*_memHandle.create<MockTseServer>());
    _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        _dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
  }

  void tearDown()
  {
    delete _po;
    _memHandle.clear();
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_emptyFlightMatrix()
  {
    ShoppingTrx::FlightMatrix& fltM = getEmptyFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 0);
    CPPUNIT_ASSERT_EQUAL(_trx->isAltDates(), true);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 0);
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftOne()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 1, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 2, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "BA");

    _trx->getOptions()->setRequestedNumberOfSolutions(1);
    ShoppingTrx::FlightMatrix& fltM = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(_trx->getOptions()->getRequestedNumberOfSolutions()), 1);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 1);
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftTwo()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 1, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 2, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "BA");

    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    ShoppingTrx::FlightMatrix& fltM = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(_trx->getOptions()->getRequestedNumberOfSolutions()), 2);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftOnePerDate()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA");
    DateTime dt = DateTime::localTime();
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 1, "LH");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA", dt.addDays(1));

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 2, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 3, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "BA");

    _trx->getOptions()->setRequestedNumberOfSolutions(1);
    ShoppingTrx::FlightMatrix& fltM = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(_trx->getOptions()->getRequestedNumberOfSolutions()), 1);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftTwoPerDate()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA");

    DateTime dt = DateTime::localTime();
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 1, "LH");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA", dt.addDays(1));

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 1, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[1], 2, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[1], "LON", "DFW", "BA");

    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    ShoppingTrx::FlightMatrix& fltM = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(_trx->getOptions()->getRequestedNumberOfSolutions()), 2);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 4);
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftOnePerDateOneLeg()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA");
    DateTime dt = DateTime::localTime();
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 1, "LH");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA", dt.addDays(1));
    _trx->legs().pop_back();
    _trx->getOptions()->setRequestedNumberOfSolutions(1);
    ShoppingTrx::FlightMatrix& fltM = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(_trx->getOptions()->getRequestedNumberOfSolutions()), 1);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
  }

  void testPricingOrchestrator_RemoveAltDateExcessiveOptions_FlightMatrixLeftTwoPerDateOneLeg()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA");

    DateTime dt = DateTime::localTime();
    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 1, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "AA", dt.addDays(1));

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 2, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "BA", dt.addDays(1));

    PricingOrchestratorTestShoppingCommon::createSOP(_dataHandle, _trx, _trx->legs()[0], 3, "LH");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        _dataHandle, _trx->legs()[0], "DFW", "LON", "LH", dt.addDays(1));

    _trx->legs().pop_back();
    _trx->getOptions()->setRequestedNumberOfSolutions(2);
    ShoppingTrx::FlightMatrix& fltM = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 4);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(_trx->getOptions()->getRequestedNumberOfSolutions()), 2);

    _po->removeExcessiveOptions(*_trx, fltM);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 3);
  }

protected:
  ShoppingTrx::FlightMatrix& getEmptyFlightMatrix(ShoppingTrx* trx) { return trx->flightMatrix(); }

  ShoppingTrx::FlightMatrix& getFullFlightMatrix(ShoppingTrx* trx)
  {
    ShoppingTrx::FlightMatrix& fltM = trx->flightMatrix();
    GroupFarePath* gfp = PricingOrchestratorTestShoppingCommon::buildGroupFarePath(_dataHandle);
    if (trx->legs().size() > 1)
    {
      std::vector<int> sopMap[2];
      for (size_t n = 0; n != trx->legs().size(); ++n)
      {
        for (size_t m = 0; m != trx->legs()[n].sop().size(); ++m)
        {
          {
            sopMap[n].push_back(m);
          }
        }
      }
      std::vector<int> dims;
      dims.push_back(sopMap[0].size());
      dims.push_back(sopMap[1].size());

      std::vector<int> sops(2);
      for (MatrixRatingIterator<EqualMatrixRater> mri(dims); !mri.atEnd(); mri.next())
      {
        const std::vector<int>& value = mri.value();
        sops[0] = sopMap[0][value[0]];
        sops[1] = sopMap[1][value[1]];
        const ShoppingTrx::FlightMatrix::value_type item(sops, gfp);
        fltM.insert(item);
      }
    }
    else
    {
      for (size_t n = 0; n != trx->legs().front().sop().size(); ++n)
      {
        std::vector<int> sops;
        sops.push_back(n);
        const ShoppingTrx::FlightMatrix::value_type item(sops, gfp);
        fltM.insert(item);
      }
    }
    return fltM;
  }

protected:
  TestMemHandle _memHandle;
  RefDataHandle _dataHandle;
  PricingOrchestratorDerived* _po;
  ShoppingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingOrchestrator_RemoveExcessiveAltDateOptionsTest);
}
