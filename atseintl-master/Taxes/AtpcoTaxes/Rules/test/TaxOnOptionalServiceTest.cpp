// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/MakePaymentDetail.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "Rules/TaxOnOptionalServiceApplicator.h"
#include "Rules/TaxOnOptionalServiceRule.h"
#include "test/PaymentDetailMock.h"
#include "test/ServicesMock.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "TestServer/Facades/ServiceBaggageServiceServer.h"

#include <memory>
#include <set>

namespace tax
{

namespace {
  const type::OcSubCode NoOcSubCode {UninitializedCode};
}

class ServiceBaggageEntryBuilder
{
  ServiceBaggageEntry _serviceBaggageEntry;
public:
  ServiceBaggageEntryBuilder()
  {

  }

  ServiceBaggageEntryBuilder&
  setTaxCode(const type::TaxCode& taxCode)
  {
    _serviceBaggageEntry.taxCode = taxCode;
    return *this;
  }

  ServiceBaggageEntryBuilder&
  setTaxTypeSubcode(const type::TaxTypeOrSubCode& taxTypeSubCode)
  {
    _serviceBaggageEntry.taxTypeSubcode = taxTypeSubCode;
    return *this;
  }

  ServiceBaggageEntryBuilder&
  setGroup(const type::ServiceGroupCode& group)
  {
    _serviceBaggageEntry.group = group;
    return *this;
  }

  ServiceBaggageEntryBuilder&
  setSubGroup(const type::ServiceGroupCode& subGroup)
  {
    _serviceBaggageEntry.subGroup = subGroup;
    return *this;
  }

  ServiceBaggageEntryBuilder&
  setFeeOwnerCarrier(const type::CarrierCode& carrier)
  {
    _serviceBaggageEntry.feeOwnerCarrier = carrier;
    return *this;
  }

  ServiceBaggageEntryBuilder&
  setOptionalServiceTag(const type::OptionalServiceTag& tag)
  {
    _serviceBaggageEntry.optionalServiceTag = tag;
    return *this;
  }

  ServiceBaggageEntry
  build()
  {
    return _serviceBaggageEntry;
  }
};

class TaxOnOptionalServiceTest : public CppUnit::TestFixture
{
  typedef std::shared_ptr<ServiceBaggage> SharedServiceBaggage;

  CPPUNIT_TEST_SUITE(TaxOnOptionalServiceTest);

  CPPUNIT_TEST(OptionalServices_itemNotFound);
  CPPUNIT_TEST(OptionalServices_itemEmpty);
  CPPUNIT_TEST(OptionalServices_matchAll);
  CPPUNIT_TEST(OptionalServices_matchOne);
  CPPUNIT_TEST(OptionalServices_matchNone);
  CPPUNIT_TEST(OptionalServices_excludeNone);
  CPPUNIT_TEST(OptionalServices_excludeOne);
  CPPUNIT_TEST(OptionalServices_excludeAll);
  CPPUNIT_TEST(testCheckOptionalServices_FlightRelated);
  CPPUNIT_TEST(testCheckOptionalServices_BaggageCharges);
  CPPUNIT_TEST(testCheckBaggage_BaggageCharges_taxCode_empty);
  CPPUNIT_TEST(testCheckBaggage_BaggageCharges_taxCode_OC);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _itemNo.reset(new type::Index(1));
    _vendor.reset(new type::Vendor("ATP"));
    _rule.reset(new TaxOnOptionalServiceRule(*_itemNo, *_vendor));

    _geo1 = new Geo();
    _geo2 = new Geo();
    _geo1->id() = 0;
    _geo1->loc().tag() = type::TaxPointTag::Departure;
    _geo2->id() = 3;
    _geo2->loc().tag() = type::TaxPointTag::Arrival;
    ((PaymentDetailMock*)_paymentDetail)->setTaxPointBegin(*_geo1);
    ((PaymentDetailMock*)_paymentDetail)->setTaxPointEnd(*_geo2);

    _taxName = new TaxName();
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    _taxName->taxCode() = "AA";
    _taxName->taxType() = "001";

    ((PaymentDetailMock*)_paymentDetail)->setTaxName(*_taxName);
    AddOptionalService(*_paymentDetail).subCode("001").flightRelated().group("ML", "LM");
    AddOptionalService(*_paymentDetail).subCode("002").flightRelated().group("BG", "GB");
  }

  void tearDown()
  {
    delete _geo1;
    delete _geo2;
    delete _taxName;
    delete _paymentDetail;
  }

