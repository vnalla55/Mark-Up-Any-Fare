// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <boost/assign/std/vector.hpp>

#include "DataModel/FarePath.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputItins.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "Taxes/LegacyFacades/GsaLogic.h"
#include "Taxes/LegacyFacades/ResponseConverter2.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"
#include "Taxes/LegacyFacades/test/PricingTrxBuilder.h"
#include "Taxes/LegacyFacades/ServicesFeesMap.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder2.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

namespace {
  const tax::type::CarrierCode CC("CC");
}

class GsaLogicTest : public CppUnit::TestFixture, public PricingTrxBuilder
{
  CPPUNIT_TEST_SUITE(GsaLogicTest);
  CPPUNIT_TEST(forwardFarePath);
  CPPUNIT_TEST(getMainFarePath);
  CPPUNIT_TEST(buildBestFarePathMap1);
  CPPUNIT_TEST(gsaE2ETest);
  CPPUNIT_TEST_SUITE_END();


  tax::FarePathLink link(FarePath& alt, FarePath& main, tax::type::CarrierCode cc, tax::type::Index i)
  {
    return tax::FarePathLink(&alt, &main, cc, i);
  }

  tax::ItinFarePathKey solutionLink(Itin& itin, FarePath& fpath, tax::type::CarrierCode cc, tax::type::Index idx)
  {
    return tax::ItinFarePathKey(&itin, &fpath, cc, idx);
  }

  class Solution
  {
    std::unique_ptr<tax::OutputItin> solution;
  public:
    Solution(tax::type::Index i) : solution(new tax::OutputItin(i)) {}
    Solution& groupId(tax::type::Index i) { solution->taxGroupId() = i; return *this; }
    std::unique_ptr<tax::OutputItin> get() { return std::move(solution); }
  };

  tax::FarePathMap makeFarePathMap()
  {
    tax::FarePathMap map;
    map += link(f1_1A, f1_1, CC, 1), link(f1_1B, f1_1, CC, 2), link(f1_1C, f1_1, CC, 3), link(f1_1, f1_1, CC, 0);
    map += link(f1_2A, f1_2, CC, 5), link(f1_2B, f1_2, CC, 6), link(f1_2, f1_2, CC, 4);
    map += link(f2_1A, f2_1, CC, 8), link(f2_1B, f2_1, CC, 9), link(f2_1, f2_1, CC, 7);
    return map;
  }

  // Itin   FPath  Alt_Cxr  Tax  Best option
  //  1      1_1    1_1      20   *
  //                1_1A     25
  //                1_1B     24
  //                1_1C     22
  //         1_2    1_2      30
  //                1_2A     30
  //                1_2B     20   *
  //  2      2_1    2_1      11
  //                2_1A     11
  //                2_1B     10   *
  //
  void makeSolutions1(tax::OutputItins& solutions)
  {
    std::vector<tax::type::MoneyAmount> amounts;
    amounts += 20, 25, 24, 22, 30, 30, 20, 11, 11, 10;

    for (int i = 0; i != 10; ++i)
    {
      std::shared_ptr<tax::OutputTaxDetails> taxDtl = std::make_shared<tax::OutputTaxDetails>();
      taxDtl->paymentAmt() = tax::type::MoneyAmount(amounts[i]);
      solutions.taxDetailsSeq().push_back(taxDtl);
    }

    for (int i = 0; i != 10; ++i)
    {
      std::shared_ptr<tax::OutputTaxGroup> taxGrp = std::make_shared<tax::OutputTaxGroup>("O");
      taxGrp->taxSeq().push_back(new tax::OutputTax());
      taxGrp->taxSeq().back().taxDetailsRef().push_back(tax::OutputTaxDetailsRef(i));
      solutions.taxGroupSeq().push_back(taxGrp);
    }

    for (int i = 0; i != 10; ++i)
      solutions.itinSeq().push_back( Solution(i).groupId(i).get().release() );
  }

  tax::ItinFarePathMapping makeSolutionMapping()
  {
    tax::ItinFarePathMapping solutionMap;
    solutionMap += solutionLink(i1, f1_1, CC, 0), solutionLink(i1, f1_1A, CC, 1), solutionLink(i1, f1_1B, CC, 2), solutionLink(i1, f1_1C, CC, 3);
    solutionMap += solutionLink(i1, f1_2, CC, 4), solutionLink(i1, f1_2A, CC, 5), solutionLink(i1, f1_2B, CC, 6);
    solutionMap += solutionLink(i2, f2_1, CC, 7), solutionLink(i2, f2_1A, CC, 8), solutionLink(i2, f2_1B, CC, 9);
    return solutionMap;
  }

