// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DBAccess/TaxNation.h"
#include "Taxes/AtpcoTaxes/Common/Timestamp.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Codes.h"
#include "Taxes/LegacyFacades/NationServiceV2.h"
#include "test/include/GtestHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>

namespace tse
{
using testing::Return;
using testing::_;

class NationServiceV2DataHandle : public DataHandleMock
{
public:
  MOCK_METHOD2(getTaxNation,
               const TaxNation*(const NationCode&, const DateTime&));
};

class NationServiceV2Test : public testing::Test
{
public:
  void SetUp()
  {
  }

  void TearDown()
  {
  }

protected:
  NationServiceV2DataHandle _dataHandle;
  tax::type::Timestamp _ticketingDate{tax::type::Timestamp::emptyTimestamp()};
};

TEST_F(NationServiceV2Test, testGetMessage)
{
  TaxNation taxNation;
  taxNation.message() = "MESSAGE";
  NationCode tseNationCode = "AA";
  tax::type::Nation nationCode = "AA";

  NationServiceV2 nationService;

  EXPECT_CALL(_dataHandle, getTaxNation(tseNationCode,  _)).
      WillOnce(Return(&taxNation));
  std::string message = nationService.getMessage(nationCode, _ticketingDate);
  ASSERT_EQ(taxNation.message(), message);


  EXPECT_CALL(_dataHandle, getTaxNation(tseNationCode,  _)).
      WillOnce(Return(nullptr));
  message = nationService.getMessage(nationCode, _ticketingDate);
  ASSERT_TRUE(message.empty());
}
}
