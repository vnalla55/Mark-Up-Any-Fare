// ----------------------------------------------------------------
//
//   Copyright Sabre 2010
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include <boost/assign/std/vector.hpp>

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Billing.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/OCEmdDataProvider.h"
#include "FreeBagService/AncillaryChargesDataStrategy.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/test/S5Builder.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using boost::assign::operator+=;

class AncillaryChargesDataStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryChargesDataStrategyTest);
  CPPUNIT_TEST(test_S5SortComparator_equal_description1_numeric);
  CPPUNIT_TEST(test_S5SortComparator_equal_description1_alpha);
  CPPUNIT_TEST(test_S5SortComparator_equal_description1_alphanumeric);
  CPPUNIT_TEST(test_S5SortComparator);
  CPPUNIT_TEST(test_S5SortComparator_PT);
  CPPUNIT_TEST(test_S5SortComparator_empty_vector);

  CPPUNIT_TEST(test_emdRecord_pass);
  CPPUNIT_TEST(test_emdRecord_failed);

  CPPUNIT_TEST(test_shouldvalidateEmd_CKI_yes);
  CPPUNIT_TEST(test_shouldvalidateEmd_CKI_no);
  CPPUNIT_TEST(test_shouldvalidateEmd_RES_FdoSdo_only);
  CPPUNIT_TEST(test_shouldvalidateEmd_RES_firstBag_andOther);
  CPPUNIT_TEST(test_shouldvalidateEmd_RES_yes);
  CPPUNIT_TEST(test_shouldvalidateEmd_RES_no);

  CPPUNIT_TEST(test_collectEmdData_2seg);
  CPPUNIT_TEST(test_collectEmdData_3seg);

  CPPUNIT_TEST(test_propagateEmdValidationResult_RES);
  CPPUNIT_TEST(test_propagateEmdValidationResult_CKI_emdPass);
  CPPUNIT_TEST(test_propagateEmdValidationResult_CKI_emdFailed);


  CPPUNIT_TEST_SUITE_END();

  class AncillaryChargesDataStrategyDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memHandle;

  public:
    AncillaryChargesDataStrategyDataHandleMock() {}

    ~AncillaryChargesDataStrategyDataHandleMock() { _memHandle.clear(); }

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      if (serviceGroup == "BG")
      {
        return getBgSubCode(vendor);
      }
      else if (serviceGroup == "PT")
      {
        return getPtSubCode(vendor);
      }
      else
      {
        std::vector<SubCodeInfo*>* subCodes = _memHandle.create<std::vector<SubCodeInfo*> >();
        const std::vector<SubCodeInfo*> bgSubCodes = getBgSubCode(vendor);
        const std::vector<SubCodeInfo*> ptSubCodes = getPtSubCode(vendor);
        std::copy(bgSubCodes.begin(), bgSubCodes.end(), back_inserter(*subCodes));
        std::copy(ptSubCodes.begin(), ptSubCodes.end(), back_inserter(*subCodes));
        return *subCodes;
      }
    }

  private:
    const std::vector<SubCodeInfo*>& getBgSubCode(const VendorCode& vendor)
    {
      std::vector<SubCodeInfo*>* bgSubCodes = _memHandle.create<std::vector<SubCodeInfo*> >();

      if (vendor == "ATP")
      {
        bgSubCodes->push_back(
            getS5("ATP", "BG", "EX", "0AA", 'X', ' ', 'C', '2', "1", 'C', 'C')); // match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "MR", "0ED", 'X', ' ', 'C', '2', "3", 'C')); // match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "EX", "0D6", 'X', ' ', 'C', '4', "3", 'C')); // match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "MR", "0AG", 'X', ' ', 'C', '2', "3", 'C', 'C')); // match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "EX", "0AI", 'X', ' ', 'C', '4', "1", 'C')); // match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "EX", "0AI", 'X', 'F', 'C', '4', "1", 'C')); // not match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "EX", "0AW", 'X', ' ', 'C', '2', "1", 'A')); // not match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "MR", "0AX", 'X', ' ', 'C', '2', "2", 'C')); // not match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "CY", "0A3", 'X', ' ', 'C', '4', "3", 'C')); // not match
        bgSubCodes->push_back(
            getS5("ATP", "BG", "CY", "0AE", '1', ' ', 'A', '2', "3", 'C')); // not match
      }
      else if (vendor == "MMGR")
      {
        bgSubCodes->push_back(
            getS5("MMGR", "BG", "EX", "0AA", 'X', ' ', 'C', '2', "1", 'C')); // match
      }

      return *bgSubCodes;
    }

    const std::vector<SubCodeInfo*>& getPtSubCode(const VendorCode& vendor)
    {
      std::vector<SubCodeInfo*>* ptSubCodes = _memHandle.create<std::vector<SubCodeInfo*> >();

      if (vendor == "ATP")
      {
        ptSubCodes->push_back(
            getS5("ATP", "PT", "MR", "0AA", 'X', ' ', 'C', '2', "5", 'C')); // not match
        ptSubCodes->push_back(
            getS5("ATP", "PT", "CY", "0AE", 'X', ' ', 'C', '4', "3", 'C')); // not match
        ptSubCodes->push_back(
            getS5("ATP", "PT", "CY", "0A6", '1', ' ', 'A', '2', "3", 'C')); // not match
        ptSubCodes->push_back(
            getS5("ATP", "PT", "CY", "0AG", '1', ' ', 'C', '5', "1", 'C')); // not match
        ptSubCodes->push_back(
            getS5("ATP", "PT", "CY", "0AW", '1', ' ', 'E', '2', "2", 'C')); // not match
        ptSubCodes->push_back(
            getS5("ATP", "PT", "CY", "0AI", '1', ' ', 'G', '5', "5", 'C')); // not match
      }
      else if (vendor == "MMGR")
      {
        ptSubCodes->push_back(
            getS5("MMGR", "PT", "EX", "0AE", 'X', ' ', 'C', '4', "3", 'C')); // match
      }

      return *ptSubCodes;
    }

    SubCodeInfo* getS5(VendorCode vendor,
                       ServiceGroup sg,
                       ServiceGroup ssg,
                       ServiceSubTypeCode sstc,
                       Indicator concur,
                       Indicator ssim,
                       Indicator rfic,
                       Indicator emd,
                       ServiceBookingInd booking,
                       Indicator fltTktMerchInd,
                       Indicator industryCarrierInd = 'I')
    {
      return S5Builder(&_memHandle)
          .withVendor(vendor)
          .withGroup(sg, ssg)
          .withSubCode(sstc)
          .withCodes(concur, ssim, rfic, emd, booking)
          .withFltTktMerchInd(fltTktMerchInd)
          .withIndustryCarrier(industryCarrierInd)
          .build();
    }
  };

  class AncillaryChargesDataStrategyMock : public AncillaryChargesDataStrategy
  {
  public:
    AncillaryChargesDataStrategyMock(PricingTrx& trx) : AncillaryChargesDataStrategy(trx) {}
    std::string getEmdValidatingCarrier() const { return "VA"; }


  };

