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
#include <memory>
#include <set>
#include <stdexcept>

#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

#include "DataModel/Services/ServiceBaggage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/YqYrPath.h"
#include "DomainDataObjects/YqYr.h"
#include "Rules/TaxOnYqYrApplicator.h"
#include "Rules/TaxOnYqYrRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "test/PaymentDetailMock.h"
#include "DataModel/Common/Types.h"

namespace tax
{
using testing::_;
using testing::ByRef;
using testing::Const;
using testing::Return;
using testing::ReturnRef;

class TaxOnYqYrTest : public CppUnit::TestFixture
{
  typedef std::shared_ptr<ServiceBaggage> SharedServiceBaggage;

  CPPUNIT_TEST_SUITE(TaxOnYqYrTest);

  CPPUNIT_TEST(testServiceBaggageRestrictedButNotDefined);
  CPPUNIT_TEST(testServiceYQOnly);
  CPPUNIT_TEST(testServiceYROnly);
  CPPUNIT_TEST(testServiceAll);
  CPPUNIT_TEST(testServiceAll_TaxIncluded_NoVat);
  CPPUNIT_TEST(testServiceAll_TaxIncluded_Vat);
  CPPUNIT_TEST(testServiceAllWithFareBreaks);
  CPPUNIT_TEST(testServiceYR02Only);
  CPPUNIT_TEST(testServiceExceptYR02);
  CPPUNIT_TEST(testServiceNoEntries);
  CPPUNIT_TEST(testServiceNo1stSegment);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _geo1 = new Geo();
    _geo2 = new Geo();
    _geo1->id() = 4;
    _geo1->loc().tag() = type::TaxPointTag::Departure;
    _geo2->id() = 5;
    _geo2->loc().tag() = type::TaxPointTag::Arrival;
    _paymentDetail->setTaxPointLoc1(_geo1);
    _paymentDetail->setTaxPointLoc2(_geo2);
    _paymentDetail->setTaxPointEnd(_paymentDetail->getTaxPointLoc2());

    for (std::size_t i = 0; i <= 9; ++i)
    {
      _taxableUnitTags.push_back(type::TaxableUnitTag::Blank);
    }

    _yqYrs.resize(2);
    _yqYr1 = &_yqYrs[0];
    _yqYr2 = &_yqYrs[1];
    _yqYr1->_amount = 20;
    _yqYr1->_code = "YQ";
    _yqYr1->_type = '1';
    _yqYr2->_amount = 30;
    _yqYr2->_code = "YR";
    _yqYr2->_type = '2';

    TaxableYqYrs& taxableYqYrs = _paymentDetail->getMutableYqYrDetails();
    std::vector<type::Index> ids(2, 0);
    taxableYqYrs.init(_yqYrs, ids);
    _yqYr1 = &taxableYqYrs._subject[0];
    _yqYr2 = &taxableYqYrs._subject[1];
    taxableYqYrs._data[0]._taxPointLoc2 = _geo2;
    taxableYqYrs._data[0]._taxPointEnd = _geo2;
    taxableYqYrs._data[1]._taxPointLoc2 = _geo2;
    taxableYqYrs._data[1]._taxPointEnd = _geo2;

    taxableYqYrs._ranges.push_back(std::make_pair(0, 5));
    taxableYqYrs._ranges.push_back(std::make_pair(2, 7));

    _paymentDetail->getMutableTaxPointsProperties().resize(8);
    _paymentDetail->getMutableTaxPointsProperties()[0].isFirst = true;
    _paymentDetail->getMutableTaxPointsProperties()[0].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isLast = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;

    _rule.reset(new TaxOnYqYrRule(0, "ATP", type::TaxAppliesToTagInd::AllBaseFare));
  }

  void tearDown()
  {
    delete _paymentDetail;
    delete _geo1;
    delete _geo2;
    std::vector<TaxableYqYr>().swap(_yqYrs);
  }

  void testServiceBaggageRestrictedButNotDefined()
  {
    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1000, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, SharedServiceBaggage(), true);
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->getYqYrDetails()._taxableAmount);
  }

  void testServiceYQOnly()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = "YQ";
    sb->entries.back().taxTypeSubcode = "";

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceYROnly()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = "YR";
    sb->entries.back().taxTypeSubcode = "";

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceAll()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceAll_TaxIncluded_NoVat()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";

    _yqYr2->_taxIncluded = true;

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceAll_TaxIncluded_Vat()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";

    _yqYr2->_taxIncluded = true;
    _paymentDetail->setIsVatTax();

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->totalYqYrAmount());
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceAllWithFareBreaks()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(0, "ATP", type::TaxAppliesToTagInd::BetweenFareBreaks));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);

    _geo1->id() = 0;
    _geo2->id() = 5;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceYR02Only()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = "YR";
    sb->entries.back().taxTypeSubcode = "2";

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceExceptYR02()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Negative;
    sb->entries.back().taxCode = "YR";
    sb->entries.back().taxTypeSubcode = "2";
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceNoEntries()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    _rule.reset(new TaxOnYqYrRule(1, "ATP", type::TaxAppliesToTagInd::AllBaseFare));

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

  void testServiceNo1stSegment()
  {
    SharedServiceBaggage sb = std::make_shared<ServiceBaggage>();
    sb->entries.push_back(new ServiceBaggageEntry());
    sb->entries.back().applTag = type::ServiceBaggageAppl::Positive;
    sb->entries.back().taxCode = type::TaxCode(UninitializedCode);
    sb->entries.back().taxTypeSubcode = "";

    _geo1->id() = 2;
    Geo geo;
    geo.id() = 5;
    geo.loc().tag() = type::TaxPointTag::Arrival;

    TaxableYqYrs& taxableYqYrs = _paymentDetail->getMutableYqYrDetails();
    taxableYqYrs.setFailedRule(0, *_rule);
    taxableYqYrs._data[1]._taxPointLoc2 = &geo;
    taxableYqYrs._data[1]._taxPointEnd = &geo;

    _taxableUnitTags[0] = type::TaxableUnitTag::Applies; // 0 element - yqyr
    _taxableUnitTags[8] = type::TaxableUnitTag::Applies; // 8 element - itinerary

    TaxOnYqYrApplicator applicator(*_rule, sb, true);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().areAllFailed());
    CPPUNIT_ASSERT(_paymentDetail->getMutableYqYrDetails().isFailedRule(0));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableYqYrDetails().isFailedRule(1));
  }

private:
  PaymentDetail* _paymentDetail;
  std::vector<TaxableYqYr> _yqYrs;
  TaxableYqYr* _yqYr1;
  TaxableYqYr* _yqYr2;
  Geo* _geo1;
  Geo* _geo2;
  std::vector<type::TaxableUnitTag> _taxableUnitTags;
  std::unique_ptr<TaxOnYqYrRule> _rule;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnYqYrTest);
} // namespace tax
