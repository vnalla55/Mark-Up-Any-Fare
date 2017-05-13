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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/YqYrPath.h"
#include "Rules/ServiceBaggageApplicator.h"
#include "Rules/ServiceBaggageRule.h"
#include "TestServer/Facades/ServiceBaggageServiceServer.h"
#include "test/PaymentDetailMock.h"

#include <memory>
#include <set>

namespace tax
{

namespace {

struct ProtectedFunctions : ServiceBaggageApplicator
{
  using ServiceBaggageApplicator::matchTaxRange;
  using ServiceBaggageApplicator::match;
};

type::TaxType emptyType(UninitializedCode);

} // anonymous namespace

class ServiceBaggageTest : public CppUnit::TestFixture
{
  typedef std::shared_ptr<ServiceBaggage> SharedServiceBaggage;
  CPPUNIT_TEST_SUITE(ServiceBaggageTest);

  CPPUNIT_TEST(ServiceBaggageNotFound);
  CPPUNIT_TEST(ServiceBaggageEmptyEntry);
  CPPUNIT_TEST(ServiceBaggagePositiveEntry);
  CPPUNIT_TEST(ServiceBaggageNegativeEntry);
  CPPUNIT_TEST(ServiceBaggagePositiveEntryWithYqYr);
  CPPUNIT_TEST(ServiceBaggageNegativeEntryWithYqYr);
  CPPUNIT_TEST(functionMatch);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _itemNo = new type::Index(1);
    _vendor = new type::Vendor("ATP");
    _rawPayments = new RawPayments(RawPayments::WithCapacity(10));

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
    _taxName->taxType() = "0AX";

    ((PaymentDetailMock*)_paymentDetail)->setTaxName(*_taxName);

    _yqYrs.resize(2);
    _yqYr1 = &_yqYrs[0];
    _yqYr2 = &_yqYrs[1];
    _yqYr1->amount() = type::MoneyAmount(20);
    _yqYr1->code() = "YQ";
    _yqYr1->type() = '1';
    _yqYr2->amount() = type::MoneyAmount(30);
    _yqYr2->code() = "YR";
    _yqYr2->type() = '2';

    _yqYrPath = new YqYrPath();
    _yqYrPath->yqYrUsages().push_back(YqYrUsage());
    _yqYrPath->yqYrUsages().back().index() = 0;
    _yqYrPath->yqYrUsages().push_back(YqYrUsage());
    _yqYrPath->yqYrUsages().back().index() = 1;

    _yqYrGeoPathMapping = new GeoPathMapping();
    _yqYrGeoPathMapping->mappings().resize(2);
    _yqYrGeoPathMapping->mappings()[0].maps().resize(2);
    _yqYrGeoPathMapping->mappings()[0].maps()[0].index() = 0;
    _yqYrGeoPathMapping->mappings()[0].maps()[1].index() = 1;
    _yqYrGeoPathMapping->mappings()[1].maps().resize(2);
    _yqYrGeoPathMapping->mappings()[1].maps()[0].index() = 2;
    _yqYrGeoPathMapping->mappings()[1].maps()[1].index() = 3;
  }

  void tearDown()
  {
    _rule.reset(0);
    _applicator.reset(0);
    delete _yqYrGeoPathMapping;
    delete _yqYrPath;
    delete _taxName;
    delete _rawPayments;
    delete _vendor;
    delete _itemNo;
    delete _paymentDetail;
    delete _geo1;
    delete _geo2;
    _geos.clear();
    std::vector<YqYr>().swap(_yqYrs);
  }

  TaxName makeTaxName(type::TaxCode taxCode, type::TaxType taxType)
  {
    TaxName ans;
    ans.taxCode() = taxCode;
    ans.taxType() = taxType;
    ans.taxPointTag() = type::TaxPointTag::Departure;
    return ans;
  }

  void emplacePayment(RawPayments& rawPayments, TaxName& taxName, type::Index beginId, type::Index endId)
  {
    static const PaymentRuleData prData(0, type::TicketedPointTag::MatchTicketedPointsOnly,
                                        TaxableUnitTagSet::none(), 0,
                                        type::CurrencyCode(UninitializedCode),
                                        type::TaxAppliesToTagInd::Blank);
    _geos.push_back(new Geo());
    Geo& taxPointBegin = _geos.back();
    taxPointBegin.id() = beginId;
    _geos.push_back(new Geo());
    Geo& taxPointEnd = _geos.back();
    taxPointEnd.id() = endId;

    rawPayments.emplace_back(prData, taxPointBegin, taxPointEnd, taxName);
  }

