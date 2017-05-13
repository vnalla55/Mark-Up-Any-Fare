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

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "FreeBagService/AncillaryResultProcessor.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

const std::string TAXTEXT_1 = "BAGGAGE IS NOT ALLOWED";
const std::string TAXTEXT_2 = "//";
const std::string TAXTEXT_3 = "//03/0F3";
const std::string TAXTEXT_4 = "//03/0F7";

using boost::assign::operator+=;

class AncillaryResultProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryResultProcessorTest);
  CPPUNIT_TEST(test_retrieveS5Records);
  CPPUNIT_TEST(test_selectS5Record);
  CPPUNIT_TEST(test_addBaggageItemProperty);
  CPPUNIT_TEST(test_add196_parseFreeText);
  CPPUNIT_TEST(test_add196_parseFormattedText);
  CPPUNIT_TEST(test_isPartOfBaggageRoute_pass);
  CPPUNIT_TEST(test_isPartOfBaggageRoute_fail);
  CPPUNIT_TEST(test_serviceTypeNeeded);
  CPPUNIT_TEST(test_createGroup);
  CPPUNIT_TEST(test_getS5_Without_Allowance);
  CPPUNIT_TEST(test_getS5_With_Allowance);
  CPPUNIT_TEST(test_getGroup_NoSubCode);
  CPPUNIT_TEST(test_getGroup_Group_Exists);
  CPPUNIT_TEST(test_getGroup_Group_Doesnt_Exists);
  CPPUNIT_TEST_SUITE_END();

  class AncillaryResultProcessorDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memHandle;
    std::vector<SubCodeInfo*>* bgSubCodes;
    std::vector<SubCodeInfo*>* ptSubCodes;

  private:
    SubCodeInfo* getS5(ServiceGroup serviceGroup,
                       ServiceTypeCode serviceCode,
                       ServiceSubTypeCode serviceSubCode,
                       Indicator fltTkt)
    {
      return S5Builder(&_memHandle)
          .withSubCode(serviceSubCode)
          .withGroup(serviceGroup)
          .withTypeCode(serviceCode)
          .withFltTktMerchInd(fltTkt)
          .build();
    }

  public:
    AncillaryResultProcessorDataHandleMock()
    {
      // fill with some test data
      bgSubCodes = _memHandle.create<std::vector<SubCodeInfo*> >();
      bgSubCodes->push_back(getS5("BG", "0C", "0F2", 'C'));
      bgSubCodes->push_back(getS5("BG", "0C", "0F3", 'C'));
      bgSubCodes->push_back(getS5("BG", "0C", "0F4", 'C'));

      // fill with some test data
      ptSubCodes = _memHandle.create<std::vector<SubCodeInfo*> >();
    }

    ~AncillaryResultProcessorDataHandleMock() { _memHandle.clear(); }

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      if (serviceGroup == "BG")
        return *bgSubCodes;
      else
        return *ptSubCodes;
    }

    const TaxText* getTaxText(const VendorCode& vendor, int itemNo)
    {
      TaxText* taxText = _memHandle.create<TaxText>();

      switch (itemNo)
      {
      case 1:
        taxText->txtMsgs().push_back(TAXTEXT_1);
        break;
      case 2:
        taxText->txtMsgs().push_back(TAXTEXT_1);
        taxText->txtMsgs().push_back(TAXTEXT_2);
        break;
      case 3:
        taxText->txtMsgs().push_back(TAXTEXT_3);
        taxText->txtMsgs().push_back(TAXTEXT_2);
        break;
      default:
        taxText->txtMsgs().push_back(TAXTEXT_3);
      }

      return taxText;
    }
  };

