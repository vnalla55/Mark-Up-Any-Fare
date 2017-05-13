#include "DataModel/test/NoPNRPricingTrxTest.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DBAccess/NoPNROptions.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "test/include/TestConfigInitializer.h"

using namespace tse;
using namespace std;

void
NoPNRPricingTrxTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  _trx = _memHandle.create<NoPNRPricingTrx>();
}

void
NoPNRPricingTrxTest::tearDown()
{
  _memHandle.clear();
}

void
NoPNRPricingTrxTest::testIsNoMatchXMCase()
{
  _trx->noRBDItin() = false;
  _trx->reprocess() = false;

  NoPNRPricingOptions options;
  _trx->_options = &options;

  options.noMatch() = 'Y';
  CPPUNIT_ASSERT(_trx->isNoMatch() == true);

  options.noMatch() = 'N';
  CPPUNIT_ASSERT(_trx->isNoMatch() == false);
}

void
NoPNRPricingTrxTest::testIsNoMatchRBDCase()
{
  NoPNRPricingOptions options;
  _trx->_options = &options;
  options.noMatch() = 'N';
  _trx->reprocess() = false;

  _trx->noRBDItin() = true;
  CPPUNIT_ASSERT(_trx->isNoMatch() == true);

  _trx->noRBDItin() = false;
  CPPUNIT_ASSERT(_trx->isNoMatch() == false);
}

void
NoPNRPricingTrxTest::testIsNoMatchReprocessCase()
{
  NoPNRPricingOptions options;
  _trx->_options = &options;
  options.noMatch() = 'N';
  _trx->noRBDItin() = false;

  _trx->reprocess() = true;
  CPPUNIT_ASSERT(_trx->isNoMatch() == true);

  _trx->reprocess() = false;
  CPPUNIT_ASSERT(_trx->isNoMatch() == false);
}

void
NoPNRPricingTrxTest::testFTGroupFromFareTypeEmpty()
{
  FareType search = "AA";
  CPPUNIT_ASSERT(_trx->fareTypes().getFareTypeGroup(search) ==
                 NoPNRPricingTrx::FareTypes::FTG_NONE);
}

void
NoPNRPricingTrxTest::testFTGroupFromFareTypeNotFound()
{
  NoPNRPricingTrx::FareTypes& fareTypes = _trx->fareTypes();

  FareType ft1 = "AA";
  NoPNRPricingTrx::FareTypes::FTGroup ftg1 = NoPNRPricingTrx::FareTypes::FTG_BUSINESS;
  fareTypes._fareTypes.insert(
      std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft1, ftg1));

  FareType sf1 = "BB";
  CPPUNIT_ASSERT(_trx->fareTypes().getFareTypeGroup(sf1) == NoPNRPricingTrx::FareTypes::FTG_NONE);

  FareType ft2 = "BB";
  NoPNRPricingTrx::FareTypes::FTGroup ftg2 = NoPNRPricingTrx::FareTypes::FTG_FIRST;
  fareTypes._fareTypes.insert(
      std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft2, ftg2));

  FareType sf2 = "CC";
  CPPUNIT_ASSERT(_trx->fareTypes().getFareTypeGroup(sf2) == NoPNRPricingTrx::FareTypes::FTG_NONE);
}

void
NoPNRPricingTrxTest::testFTGroupFromFareTypeFound()
{
  NoPNRPricingTrx::FareTypes& fareTypes = _trx->fareTypes();

  FareType ft1 = "AA";
  FareType ft2 = "BB";

  NoPNRPricingTrx::FareTypes::FTGroup ftg1 = NoPNRPricingTrx::FareTypes::FTG_BUSINESS;
  NoPNRPricingTrx::FareTypes::FTGroup ftg2 = NoPNRPricingTrx::FareTypes::FTG_ECONOMY;

  fareTypes._fareTypes.insert(
      std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft1, ftg1));
  fareTypes._fareTypes.insert(
      std::pair<FareType, NoPNRPricingTrx::FareTypes::FareTypeGroup>(ft2, ftg2));

  FareType sf1 = "AA";
  CPPUNIT_ASSERT(_trx->fareTypes().getFareTypeGroup(sf1) ==
                 NoPNRPricingTrx::FareTypes::FTG_BUSINESS);

  FareType sf2 = "BB";
  CPPUNIT_ASSERT(_trx->fareTypes().getFareTypeGroup(sf2) ==
                 NoPNRPricingTrx::FareTypes::FTG_ECONOMY);
}