  static tse::Itin* itin(tax::ItinFarePathKey& val) { return std::get<0>(val); }
  static tse::FarePath* fpath(tax::ItinFarePathKey& val) { return std::get<1>(val); }
  static tax::type::CarrierCode carrierCode(tax::ItinFarePathKey& val) { return std::get<2>(val); }
  static tax::type::Index index(tax::ItinFarePathKey& val) { return std::get<3>(val); }

  void addSolution(tax::OutputItins& solutions, tax::type::Index solIdx, tax::type::MoneyAmount amount)
  {
    size_t taxDetailsSeqIndex = solutions.taxDetailsSeq().size();
    solutions.taxDetailsSeq().push_back(std::make_shared<tax::OutputTaxDetails>());
    solutions.taxDetailsSeq().back()->paymentAmt() = amount;

    solutions.taxGroupSeq().push_back(std::make_shared<tax::OutputTaxGroup>(std::string{"TGT"}));
    solutions.taxGroupSeq().back()->taxSeq().push_back( new tax::OutputTax{} );
    std::vector<tax::OutputTaxDetailsRef>& taxDetailsRef = solutions.taxGroupSeq().back()->taxSeq().back().taxDetailsRef();
    taxDetailsRef.push_back( tax::OutputTaxDetailsRef{taxDetailsSeqIndex} );

    solutions.itinSeq().push_back( new tax::OutputItin{solIdx} );
    solutions.itinSeq().back().taxGroupId() = solutions.taxGroupSeq().size() - 1;
  }

  void addSolutionRef(tax::OutputItins& solutions, tax::type::Index solIdx, tax::type::Index refSolIdx)
  {
    assert (refSolIdx < solIdx);
    solutions.itinSeq().push_back( new tax::OutputItin{solIdx} );
    solutions.itinSeq().back().taxGroupId() = solutions.itinSeq()[refSolIdx].taxGroupId();
  }

  void fillOutputResponse(tax::OutputResponse& outputResponse)
  {
    assert (outputResponse.itins() == boost::none);
    outputResponse.itins().emplace(); // initialize optional
    tax::OutputItins& solutions = *outputResponse.itins();
    addSolution(solutions, 0U, tax::type::MoneyAmount{200});
    addSolution(solutions, 1U, tax::type::MoneyAmount{300});
    addSolution(solutions, 2U, tax::type::MoneyAmount{500});
    addSolutionRef(solutions, 3U, 2U);
  }

  tse::TestMemHandle _memHandle;

public:

