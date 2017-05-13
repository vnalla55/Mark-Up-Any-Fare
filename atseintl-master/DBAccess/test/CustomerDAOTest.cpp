#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "DBAccess/Customer.h"
#include "DBAccess/CustomerDAO.h"

namespace tse
{
class CustomerDAOMock : public CustomerDAO
{
public:
  static CustomerDAOMock& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return CustomerDAO::cache(); }

  virtual std::vector<Customer*>*
  create(CustomerKey key)
  {
    std::vector<Customer*>* customerList( new std::vector<Customer*> );
    const DateTime today    = DateTime::localTime();

    Customer* customer(new Customer);
    Customer::dummyData( *customer );

    customer->pseudoCity() = static_cast<PseudoCityCode>( key._a );
    customer->createDate() = today;
    customerList->push_back(customer);

    return customerList;
  }

private:
  static CustomerDAOMock* _instance;
  friend class DAOHelper<CustomerDAOMock>;
  static DAOHelper<CustomerDAOMock> _helper;
  CustomerDAOMock(int cacheSize = 0, const std::string& cacheType = "")
    : CustomerDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<CustomerDAOMock>
CustomerDAOMock::_helper(_name);
CustomerDAOMock*
CustomerDAOMock::_instance(0);

class SpecificTestConfigInitializer : public TestConfigInitializer
{
public:
  SpecificTestConfigInitializer()
  {
    DiskCache::initialize(_config);
    _memHandle.create<MockDataManager>();
  }

  ~SpecificTestConfigInitializer() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

class CustomerDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CustomerDAOTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInvalidate);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    MockGlobal::setStartTime();
  }

  void tearDown() { _memHandle.clear(); }

  void testGet()
  {
    CustomerDAOMock& dao(CustomerDAOMock::instance());
    DataHandle dh;
    const PseudoCityCode pcc = "02AA";
    const DateTime ticketDate = DateTime::localTime();

    const std::vector<Customer*>& customerList(
        dao.get(dh.deleteList(), pcc, ticketDate));

    CPPUNIT_ASSERT( 1 == customerList.size() );

    CPPUNIT_ASSERT_EQUAL( pcc, customerList[0]->pseudoCity() );

    // successive calls should bring the same entries
    const std::vector<Customer*>& customerList2(
        dao.get(dh.deleteList(), pcc, ticketDate));
    CPPUNIT_ASSERT( customerList.size() == customerList2.size());

    for (size_t i = 0; i < customerList.size(); ++i)
    {
      CPPUNIT_ASSERT( *customerList[i] == *customerList2[i] );
    }
  }

  void testInvalidate()
  {
    CustomerDAOMock& dao(CustomerDAOMock::instance());
    DataHandle dh;
    const PseudoCityCode pcc = "02AA";
    const DateTime ticketDate = DateTime::localTime();

    dao.get(dh.deleteList(), pcc, ticketDate);
    CustomerKey key( pcc );
    CPPUNIT_ASSERT( dao.cache().getIfResident( key ) != 0);
    dao.cache().invalidate( key );
    CPPUNIT_ASSERT( 0 == dao.cache().getIfResident( key ) );
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CustomerDAOTest);
}
