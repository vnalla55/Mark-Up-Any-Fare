#include <algorithm>

#include <boost/assign/std/vector.hpp>

#include "Common/MatrixUtils.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TseServerStub.h"

namespace tse
{

class ShoppingPQTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShoppingPQTest);
  CPPUNIT_TEST(testLookForMoreSolutions_NoSolutions_False);
  CPPUNIT_TEST(testLookForMoreSolutions_NoSolutions_OneSolNeeded);
  CPPUNIT_TEST(testLookForMoreSolutions_NoSolutions_TwoSolNeeded);
  CPPUNIT_TEST(testLookForMoreSolutions_TwoSolutions_TwoSolNeeded);
  CPPUNIT_TEST(testLookForMoreSolutions_TwoSolutions_ThreeSolNeeded);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_NoSolutions_OneSolNeeded);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_NoSolutions_NoSolNeeded);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_NoSolutions_TwoSolNeeded);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_OneSolutions_OneSolNeeded);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_OneSolutions_TwoSolNeeded);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_NoSolutions_TwoSolNeeded_Deactivate);
  CPPUNIT_SKIP_TEST(testLookForMoreSolutionsBeyond_DefaultInitialization);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<DataHandle>();
    _cxr = "DL";
    _po = _memHandle.create<PricingOrchestratorDerived>(*_memHandle.create<TseServerStub>());
    _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *_dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    _orderer = _memHandle.create<BitmapOpOrderer>(Global::config());
    _pq = _memHandle.create<ShoppingPQDerived>(*_po, *_trx, 4, 0, &_cxr, *_orderer);
  }

  void tearDown() { _memHandle.clear(); }

  void testLookForMoreSolutions_NoSolutions_False()
  {
    _pq->flightMatrix() = getEmptyFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(_pq->getFlightMatrix().size(), 0ul);

    CPPUNIT_ASSERT_EQUAL(_pq->lookForMoreSolutions(0), false);
  }

  void testLookForMoreSolutions_NoSolutions_OneSolNeeded()
  {
    _pq->flightMatrix() = getEmptyFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(0ul, _pq->getFlightMatrix().size());

    CPPUNIT_ASSERT_EQUAL(_pq->lookForMoreSolutions(1), true);
  }

  void testLookForMoreSolutions_NoSolutions_TwoSolNeeded()
  {
    _pq->flightMatrix() = getEmptyFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(0ul, _pq->getFlightMatrix().size());

    CPPUNIT_ASSERT_EQUAL(true, _pq->lookForMoreSolutions(2));
  }

  void testLookForMoreSolutions_TwoSolutions_TwoSolNeeded()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(*_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *_dataHandle, _trx->legs()[0], "DFW", "LON", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(*_dataHandle, _trx, _trx->legs()[1], 1, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *_dataHandle, _trx->legs()[1], "LON", "DFW", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(*_dataHandle, _trx, _trx->legs()[1], 2, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *_dataHandle, _trx->legs()[1], "LON", "DFW", "BA");

    _pq->flightMatrix() = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(_pq->getFlightMatrix().size()));

    CPPUNIT_ASSERT_EQUAL(false, _pq->lookForMoreSolutions(2));
  }

  void testLookForMoreSolutions_TwoSolutions_ThreeSolNeeded()
  {
    PricingOrchestratorTestShoppingCommon::createSOP(*_dataHandle, _trx, _trx->legs()[0], 0, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *_dataHandle, _trx->legs()[0], "DFW", "LON", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(*_dataHandle, _trx, _trx->legs()[1], 1, "AA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *_dataHandle, _trx->legs()[1], "LON", "DFW", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(*_dataHandle, _trx, _trx->legs()[1], 2, "BA");
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *_dataHandle, _trx->legs()[1], "LON", "DFW", "BA");

    _pq->flightMatrix() = getFullFlightMatrix(_trx);
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(_pq->getFlightMatrix().size()));

    CPPUNIT_ASSERT_EQUAL(true, _pq->lookForMoreSolutions(3));
  }

  void testLookForMoreSolutionsBeyond_NoSolutions_NoSolNeeded()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_cPoints.size()));
    _pq->_searchBeyondActivated = true;
    _pq->_noOfTaxCallsForDiffCnxPoints = 0;

    CPPUNIT_ASSERT_EQUAL(false, _pq->lookForMoreSolutionsBeyond());
    CPPUNIT_ASSERT_EQUAL(false, _pq->_searchBeyondActivated);
  }

  void testLookForMoreSolutionsBeyond_NoSolutions_OneSolNeeded()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_cPoints.size()));
    _pq->_searchBeyondActivated = true;
    _pq->_noOfTaxCallsForDiffCnxPoints = 1;

    CPPUNIT_ASSERT_EQUAL(true, _pq->lookForMoreSolutionsBeyond());
    CPPUNIT_ASSERT_EQUAL(true, _pq->_searchBeyondActivated);
  }

  void testLookForMoreSolutionsBeyond_NoSolutions_TwoSolNeeded()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_cPoints.size()));
    _pq->_searchBeyondActivated = true;
    _pq->_noOfTaxCallsForDiffCnxPoints = 2;

    CPPUNIT_ASSERT_EQUAL(true, _pq->lookForMoreSolutionsBeyond());
    CPPUNIT_ASSERT_EQUAL(true, _pq->_searchBeyondActivated);
  }

  void testLookForMoreSolutionsBeyond_OneSolutions_OneSolNeeded()
  {
    std::string key = "LON";
    MoneyAmount amount = 10;
    std::pair<std::string, MoneyAmount> pair(key, amount);
    _pq->_cPoints.insert(key);

    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_pq->_cPoints.size()));
    _pq->_searchBeyondActivated = true;
    _pq->_noOfTaxCallsForDiffCnxPoints = 1;

    CPPUNIT_ASSERT_EQUAL(false, _pq->lookForMoreSolutionsBeyond());
    CPPUNIT_ASSERT_EQUAL(false, _pq->_searchBeyondActivated);
    std::tr1::unordered_set<std::string, boost::hash<std::string> >::iterator it =
        _pq->_cPoints.find(key);
    CPPUNIT_ASSERT(_pq->_cPoints.end() != it);
  }

  void testLookForMoreSolutionsBeyond_OneSolutions_TwoSolNeeded()
  {
    std::string key = "LON";
    MoneyAmount amount = 10;
    std::pair<std::string, MoneyAmount> pair(key, amount);
    _pq->_cPoints.insert(key);

    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(_pq->_cPoints.size()));
    _pq->_searchBeyondActivated = true;
    _pq->_noOfTaxCallsForDiffCnxPoints = 2;

    CPPUNIT_ASSERT_EQUAL(true, _pq->lookForMoreSolutionsBeyond());
    CPPUNIT_ASSERT_EQUAL(true, _pq->_searchBeyondActivated);
  }

  void testLookForMoreSolutionsBeyond_NoSolutions_TwoSolNeeded_Deactivate()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_cPoints.size()));
    _pq->_searchBeyondActivated = false;
    _pq->_noOfTaxCallsForDiffCnxPoints = 2;

    CPPUNIT_ASSERT_EQUAL(false, _pq->lookForMoreSolutionsBeyond());
    CPPUNIT_ASSERT_EQUAL(false, _pq->_searchBeyondActivated);
  }

  void testLookForMoreSolutionsBeyond_DefaultInitialization()
  {
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_cPoints.size()));
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_noOfTaxCallsForDiffCnxPoints));
    CPPUNIT_ASSERT_EQUAL(false, _pq->_searchBeyondActivated);
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_fltCombMaxForSB));
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_farePathForRuleValMaxForSB));
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(_pq->_farePathForRuleValWithFltMaxForSB));
  }

protected:
  ShoppingTrx::FlightMatrix& getEmptyFlightMatrix(ShoppingTrx* trx) { return trx->flightMatrix(); }

  ShoppingTrx::FlightMatrix& getFullFlightMatrix(ShoppingTrx* trx)
  {
    ShoppingTrx::FlightMatrix& fltM = trx->flightMatrix();
    GroupFarePath* gfp = PricingOrchestratorTestShoppingCommon::buildGroupFarePath(*_dataHandle);
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

  TestMemHandle _memHandle;
  DataHandle* _dataHandle;
  CarrierCode _cxr;
  PricingOrchestratorDerived* _po;
  BitmapOpOrderer* _orderer;
  tse::ConfigMan _config;
  ShoppingPQDerived* _pq;
  ShoppingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ShoppingPQTest);

} // tse