private:
  AncillaryChargesDataStrategyDataHandleMock* _dataHandleMock;
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _ancTrx;
  AncRequest* _ancRequest;
  AncillaryChargesDataStrategy* _dataStrategy;

public:
  void setUp()
  {
    MockGlobal::setMetricsMan(_memHandle.create<tse::MetricsMan>());
    _memHandle.create<TestConfigInitializer>();

    _ancTrx = _memHandle.create<AncillaryPricingTrx>();
    _dataHandleMock = _memHandle.create<AncillaryChargesDataStrategyDataHandleMock>();
    _dataStrategy = _memHandle.create<AncillaryChargesDataStrategyMock>(*_ancTrx);
    _ancRequest = _memHandle.create<AncRequest>();
    _ancTrx->setRequest(_ancRequest);
    _ancTrx->billing() = _memHandle.create<Billing>();
    _ancTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    (static_cast<AncRequest*>(_ancTrx->getRequest()))->ancRequestType() = AncRequest::WPBGRequest;
  }

  void tearDown()
  {
    MockGlobal::clear();
    _memHandle.clear();
  }

  SubCodeInfo* getS5(VendorCode vendor,
                     ServiceGroup sg,
                     ServiceGroup ssg,
                     ServiceSubTypeCode sstc,
                     Indicator concur,
                     Indicator ssim,
                     Indicator rfic,
                     Indicator emd,
                     ServiceBookingInd booking,
                     Indicator fltTktMerchInd,
                     Indicator industryCarrierInd = 'I')
  {
    return S5Builder(&_memHandle)
        .withVendor(vendor)
        .withGroup(sg, ssg)
        .withSubCode(sstc)
        .withCodes(concur, ssim, rfic, emd, booking)
        .withFltTktMerchInd(fltTktMerchInd)
        .withIndustryCarrier(industryCarrierInd)
        .build();
  }

  void test_S5SortComparator_equal_description1_numeric()
  {
    std::vector<const SubCodeInfo*> vector;
    vector.push_back(
        S5Builder(&_memHandle).withSubCode("0AE").withGroup("BG").withDesc("2").build());
    vector.push_back(
        S5Builder(&_memHandle).withSubCode("0AA").withGroup("BG").withDesc("2").build());

    _dataStrategy->sort(vector);

    std::vector<const SubCodeInfo*> expectedResult;
    expectedResult.push_back(
        S5Builder(&_memHandle).withSubCode("0AA").withGroup("BG").withDesc("2").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withSubCode("0AE").withGroup("BG").withDesc("2").build());

    CPPUNIT_ASSERT(vector.size() == expectedResult.size());

    int index = 0;
    for (const SubCodeInfo* subCode : vector)
    {
      CPPUNIT_ASSERT(*subCode == *expectedResult[index++]);
    }
  }

  void test_S5SortComparator_equal_description1_alpha()
  {
    std::vector<const SubCodeInfo*> vector;
    vector.push_back(S5Builder(&_memHandle)
                         .withSubCode("0AE")
                         .withGroup("BG")
                         .withDesc("AA")
                         .build());
    vector.push_back(S5Builder(&_memHandle)
                         .withSubCode("0AA")
                         .withGroup("BG")
                         .withDesc("AA")
                         .build());

    _dataStrategy->sort(vector);

    std::vector<const SubCodeInfo*> expectedResult;
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withSubCode("0AA")
                                 .withGroup("BG")
                                 .withDesc("AA")
                                 .build());
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withSubCode("0AE")
                                 .withGroup("BG")
                                 .withDesc("AA")
                                 .build());

    CPPUNIT_ASSERT(vector.size() == expectedResult.size());

    int index = 0;
    for (const SubCodeInfo* subCode : vector)
    {
      CPPUNIT_ASSERT(*subCode == *expectedResult[index++]);
    }
  }

  void test_S5SortComparator_equal_description1_alphanumeric()
  {
    std::vector<const SubCodeInfo*> vector;
    vector.push_back(S5Builder(&_memHandle)
                         .withSubCode("0AE")
                         .withGroup("BG")
                         .withDesc("A2")
                         .build());
    vector.push_back(S5Builder(&_memHandle)
                         .withSubCode("0AA")
                         .withGroup("BG")
                         .withDesc("A2")
                         .build());

    _dataStrategy->sort(vector);

    std::vector<const SubCodeInfo*> expectedResult;
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withSubCode("0AA")
                                 .withGroup("BG")
                                 .withDesc("A2")
                                 .build());
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withSubCode("0AE")
                                 .withGroup("BG")
                                 .withDesc("A2")
                                 .build());

    CPPUNIT_ASSERT(vector.size() == expectedResult.size());

    int index = 0;
    for (const SubCodeInfo* subCode : vector)
    {
      CPPUNIT_ASSERT(*subCode == *expectedResult[index++]);
    }
  }

  void test_S5SortComparator()
  {
    std::vector<const SubCodeInfo*> vector;

    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("AX").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("PT").withDesc("2A").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("2").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("PT").withDesc("").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("AA").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("2D").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("7X").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("12").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("11").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("X0").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("X0").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("2V").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("X1").build());
    vector.push_back(S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("3").build());

    _dataStrategy->sort(vector);

    std::vector<const SubCodeInfo*> expectedResult;
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("2").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("12").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("2V").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("7X").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("X0").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("X1").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("BG").withDesc("AX").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("3").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("11").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("2D").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("X0").build());
    expectedResult.push_back(
        S5Builder(&_memHandle).withGroup("BG", "SP").withDesc("AA").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("PT").withDesc("2A").build());
    expectedResult.push_back(S5Builder(&_memHandle).withGroup("PT").withDesc("").build());

    CPPUNIT_ASSERT(vector.size() == expectedResult.size());

    int index = 0;
    for (const SubCodeInfo* subCode : vector)
    {
      CPPUNIT_ASSERT(*subCode == *expectedResult[index++]);
    }
  }

  void test_S5SortComparator_PT()
  {
    std::vector<const SubCodeInfo*> vector;

    vector.push_back(S5Builder(&_memHandle)
                         .withGroup("PT")
                         .withDesc("")
                         .withCommercialName("PET IN HOLD")
                         .build());
    vector.push_back(S5Builder(&_memHandle)
                         .withGroup("PT")
                         .withDesc("")
                         .withCommercialName("PET IN CABIN")
                         .build());
    vector.push_back(S5Builder(&_memHandle)
                         .withGroup("PT")
                         .withDesc("")
                         .withCommercialName("PET IN HOLD UP TO 80KG")
                         .build());
    vector.push_back(S5Builder(&_memHandle)
                         .withGroup("PT")
                         .withDesc("")
                         .withCommercialName("SMALL PET IN HOLD UP TO 8KG")
                         .build());
    vector.push_back(S5Builder(&_memHandle)
                         .withGroup("PT")
                         .withDesc("")
                         .withCommercialName("CHECKED KENNEL SMALL")
                         .build());

    _dataStrategy->sort(vector);

    std::vector<const SubCodeInfo*> expectedResult;

    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withGroup("PT")
                                 .withDesc("")
                                 .withCommercialName("PET IN HOLD")
                                 .build());
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withGroup("PT")
                                 .withDesc("")
                                 .withCommercialName("PET IN CABIN")
                                 .build());
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withGroup("PT")
                                 .withDesc("")
                                 .withCommercialName("PET IN HOLD UP TO 80KG")
                                 .build());
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withGroup("PT")
                                 .withDesc("")
                                 .withCommercialName("SMALL PET IN HOLD UP TO 8KG")
                                 .build());
    expectedResult.push_back(S5Builder(&_memHandle)
                                 .withGroup("PT")
                                 .withDesc("")
                                 .withCommercialName("CHECKED KENNEL SMALL")
                                 .build());

    CPPUNIT_ASSERT(vector.size() == expectedResult.size());

    int index = 0;
    for (const SubCodeInfo* subCode : vector)
    {
      CPPUNIT_ASSERT(*subCode == *expectedResult[index++]);
    }
  }

  void test_S5SortComparator_empty_vector()
  {
    std::vector<const SubCodeInfo*> s5vector;
    _dataStrategy->sort(s5vector);
    CPPUNIT_ASSERT(s5vector.empty());
  }

  BaggageCharge* buildBaggageCharge(Indicator emdType)
  {
    BaggageCharge* bc = _memHandle.create<BaggageCharge>();
    SubCodeInfo* sc = _memHandle.create<SubCodeInfo>();
    bc->optFee() = _memHandle.create<OptionalServicesInfo>();
    sc->emdType() = emdType;
    bc->subCodeInfo() = sc;
    return bc;
  }

  void test_emdRecord_pass()
  {
    CPPUNIT_ASSERT(_dataStrategy->isEmdRecord(buildBaggageCharge('2')));
    CPPUNIT_ASSERT(_dataStrategy->isEmdRecord(buildBaggageCharge('3')));
  }

  void test_emdRecord_failed()
  {
    CPPUNIT_ASSERT(!_dataStrategy->isEmdRecord(buildBaggageCharge('1')));
    CPPUNIT_ASSERT(!_dataStrategy->isEmdRecord(buildBaggageCharge('4')));
    CPPUNIT_ASSERT(!_dataStrategy->isEmdRecord(buildBaggageCharge('5')));
  }


