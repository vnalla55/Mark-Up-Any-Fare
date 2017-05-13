
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/Shopping/Utils/SopDistributor.h"
#include "Pricing/Shopping/Utils/test/TestTrxBuilder.h"

#include <set>

using namespace std;
using namespace tse::utils;

namespace tse
{

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class TakeAll : public IPredicate<SopCandidate>
{
public:
  bool operator()(const SopCandidate& candidate) override
  {
    return true;
  }
};

class TakeCarrier : public IPredicate<SopCandidate>
{
public:
  TakeCarrier(const CarrierCode& code) : _code(code) {}
  bool operator()(const SopCandidate& candidate) override
  {
    return (candidate.carrierCode == _code);
  }

private:
  CarrierCode _code;
};

class TakeDirect : public IPredicate<SopCandidate>
{
public:
  bool operator()(const SopCandidate& candidate) override
  {
    return candidate.isFlightDirect;
  }
};

// Collector for testing purposes

class SimpleCollector : public ICollector<SopCandidate>
{
public:
  SimpleCollector(IPredicate<SopCandidate>& pred) : _pred(pred) {}
  void collect(const SopCandidate& sopInfo)
  {
    if (_pred(sopInfo))
    {
      _sops.insert(make_pair(sopInfo.legId, sopInfo.sopId));
    }
  }
  const set<LegSopTuple> getSops() const { return _sops; }

private:
  set<LegSopTuple> _sops;
  IPredicate<SopCandidate>& _pred;
};

class SopDistributorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SopDistributorTest);
  CPPUNIT_TEST(iterateWithNoCollectors);
  CPPUNIT_TEST(iterateWithOneCollector);
  CPPUNIT_TEST(iterateWithThreeCollectors);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _distributor = _memHandle.create<SopDistributor>();

    IPredicate<SopCandidate>* takeAll = _memHandle.create<TakeAll>();
    _all = _memHandle.create<SimpleCollector>(*takeAll);

    IPredicate<SopCandidate>* takeCarrier = _memHandle.create<TakeCarrier>("BB");
    _lineBB = _memHandle.create<SimpleCollector>(*takeCarrier);

    IPredicate<SopCandidate>* takeDir = _memHandle.create<TakeDirect>();
    _directs = _memHandle.create<SimpleCollector>(*takeDir);

    _builder = _memHandle.create<SimpleTransactionBuilder>(_memHandle);
    _trx = _builder->buildSimpleTrx();
  }

  void tearDown() { _memHandle.clear(); }

  void iterateWithNoCollectors()
  {
    CPPUNIT_ASSERT_THROW(_distributor->distributeSops(*_trx), tse::ErrorResponseException);
  }

  void iterateWithOneCollector()
  {
    _distributor->addCollector(_all);
    _distributor->distributeSops(*_trx);

    CPPUNIT_ASSERT(_builder->getAll() == _all->getSops());
  }

  void iterateWithThreeCollectors()
  {
    _distributor->addCollector(_all);
    _distributor->addCollector(_lineBB);
    _distributor->addCollector(_directs);
    _distributor->distributeSops(*_trx);

    CPPUNIT_ASSERT(_builder->getAll() == _all->getSops());
    CPPUNIT_ASSERT(_builder->getBB() == _lineBB->getSops());
    CPPUNIT_ASSERT(_builder->getDirects() == _directs->getSops());
  }

private:
  TestMemHandle _memHandle;
  SopDistributor* _distributor;

  SimpleTransactionBuilder* _builder;
  ShoppingTrx* _trx;

  SimpleCollector* _all;
  SimpleCollector* _lineBB;
  SimpleCollector* _directs;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SopDistributorTest);

} // namespace tse