  void ServiceBaggageNotFound()
  {
    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    _applicator.reset(new ServiceBaggageApplicator(
        _rule.get(), 0, 0, 0, SharedServiceBaggage(), *_rawPayments));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void ServiceBaggageEmptyEntry()
  {
    SharedServiceBaggage sb;

    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    _applicator.reset(new ServiceBaggageApplicator(_rule.get(), 0, 0, 0, sb, *_rawPayments));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void ServiceBaggagePositiveEntry()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = "AA";
    sb->entries.back().taxTypeSubcode = "";

    TaxName taxName1 = makeTaxName("XA", emptyType);
    emplacePayment(*_rawPayments, taxName1, 1, 2);

    TaxName taxName2 = makeTaxName("AA", emptyType);
    emplacePayment(*_rawPayments, taxName2, 0, 3);

    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    _applicator.reset(new ServiceBaggageApplicator(_rule.get(), 0, 0, 0, sb, *_rawPayments));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void ServiceBaggageNegativeEntry()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";
    sb->entries.back().applTag = type::ServiceBaggageAppl::Negative;
    sb->entries.back().taxCode = "AA";
    sb->entries.back().taxTypeSubcode = "";
    sb->entries.push_back(new ServiceBaggageEntry());

    TaxName taxName1 = makeTaxName("XA", emptyType);
    emplacePayment(*_rawPayments, taxName1, 1, 2);
    TaxName taxName2 = makeTaxName("AA", emptyType);
    emplacePayment(*_rawPayments, taxName2, 0, 3);
    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    _applicator.reset(new ServiceBaggageApplicator(_rule.get(), 0, 0, 0, sb, *_rawPayments));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void ServiceBaggagePositiveEntryWithYqYr()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = "YQ";
    sb->entries.back().taxTypeSubcode = "";
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = "YR";
    sb->entries.back().taxTypeSubcode = "";

    TaxName taxName1 = makeTaxName("XY", emptyType);
    emplacePayment(*_rawPayments, taxName1, 1, 2);
    TaxName taxName2 = makeTaxName("YQ", emptyType);
    emplacePayment(*_rawPayments, taxName2, 0, 3);
    TaxName taxName3 = makeTaxName("YR", emptyType);
    emplacePayment(*_rawPayments, taxName3, 0, 3);
    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    _applicator.reset(new ServiceBaggageApplicator(
        _rule.get(), &_yqYrs, _yqYrPath, _yqYrGeoPathMapping, sb, *_rawPayments));

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void ServiceBaggageNegativeEntryWithYqYr()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Negative;
    sb->entries.back().taxCode = "YQ";
    sb->entries.back().taxTypeSubcode = "";
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Negative;
    sb->entries.back().taxCode = "YR";
    sb->entries.back().taxTypeSubcode = "";

    TaxName taxName1 = makeTaxName("XY", emptyType);
    emplacePayment(*_rawPayments, taxName1, 1, 2);
    TaxName taxName2 = makeTaxName("YQ", emptyType);
    emplacePayment(*_rawPayments, taxName2, 0, 3);
    TaxName taxName3 = makeTaxName("YR", emptyType);
    emplacePayment(*_rawPayments, taxName3, 0, 3);

    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    _applicator.reset(new ServiceBaggageApplicator(
        _rule.get(), &_yqYrs, _yqYrPath, _yqYrGeoPathMapping, sb, *_rawPayments));

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void functionMatch()
  {
    TaxName taxName = makeTaxName("AA", emptyType);
    emplacePayment(*_rawPayments, taxName, 0, 3);
    emplacePayment(*_rawPayments, taxName, 0, 3);
    emplacePayment(*_rawPayments, taxName, 0, 3);
    _rule.reset(new ServiceBaggageRule(*_itemNo, *_vendor));
    (*_rawPayments)[0].detail.getMutableItineraryDetail().setFailedRule(_rule.get());
    (*_rawPayments)[1].detail.setExempt();
    CPPUNIT_ASSERT(!ProtectedFunctions::match((*_rawPayments)[0].detail, (*_rawPayments)[0]));
    CPPUNIT_ASSERT(!ProtectedFunctions::match(*_paymentDetail, (*_rawPayments)[0]));
    CPPUNIT_ASSERT(!ProtectedFunctions::match(*_paymentDetail, (*_rawPayments)[1]));
    CPPUNIT_ASSERT( ProtectedFunctions::match(*_paymentDetail, (*_rawPayments)[2]));
  }

private:
  std::unique_ptr<ServiceBaggageRule> _rule;
  std::unique_ptr<ServiceBaggageApplicator> _applicator;

  PaymentDetail* _paymentDetail;
  RawPayments* _rawPayments;
  type::Index* _itemNo;
  type::Vendor* _vendor;
  Geo* _geo1;
  Geo* _geo2;
  std::vector<YqYr> _yqYrs;
  YqYrPath* _yqYrPath;
  YqYr* _yqYr1;
  YqYr* _yqYr2;
  GeoPathMapping* _yqYrGeoPathMapping;
  TaxName* _taxName;

  boost::ptr_vector<Geo> _geos;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceBaggageTest);
} // namespace tax