private:
  AncillaryResultProcessorDataHandleMock* _dataHandleMock;
  TestMemHandle _memHandle;
  AncillaryPricingTrx* _ancTrx;
  AncRequest* _ancRequest;
  AncillaryResultProcessor* _processor;
  AncillaryResultProcessor::OCFeesInitializer* _ocFeesInit;
  BaggageTravel* _baggageTravel;
  OCFees* _ocFees;

  OptionalServicesInfo* getS7(VendorCode vendor,
                              CarrierCode carrier,
                              ServiceSubTypeCode serviceSubCode,
                              Indicator fltTkt,
                              uint32_t taxTblItemNo)
  {
    return S7Builder(&_memHandle)
        .withFltTktMerchInd(fltTkt)
        .withTaxTblItemNo(taxTblItemNo)
        .withVendCarr(vendor, carrier)
        .withSubTypeCode(serviceSubCode)
        .build();
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _ancTrx = _memHandle.create<AncillaryPricingTrx>();
    _dataHandleMock = _memHandle.create<AncillaryResultProcessorDataHandleMock>();
    _processor = _memHandle.create<AncillaryResultProcessor>(*_ancTrx);
    _ancRequest = _memHandle.create<AncRequest>();
    _ancTrx->setRequest(_ancRequest);
    _baggageTravel = _memHandle.create<BaggageTravel>();
    _ocFees = _memHandle.create<OCFees>();

    _ocFeesInit = _memHandle.insert(
        new AncillaryResultProcessor::OCFeesInitializer(*_ancTrx, *_baggageTravel, *_ocFees, 0));

    MockGlobal::setMetricsMan(_memHandle.create<tse::MetricsMan>());
  }

  void tearDown()
  {
    _memHandle.clear();
    MockGlobal::clear();
  }

  TravelSeg* getTravelSegment(char checkedPortionOfTravelIndicator)
  {
    TravelSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->setCheckedPortionOfTravelInd(checkedPortionOfTravelIndicator);
    return travelSeg;
  }

  Itin* getItin()
  {
    Itin* itin = _memHandle.create<Itin>();
    return itin;
  }

  void test_add196_parseFreeText()
  {
    OCFees::OCFeesSeg ocSeg;
    ocSeg._optFee = getS7("ATP", "AA", "0F3", 'A', 2);
    _ocFeesInit->add196(&ocSeg);

    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[0].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[0].getFreeText() == TAXTEXT_1);
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[1].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[1].getFreeText() == TAXTEXT_2);

    ocSeg._baggageItemProperties.erase(ocSeg._baggageItemProperties.begin(),
                                       ocSeg._baggageItemProperties.end());
  }

  void test_add196_parseFormattedText()
  {
    OCFees::OCFeesSeg ocSeg;
    ocSeg._optFee = getS7("ATP", "AA", "0F3", 'A', 3);
    _ocFeesInit->add196(&ocSeg);

    CPPUNIT_ASSERT(!ocSeg._baggageItemProperties[0].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[1].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[1].getFreeText() == TAXTEXT_2);
  }

  void test_addBaggageItemProperty()
  {
    OCFees::OCFeesSeg ocSeg;

    ocSeg._optFee = S7Builder(&_memHandle)
                        .withFltTktMerchInd(BAGGAGE_ALLOWANCE)
                        .withVendCarr("ATP", "AA")
                        .withSubTypeCode("0F3")
                        .build();

    _ocFeesInit->addBaggageItemProperty(&ocSeg, TAXTEXT_1);
    _ocFeesInit->addBaggageItemProperty(&ocSeg, TAXTEXT_2);
    _ocFeesInit->addBaggageItemProperty(&ocSeg, TAXTEXT_3);
    _ocFeesInit->addBaggageItemProperty(&ocSeg, TAXTEXT_4);

    CPPUNIT_ASSERT(ocSeg._baggageItemProperties.size() == 3);
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[0].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[0].getFreeText() == TAXTEXT_1);
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[1].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[1].getFreeText() == TAXTEXT_2);
    CPPUNIT_ASSERT(!ocSeg._baggageItemProperties[2].isFreeText());
    CPPUNIT_ASSERT(ocSeg._baggageItemProperties[2].getNumber() == 3);
  }

  RequestedOcFeeGroup*
  getRequestedOcFeeGroup(const ServiceGroup& serviceGroup, const Indicator& ancillaryServiceType)
  {
    RequestedOcFeeGroup* requestedOcFeeGroup = _memHandle.create<RequestedOcFeeGroup>();
    requestedOcFeeGroup->groupCode() = serviceGroup;
    requestedOcFeeGroup->addAncillaryServiceType(ancillaryServiceType);
    return requestedOcFeeGroup;
  }

  void test_selectS5Record()
  {
    OCFees::OCFeesSeg ocSeg;

    ocSeg._optFee = S7Builder(&_memHandle).withFltTktMerchInd(1).withVendCarr("ATP", "AA").build();

    const SubCodeInfo* s5_1 = _ocFeesInit->selectS5Record(&ocSeg, "0F3");

    CPPUNIT_ASSERT(s5_1->serviceSubTypeCode() == "0F3");
    CPPUNIT_ASSERT(s5_1->serviceTypeCode() == "0C");
    CPPUNIT_ASSERT(s5_1->serviceGroup() == "BG");
    CPPUNIT_ASSERT_EQUAL(s5_1->fltTktMerchInd(), BAGGAGE_CHARGE);

    const SubCodeInfo* s5_2 = _ocFeesInit->selectS5Record(&ocSeg, "07F");
    CPPUNIT_ASSERT(s5_2 == 0);
  }

  void test_retrieveS5Records()
  {
    VendorCode _vendorCode = "ATP";
    CarrierCode _carrierCode = "AA";
    std::vector<SubCodeInfo*> subCodes;
    _ocFeesInit->retrieveS5Records(_vendorCode, _carrierCode, subCodes);
    CPPUNIT_ASSERT(subCodes.size() == 3);
  }

  void test_isPartOfBaggageRoute_pass()
  {
    std::vector<TravelSeg*> travelSegments;
    travelSegments.push_back(getTravelSegment('F'));
    travelSegments.push_back(getTravelSegment('F'));
    travelSegments.push_back(getTravelSegment('T'));

    CPPUNIT_ASSERT(_processor->isPartOfBaggageRoute(
        BaggageTravelBuilder(&_memHandle).withTravelSegMore(travelSegments).build()));
  }

  void test_isPartOfBaggageRoute_fail()
  {
    std::vector<TravelSeg*> travelSegments;
    travelSegments.push_back(getTravelSegment('F'));
    travelSegments.push_back(getTravelSegment('F'));
    travelSegments.push_back(getTravelSegment('F'));

    CPPUNIT_ASSERT(!_processor->isPartOfBaggageRoute(
        BaggageTravelBuilder(&_memHandle).withTravelSegMore(travelSegments).build()));
  }

  void test_serviceTypeNeeded()
  {
    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    pricingOptions->serviceGroupsVec().push_back(*getRequestedOcFeeGroup("BG", 'A'));
    pricingOptions->serviceGroupsVec().push_back(*getRequestedOcFeeGroup("PT", 'C'));
    _ancTrx->setOptions(pricingOptions);

    SubCodeInfo* s5_1 = S5Builder(&_memHandle).withGroup("BG").build();
    CPPUNIT_ASSERT(_processor->serviceTypeNeeded(s5_1, 'A'));
    CPPUNIT_ASSERT(!_processor->serviceTypeNeeded(s5_1, 'C'));

    SubCodeInfo* s5_2 = S5Builder(&_memHandle).withGroup("PT").build();
    CPPUNIT_ASSERT(!_processor->serviceTypeNeeded(s5_2, 'A'));
    CPPUNIT_ASSERT(_processor->serviceTypeNeeded(s5_2, 'C'));
  }

  void test_createGroup()
  {
    ServiceGroup srvGrp("BG");
    ServiceFeesGroup* group = _processor->createGroup(srvGrp);
    CPPUNIT_ASSERT(group);
    CPPUNIT_ASSERT_EQUAL(srvGrp, group->groupCode());
  }

  void test_getS5_Without_Allowance()
  {
    BaggageTravel* bt = BaggageTravelBuilder(&_memHandle).withNoAllowance().build();

    CPPUNIT_ASSERT_EQUAL(static_cast<const SubCodeInfo*>(0), _processor->getS5(bt));
  }

  void test_getS5_With_Allowance()
  {
    const SubCodeInfo* allowance = S5Builder(&_memHandle).build();
    BaggageTravel* bt = BaggageTravelBuilder(&_memHandle).withAllowanceS5(allowance).build();

    CPPUNIT_ASSERT_EQUAL(allowance, _processor->getS5(bt));
  }

  void test_getGroup_NoSubCode()
  {
    std::vector<ServiceFeesGroup*> groups;
    CPPUNIT_ASSERT_EQUAL(static_cast<ServiceFeesGroup*>(0), _processor->getGroup(groups, 0));
  }

  void test_getGroup_Group_Exists()
  {
    ServiceGroup groupCode("BG");
    std::vector<ServiceFeesGroup*> groups;
    SubCodeInfo* subCode = S5Builder(&_memHandle).withGroup(groupCode).build();
    groups.push_back(_processor->createGroup(groupCode));

    ServiceFeesGroup* group = _processor->getGroup(groups, subCode);

    CPPUNIT_ASSERT(group);
    CPPUNIT_ASSERT_EQUAL(groupCode, group->groupCode());
  }

  void test_getGroup_Group_Doesnt_Exists()
  {
    ServiceGroup groupCode("BG");
    std::vector<ServiceFeesGroup*> groups;
    SubCodeInfo* subCode = S5Builder(&_memHandle).withGroup(groupCode).build();

    ServiceFeesGroup* group = _processor->getGroup(groups, subCode);

    CPPUNIT_ASSERT(group);
    CPPUNIT_ASSERT_EQUAL(groupCode, group->groupCode());
    CPPUNIT_ASSERT_EQUAL(groupCode, groups.front()->groupCode());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryResultProcessorTest);
} // tse
