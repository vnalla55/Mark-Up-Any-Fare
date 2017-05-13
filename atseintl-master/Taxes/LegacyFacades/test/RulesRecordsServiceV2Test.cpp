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

#include "test/include/GtestHelperMacros.h"
#include "DBAccess/TaxRulesRecord.h"
#include "Taxes/AtpcoTaxes/Common/TaxName.h"
#include "Taxes/AtpcoTaxes/Rules/TaxData.h"
#include "Taxes/LegacyFacades/RulesRecordsServiceV2.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace tse
{
using testing::ReturnRef;
using testing::StrictMock;
using testing::_;

typedef RulesRecordsServiceV2::ByNationConstValue SharedConstValue;
typedef RulesRecordsServiceV2::RulesContainersGroupedByTaxName Value;

class MyDataHandle : public DataHandleMock
{
public:
  MOCK_METHOD3(getTaxRulesRecord,
               const std::vector<const TaxRulesRecord*>&(const NationCode&,
                                                         const Indicator&,
                                                         const DateTime&));
};

class RulesRecordsServiceV2Test : public testing::Test
{
public:
  void SetUp()
  {
    _dataHandle = _memHandle.create<MyDataHandle>();
    _ticketingDate =
        _memHandle.create<tax::type::Timestamp>(tax::type::Timestamp::emptyTimestamp());
  }

  void TearDown()
  {
    _memHandle.clear();
  }

protected:
  TestMemHandle _memHandle;
  MyDataHandle* _dataHandle;
  tax::type::Timestamp* _ticketingDate;

  void addRecord(std::vector<const TaxRulesRecord*>& recordVector,
                 const NationCode& nation,
                 const Indicator& taxPointTag,
                 const TaxCode& taxCode,
                 int32_t seqNo)
  {
    TaxRulesRecord* record = _memHandle.create<TaxRulesRecord>();
    record->nation() = nation;
    record->taxPointTag() = taxPointTag;
    record->taxCode() = taxCode;
    record->seqNo() = seqNo;
    record->percentFlatTag() = 'F';

    recordVector.push_back(record);
  }
};

TEST_F(RulesRecordsServiceV2Test, testAll)
{
  // Since cache for RulesRecords keeps data per application, everything must be tested in one test
  // case.
  NationCode PL = "PL";
  NationCode GB = "GB";
  Indicator Departure = 'D';
  std::vector<const TaxRulesRecord*> recordsPL1;
  std::vector<const TaxRulesRecord*> recordsPL2;
  std::vector<const TaxRulesRecord*> recordsGB1;
  std::vector<const TaxRulesRecord*> recordsGB2;

  addRecord(recordsPL1, "PL", 'D', "PL", 100);
  addRecord(recordsGB1, "GB", 'D', "GB", 101);
  addRecord(recordsGB2, "GB", 'D', "GB", 101);
  addRecord(recordsGB2, "GB", 'D', "GB", 103);

  EXPECT_CALL(*_dataHandle, getTaxRulesRecord(PL, Departure, _))
      .WillOnce(ReturnRef(recordsPL1));
  EXPECT_CALL(*_dataHandle, getTaxRulesRecord(GB, Departure, _))
      .WillOnce(ReturnRef(recordsGB1));

  tax::TaxName taxNameGB;
  taxNameGB.nation() = "GB";
  taxNameGB.taxCode() = "GB";
  taxNameGB.taxPointTag() = tax::type::TaxPointTag::Departure;
  taxNameGB.percentFlatTag() = tax::type::PercentFlatTag::Flat;

  tax::TaxName taxNamePL;
  taxNamePL.nation() = "PL";
  taxNamePL.taxCode() = "PL";
  taxNamePL.taxPointTag() = tax::type::TaxPointTag::Departure;
  taxNamePL.percentFlatTag() = tax::type::PercentFlatTag::Flat;

  SharedConstValue value;
  value = RulesRecordsServiceV2::instance().getTaxRulesContainers(
      "PL", tax::type::TaxPointTag::Departure, *_ticketingDate);

  ASSERT_TRUE(static_cast<bool>(value));
  ASSERT_EQ(1, value->size());
  ASSERT_EQ(taxNamePL, value->front().getTaxName());

  value = RulesRecordsServiceV2::instance().getTaxRulesContainers(
      "GB", tax::type::TaxPointTag::Departure, *_ticketingDate);

  ASSERT_TRUE(static_cast<bool>(value));
  ASSERT_EQ(1, value->size());
  ASSERT_EQ(taxNameGB, value->front().getTaxName());
}
}
