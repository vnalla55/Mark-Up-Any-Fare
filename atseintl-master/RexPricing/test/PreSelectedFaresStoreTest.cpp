#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "RexPricing/PreSelectedFaresStore.h"
#include "RexPricing/test/MockFareCompInfo.h"
#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class PreSelectedFaresStoreTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PreSelectedFaresStoreTest);

  CPPUNIT_TEST(testStore);
  CPPUNIT_TEST(testRestore);
  CPPUNIT_TEST(testIsStored);
  CPPUNIT_TEST(testCleaning);
  CPPUNIT_TEST(testCompareKeys_FareComponent);
  CPPUNIT_TEST(testCompareKeys_FareMarket);
  CPPUNIT_TEST(testCompareKeys_Date);
  CPPUNIT_TEST(testCompareKeys_Equal);

  CPPUNIT_TEST_SUITE_END();

private:
  PreSelectedFaresStore* _store;
  MockFareCompInfo* _fc;

  std::vector<PreSelectedFaresStore::Key> _keys;
  std::vector<std::vector<PaxTypeFareWrapper> > _fares;
  std::vector<PreSelectedFaresStore::Item> _items;

  FareCompInfo* _fcAddress;
  FareCompInfo* _fcHighAddress;
  FareMarket* _fmAddress;
  FareMarket* _fmHighAddress;
  RexDateSeqStatus _dateStatus, _dateHighStatus;

  TestMemHandle _memH;

public:
  void setUp()
  {
    _store = _memH.create<PreSelectedFaresStore>();
    _fc = _memH.create<MockFareCompInfo>();

    _keys += PreSelectedFaresStore::Key(_fc, _fc->fareMarket(), ORIGINAL_TICKET_DATE),
        PreSelectedFaresStore::Key(_fc, _fc->fareMarket(), REISSUE_TICKET_DATE),
        PreSelectedFaresStore::Key(_fc, _fc->secondaryFareMarket(), ORIGINAL_TICKET_DATE);

    PaxTypeFare* ptf[] = {(PaxTypeFare*)1, (PaxTypeFare*)2, (PaxTypeFare*)3,
                          (PaxTypeFare*)4, (PaxTypeFare*)5, (PaxTypeFare*)6,
                          (PaxTypeFare*)7, (PaxTypeFare*)8, (PaxTypeFare*)9 };

    _fares += std::vector<PaxTypeFareWrapper>(ptf, ptf + 3),
        std::vector<PaxTypeFareWrapper>(ptf + 3, ptf + 7),
        std::vector<PaxTypeFareWrapper>(ptf + 7, ptf + 9);

    _items += PreSelectedFaresStore::Item(_fares[0]), PreSelectedFaresStore::Item(_fares[1]),
        PreSelectedFaresStore::Item(_fares[2]);

    _fcAddress = (FareCompInfo*)1;
    _fcHighAddress = (FareCompInfo*)2;
    _fmAddress = (FareMarket*)3;
    _fmHighAddress = (FareMarket*)4;
    _dateStatus = RexDateSeqStatus(5);
    _dateHighStatus = RexDateSeqStatus(6);
  }

  void tearDown() { _memH.clear(); }

  void testStore()
  {
    _store->store(*_fc, ORIGINAL_TICKET_DATE, _fares[0]);
    _store->store(*_fc, REISSUE_TICKET_DATE, _fares[1]);

    _fc->swapFareMarkets();

    _store->store(*_fc, ORIGINAL_TICKET_DATE, _fares[2]);

    CPPUNIT_ASSERT_EQUAL(_items[0], _store->_store[_keys[0]]);
    CPPUNIT_ASSERT_EQUAL(_items[1], _store->_store[_keys[1]]);
    CPPUNIT_ASSERT_EQUAL(_items[2], _store->_store[_keys[2]]);
  }

  void testRestore()
  {
    _store->_store[_keys[0]] = _items[0];
    _store->_store[_keys[1]] = _items[1];
    _store->_store[_keys[2]] = _items[2];

    std::vector<PaxTypeFareWrapper> fares;

    _store->restore(*_fc, ORIGINAL_TICKET_DATE, fares);
    CPPUNIT_ASSERT_EQUAL(_fares[0], fares);

    _store->restore(*_fc, REISSUE_TICKET_DATE, fares);
    CPPUNIT_ASSERT_EQUAL(_fares[1], fares);

    _fc->swapFareMarkets();

    _store->restore(*_fc, ORIGINAL_TICKET_DATE, fares);
    CPPUNIT_ASSERT_EQUAL(_fares[2], fares);
  }

  void testIsStored()
  {
    _store->store(*_fc, ORIGINAL_TICKET_DATE, _fares[0]);

    std::vector<PaxTypeFareWrapper> fares;
    std::vector<PaxTypeFareWrapper>::iterator endTO, endFROM;

    MockFareCompInfo fc;

    CPPUNIT_ASSERT(_store->restore(*_fc, ORIGINAL_TICKET_DATE, fares));
    CPPUNIT_ASSERT(!_store->restore(*_fc, REISSUE_TICKET_DATE, fares));
    CPPUNIT_ASSERT(!_store->restore(fc, ORIGINAL_TICKET_DATE, fares));

    _fc->swapFareMarkets();

    CPPUNIT_ASSERT(!_store->restore(*_fc, ORIGINAL_TICKET_DATE, fares));
  }

  void testCleaning()
  {
    CPPUNIT_ASSERT(_store->empty());

    _store->store(*_fc, ORIGINAL_TICKET_DATE, _fares[0]);

    CPPUNIT_ASSERT(!_store->empty());

    _store->clear();
    CPPUNIT_ASSERT(_store->empty());
  }

  template <typename T>
  void assertLess(const T& left, const T& right)
  {
    CPPUNIT_ASSERT(left < right);
    CPPUNIT_ASSERT(!(right < left));
  }

  void testCompareKeys_FareComponent()
  {
    assertLess(_fcAddress, _fcHighAddress);

    assertLess(PreSelectedFaresStore::Key(_fcAddress, _fmAddress, _dateStatus),
               PreSelectedFaresStore::Key(_fcHighAddress, _fmAddress, _dateStatus));
  }

  void testCompareKeys_FareMarket()
  {
    assertLess(_fmAddress, _fmHighAddress);

    assertLess(PreSelectedFaresStore::Key(_fcAddress, _fmAddress, _dateStatus),
               PreSelectedFaresStore::Key(_fcAddress, _fmHighAddress, _dateStatus));
  }

  void testCompareKeys_Date()
  {
    assertLess(_dateStatus, _dateHighStatus);

    assertLess(PreSelectedFaresStore::Key(_fcAddress, _fmAddress, _dateStatus),
               PreSelectedFaresStore::Key(_fcAddress, _fmAddress, _dateHighStatus));
  }

  void testCompareKeys_Equal()
  {
    PreSelectedFaresStore::Key key(_fcAddress, _fmAddress, _dateStatus), copy(key);

    CPPUNIT_ASSERT(!(key < copy));
    CPPUNIT_ASSERT(!(copy < key));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PreSelectedFaresStoreTest);
}
