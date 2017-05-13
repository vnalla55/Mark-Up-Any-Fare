#include "test/include/CppUnitHelperMacros.h"
#include <string>
#include "DataModel/PrivateIndicator.h"
#include "DataModel/Fare.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

using namespace std;
namespace tse
{
class PrivateIndicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PrivateIndicatorTest);
  CPPUNIT_TEST(testPrivateIndicatorDoesNothingForNotPrivateTariffs);
  CPPUNIT_TEST(testResolvePrivateFareInd_From_NotPrivate);
  CPPUNIT_TEST(testResolvePrivateFareInd_From_CorpIDSeq);
  CPPUNIT_TEST(testResolvePrivateFareInd_From_Cat35Seq);
  CPPUNIT_TEST(testResolvePrivateFareInd_From_XTktC35);
  CPPUNIT_TEST(testResolvePrivateFareInd_From_Private);
  CPPUNIT_TEST(testResolvePrivateFareIndNew_From_NotPrivate);
  CPPUNIT_TEST(testResolvePrivateFareIndNew_From_CorpIDSeq);
  CPPUNIT_TEST(testResolvePrivateFareIndNew_From_Cat35Seq);
  CPPUNIT_TEST(testResolvePrivateFareIndNew_From_XTktC35);
  CPPUNIT_TEST(testResolvePrivateFareIndNew_From_Private);
  CPPUNIT_TEST_SUITE_END();

  void testPrivateIndicatorDoesNothingForNotPrivateTariffs()
  {
    TariffCrossRefInfo tariffCrossRef;
    tariffCrossRef.tariffCat() = 0;
    CPPUNIT_ASSERT(tariffCrossRef.tariffCat() != RuleConst::PRIVATE_TARIFF);

    Fare fare;
    fare.setTariffCrossRefInfo(&tariffCrossRef);

    PaxTypeFare ptFare;
    ptFare.setFare(&fare);

    string privateInd("");
    bool setToBlank(false);
    bool isFQ(false);

    PrivateIndicator::privateIndicatorOld(ptFare, privateInd, setToBlank, isFQ);
    CPPUNIT_ASSERT_EQUAL(string(""), privateInd);
  }

  void test(uint16_t targetIndSeq, uint16_t nextIndSeq, uint16_t resultIndSeq)
  {
    uint16_t targetInd = targetIndSeq;
    PrivateIndicator::resolvePrivateFareIndOld(targetInd, nextIndSeq);
    CPPUNIT_ASSERT_EQUAL(resultIndSeq, targetInd);
  }

  void testNew(uint16_t targetIndSeq, uint16_t nextIndSeq, uint16_t resultIndSeq)
  {
    uint16_t targetInd = targetIndSeq;
    PrivateIndicator::resolvePrivateFareInd(targetInd, nextIndSeq);
    CPPUNIT_ASSERT_EQUAL(resultIndSeq, targetInd);
  }

  void testResolvePrivateFareInd_From_NotPrivate()
  {
    test(PrivateIndicator::NotPrivate, PrivateIndicator::NotPrivate, PrivateIndicator::NotPrivate);
    test(PrivateIndicator::NotPrivate, PrivateIndicator::CorpIDSeq, PrivateIndicator::CorpIDSeq);
    test(PrivateIndicator::NotPrivate, PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq);
    test(PrivateIndicator::NotPrivate, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    test(PrivateIndicator::NotPrivate, PrivateIndicator::Private, PrivateIndicator::Private);
  }

  void testResolvePrivateFareInd_From_CorpIDSeq()
  {
    test(PrivateIndicator::CorpIDSeq, PrivateIndicator::NotPrivate, PrivateIndicator::CorpIDSeq);
    test(PrivateIndicator::CorpIDSeq, PrivateIndicator::CorpIDSeq, PrivateIndicator::CorpIDSeq);
    test(PrivateIndicator::CorpIDSeq, PrivateIndicator::Cat35Seq, PrivateIndicator::Private);
    test(PrivateIndicator::CorpIDSeq, PrivateIndicator::XTktC35, PrivateIndicator::Private);
    test(PrivateIndicator::CorpIDSeq, PrivateIndicator::Private, PrivateIndicator::Private);
  }

  void testResolvePrivateFareInd_From_Cat35Seq()
  {
    test(PrivateIndicator::Cat35Seq, PrivateIndicator::NotPrivate, PrivateIndicator::Cat35Seq);
    test(PrivateIndicator::Cat35Seq, PrivateIndicator::CorpIDSeq, PrivateIndicator::Private);
    test(PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq);
    test(PrivateIndicator::Cat35Seq, PrivateIndicator::XTktC35, PrivateIndicator::Private);
    test(PrivateIndicator::Cat35Seq, PrivateIndicator::Private, PrivateIndicator::Private);
  }

  void testResolvePrivateFareInd_From_XTktC35()
  {
    test(PrivateIndicator::XTktC35, PrivateIndicator::NotPrivate, PrivateIndicator::XTktC35);
    test(PrivateIndicator::XTktC35, PrivateIndicator::CorpIDSeq, PrivateIndicator::Private);
    test(PrivateIndicator::XTktC35, PrivateIndicator::Cat35Seq, PrivateIndicator::Private);
    test(PrivateIndicator::XTktC35, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    test(PrivateIndicator::XTktC35, PrivateIndicator::Private, PrivateIndicator::Private);
  }

  void testResolvePrivateFareInd_From_Private()
  {
    test(PrivateIndicator::Private, PrivateIndicator::NotPrivate, PrivateIndicator::Private);
    test(PrivateIndicator::Private, PrivateIndicator::CorpIDSeq, PrivateIndicator::Private);
    test(PrivateIndicator::Private, PrivateIndicator::Cat35Seq, PrivateIndicator::Private);
    test(PrivateIndicator::Private, PrivateIndicator::XTktC35, PrivateIndicator::Private);
    test(PrivateIndicator::Private, PrivateIndicator::Private, PrivateIndicator::Private);
  }

  void testResolvePrivateFareIndNew_From_NotPrivate()
  {
    testNew(PrivateIndicator::NotPrivate, PrivateIndicator::NotPrivate, PrivateIndicator::NotPrivate);
    testNew(PrivateIndicator::NotPrivate, PrivateIndicator::CorpIDSeq, PrivateIndicator::CorpIDSeq);
    testNew(PrivateIndicator::NotPrivate, PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq);
    testNew(PrivateIndicator::NotPrivate, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::NotPrivate, PrivateIndicator::Private, PrivateIndicator::Private);
    testNew(PrivateIndicator::Private, PrivateIndicator::NotPrivate, PrivateIndicator::Private);
  }

  void testResolvePrivateFareIndNew_From_CorpIDSeq()
  {
    testNew(PrivateIndicator::CorpIDSeq, PrivateIndicator::NotPrivate, PrivateIndicator::CorpIDSeq);
    testNew(PrivateIndicator::CorpIDSeq, PrivateIndicator::CorpIDSeq, PrivateIndicator::CorpIDSeq);
    testNew(PrivateIndicator::CorpIDSeq, PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq);
    testNew(PrivateIndicator::CorpIDSeq, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::CorpIDSeq, PrivateIndicator::Private, PrivateIndicator::Private);
  }

  void testResolvePrivateFareIndNew_From_Cat35Seq()
  {
    testNew(PrivateIndicator::Cat35Seq, PrivateIndicator::NotPrivate, PrivateIndicator::Cat35Seq);
    testNew(PrivateIndicator::Cat35Seq, PrivateIndicator::CorpIDSeq, PrivateIndicator::Cat35Seq);
    testNew(PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq);
    testNew(PrivateIndicator::Cat35Seq, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::Cat35Seq, PrivateIndicator::Private, PrivateIndicator::Cat35Seq);
  }

  void testResolvePrivateFareIndNew_From_XTktC35()
  {
    testNew(PrivateIndicator::XTktC35, PrivateIndicator::NotPrivate, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::XTktC35, PrivateIndicator::CorpIDSeq, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::XTktC35, PrivateIndicator::Cat35Seq, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::XTktC35, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::XTktC35, PrivateIndicator::Private, PrivateIndicator::XTktC35);
  }

  void testResolvePrivateFareIndNew_From_Private()
  {
    testNew(PrivateIndicator::Private, PrivateIndicator::NotPrivate, PrivateIndicator::Private);
    testNew(PrivateIndicator::Private, PrivateIndicator::CorpIDSeq, PrivateIndicator::Private);
    testNew(PrivateIndicator::Private, PrivateIndicator::Cat35Seq, PrivateIndicator::Cat35Seq);
    testNew(PrivateIndicator::Private, PrivateIndicator::XTktC35, PrivateIndicator::XTktC35);
    testNew(PrivateIndicator::Private, PrivateIndicator::Private, PrivateIndicator::Private);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PrivateIndicatorTest);
};
