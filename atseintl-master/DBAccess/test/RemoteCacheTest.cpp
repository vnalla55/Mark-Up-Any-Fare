#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "Common/TseSrvStats.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareDAO.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"
#include "DBAccess/RemoteCache/ASIOServer/Request.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"

const std::string host("localhost");

namespace tse
{

typedef HashKey<CarrierCode, BoostString, DateTime, DateTime> DummyHistoricalKey;

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

class FareMasterDAO : public FareDAO
{
public:
  DAOCache& cache() { return FareDAO::cache(); }

  virtual std::vector<const FareInfo*>* create(FareKey key) override
  {
    std::vector<const FareInfo*>* vect(new std::vector<const FareInfo*>);
    FareInfo* info1(new FareInfo);
    info1->inhibit() = 'N';
    info1->_effInterval.createDate() = DateTime(time(0)).subtractDays(100);
    info1->_effInterval.effDate() = info1->_effInterval.createDate();
    info1->_effInterval.expireDate() = DateTime(time(0)).addDays(100);
    info1->_effInterval.discDate() = info1->_effInterval.expireDate();
    info1->market1() = key._a;
    info1->market2() = key._b;
    info1->carrier() = key._c;
    info1->vendor() = "ATP";
    info1->fareClass() = "QWERTY1";
    vect->push_back(info1);
    FareInfo* info2(new FareInfo);
    info1->clone(*info2);
    info2->fareClass() = "ABCDEF1";
    vect->push_back(info2);
    return vect;
  }

  FareMasterDAO(int cacheSize, const std::string& cacheType) : FareDAO(cacheSize, cacheType)
  {
    cache().setName("Fare");
  }

  ~FareMasterDAO() {}

protected:
  friend class DAOHelper<FareMasterDAO>;
  static DAOHelper<FareMasterDAO> _helper;
  static FareMasterDAO* _instance;
};

DAOHelper<FareMasterDAO> FareMasterDAO::_helper(_name);
FareMasterDAO* FareMasterDAO::_instance(0);

class FareSlaveDAO : public FareDAO
{
public:
  FareSlaveDAO(int cacheSize, const std::string& cacheType) : FareDAO(cacheSize, cacheType)
  {
    cache().setName("Fare");
  }

  void setRemote(sfc::CompressedDataPtr remote) { _remote = remote; }

  virtual CreateResult<std::vector<const FareInfo*>> create(const FareKey& key, bool) override
  {
    CreateResult<std::vector<const FareInfo*>> result;
    result._compressed = _remote;
    result._status = RemoteCache::RC_COMPRESSED_VALUE;
    result._ptr = uncompress(*result._compressed);
    return result;
  }

  std::vector<const FareInfo*>* get(const FareKey& key) { return cache().get(key).get(); }

private:
  sfc::CompressedDataPtr _remote;
  static FareSlaveDAO* _instance;
  friend class DAOHelper<FareSlaveDAO>;
  static DAOHelper<FareSlaveDAO> _helper;
};

DAOHelper<FareSlaveDAO>
FareSlaveDAO::_helper(_name);
FareSlaveDAO*
FareSlaveDAO::_instance(0);

class RemoteCacheTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RemoteCacheTest);
  CPPUNIT_TEST(testRemoteCompressedToCompressed);
  CPPUNIT_TEST(testRemoteSimpleToSimple);
  CPPUNIT_TEST(testRemoteSimpleToCompressed);
  CPPUNIT_TEST(testRemoteCompressedToSimple);
  CPPUNIT_TEST(testRCRequest);
  CPPUNIT_TEST(testSerialization);
  CPPUNIT_TEST(testWriteReadHeader);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  std::string _dataType;
  CarrierCode _carrier;
  AccountCode _accountCode;
  DateTime _dt1;
  DateTime _dt2;
  uint32_t _daoVersion;

