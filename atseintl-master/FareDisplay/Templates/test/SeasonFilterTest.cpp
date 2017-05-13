#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/Agent.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/SeasonsInfo.h"
#include "DBAccess/Customer.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/Field.h"
#include "FareDisplay/Templates/SeasonFilter.h"
#include <vector>
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class SeasonFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SeasonFilterTest);

  CPPUNIT_TEST(testFormatDataY2009F1Abacus);
  CPPUNIT_TEST(testFormatDataY2009F2Abacus);
  CPPUNIT_TEST(testFormatDataY2009F3Abacus);
  CPPUNIT_TEST(testFormatDataY2009F4Abacus);
  CPPUNIT_TEST(testFormatDataY2009F5Abacus);
  CPPUNIT_TEST(testFormatDataY2010F1Abacus);
  CPPUNIT_TEST(testFormatDataY2010F2Abacus);
  CPPUNIT_TEST(testFormatDataY2010F3Abacus);
  CPPUNIT_TEST(testFormatDataY2010F4Abacus);
  CPPUNIT_TEST(testFormatDataY2010F5Abacus);
  CPPUNIT_TEST(testFormatDataY2011F1Abacus);
  CPPUNIT_TEST(testFormatDataY2011F2Abacus);
  CPPUNIT_TEST(testFormatDataY2011F3Abacus);
  CPPUNIT_TEST(testFormatDataY2011F4Abacus);
  CPPUNIT_TEST(testFormatDataY2011F5Abacus);
  CPPUNIT_TEST(testFormatDataY2020F1Abacus);
  CPPUNIT_TEST(testFormatDataY2020F2Abacus);
  CPPUNIT_TEST(testFormatDataY2020F3Abacus);
  CPPUNIT_TEST(testFormatDataY2020F4Abacus);
  CPPUNIT_TEST(testFormatDataY2020F5Abacus);
  CPPUNIT_TEST(testFormatDataY2023F1Abacus);
  CPPUNIT_TEST(testFormatDataY2023F2Abacus);
  CPPUNIT_TEST(testFormatDataY2023F3Abacus);
  CPPUNIT_TEST(testFormatDataY2023F4Abacus);
  CPPUNIT_TEST(testFormatDataY2023F5Abacus);
  CPPUNIT_TEST(testFormatDataY2009_2010F1Abacus);
  CPPUNIT_TEST(testFormatDataY2009_2010F2Abacus);
  CPPUNIT_TEST(testFormatDataY2009_2010F3Abacus);
  CPPUNIT_TEST(testFormatDataY2009_2010F4Abacus);
  CPPUNIT_TEST(testFormatDataY2009_2010F5Abacus);

  CPPUNIT_TEST(testFormatDataY2009F1Sabre);
  CPPUNIT_TEST(testFormatDataY2009F2Sabre);
  CPPUNIT_TEST(testFormatDataY2009F3Sabre);
  CPPUNIT_TEST(testFormatDataY2009F4Sabre);
  CPPUNIT_TEST(testFormatDataY2009F5Sabre);
  CPPUNIT_TEST(testFormatDataY2010F1Sabre);
  CPPUNIT_TEST(testFormatDataY2010F2Sabre);
  CPPUNIT_TEST(testFormatDataY2010F3Sabre);
  CPPUNIT_TEST(testFormatDataY2010F4Sabre);
  CPPUNIT_TEST(testFormatDataY2010F5Sabre);
  CPPUNIT_TEST(testFormatDataY2011F1Sabre);
  CPPUNIT_TEST(testFormatDataY2011F2Sabre);
  CPPUNIT_TEST(testFormatDataY2011F3Sabre);
  CPPUNIT_TEST(testFormatDataY2011F4Sabre);
  CPPUNIT_TEST(testFormatDataY2011F5Sabre);
  CPPUNIT_TEST(testFormatDataY2020F1Sabre);
  CPPUNIT_TEST(testFormatDataY2020F2Sabre);
  CPPUNIT_TEST(testFormatDataY2020F3Sabre);
  CPPUNIT_TEST(testFormatDataY2020F4Sabre);
  CPPUNIT_TEST(testFormatDataY2020F5Sabre);
  CPPUNIT_TEST(testFormatDataY2023F1Sabre);
  CPPUNIT_TEST(testFormatDataY2023F2Sabre);
  CPPUNIT_TEST(testFormatDataY2023F3Sabre);
  CPPUNIT_TEST(testFormatDataY2023F4Sabre);
  CPPUNIT_TEST(testFormatDataY2023F5Sabre);
  CPPUNIT_TEST(testFormatDataY2009_2010F1Sabre);
  CPPUNIT_TEST(testFormatDataY2009_2010F2Sabre);
  CPPUNIT_TEST(testFormatDataY2009_2010F3Sabre);
  CPPUNIT_TEST(testFormatDataY2009_2010F4Sabre);
  CPPUNIT_TEST(testFormatDataY2009_2010F5Sabre);

  CPPUNIT_TEST(testFormatDataY2009F1JLAxess);
  CPPUNIT_TEST(testFormatDataY2009F2JLAxess);
  CPPUNIT_TEST(testFormatDataY2009F3JLAxess);
  CPPUNIT_TEST(testFormatDataY2009F4JLAxess);
  CPPUNIT_TEST(testFormatDataY2009F5JLAxess);
  CPPUNIT_TEST(testFormatDataY2010F1JLAxess);
  CPPUNIT_TEST(testFormatDataY2010F2JLAxess);
  CPPUNIT_TEST(testFormatDataY2010F3JLAxess);
  CPPUNIT_TEST(testFormatDataY2010F4JLAxess);
  CPPUNIT_TEST(testFormatDataY2010F5JLAxess);
  CPPUNIT_TEST(testFormatDataY2011F1JLAxess);
  CPPUNIT_TEST(testFormatDataY2011F2JLAxess);
  CPPUNIT_TEST(testFormatDataY2011F3JLAxess);
  CPPUNIT_TEST(testFormatDataY2011F4JLAxess);
  CPPUNIT_TEST(testFormatDataY2011F5JLAxess);
  CPPUNIT_TEST(testFormatDataY2020F1JLAxess);
  CPPUNIT_TEST(testFormatDataY2020F2JLAxess);
  CPPUNIT_TEST(testFormatDataY2020F3JLAxess);
  CPPUNIT_TEST(testFormatDataY2020F4JLAxess);
  CPPUNIT_TEST(testFormatDataY2020F5JLAxess);
  CPPUNIT_TEST(testFormatDataY2023F1JLAxess);
  CPPUNIT_TEST(testFormatDataY2023F2JLAxess);
  CPPUNIT_TEST(testFormatDataY2023F3JLAxess);
  CPPUNIT_TEST(testFormatDataY2023F4JLAxess);
  CPPUNIT_TEST(testFormatDataY2023F5JLAxess);
  CPPUNIT_TEST(testFormatDataY2009_2010F1JLAxess);
  CPPUNIT_TEST(testFormatDataY2009_2010F2JLAxess);
  CPPUNIT_TEST(testFormatDataY2009_2010F3JLAxess);
  CPPUNIT_TEST(testFormatDataY2009_2010F4JLAxess);
  CPPUNIT_TEST(testFormatDataY2009_2010F5JLAxess);

  CPPUNIT_TEST_SUITE_END();

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

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _field = _memHandle.create<ElementField>();
    _fdtrx = _memHandle.create<FareDisplayTrx>();
  }

  void setUpAbacus()
  {
    _request = _memHandle.create<FareDisplayRequest>();
    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->agentTJR() = _customer;
    _request->ticketingAgent() = _agent;
    _fdtrx->setRequest(_request);
    _fdtrx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
    _fdtrx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
  }

  void setUpSabre()
  {
    _request = _memHandle.create<FareDisplayRequest>();
    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->agentTJR() = _customer;
    _request->ticketingAgent() = _agent;
    _fdtrx->setRequest(_request);
    _fdtrx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "SABR";
    _fdtrx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1S";
  }

  void setUpJLAxess()
  {
    _request = _memHandle.create<FareDisplayRequest>();
    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->agentTJR() = _customer;
    _request->ticketingAgent() = _agent;
    _fdtrx->setRequest(_request);
    _fdtrx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "AXES";
    _fdtrx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1J";
  }

  void tearDown()
  {
    _fields.clear();
    _memHandle.clear();
  }

  // Abacus

  void testFormatDataY2009F1Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F2Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F3Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC9-20OC9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F4Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT9-20OCT9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F5Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT9-20OCT9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F1Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F2Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F3Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC10-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F4Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT10-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F5Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT10-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F1Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F2Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F3Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC11-20OC1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F4Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT11-20OCT1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F5Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT11-20OCT1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F1Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F2Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F3Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC20-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F4Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT20-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F5Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT20-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F1Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F2Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F3Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC23-20OC3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F4Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT23-20OCT3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F5Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT23-20OCT3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F1Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F2Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F3Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC9-20OC10";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F4Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT9-20OCT10";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F5Abacus()
  {
    setUpAbacus();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT9-20OCT10";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  // Sabre

  void testFormatDataY2009F1Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F2Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F3Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC9-20OC9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F4Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT9-20OCT9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F5Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT9-20OCT9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F1Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F2Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F3Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC10-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F4Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT10-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F5Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT10-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F1Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F2Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F3Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC11-20OC1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F4Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT11-20OCT1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F5Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT11-20OCT1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F1Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F2Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F3Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC20-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F4Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT20-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F5Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT20-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F1Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F2Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F3Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC23-20OC3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F4Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT23-20OCT3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F5Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT23-20OCT3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F1Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F2Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F3Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC9-20OC10";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F4Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT9-20OCT10";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F5Sabre()
  {
    setUpSabre();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT9-20OCT10";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  // JLAxess

  void testFormatDataY2009F1JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F2JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F3JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC9-20OC9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F4JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT9-20OCT9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009F5JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2009);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT9-20OCT9";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F1JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F2JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F3JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC0-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F4JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT0-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2010F5JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2010, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT0-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F1JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F2JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F3JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC1-20OC1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F4JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT1-20OCT1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2011F5JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2011, 20, 10, 2011);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT1-20OCT1";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F1JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F2JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F3JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC0-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F4JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT0-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2020F5JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2020, 20, 10, 2020);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT0-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F1JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F2JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F3JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC3-20OC3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F4JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT3-20OCT3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2023F5JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2023, 20, 10, 2023);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT3-20OCT3";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F1JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '1');
    std::string dateVal = "10OC-20OC";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F2JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '2');
    std::string dateVal = "10OCT-20OCT";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F3JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '3');
    std::string dateVal = "10OC9-20OC0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F4JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '4');
    std::string dateVal = "10OCT9-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  void testFormatDataY2009_2010F5JLAxess()
  {
    setUpJLAxess();
    FareDisplayInfo* fdi = createFareDisplayInfoWithDate(10, 10, 2009, 20, 10, 2010);
    SeasonFilter::formatData(*fdi, *_field, _fields, *_fdtrx, '5');
    std::string dateVal = "10OCT9-20OCT0";
    CPPUNIT_ASSERT_EQUAL(dateVal, _fields.front().strValue());
  }

  FareDisplayInfo* createFareDisplayInfoWithDate(int32_t tvlStartDay,
                                                 int32_t tvlStartMonth,
                                                 int32_t tvlStartYear,
                                                 int32_t tvlStopDay,
                                                 int32_t tvlStopMonth,
                                                 int32_t tvlStopYear)
  {
    FareDisplayInfo* fdi = _memHandle.create<FareDisplayInfo>();

    SeasonsInfo* si = _memHandle.create<SeasonsInfo>();
    si->tvlstartDay() = tvlStartDay;
    si->tvlstartmonth() = tvlStartMonth;
    si->tvlstartyear() = tvlStartYear;
    si->tvlStopDay() = tvlStopDay;
    si->tvlStopmonth() = tvlStopMonth;
    si->tvlStopyear() = tvlStopYear;

    fdi->seasons().push_back(si);
    return fdi;
  }

protected:
  TestMemHandle _memHandle;
  ElementField* _field;
  FareDisplayTrx* _fdtrx;
  FareDisplayRequest* _request;
  Agent* _agent;
  Customer* _customer;
  std::vector<ElementField> _fields;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SeasonFilterTest);
}
