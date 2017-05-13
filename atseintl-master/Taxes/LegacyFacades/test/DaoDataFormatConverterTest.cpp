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
#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/PaxTypeCodeInfo.h"
#include "DBAccess/SectorDetailInfo.h"
#include "DBAccess/ServiceBaggageInfo.h"
#include "DBAccess/TaxReportingRecordInfo.h"
#include "DBAccess/TaxRulesRecord.h"
#include "DBAccess/DiskCache.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeOps.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierFlight.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierFlightSegment.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierApplication.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/PassengerTypeCode.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/ReportingRecord.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/RulesRecord.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/SectorDetail.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/ServiceBaggage.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace
{

template <typename ET>
bool equalAsEnum(const ET& et, char ch)
{
  ET et2;
  tse::setTaxEnumValue(et2, ch);
  return et == et2;
}

void
addCFSegment(tse::TaxCarrierFlightInfo& rec,
             int order,
             std::string marketingCarrier,
             std::string operatingCarrier,
             int from,
             int to)
{
  std::unique_ptr<tse::CarrierFlightSeg> cfs1(new tse::CarrierFlightSeg);
  tse::CarrierFlightSeg::dummyData(*cfs1);
  cfs1->orderNo() = order;
  cfs1->marketingCarrier() = marketingCarrier;
  cfs1->operatingCarrier() = operatingCarrier;
  cfs1->flt1() = from;
  cfs1->flt2() = to;
  rec.segs().push_back(cfs1.release());
  // cfs1.release();
}

} // anonymous namespace

class DaoDataFormatConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DaoDataFormatConverterTest);
  CPPUNIT_TEST(testRulesRecordConverter);
  CPPUNIT_TEST(testCarrierFlightConverter);
  CPPUNIT_TEST(testCarrierApplicationConverter);
  CPPUNIT_TEST(testPassengerTypeCodeConverter);
  CPPUNIT_TEST(testServiceBaggageConverter);
  CPPUNIT_TEST(testSectorDetailConverter);
  CPPUNIT_TEST(testReportingRecordConverter);
  CPPUNIT_TEST_SUITE_END();

  tse::TestMemHandle _memHandle;

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      tse::DiskCache::initialize(_config);
      _memHandle.create<tse::MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    tse::TestMemHandle _memHandle;
  };