public:
  void setUp() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testRemote(const std::string& slaveType, const std::string& masterType)
  {
    FareMasterDAO masterDAO(100, masterType);
    FareSlaveDAO slaveDAO(100, slaveType);
    LocCode mkt1("DFW");
    LocCode mkt2("JFK");
    CarrierCode cxr("AA");
    FareKey key(mkt1, mkt2, cxr);
    WBuffer os;
    os.write(key);
    std::vector<char> payload(os.size());
    std::memcpy(&payload[0], os.buffer(), os.size());
    RBuffer is(payload);
    FareKey readKey;
    is.read(readKey);
    CPPUNIT_ASSERT(key == readKey);
    RemoteCache::RCStatus status;
    bool fromQuery(false);
    sfc::CompressedDataPtr compressed(masterDAO.getCompressed(is, status, fromQuery));
    CPPUNIT_ASSERT(compressed);
    slaveDAO.setRemote(compressed);
    std::vector<const FareInfo*>* res(slaveDAO.get(key));
    CPPUNIT_ASSERT(res != 0);
    CPPUNIT_ASSERT(res->size() > 0);
  }

  void testWriteRead(RemoteCache::RCRequest& request,
                     const char* requestPayload,
                     size_t requestPayloadSz,
                     uint32_t version)
  {
    request._requestVector.resize(RemoteCache::_headerSz + requestPayloadSz);
    std::memcpy(&request._requestVector[RemoteCache::_headerSz], requestPayload, requestPayloadSz);
    RemoteCache::RemoteCacheHeader requestHeader(0);
    requestHeader._daoVersion = version;
    requestHeader._payloadSize = requestPayloadSz;
    RemoteCache::writeHeader(requestHeader, request._requestVector);
    std::vector<boost::asio::const_buffer> sendBuffers;
    size_t requestSize(request.fillSendBuffers(sendBuffers));
    std::vector<char> requestData;
    requestData.insert(requestData.end(),
                       request._requestVector.begin(),
                       request._requestVector.begin() + RemoteCache::_headerSz);
    requestData.insert(requestData.end(),
                       request._requestVector.begin() + RemoteCache::_headerSz,
                       request._requestVector.end());
    CPPUNIT_ASSERT(requestSize == requestData.size());
    RemoteCache::RemoteCacheHeader headerOut(0);
    RemoteCache::readHeader(headerOut, request._requestVector);
    CPPUNIT_ASSERT(headerOut._daoVersion == _daoVersion);
    std::vector<char> payloadVector(requestData.begin() + RemoteCache::_headerSz, requestData.end());
    RBuffer is(payloadVector);
    std::string dataType;
    is.read(dataType);
    CPPUNIT_ASSERT(_dataType == dataType);
    std::string clientDatabase;
    is.read(clientDatabase);
    CPPUNIT_ASSERT(TseSrvStats::getCurrentDatabase() == clientDatabase);
    DummyHistoricalKey key;
    is.read(key);
    CPPUNIT_ASSERT(key._a == _carrier);
    CPPUNIT_ASSERT(key._b == _accountCode);
    CPPUNIT_ASSERT(key._c == _dt1);
    CPPUNIT_ASSERT(key._d == _dt2);
  }

  void testRCRequest()
  {
    _carrier = "LA";
    const char* accountCodeStr = "012345678901234567890123456789";
    _accountCode = accountCodeStr;
    CPPUNIT_ASSERT(_accountCode.length() == strlen(accountCodeStr));
    std::string dtStr("2013-11-07 23:13:44.000");
    _dt1 = dtStr;
    dtStr = "2014-03-16 22:12:37.000";
    _dt2 = dtStr;
    DummyHistoricalKey key(_carrier, _accountCode, _dt1, _dt2);
    _dataType.assign("FAREBYRULEAPPHISTORICAL");
    _daoVersion = 4;
    WBuffer os;
    os.write(_dataType);
    os.write(TseSrvStats::getCurrentDatabase());
    os.write(key);
    size_t requestPayloadSz(os.size());
    RemoteCache::RCRequest request(_dataType);
    testWriteRead(request, os.buffer(), requestPayloadSz, _daoVersion);
  }

  void testSerialization()
  {
    const char* charPtr = "012345678901234567890123456789";
    BoostString dummyOut(charPtr);
    WBuffer os;
    os.write(dummyOut);
    std::vector<char> buf;
    buf.resize(os.size());
    std::memcpy(&buf[0], os.buffer(), os.size());
    RBuffer is(buf);
    BoostString dummyIn;
    is.read(dummyIn);
    CPPUNIT_ASSERT(dummyOut == dummyIn);
  }

  void testWriteReadHeader()
  {
    uint32_t version(5);
    uint64_t payloadSize(12345678);
    uint64_t inflatedSize(87654321);
    RemoteCache::RemoteCacheHeader headerIn(0);
    headerIn._status = RemoteCache::RC_NONE;
    headerIn._daoVersion = version;
    headerIn._payloadSize = payloadSize;
    headerIn._inflatedSize = inflatedSize;
    std::vector<char> headerVector(RemoteCache::_headerSz);
    RemoteCache::writeHeader(headerIn, headerVector);
    RemoteCache::RemoteCacheHeader headerOut(0);
    CPPUNIT_ASSERT(RemoteCache::readHeader(headerOut, headerVector));
    CPPUNIT_ASSERT(headerOut._status == headerIn._status);
    CPPUNIT_ASSERT(headerOut._daoVersion == headerIn._daoVersion);
    CPPUNIT_ASSERT(headerOut._payloadSize == headerIn._payloadSize);
    CPPUNIT_ASSERT(headerOut._inflatedSize == headerIn._inflatedSize);
  }

  void testRemoteCompressedToCompressed()
  {
    testRemote(CACHE_TYPE_COMPRESSED, CACHE_TYPE_COMPRESSED);
  }

  void testRemoteSimpleToSimple() { testRemote(CACHE_TYPE_SIMPLE, CACHE_TYPE_SIMPLE); }

  void testRemoteSimpleToCompressed() { testRemote(CACHE_TYPE_COMPRESSED, CACHE_TYPE_SIMPLE); }

  void testRemoteCompressedToSimple() { testRemote(CACHE_TYPE_SIMPLE, CACHE_TYPE_COMPRESSED); }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RemoteCacheTest);
}
