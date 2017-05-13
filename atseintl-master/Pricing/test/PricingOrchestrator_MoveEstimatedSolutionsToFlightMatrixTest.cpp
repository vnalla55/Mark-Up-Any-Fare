#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "Common/MatrixUtils.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace std;

class PricingOrchestrator_MoveEstimatedSolutionsToFlightMatrixTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingOrchestrator_MoveEstimatedSolutionsToFlightMatrixTest);
  CPPUNIT_TEST(testPricingOrchestrator_MoveEstimatedSolutionsToFlightMatrix_emptyFlightMatrix);
  CPPUNIT_TEST(testPricingOrchestrator_MoveEstimatedSolutionsToFlightMatrix_emptyEstimateMatrix);
  CPPUNIT_TEST(testPricingOrchestrator_MoveEstimatedSolutionsToFlightMatrix_bothMatricesNotEmpty);
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

  void testPricingOrchestrator_MoveEstimatedSolutionsToFlightMatrix_emptyFlightMatrix()
  {
    ShoppingTrx::FlightMatrix& fltM = getEmptyFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 0);

    ShoppingTrx::EstimateMatrix& estM = getEmptyEstimateMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(estM.size()), 0);

    _po->moveEstimatedSolutionsToFlightMatrix(*_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 0);
  }

  void testPricingOrchestrator_MoveEstimatedSolutionsToFlightMatrix_emptyEstimateMatrix()
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

    ShoppingTrx::EstimateMatrix& estM = getEmptyEstimateMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(estM.size()), 0);

    _po->moveEstimatedSolutionsToFlightMatrix(*_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 2);
  }

  void testPricingOrchestrator_MoveEstimatedSolutionsToFlightMatrix_bothMatricesNotEmpty()
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

    ShoppingTrx::EstimateMatrix& estM = getFullEstimateMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(estM.size()), 2);

    _po->moveEstimatedSolutionsToFlightMatrix(*_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(fltM.size()), 4);
  }

protected:
  ShoppingTrx::FlightMatrix& getEmptyFlightMatrix(ShoppingTrx* trx) { return trx->flightMatrix(); }
  ShoppingTrx::EstimateMatrix& getEmptyEstimateMatrix(ShoppingTrx* trx)
  {
    return trx->estimateMatrix();
  }

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

  ShoppingTrx::EstimateMatrix& getFullEstimateMatrix(ShoppingTrx* trx)
  {
    ShoppingTrx::EstimateMatrix& estM = trx->estimateMatrix();
    GroupFarePath* gfp = PricingOrchestratorTestShoppingCommon::buildGroupFarePath(_dataHandle);
    if (trx->legs().size() > 1)
    {
      std::vector<int> sopMap[2];
      for (size_t n = 0; n != trx->legs().size(); ++n)
      {
        for (size_t m = 0; m != trx->legs()[n].sop().size(); ++m)
        {
          sopMap[n].push_back(m);
        }
      }

      std::vector<int> dims;
      dims.push_back(sopMap[0].size());
      dims.push_back(sopMap[1].size());

      std::vector<int> sops(3);
      for (MatrixRatingIterator<EqualMatrixRater> mri(dims); !mri.atEnd(); mri.next())
      {
        const std::vector<int>& value = mri.value();
        sops[0] = sopMap[0][value[0]];
        sops[1] = sopMap[1][value[1]];
        sops[2] = sopMap[0][value[0]];
        const std::vector<int>& flightMatrixPos = mri.value();
        ShoppingTrx::EstimatedSolution estimate(flightMatrixPos, gfp);
        estM[sops] = estimate;
      }
    }
    else
    {
      for (size_t n = 0; n != trx->legs().front().sop().size(); ++n)
      {
        std::vector<int> sops;
        sops.push_back(2 * n);
        const std::vector<int>& flightMatrixPos = sops;
        ShoppingTrx::EstimatedSolution estimate(flightMatrixPos, gfp);
        estM[sops] = estimate;
      }
    }
    return estM;
  }

protected:
  TestMemHandle _memHandle;
  RefDataHandle _dataHandle;
  PricingOrchestratorDerived* _po;
  ShoppingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingOrchestrator_MoveEstimatedSolutionsToFlightMatrixTest);
}