  void addOptionalServiceEntry(SharedServiceBaggage serviceBaggage,
                               bool isPositive,
                               const type::OcSubCode& subCode,
                               const type::OptionalServiceTag& optionalServiceTag,
                               const type::ServiceGroupCode& group,
                               const type::ServiceGroupCode& subGroup)
  {
    serviceBaggage->entries.push_back(new ServiceBaggage::entry_type());
    ServiceBaggage::entry_type& entry = serviceBaggage->entries.back();
    entry.applTag = isPositive ? type::ServiceBaggageAppl::Positive
                                : type::ServiceBaggageAppl::Negative;
    entry.taxCode = "OC";
    entry.taxTypeSubcode = subCode.asString();
    entry.optionalServiceTag = optionalServiceTag;
    entry.group = group;
    entry.subGroup = subGroup;
  }

  void OptionalServices_itemNotFound()
  {
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, SharedServiceBaggage(), _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail)); // all OC failed -> apply failed
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_itemEmpty()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_matchAll()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    addOptionalServiceEntry(sb, true, NoOcSubCode, type::OptionalServiceTag::FlightRelated, "", "");
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_matchOne()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    addOptionalServiceEntry(sb, true, "001", type::OptionalServiceTag::FlightRelated, "ML", "LM");
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_matchNone()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    addOptionalServiceEntry(sb, true, "003", type::OptionalServiceTag::FlightRelated, "AB", "BA");
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail)); // all OC failed -> apply failed
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_excludeNone()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    addOptionalServiceEntry(sb, false, "003", type::OptionalServiceTag::FlightRelated, "AB", "BA");
    addOptionalServiceEntry(sb, true, NoOcSubCode, type::OptionalServiceTag::FlightRelated, "", "");
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_excludeOne()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    addOptionalServiceEntry(sb, false, "001", type::OptionalServiceTag::FlightRelated, "ML", "LM");
    addOptionalServiceEntry(sb, true, NoOcSubCode, type::OptionalServiceTag::FlightRelated, "", "");
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void OptionalServices_excludeAll()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    addOptionalServiceEntry(sb, false, "001", type::OptionalServiceTag::FlightRelated, "ML", "LM");
    addOptionalServiceEntry(sb, false, "002", type::OptionalServiceTag::FlightRelated, "", "");
    addOptionalServiceEntry(sb, true, NoOcSubCode, type::OptionalServiceTag::FlightRelated, "", "");
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, sb, _fallbackService, true));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail)); // all OC failed -> apply failed
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[1].isFailed());
  }

  void testCheckOptionalServices_FlightRelated()
  {
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, SharedServiceBaggage(), _fallbackService, true));

    ServiceBaggageEntry serviceBaggageEntry = ServiceBaggageEntryBuilder()
        .setOptionalServiceTag(type::OptionalServiceTag::FlightRelated)
        .setTaxCode("OC")
        .build();

    OptionalService optionalService = MakeOptionalService().flightRelated();

    CPPUNIT_ASSERT(_applicator->checkOptionalService(serviceBaggageEntry, optionalService));
  }

  void testCheckOptionalServices_BaggageCharges()
  {
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, SharedServiceBaggage(), _fallbackService, true));

    ServiceBaggageEntry serviceBaggageEntry = ServiceBaggageEntryBuilder()
        .setOptionalServiceTag(type::OptionalServiceTag::BaggageCharge)
        .setTaxCode("OC")
        .build();

    OptionalService optionalService = MakeOptionalService().baggageCharge();

    CPPUNIT_ASSERT(_applicator->checkOptionalService(serviceBaggageEntry, optionalService));
  }

  void testCheckBaggage_BaggageCharges_taxCode_empty()
  {
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, SharedServiceBaggage(), _fallbackService, true));

    ServiceBaggageEntry serviceBaggageEntry = ServiceBaggageEntryBuilder()
        .setOptionalServiceTag(type::OptionalServiceTag::BaggageCharge)
        .build();

    OptionalService optionalService = MakeOptionalService().baggageCharge();

    CPPUNIT_ASSERT(!_applicator->checkOptionalService(serviceBaggageEntry, optionalService));
  }

  void testCheckBaggage_BaggageCharges_taxCode_OC()
  {
    _applicator.reset(new TaxOnOptionalServiceApplicator(*_rule, SharedServiceBaggage(), _fallbackService, true));

    ServiceBaggageEntry serviceBaggageEntry = ServiceBaggageEntryBuilder()
        .setOptionalServiceTag(type::OptionalServiceTag::BaggageCharge)
        .setTaxCode("OC")
        .build();

    OptionalService optionalService = MakeOptionalService().baggageCharge();

    CPPUNIT_ASSERT(_applicator->checkOptionalService(serviceBaggageEntry, optionalService));
  }

private:
  std::unique_ptr<TaxOnOptionalServiceRule> _rule;
  std::unique_ptr<TaxOnOptionalServiceApplicator> _applicator;
  std::unique_ptr<type::Index> _itemNo;
  std::unique_ptr<type::Vendor> _vendor;
  FallbackServiceServer _fallbackService;

  PaymentDetail* _paymentDetail;
  Geo* _geo1;
  Geo* _geo2;
  TaxName* _taxName;

};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnOptionalServiceTest);
} // namespace tax