void test_shouldvalidateEmd_CKI_yes()
{
  BaggageTravel bt;
  AncRequestPath requestedPath = AncRequestPath::AncCheckInPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3'));

  CPPUNIT_ASSERT(_dataStrategy->shouldvalidateEmd(requestedPath, &bt));
}

void test_shouldvalidateEmd_CKI_no()
{
  BaggageTravel bt;
  AncRequestPath requestedPath = AncRequestPath::AncCheckInPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('4'));

  CPPUNIT_ASSERT(!_dataStrategy->shouldvalidateEmd(requestedPath, &bt));
}


void test_shouldvalidateEmd_RES_FdoSdo_only()
{
  BaggageTravel bt;
  AncRequestPath requestedPath = AncRequestPath::AncReservationPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('2')); bt._charges[0] = bt._chargeVector.back();
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3')); bt._charges[1] = bt._chargeVector.back();

  CPPUNIT_ASSERT(!_dataStrategy->shouldvalidateEmd(requestedPath, &bt));
}

void test_shouldvalidateEmd_RES_firstBag_andOther()
{
  BaggageTravel bt;
  AncRequestPath requestedPath = AncRequestPath::AncReservationPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('2')); bt._charges[0] = bt._chargeVector.back();
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3'));

  CPPUNIT_ASSERT(_dataStrategy->shouldvalidateEmd(requestedPath, &bt));
}