public:
  void setUp() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  template <class T>
  struct remove_const
  {
    typedef T type;
  };
  template <class T>
  struct remove_const<const T>
  {
    typedef T type;
  };

  template <class V, class T>
  void checkMainAttributes(const V& v2, const T& tax)
  {
    CPPUNIT_ASSERT_EQUAL(tse::toTaxVendorCode(v2.vendor()), tax.vendor);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(v2.itemNo()), tax.itemNo);
  }

  void checkMainAttributes(const tse::TaxReportingRecordInfo& v2, const tax::ReportingRecord& tax)
  {
    tax::type::Nation ans_nation(tax::UninitializedCode);
    CPPUNIT_ASSERT(codeFromString(v2.nationCode(), ans_nation));
    CPPUNIT_ASSERT_EQUAL(ans_nation, tax.nation);
    CPPUNIT_ASSERT_EQUAL(tse::toTaxVendorCode(v2.vendor()), tax.vendor);
    CPPUNIT_ASSERT_EQUAL(tse::toTaxCarrierCode(v2.taxCarrier()), tax.taxCarrier);
    CPPUNIT_ASSERT_EQUAL(tse::toTaxCode(v2.taxCode()), tax.taxCode);
    CPPUNIT_ASSERT_EQUAL(tse::toTaxType(v2.taxType()), tax.taxType);
    CPPUNIT_ASSERT_EQUAL(tax.isVatTax, v2.vatInd() == 'Y');
    CPPUNIT_ASSERT_EQUAL(tax.isCommissionable, v2.commisionableTaxTag() == 'Y');
    CPPUNIT_ASSERT_EQUAL(tax.isInterlineable, v2.interlineAbleTaxTag() == 'Y');
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(tax.taxOrChargeTag), static_cast<unsigned char>(v2.taxCharge()));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(tax.refundableTag), static_cast<unsigned char>(v2.refundableTaxTag()));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(tax.accountableDocTag), static_cast<unsigned char>(v2.accountableDocTaxTag()));
    CPPUNIT_ASSERT_EQUAL(tax.taxTextItemNo, tax::type::Index(v2.taxTextItemNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxApplicableToItemNo, tax::type::Index(v2.taxApplicableToTblNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxRateItemNo,tax::type::Index( v2.taxRateTextTblNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxExemptionsItemNo, tax::type::Index(v2.taxExemptionTextTblNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxCollectRemitItemNo, tax::type::Index(v2.taxCollectNrEmmitTblNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxingAuthorityItemNo, tax::type::Index(v2.taxingAuthorityTextTblNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxCommentsItemNo, tax::type::Index(v2.taxCommentsTextTblNo()));
    CPPUNIT_ASSERT_EQUAL(tax.taxSpecialInstructionsItemNo,tax::type::Index( v2.taxSplInstructionsTblNo()));
  }

  template <class V, class T>
  void dataVectorConverterTest()
  {
    std::vector<V*> v2RecordVector;
    for (int i = 0; i < 3; ++i)
    {
      typedef typename remove_const<V>::type NcV;
      NcV* v2Record = new NcV();
      NcV::dummyData(*v2Record);
      v2RecordVector.push_back(v2Record);
    }
    T taxRecord;
    tse::DaoDataFormatConverter::convert(v2RecordVector, taxRecord);

    checkMainAttributes(*v2RecordVector.front(), taxRecord);

    typename std::vector<V*>::const_iterator itV2 = v2RecordVector.begin();
    typename boost::ptr_vector<typename T::entry_type>::const_iterator itTax =
        taxRecord.entries.begin();
    for (; itV2 != v2RecordVector.end(); ++itV2, ++itTax)
    {
      CPPUNIT_ASSERT_EQUAL(isRecordEqual(**itV2, *itTax), true);
      delete *itV2;
    }
  }

  void testRulesRecordConverter()
  {
    tax::RulesRecord taxRecord;
    tse::TaxRulesRecord tseRecord;
    tse::TaxRulesRecord::dummyData(tseRecord);

    tse::DaoDataFormatConverter::convert(tseRecord, taxRecord);

    CPPUNIT_ASSERT_EQUAL(tse::toTaxCode(tseRecord.taxCode()), taxRecord.taxName.taxCode());
    CPPUNIT_ASSERT_EQUAL(tse::toTaxType(tseRecord.taxType()), taxRecord.taxName.taxType());
    CPPUNIT_ASSERT_EQUAL(tseRecord.taxPointTag(),
                         static_cast<tse::Indicator>(taxRecord.taxName.taxPointTag()));
    CPPUNIT_ASSERT_EQUAL(tax::type::SeqNo(tseRecord.seqNo()), taxRecord.seqNo);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(tseRecord.svcFeesSecurityItemNo()),
        taxRecord.svcFeesSecurityItemNo);
  }

  void testCarrierFlightConverter()
  {
    {
      tax::CarrierFlight taxRecord;
      tse::TaxCarrierFlightInfo tseRecord;
      tse::TaxCarrierFlightInfo::dummyData(tseRecord);

      tse::DaoDataFormatConverter::convert(tseRecord, taxRecord);

      CPPUNIT_ASSERT_EQUAL(tse::toTaxVendorCode(tseRecord.vendor()), taxRecord.vendor);
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(tseRecord.itemNo()), taxRecord.itemNo);
    }
    {
      tse::TaxCarrierFlightInfo tseRecord;
      // tse::CarrierFlight::dummyData(tseRecord);
      tseRecord.segCnt() = 10;
      addCFSegment(tseRecord, 1, "BA", "CA", 1000, 1999); // 1000 through 1999
      addCFSegment(tseRecord, 2, "CA", "CA", 2000, 0); // 2000 only
      addCFSegment(tseRecord, 3, "DA", "CA", -1, 0); // all numbers
      addCFSegment(tseRecord, 4, "EA", "CA", -1, 1000); // all numbers
      addCFSegment(tseRecord, 5, "QA", "ER", 0, -1); // error
      addCFSegment(tseRecord, 6, "RA", "ER", 0, 1000); // error
      addCFSegment(tseRecord, 7, "SA", "ER", 3000, -1); // error
      addCFSegment(tseRecord, 8, "TA", "ER", 0, 0); // error
      addCFSegment(tseRecord, 9, "UA", "ER", -1, -1); // error, but treat it as all numbers
      addCFSegment(tseRecord, 10, "VA", "ER", 2000, 1000); // error

      tax::CarrierFlight taxRecord;
      tse::DaoDataFormatConverter::convert(tseRecord, taxRecord);
      CPPUNIT_ASSERT_EQUAL((size_t)10, taxRecord.segments.size());
      CPPUNIT_ASSERT_EQUAL((tax::type::FlightNumber)1000, taxRecord.segments[0].flightFrom);
      CPPUNIT_ASSERT_EQUAL((tax::type::FlightNumber)1999, taxRecord.segments[0].flightTo);
      CPPUNIT_ASSERT_EQUAL((tax::type::FlightNumber)2000, taxRecord.segments[1].flightFrom);
      CPPUNIT_ASSERT_EQUAL((tax::type::FlightNumber)2000, taxRecord.segments[1].flightTo);
      CPPUNIT_ASSERT(taxRecord.segments[2].flightFrom <= 1); // any flight from 1 to 9999
      CPPUNIT_ASSERT(taxRecord.segments[2].flightTo >= 9999); // should match
      CPPUNIT_ASSERT(taxRecord.segments[3].flightFrom <= 1);
      CPPUNIT_ASSERT(taxRecord.segments[3].flightTo >= 9999);
      CPPUNIT_ASSERT(taxRecord.segments[4].flightFrom > taxRecord.segments[4].flightTo);
      CPPUNIT_ASSERT(taxRecord.segments[5].flightFrom > taxRecord.segments[5].flightTo);
      CPPUNIT_ASSERT(taxRecord.segments[6].flightFrom > taxRecord.segments[6].flightTo);
      CPPUNIT_ASSERT(taxRecord.segments[7].flightFrom > taxRecord.segments[7].flightTo);
      CPPUNIT_ASSERT(taxRecord.segments[8].flightFrom <= 1);
      CPPUNIT_ASSERT(taxRecord.segments[8].flightTo >= 9999);
      CPPUNIT_ASSERT(taxRecord.segments[9].flightFrom > taxRecord.segments[9].flightTo);
    }
  }

  void testCarrierApplicationConverter()
  {
    tax::CarrierApplication taxRecord;
    tse::TaxCarrierAppl tseRecord;

    tse::TaxCarrierAppl::dummyData(tseRecord);

    tse::DaoDataFormatConverter::convert(tseRecord, taxRecord);

    CPPUNIT_ASSERT_EQUAL(tse::toTaxVendorCode(tseRecord.vendor()), taxRecord.vendor);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(tseRecord.itemNo()), taxRecord.itemNo);
  }

  bool isRecordEqual(const tse::PaxTypeCodeInfo& v2pax, const tax::PassengerTypeCodeItem& pas)
  {
    return (equalAsEnum(pas.applTag, v2pax.applyTag()) &&
            equal(pas.passengerType, v2pax.psgrType()) &&
            pas.minimumAge == static_cast<uint16_t>(v2pax.paxMinAge()) &&
            pas.maximumAge == static_cast<uint16_t>(v2pax.paxMaxAge()) &&
            equalAsEnum(pas.statusTag, v2pax.paxStatus()) &&
            equalAsEnum(pas.location.type(), v2pax.loc().locType()) &&
            equal(pas.location.code(), v2pax.loc().loc()) &&
            equalAsEnum(pas.matchIndicator, v2pax.ptcMatchIndicator()));
  }

  void testPassengerTypeCodeConverter()
  {
    dataVectorConverterTest<const tse::PaxTypeCodeInfo, tax::PassengerTypeCode>();
  }

  bool
  isRecordEqual(const tse::ServiceBaggageInfo& v2Entry, const tax::ServiceBaggageEntry& taxEntry)
  {
    tax::type::TaxCode  ans_taxCode(tax::UninitializedCode);
    CPPUNIT_ASSERT(codeFromString(v2Entry.taxCode(), ans_taxCode));
    return (equalAsEnum(taxEntry.applTag, v2Entry.applyTag()) &&
            taxEntry.taxTypeSubcode == v2Entry.taxTypeSubCode() &&
            taxEntry.taxCode == ans_taxCode &&
            equalAsEnum(taxEntry.optionalServiceTag, v2Entry.svcType()) &&
            taxEntry.group == v2Entry.attrGroup() && taxEntry.subGroup == v2Entry.attrSubGroup());
  }

  void testServiceBaggageConverter()
  {
    dataVectorConverterTest<const tse::ServiceBaggageInfo, tax::ServiceBaggage>();
  }

  bool isRecordEqual(const tse::SectorDetailInfo& v2Entry, const tax::SectorDetailEntry& taxEntry)
  {
    return equalAsEnum(taxEntry.applTag, v2Entry.applyTag()) &&
           taxEntry.equipmentCode == v2Entry.equipmentCode();
  }

  void testSectorDetailConverter()
  {
    dataVectorConverterTest<const tse::SectorDetailInfo, tax::SectorDetail>();
  }

  bool isRecordEqual(const tse::TaxReportingRecordInfo& v2Entry,
                     const tax::ReportingRecordEntry& taxEntry)
  {
    return taxEntry.taxLabel == v2Entry.taxName();
  }

  void testReportingRecordConverter()
  {
    dataVectorConverterTest<const tse::TaxReportingRecordInfo, tax::ReportingRecord>();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DaoDataFormatConverterTest);
