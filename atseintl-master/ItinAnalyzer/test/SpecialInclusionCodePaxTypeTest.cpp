#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <vector>
#include <set>
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/Itin.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "ItinAnalyzer/SpecialInclusionCodePaxType.h"
#include "Common/NonFatalErrorResponseException.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class MySpecialInclusionCodePaxType : public SpecialInclusionCodePaxType
{
private:
  std::set<std::pair<PaxTypeCode, VendorCode> > m_PaxTypesEmpty;
  std::set<std::pair<PaxTypeCode, VendorCode> > m_PaxTypesPsx;

  PaxTypeInfo _paxTypeInfo;

public:
  MySpecialInclusionCodePaxType()
  {
    m_PaxTypesPsx.insert(std::pair<PaxTypeCode, VendorCode>("ADT", "ATP"));
    m_PaxTypesPsx.insert(std::pair<PaxTypeCode, VendorCode>("CHD", "ATP"));
    m_PaxTypesPsx.insert(std::pair<PaxTypeCode, VendorCode>("MIL", "ATP"));
    m_PaxTypesPsx.insert(std::pair<PaxTypeCode, VendorCode>("INF", "ATP"));
  }
  virtual const std::set<std::pair<PaxTypeCode, VendorCode> >&
  getFareDisplayWebPaxForCxr(FareDisplayTrx& trx, const CarrierCode& carrier)
  {
    if (carrier == "EM")
      return m_PaxTypesEmpty;
    return m_PaxTypesPsx;
  }
  virtual const PaxTypeInfo*
  getPaxType(FareDisplayTrx& trx, const PaxTypeCode& paxType, const VendorCode& vendor)
  {
    _paxTypeInfo.vendor() = vendor;
    _paxTypeInfo.paxType() = paxType;
    _paxTypeInfo.childInd() = 'N';
    _paxTypeInfo.adultInd() = 'N';
    _paxTypeInfo.infantInd() = 'N';
    if (paxType == "CHD")
      _paxTypeInfo.childInd() = 'Y';
    else if (paxType == "INF")
      _paxTypeInfo.infantInd() = 'Y';
    else
      _paxTypeInfo.adultInd() = 'Y';
    _paxTypeInfo.initPsgType();
    return &_paxTypeInfo;
  }
};

class SpecialInclusionCodePaxTypeTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(SpecialInclusionCodePaxTypeTest);
  CPPUNIT_TEST(testNoWebFares);
  CPPUNIT_TEST(testWebFaresIsPax);
  CPPUNIT_TEST(testWebFaresIsAdtPax);
  CPPUNIT_TEST(testWebFaresIsChdPax);
  CPPUNIT_TEST(testWebFaresIsInfPax);
  CPPUNIT_TEST(testWebFaresIsChdInfPax);
  CPPUNIT_TEST(testAddonPaxType);
  CPPUNIT_TEST_SUITE_END();

public:
  // used for all tests
  void setUp()
  {
    _trx = _memHandle.create<FareDisplayTrx>();
    _request = _memHandle.create<FareDisplayRequest>();
    _options = _memHandle.create<FareDisplayOptions>();
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _specIncl = _memHandle.create<MySpecialInclusionCodePaxType>();
    _trx->getRequest()->inclusionCode() = "WEB";
    _trx->preferredCarriers().insert("13");
  }
  void tearDown() { _memHandle.clear(); }

  void testNoWebFares()
  {
    _trx->preferredCarriers().clear();
    _trx->preferredCarriers().insert("EM"); // no pax types

    CPPUNIT_ASSERT_THROW(_specIncl->getPaxType(*_trx), ErrorResponseException);
  }

  void testWebFaresIsPax()
  {
    _specIncl->getPaxType(*_trx);
    CPPUNIT_ASSERT(!_trx->getRequest()->passengerTypes().empty());
  }

  void testWebFaresIsAdtPax()
  {
    _specIncl->getPaxType(*_trx); // should be only adult pax types
    bool isCHD = false;
    bool isINF = false;
    std::set<PaxTypeCode>::iterator it = _trx->getRequest()->passengerTypes().begin();
    std::set<PaxTypeCode>::iterator ite = _trx->getRequest()->passengerTypes().end();
    for (; it != ite; it++)
    {
      if ((*it) == "INF")
        isINF = true;
      if ((*it) == "CHD")
        isCHD = true;
    }
    CPPUNIT_ASSERT(!isINF && !isCHD);
  }
  void testWebFaresIsChdPax()
  {
    _trx->getOptions()->childFares() = 'Y';
    _specIncl->getPaxType(*_trx); // should adult pax typesand child
    bool isCHD = false;
    bool isINF = false;
    std::set<PaxTypeCode>::iterator it = _trx->getRequest()->passengerTypes().begin();
    std::set<PaxTypeCode>::iterator ite = _trx->getRequest()->passengerTypes().end();
    for (; it != ite; it++)
    {
      if ((*it) == "INF")
        isINF = true;
      if ((*it) == "CHD")
        isCHD = true;
    }
    CPPUNIT_ASSERT(!isINF && isCHD);
  }
  void testWebFaresIsInfPax()
  {
    _trx->getOptions()->infantFares() = 'Y';
    _specIncl->getPaxType(*_trx); // should adult pax types and infant
    bool isCHD = false;
    bool isINF = false;
    std::set<PaxTypeCode>::iterator it = _trx->getRequest()->passengerTypes().begin();
    std::set<PaxTypeCode>::iterator ite = _trx->getRequest()->passengerTypes().end();
    for (; it != ite; it++)
    {
      if ((*it) == "INF")
        isINF = true;
      if ((*it) == "CHD")
        isCHD = true;
    }
    CPPUNIT_ASSERT(isINF && !isCHD);
  }
  void testWebFaresIsChdInfPax()
  {
    _trx->getOptions()->childFares() = 'Y';
    _trx->getOptions()->infantFares() = 'Y';
    _specIncl->getPaxType(*_trx); // should  pax types
    bool isCHD = false;
    bool isINF = false;
    std::set<PaxTypeCode>::iterator it = _trx->getRequest()->passengerTypes().begin();
    std::set<PaxTypeCode>::iterator ite = _trx->getRequest()->passengerTypes().end();
    for (; it != ite; it++)
    {
      if ((*it) == "INF")
        isINF = true;
      if ((*it) == "CHD")
        isCHD = true;
    }
    CPPUNIT_ASSERT(isINF && isCHD);
  }

  void testAddonPaxType()
  {
    _trx->getRequest()->inclusionCode() = "WEB";
    _specIncl->getPaxType(*_trx);
    // CPPUNIT_ASSERT(_trx->getRequest()->passengerTypes().size()==1);
    CPPUNIT_ASSERT(*(_trx->getRequest()->passengerTypes().begin()) == "ADT");
  }

private:
  SpecialInclusionCodePaxType* _specIncl;
  FareDisplayTrx* _trx;
  FareDisplayRequest* _request;
  FareDisplayOptions* _options;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SpecialInclusionCodePaxTypeTest);

} // namespace tse