void test_shouldvalidateEmd_RES_yes()
{
  BaggageTravel bt;
  AncRequestPath requestedPath = AncRequestPath::AncReservationPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3'));

  CPPUNIT_ASSERT(_dataStrategy->shouldvalidateEmd(requestedPath, &bt));
}

void test_shouldvalidateEmd_RES_no()
{
  BaggageTravel bt;
  AncRequestPath requestedPath = AncRequestPath::AncReservationPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('4'));

  CPPUNIT_ASSERT(!_dataStrategy->shouldvalidateEmd(requestedPath, &bt));
}


AirSeg* createSeg(const CarrierCode& marketingCarrierCode, const CarrierCode& operatingCarrierCode)
{
  AirSeg* seg = _memHandle.create<AirSeg>();
  seg->setOperatingCarrierCode(operatingCarrierCode);
  seg->setMarketingCarrierCode(marketingCarrierCode);
  return seg;
}

void createSegments(std::vector<TravelSeg*>& container)
{
  container.push_back(createSeg("VA", "VA"));
  container.push_back(createSeg("VA", "DL"));
  container.push_back(createSeg("QF", "QF"));
}

void test_collectEmdData_2seg()
{
  std::vector<TravelSeg*> container;
  createSegments(container);

  BaggageTravel* baggageTravel = _memHandle.create<BaggageTravel>();
  baggageTravel->updateSegmentsRange(container.begin(), (container.begin() + 2));
  baggageTravel->_MSS = container.begin();

  OCEmdDataProvider oCEmdDataProvider;
  BaggageTravelInfo bagInfo(0, 0);
  _dataStrategy->collectEmdData(baggageTravel, bagInfo, oCEmdDataProvider, false, nullptr);

  CPPUNIT_ASSERT_EQUAL(1, (int)oCEmdDataProvider.marketingCarriers().size());
  CPPUNIT_ASSERT_EQUAL(2, (int)oCEmdDataProvider.operatingCarriers().size());

}

