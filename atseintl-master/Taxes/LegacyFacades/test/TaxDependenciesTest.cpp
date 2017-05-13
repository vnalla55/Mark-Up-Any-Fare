// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputTaxDetails.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/RulesRecord.h"
#include "Taxes/LegacyFacades/TaxDependencies.h"
#include "Taxes/TestServer/Facades/RulesRecordsServiceServer.h"
#include "Taxes/TestServer/Facades/ServiceBaggageServiceServer.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class TaxDependenciesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE (TaxDependenciesTest);
  CPPUNIT_TEST (emptyDB);
  CPPUNIT_TEST (found2);
  CPPUNIT_TEST (asterisk);
  CPPUNIT_TEST_SUITE_END ();

  tax::RulesRecordsServiceServer _x1Service;
  tax::ServiceBaggageServiceServer _SnBService;
  tax::type::Timestamp _tktDate;
  TaxDependencies _deps;

public:
  TaxDependenciesTest() : _deps(_x1Service, _SnBService, _tktDate) {}

  void tearDown() override
  {
    _x1Service.rulesRecords().clear();
    _SnBService.serviceBaggage().clear();
  }

  void emptyDB()
  {
    tax::OutputTaxDetails taxDtl;
    std::vector<std::string> ans = _deps.taxDependencies(taxDtl);
    CPPUNIT_ASSERT(ans.empty());
  }

  void found2()
  {
    tax::OutputTaxDetails taxDtl;
    taxDtl.seqNo() = 1050;
    taxDtl.type() = "002";
    taxDtl.code() = "AA";
    taxDtl.nation() = "NA";

    std::unique_ptr<tax::RulesRecord> rulesRec (new tax::RulesRecord);
    rulesRec->vendor = "SBR";
    rulesRec->taxName.nation() = "NA";
    rulesRec->taxName.taxCode() = "AA";
    rulesRec->taxName.taxType() = "002";
    rulesRec->seqNo = 1050;
    rulesRec->serviceBaggageItemNo = 1;
    rulesRec->serviceBaggageApplTag = tax::type::ServiceBaggageApplTag::E;
    _x1Service.rulesRecords().push_back(rulesRec.release());

    std::unique_ptr<tax::ServiceBaggage> snbRec(new tax::ServiceBaggage);
    snbRec->vendor = "SBR";
    snbRec->itemNo = 1;
    snbRec->entries.push_back(new tax::ServiceBaggageEntry);
    snbRec->entries.back().applTag = tax::type::ServiceBaggageAppl::Positive;
    snbRec->entries.back().taxCode = "UU";
    snbRec->entries.back().taxTypeSubcode = "002";

    snbRec->entries.push_back(new tax::ServiceBaggageEntry);
    snbRec->entries.back().applTag = tax::type::ServiceBaggageAppl::Positive;
    snbRec->entries.back().taxCode = "UR";
    snbRec->entries.back().taxTypeSubcode = "001";
    _SnBService.serviceBaggage().push_back(snbRec.release());

    std::vector<std::string> ans = _deps.taxDependencies(taxDtl);
    CPPUNIT_ASSERT_EQUAL(size_t(2), ans.size());
    CPPUNIT_ASSERT_EQUAL(std::string("UU2"), ans[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("UR"), ans[1]);
  }

  void asterisk()
  {
    tax::OutputTaxDetails taxDtl;
    taxDtl.seqNo() = 1050;
    taxDtl.type() = "002";
    taxDtl.code() = "AA";
    taxDtl.nation() = "NA";

    std::unique_ptr<tax::RulesRecord> rulesRec (new tax::RulesRecord);
    rulesRec->vendor = "SBR";
    rulesRec->taxName.nation() = "NA";
    rulesRec->taxName.taxCode() = "AA";
    rulesRec->taxName.taxType() = "002";
    rulesRec->seqNo = 1050;
    rulesRec->serviceBaggageItemNo = 1;
    rulesRec->serviceBaggageApplTag = tax::type::ServiceBaggageApplTag::E;
    _x1Service.rulesRecords().push_back(rulesRec.release());

    std::unique_ptr<tax::ServiceBaggage> snbRec(new tax::ServiceBaggage);
    snbRec->vendor = "SBR";
    snbRec->itemNo = 1;
    snbRec->entries.push_back(new tax::ServiceBaggageEntry);
    snbRec->entries.back().applTag = tax::type::ServiceBaggageAppl::Positive;
    snbRec->entries.back().taxCode = "UU";
    snbRec->entries.back().taxTypeSubcode = "";
    _SnBService.serviceBaggage().push_back(snbRec.release());

    std::vector<std::string> ans = _deps.taxDependencies(taxDtl);
    CPPUNIT_ASSERT_EQUAL(size_t(1), ans.size());
    CPPUNIT_ASSERT_EQUAL(std::string("UU*"), ans[0]);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION (TaxDependenciesTest);

} // namespace tse