void
NoPNRPricingTrxTest::testSolutionsMax()
{
  NoPNROptions options;
  options.maxNoOptions() = 12;
  _trx->_noPNROptions = &options;

  NoPNRPricingTrx::Solutions& solutions = _trx->solutions();

  solutions.initialize();
  CPPUNIT_ASSERT(solutions.max() == 12);
}

void
NoPNRPricingTrxTest::testSolutionsLimit()
{
  NoPNROptions options;
  options.maxNoOptions() = 12;
  _trx->_noPNROptions = &options;

  NoPNRPricingTrx::Solutions& solutions = _trx->solutions();

  PaxType pax1, pax2;
  pax1.paxType() = "ADT";
  pax2.paxType() = "CNN";

  _trx->paxType().push_back(&pax1);
  _trx->paxType().push_back(&pax2);

  solutions.initialize();
  solutions.limit(10);

  CPPUNIT_ASSERT(solutions.limit(&pax1) == 10);
  CPPUNIT_ASSERT(solutions.limit(&pax2) == 10);
}

void
NoPNRPricingTrxTest::testSolutionsLimitWithFound()
{
  NoPNROptions options;
  options.maxNoOptions() = 12;
  _trx->_noPNROptions = &options;

  NoPNRPricingTrx::Solutions& solutions = _trx->solutions();

  PaxType pax;
  pax.paxType() = "ADT";
  _trx->paxType().push_back(&pax);

  solutions.initialize();
  solutions.found(&pax, 10);
  solutions.limit(10);

  CPPUNIT_ASSERT(solutions.limit(&pax) == 2);
}

void
NoPNRPricingTrxTest::testSolutionsNone()
{
  NoPNROptions options;
  options.maxNoOptions() = 12;
  _trx->_noPNROptions = &options;

  PaxType pax;
  pax.paxType() = "ADT";
  _trx->paxType().push_back(&pax);

  NoPNRPricingTrx::Solutions& solutions = _trx->solutions();
  solutions.initialize();

  CPPUNIT_ASSERT(solutions.none());

  solutions.found(&pax, 1);

  CPPUNIT_ASSERT(!solutions.none());
}

void
NoPNRPricingTrxTest::testSolutionsAll()
{
  NoPNROptions options;
  options.maxNoOptions() = 12;
  _trx->_noPNROptions = &options;

  PaxType pax1, pax2;
  pax1.paxType() = "ADT";
  pax2.paxType() = "NEG";
  _trx->paxType().push_back(&pax1);
  _trx->paxType().push_back(&pax2);

  NoPNRPricingTrx::Solutions& solutions = _trx->solutions();
  solutions.initialize();

  CPPUNIT_ASSERT(!solutions.all());

  solutions.found(&pax1, 12);
  CPPUNIT_ASSERT(!solutions.all());

  solutions.found(&pax2, 10);
  CPPUNIT_ASSERT(!solutions.all());

  solutions.found(&pax2, 2, true);
  CPPUNIT_ASSERT(solutions.all());
}

void
NoPNRPricingTrxTest::testIsXMMatch()
{
  NoPNRPricingOptions options;
  _trx->_options = &options;

  options.noMatch() = 'N';
  CPPUNIT_ASSERT(_trx->isNoMatch() == false);
}

void
NoPNRPricingTrxTest::testIsXMNoMatch()
{
  NoPNRPricingOptions options;
  _trx->_options = &options;

  options.noMatch() = 'Y';
  CPPUNIT_ASSERT(_trx->isNoMatch() == true);
}