void test_collectEmdData_3seg()
{
  std::vector<TravelSeg*> container;
  createSegments(container);

  BaggageTravel* baggageTravel = _memHandle.create<BaggageTravel>();
  baggageTravel->updateSegmentsRange(container.begin(), container.end());
  baggageTravel->_MSS = container.begin();

  OCEmdDataProvider oCEmdDataProvider;
  BaggageTravelInfo bagInfo(0, 0);
  _dataStrategy->collectEmdData(baggageTravel, bagInfo, oCEmdDataProvider, false, nullptr);

  CPPUNIT_ASSERT_EQUAL(2, (int)oCEmdDataProvider.marketingCarriers().size());
  CPPUNIT_ASSERT_EQUAL(3, (int)oCEmdDataProvider.operatingCarriers().size());
}

void test_propagateEmdValidationResult_RES()
{
  BaggageTravel bt;
  BaggageTravelInfo bagInfo(0, 0);
  AncRequestPath requestedPath = AncRequestPath::AncReservationPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('2')); bt._charges[0] = bt._chargeVector.back();
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3'));

  _dataStrategy->propagateEmdValidationResult(false, requestedPath, &bt, bagInfo, nullptr);

  CPPUNIT_ASSERT_EQUAL(3, (int)bt._chargeVector.size());
  CPPUNIT_ASSERT(bt._chargeVector.end() == std::find_if(bt._chargeVector.begin(), bt._chargeVector.end(), [](BaggageCharge* bc) {return bc->subCodeInfo()->emdType() == '3'; } ));
  CPPUNIT_ASSERT(bt._chargeVector.end() != std::find_if(bt._chargeVector.begin(), bt._chargeVector.end(), [](BaggageCharge* bc) {return bc->subCodeInfo()->emdType() == '1'; } ));
  CPPUNIT_ASSERT(bt._chargeVector.end() != std::find_if(bt._chargeVector.begin(), bt._chargeVector.end(), [](BaggageCharge* bc) {return bc->subCodeInfo()->emdType() == '2'; } ));
  CPPUNIT_ASSERT(bt._chargeVector.end() != std::find_if(bt._chargeVector.begin(), bt._chargeVector.end(), [](BaggageCharge* bc) {return bc->subCodeInfo()->emdType() == '5'; } ));
}