  void setUp() override { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() override { _memHandle.clear(); }

  void forwardFarePath()
  {
    FarePath fp1;
    FarePath fp2;
    tax::ForwardFarePath forward;
    CPPUNIT_ASSERT(&fp1 == forward(&fp1));
    CPPUNIT_ASSERT(&fp2 == forward(&fp2));
  }

  void getMainFarePath()
  {
    tax::FarePathMap map = makeFarePathMap();
    tax::GetMainFarePath main(map);

    CPPUNIT_ASSERT(&f1_1 == main(&f1_1A));
    CPPUNIT_ASSERT(&f1_1 == main(&f1_1B));
    CPPUNIT_ASSERT(&f1_1 == main(&f1_1C));
    CPPUNIT_ASSERT(&f1_1 == main(&f1_1 ));
    CPPUNIT_ASSERT(&f1_2 == main(&f1_2A));
    CPPUNIT_ASSERT(&f1_2 == main(&f1_2A));
    CPPUNIT_ASSERT(&f1_2 == main(&f1_2 ));
    CPPUNIT_ASSERT(&f2_1 == main(&f2_1A));
    CPPUNIT_ASSERT(&f2_1 == main(&f2_1B));
    CPPUNIT_ASSERT(&f2_1 == main(&f2_1 ));
  }

  void buildBestFarePathMap1()
  {
    tax::V2TrxMappingDetails trxMappings;
    trxMappings._farePathMap = makeFarePathMap();
    trxMappings._itinFarePathMapping = makeSolutionMapping();

    tax::OutputItins solutions;
    makeSolutions1(solutions);
    PricingRequest pricingRequest;
    tse::PricingTrx trx;
    trx.setTrxType(PricingTrx::IS_TRX);
    trx.setRequest(&pricingRequest);

    tax::ItinFarePathMapping ans = tax::buildBestFarePathMap(trxMappings, solutions, trx);
    CPPUNIT_ASSERT_EQUAL(i1.farePath().size(), size_t(2));
    CPPUNIT_ASSERT(i1.farePath()[0] == &f1_1C);
    CPPUNIT_ASSERT(i1.farePath()[1] == &f1_2B);
    CPPUNIT_ASSERT_EQUAL(i2.farePath().size(), size_t(1));
    CPPUNIT_ASSERT(i2.farePath()[0] == &f2_1B);
  }

  void gsaE2ETest()
  {
    PricingTrxBuilder builder {itinLayoutA};
    tse::PricingTrx& trx = builder.trx();
    const tax::ServicesFeesMap servicesFees;
    tax::V2TrxMappingDetails mapping;
    tax::TaxRequestBuilder{trx, trx.atpcoTaxesActivationStatus()}.buildInputRequest(servicesFees, mapping);

    CPPUNIT_ASSERT_EQUAL(size_t(4), mapping._farePathMap.size());

    CPPUNIT_ASSERT(&builder.f1_1A == mapping._farePathMap[0].altPath);
    CPPUNIT_ASSERT(&builder.f1_1 == mapping._farePathMap[0].mainPath);
    CPPUNIT_ASSERT("1A" == mapping._farePathMap[0].validatingCarrier);
    CPPUNIT_ASSERT_EQUAL(size_t(0), mapping._farePathMap[0].index);

    CPPUNIT_ASSERT_EQUAL(size_t(4), mapping._itinFarePathMapping.size());

    CPPUNIT_ASSERT(&builder.i1 == itin(mapping._itinFarePathMapping[0]));
    CPPUNIT_ASSERT(&builder.f1_1A == fpath(mapping._itinFarePathMapping[0]));
    CPPUNIT_ASSERT_EQUAL(size_t(0), index(mapping._itinFarePathMapping[0]));

    CPPUNIT_ASSERT(&builder.i1 == itin(mapping._itinFarePathMapping[1]));
    CPPUNIT_ASSERT(&builder.f1_1 == fpath(mapping._itinFarePathMapping[1]));
    CPPUNIT_ASSERT_EQUAL(size_t(1), index(mapping._itinFarePathMapping[1]));

    CPPUNIT_ASSERT(&builder.i2 == itin(mapping._itinFarePathMapping[2]));
    CPPUNIT_ASSERT(&builder.f2_1 == fpath(mapping._itinFarePathMapping[2]));
    CPPUNIT_ASSERT_EQUAL(size_t(2), index(mapping._itinFarePathMapping[2]));

    CPPUNIT_ASSERT(&builder.i2 == itin(mapping._itinFarePathMapping[3]));
    CPPUNIT_ASSERT(&builder.f2_1 == fpath(mapping._itinFarePathMapping[3]));
    CPPUNIT_ASSERT_EQUAL(size_t(3), index(mapping._itinFarePathMapping[3]));

    tax::OutputResponse outResponse;
    fillOutputResponse(outResponse);
    tax::ResponseConverter(trx, trx.atpcoTaxesActivationStatus()).updateV2Trx(outResponse, mapping, 0, //&std::clog,
                                        [&](const tax::OutputTaxDetails& dt) { return std::vector<std::string>{}; });

    CPPUNIT_ASSERT(builder.i1.farePath()[0] == &builder.f1_1A);
    CPPUNIT_ASSERT_EQUAL(tse::CarrierCode{"1A"}, builder.i1.validatingCarrier());

    CPPUNIT_ASSERT(builder.i2.farePath()[0] == &builder.f2_1);
    CPPUNIT_ASSERT_EQUAL(tse::CarrierCode{"LA"}, builder.i2.validatingCarrier()); // read from segments

    const tse::FarePath& fp2 = *builder.i2.farePath()[0];
    CPPUNIT_ASSERT_EQUAL(size_t(1), fp2.validatingCarriers().size());
    CPPUNIT_ASSERT_EQUAL(tse::CarrierCode{"2A"}, fp2.validatingCarriers()[0]);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(GsaLogicTest);

} // namespace tse

