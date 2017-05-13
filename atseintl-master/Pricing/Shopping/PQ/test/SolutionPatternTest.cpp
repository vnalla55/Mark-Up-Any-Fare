// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         15-09-2011
//! \file         SolutionPatternTest.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#include <stdexcept>
#include <boost/format.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/Shopping/PQ/SolutionPattern.h"

namespace tse
{
namespace shpq
{

class SolutionPatternTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SolutionPatternTest);
  CPPUNIT_TEST(testSPNumberOfPatterns);
  CPPUNIT_TEST(testSPOrder);
  CPPUNIT_TEST(testSPGetInvalid);

  CPPUNIT_TEST(testSP10);
  CPPUNIT_TEST(testSP11);
  CPPUNIT_TEST(testSP12);

  CPPUNIT_TEST(testSP20);
  CPPUNIT_TEST(testSP21);

  CPPUNIT_TEST(testSP30);
  CPPUNIT_TEST(testSP31);
  CPPUNIT_TEST(testSP32);
  CPPUNIT_TEST(testSP33);
  CPPUNIT_TEST(testSP34);
  CPPUNIT_TEST(testSP35);
  CPPUNIT_TEST(testSP36);
  CPPUNIT_TEST(testSP37);

  CPPUNIT_TEST(testSP40);
  CPPUNIT_TEST(testSP41);
  CPPUNIT_TEST(testSP42);
  CPPUNIT_TEST(testSP43);
  CPPUNIT_TEST(testSP44);
  CPPUNIT_TEST(testSP45);
  CPPUNIT_TEST(testSP46);
  CPPUNIT_TEST(testSP47);
  CPPUNIT_TEST(testSP48);
  CPPUNIT_TEST(testSP49);
  CPPUNIT_TEST_SUITE_END();

private:
  void checkSP(const SolutionPattern& sp,
               const SolutionPattern& spByNumber,
               const SolutionPattern::SPEnumType& id,
               const std::string& idAsString,
               const unsigned int& number,
               const SolutionType& outboundType,
               const std::string& outboundTypeStr,
               const SolutionType& inboundType,
               const std::string& inboundTypeStr,
               const SolutionPattern::PUTypeVector::size_type& numberOfPus)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("id", id, sp.getSPId());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("idStr", idAsString, static_cast<std::string>(sp.getSPIdStr()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("number", number, static_cast<unsigned>(sp.getSPNumber()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("obType", outboundType, sp.getOutboundSolution());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("ibType", inboundType, sp.getInboundSolution());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "obTypeStr", outboundTypeStr, static_cast<std::string>(sp.getOutboundSolCStr()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "ibTypeStr", inboundTypeStr, static_cast<std::string>(sp.getInboundSolCStr()));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("hasOb", outboundType != NONE, sp.hasOutboundSol());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("hasIb", inboundType != NONE, sp.hasInboundSol());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("noOfPus", numberOfPus, sp.getPUTypeVector().size());

    if (&sp != &spByNumber)
    {
      CPPUNIT_FAIL(std::string("INVALID SP by number : ") + spByNumber.getSPIdStr());
    }
  }
  void checkPUofSP(const SolutionPattern& sp,
                   const size_t& number,
                   const std::string& puAsString,
                   const PricingUnit::Type& puType,
                   const PricingUnit::PUSubType& puSubType)
  {
    CPPUNIT_ASSERT(number < sp.getPUTypeVector().size());

    const std::string msgPrefix(boost::str(boost::format("Checking PU %d: ") % number));

    PUFullTypePtr pu(sp.getPUTypeVector()[number]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        msgPrefix + "puStr", puAsString, static_cast<std::string>(pu->getPUIdStr()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgPrefix + "puType", puType, pu->getType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgPrefix + "puSubType", puSubType, pu->getSubType());
  }

public:
  void testSPNumberOfPatterns()
  {
    const SolutionPatternStorage& storage(SolutionPatternStorage::instance());

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(SolutionPattern::NUMBER_OF_SPS),
                         static_cast<size_t>(storage.end() - storage.begin()));
  }

  void testSPOrder()
  {
    const SolutionPatternStorage& storage(SolutionPatternStorage::instance());
    size_t i(0);
    for (SolutionPatternStorage::const_iterator it = storage.begin(); it != storage.end();
         ++it, ++i)
    {
      if (i)
      {
        const unsigned int prevNo((it - 1)->getSPNumber());
        const SolutionPattern::SPEnumType prevId((it - 1)->getSPId());

        const unsigned int thisNo(it->getSPNumber());
        const SolutionPattern::SPEnumType thisId(it->getSPId());

        boost::format fm("checking %s %d (%d < %d)");
        CPPUNIT_ASSERT_MESSAGE(boost::str(fm % "number" % i % prevNo % thisNo), prevNo < thisNo);
        CPPUNIT_ASSERT_MESSAGE(boost::str(fm % "enum" % i % prevId % thisId), prevId < thisId);
      }
    }
  }

  void testSPGetInvalid()
  {
    const SolutionPatternStorage& storage(SolutionPatternStorage::instance());
    CPPUNIT_ASSERT_THROW(storage.getSPByNumber(100), std::out_of_range);
  }

#define TEST_SP(spId, spNumber, obType, ibType, noOfPus)                                           \
  const SolutionPatternStorage& storage(SolutionPatternStorage::instance());                       \
  const SolutionPattern& spById(storage.getSPById(SolutionPattern::spId));                         \
  const SolutionPattern& spByNo(storage.getSPByNumber(spNumber));                                  \
  checkSP(spById,                                                                                  \
          spByNo,                                                                                  \
          SolutionPattern::spId,                                                                   \
          #spId,                                                                                   \
          spNumber,                                                                                \
          obType,                                                                                  \
          #obType,                                                                                 \
          ibType,                                                                                  \
          #ibType,                                                                                 \
          noOfPus)

#define TEST_SP_PU(puNumber, ident, puType, puSubType)                                             \
  checkPUofSP(spById, puNumber, ident, PricingUnit::Type::puType, PricingUnit::puSubType)

#define TEST_SP_HRT(hasHrt) CPPUNIT_ASSERT_EQUAL(hasHrt, spById.hasHRT())

  void testSP10()
  {
    TEST_SP(SP10, 10u, OW, NONE, 1u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(false);
  }

  void testSP11()
  {
    TEST_SP(SP11, 11u, OW_OW, NONE, 2u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(false);
  }

  void testSP12()
  {
    TEST_SP(SP12, 12u, HRT_HRT, NONE, 1u);
    TEST_SP_PU(0u, "OOJ", OPENJAW, ORIG_OPENJAW);
    TEST_SP_HRT(true);
  }

  void testSP20()
  {
    TEST_SP(SP20, 20u, OW, OW, 2u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(false);
  }

  void testSP21()
  {
    TEST_SP(SP21, 21u, HRT, HRT, 1u);
    TEST_SP_PU(0u, "RT", ROUNDTRIP, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP30()
  {
    TEST_SP(SP30, 30u, OW_OW, OW, 3u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(false);
  }

  void testSP31()
  {
    TEST_SP(SP31, 31u, HRT_OW, HRT, 2u);
    TEST_SP_PU(0u, "TOJ", OPENJAW, DEST_OPENJAW);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP32()
  {
    TEST_SP(SP32, 32u, OW_HRT, HRT, 2u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OOJ", OPENJAW, ORIG_OPENJAW);
    TEST_SP_HRT(true);
  }

  void testSP33()
  {
    TEST_SP(SP33, 33u, OW, OW_OW, 3u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(false);
  }

  void testSP34()
  {
    TEST_SP(SP34, 34u, HRT, OW_HRT, 2u);
    TEST_SP_PU(0u, "TOJ", OPENJAW, DEST_OPENJAW);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP35()
  {
    TEST_SP(SP35, 35u, HRT, HRT_OW, 2u);
    TEST_SP_PU(0u, "OOJ", OPENJAW, ORIG_OPENJAW);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }
  void testSP36()
  {
    TEST_SP(SP36, 36u, HRT, HRT_HRT, 1u);
    TEST_SP_PU(0u, "CT", CIRCLETRIP, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP37()
  {
    TEST_SP(SP37, 37u, HRT_HRT, HRT, 1u);
    TEST_SP_PU(0u, "CT", CIRCLETRIP, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP40()
  {
    TEST_SP(SP40, 40u, OW_OW, OW_OW, 4u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(3u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(false);
  }

  void testSP41()
  {
    TEST_SP(SP41, 41u, HRT_HRT, HRT_HRT, 2u);
    TEST_SP_PU(0u, "RT", ROUNDTRIP, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "RT", ROUNDTRIP, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP42()
  {
    TEST_SP(SP42, 42u, HRT_OW, OW_HRT, 3u);
    TEST_SP_PU(0u, "RT", ROUNDTRIP, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP43()
  {
    TEST_SP(SP43, 43u, OW_HRT, HRT_OW, 3u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "RT", ROUNDTRIP, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP44()
  {
    TEST_SP(SP44, 44u, HRT_HRT, HRT_HRT, 2u);
    TEST_SP_PU(0u, "TOJ", OPENJAW, DEST_OPENJAW);
    TEST_SP_PU(1u, "OOJ", OPENJAW, ORIG_OPENJAW);
    TEST_SP_HRT(true);
  }

  void testSP45()
  {
    TEST_SP(SP45, 45u, HRT_OW, OW_HRT, 3u);
    TEST_SP_PU(0u, "TOJ", OPENJAW, DEST_OPENJAW);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP46()
  {
    TEST_SP(SP46, 46u, OW_HRT, HRT_OW, 3u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "OOJ", OPENJAW, ORIG_OPENJAW);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP47()
  {
    TEST_SP(SP47, 47u, OW_HRT, OW_HRT, 3u);
    TEST_SP_PU(0u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(1u, "DOW", OPENJAW, DOUBLE_OPENJAW);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP48()
  {
    TEST_SP(SP48, 48u, HRT_OW, HRT_OW, 3u);
    TEST_SP_PU(0u, "DOW", OPENJAW, DOUBLE_OPENJAW);
    TEST_SP_PU(1u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_PU(2u, "OW", ONEWAY, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }

  void testSP49()
  {
    TEST_SP(SP49, 49u, HRT_HRT, HRT_HRT, 1u);
    TEST_SP_PU(0u, "CT", CIRCLETRIP, UNKNOWN_SUBTYPE);
    TEST_SP_HRT(true);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SolutionPatternTest);

} /* namespace shpq */
} /* namespace tse */