void test_propagateEmdValidationResult_CKI_emdPass()
{
  BaggageTravel bt;
  BaggageTravelInfo bagInfo(0, 0);
  AncRequestPath requestedPath = AncRequestPath::AncCheckInPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('2')); bt._charges[0] = bt._chargeVector.back();
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3'));

  _dataStrategy->propagateEmdValidationResult(true, requestedPath, &bt, bagInfo, nullptr);
  CPPUNIT_ASSERT_EQUAL(4, (int)bt._chargeVector.size());

  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::NoEmdIndicator, (int)bt._chargeVector[0]->getEmdSoftPassChargeIndicator());
  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::EmdPassOrNoEmdValidation, (int)bt._chargeVector[1]->getEmdSoftPassChargeIndicator());
  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::NoEmdIndicator, (int)bt._chargeVector[2]->getEmdSoftPassChargeIndicator());
  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::EmdPassOrNoEmdValidation, (int)bt._chargeVector[3]->getEmdSoftPassChargeIndicator());
}

void test_propagateEmdValidationResult_CKI_emdFailed()
{
  BaggageTravel bt;
  BaggageTravelInfo bagInfo(0, 0);
  AncRequestPath requestedPath = AncRequestPath::AncCheckInPath;
  bt._chargeVector.push_back(buildBaggageCharge('1'));
  bt._chargeVector.push_back(buildBaggageCharge('2')); bt._charges[0] = bt._chargeVector.back();
  bt._chargeVector.push_back(buildBaggageCharge('5'));
  bt._chargeVector.push_back(buildBaggageCharge('3'));

  _dataStrategy->propagateEmdValidationResult(false, requestedPath, &bt, bagInfo, nullptr);
  CPPUNIT_ASSERT_EQUAL(4, (int)bt._chargeVector.size());

  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::NoEmdIndicator, (int)bt._chargeVector[0]->getEmdSoftPassChargeIndicator());
  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::EmdSoftPass, (int)bt._chargeVector[1]->getEmdSoftPassChargeIndicator());
  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::NoEmdIndicator, (int)bt._chargeVector[2]->getEmdSoftPassChargeIndicator());
  CPPUNIT_ASSERT_EQUAL((int)EmdSoftPassIndicator::EmdSoftPass, (int)bt._chargeVector[3]->getEmdSoftPassChargeIndicator());
}

};
CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryChargesDataStrategyTest);
} // tse