void
NoPNRPricingTrxTest::testNoMatchItin()
{
  const BookingCode NO_RBD_BOOKINGCODE('1');

  std::vector<TravelSeg*>::iterator iter = _trx->_travelSeg.begin();
  std::vector<TravelSeg*>::iterator iterEnd = _trx->_travelSeg.end();
  while (iter != iterEnd)
  {
    TravelSeg* seg = *iter;
    if (seg)
    {
      CPPUNIT_ASSERT(seg->bookedCabin().isEconomyClass());
      CPPUNIT_ASSERT(seg->getBookingCode() == NO_RBD_BOOKINGCODE);
    }
    ++iter;
  }
}

void
NoPNRPricingTrxTest::testPrepareNoMatchItinEmpty()
{
  _trx->_travelSeg.clear();
  _trx->prepareNoMatchItin();
  testNoMatchItin();
}

void
NoPNRPricingTrxTest::testPrepareNoMatchItinOneSeg()
{
  AirSeg seg;

  _trx->_travelSeg.push_back(&seg);
  _trx->prepareNoMatchItin();

  testNoMatchItin();
}

void
NoPNRPricingTrxTest::testPrepareNoMatchItinManySegs()
{
  AirSeg seg1, seg2;

  _trx->_travelSeg.push_back(&seg1);
  _trx->_travelSeg.push_back(&seg2);
  _trx->prepareNoMatchItin();

  testNoMatchItin();
}

void
NoPNRPricingTrxTest::testPrepareNoMatchItinFilled()
{
  AirSeg seg1, seg2, seg3;
  seg1.bookedCabin().setFirstClass(); // == '1';
  seg1.setBookingCode(BookingCode('Y'));
  seg2.bookedCabin().setFirstClass(); // == '1';
  seg2.setBookingCode(BookingCode('Y'));
  seg3.bookedCabin().setFirstClass(); // == '1';
  seg3.setBookingCode(BookingCode('Y'));

  _trx->_travelSeg.push_back(&seg1);
  _trx->_travelSeg.push_back(&seg2);
  _trx->_travelSeg.push_back(&seg3);
  _trx->prepareNoMatchItin();

  testNoMatchItin();
}

void
NoPNRPricingTrxTest::setUpFullFBCItin()
{
  TestConfigInitializer::setValue("DNATA_ENABLED", "Y", "PRICING_SVC", true);

  Itin* itin = _memHandle(new Itin);
  itin->travelSeg().push_back(_memHandle.create<AirSeg>());
  itin->travelSeg().push_back(_memHandle.create<AirSeg>());
  itin->travelSeg().push_back(_memHandle.create<AirSeg>());

  _trx->itin().push_back(itin);
}

void
NoPNRPricingTrxTest::testSetFullFBCItinNo()
{
  setUpFullFBCItin();
  CPPUNIT_ASSERT(!_trx->isFullFBCItin());
  _trx->setFullFBCItin();
  CPPUNIT_ASSERT(!_trx->isFullFBCItin());
}

void
NoPNRPricingTrxTest::testSetFullFBCItinNoPartially()
{
  setUpFullFBCItin();
  CPPUNIT_ASSERT(!_trx->isFullFBCItin());
  _trx->itin().front()->travelSeg()[0]->fareBasisCode() = "ABC";
  _trx->itin().front()->travelSeg()[1]->fareBasisCode() = "ABC";
  _trx->setFullFBCItin();
  CPPUNIT_ASSERT(!_trx->isFullFBCItin());
}

void
NoPNRPricingTrxTest::testSetFullFBCItinYes()
{
  setUpFullFBCItin();
  CPPUNIT_ASSERT(!_trx->isFullFBCItin());

  for (TravelSeg* ts : _trx->itin().front()->travelSeg())
    ts->fareBasisCode() = "ABC";

  _trx->setFullFBCItin();
  CPPUNIT_ASSERT(_trx->isFullFBCItin());
}

class PaxTypeFareStub : public PaxTypeFare
{
public:
  virtual bool getPrimeBookingCode(std::vector<BookingCode>& bookingCodeVec) const
  {
    bookingCodeVec.push_back(BookingCode('X'));
    return true;
  }
};
