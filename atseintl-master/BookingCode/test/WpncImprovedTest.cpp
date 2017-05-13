//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "BookingCode/BookingCodeExceptionValidator.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>
#include <gtest/gtest.h>

#include <algorithm>

namespace tse
{
using namespace ::testing;
using boost::assign::operator+=;

class BookingCodeExceptionValidatorMock : public BookingCodeExceptionValidator
{
public:
  BookingCodeExceptionValidatorMock(Itin& itin, PricingTrx& trx)
    : BookingCodeExceptionValidator(itin, false, false, false), _trx(trx)

  {
    _cs.bookingCode() = "H";
    _cs.cabin().setEconomyClass();
  }

  void callRebookSegmentWPNC(PaxTypeFare& paxTypeFare)
  {
    return rebookSegmentWPNC(_trx, paxTypeFare, 0, _cs, paxTypeFare.segmentStatus());
  }

  PricingTrx& _trx;
  ClassOfService _cs;
};

class FareBookingCodeValidatorMock : public FareBookingCodeValidator
{
public:
  FareBookingCodeValidatorMock(PricingTrx& trx,
                               FareMarket& fareMarket,
                               Itin* itin,
                               BookingCodeExceptionValidatorMock& bcev)
    : FareBookingCodeValidator(trx, fareMarket, itin), _bcev(bcev), _asBookedValidation(false)
  {
  }

  virtual void
  validateAndSetUp(std::vector<PaxTypeFare*>& ptcFares, bool noRBDChk, bool& passAnyFares)
  {
    if (!_asBookedValidation)
    {
      EXPECT_TRUE(_trx.getRequest()->isLowFareRequested());
      for (PaxTypeFare* ptf : ptcFares)
      {
        _bcev.callRebookSegmentWPNC(*ptf);
        EXPECT_FALSE(ptf->iAmAsBookedClone());
        EXPECT_TRUE(ptf->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
        EXPECT_EQ(BookingCode("H"), ptf->segmentStatus()[0]._bkgCodeReBook);
        EXPECT_TRUE(ptf->segmentStatus()[0]._reBookCabin.isEconomyClass());
      }
    }
    else
    {
      EXPECT_EQ('N', _trx.getRequest()->lowFareRequested());
      for (PaxTypeFare* ptf : ptcFares)
      {
        EXPECT_EQ(NULL, ptf->getAsBookedClone());
        EXPECT_TRUE(ptf->iAmAsBookedClone());
        EXPECT_FALSE(ptf->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
      }
    }
  }

  void callValidateAndSetUp(std::vector<PaxTypeFare*>& ptfs)
  {
    bool d;
    validateAndSetUp(ptfs, false, d);
  }

  void callHandleClones()
  {
    _asBookedValidation = true;
    bool d;
    handleAsBookedClones(false, d);
  }

  BookingCodeExceptionValidatorMock& _bcev;
  bool _asBookedValidation;
};

class WpncImprovedTest : public ::testing::Test
{
public:
  PricingTrx* _trx;
  Itin* _itin;
  FareMarket* _fareMarket;
  PaxTypeBucket* _adtCortege;
  PaxTypeBucket* _skyCortege;

  BookingCodeExceptionValidatorMock* _bceVal;
  FareBookingCodeValidatorMock* _fbcVal;

  TestMemHandle _memHandle;

  void SetUp()
  {
    _memHandle(new TestConfigInitializer);

    _trx = _memHandle(new PricingTrx);
    _itin = _memHandle(new Itin);

    _trx->setOptions(_memHandle(new PricingOptions));
    _trx->getOptions()->setRtw(true);
    _trx->setRequest(_memHandle(new PricingRequest));
    _trx->getRequest()->lowFareRequested() = 'Y';
    _fareMarket = _memHandle(new FareMarket);
    _fareMarket->paxTypeCortege().resize(2u);
    _adtCortege = &_fareMarket->paxTypeCortege()[0];
    _skyCortege = &_fareMarket->paxTypeCortege()[1];

    _bceVal = _memHandle(new BookingCodeExceptionValidatorMock(*_itin, *_trx));
    _fbcVal = _memHandle(new FareBookingCodeValidatorMock(*_trx, *_fareMarket, _itin, *_bceVal));
  }

  void TearDown() { _memHandle.clear(); }

  PaxTypeFare* createPtf()
  {
    PaxTypeFare* ptf = _memHandle(new PaxTypeFare);
    ptf->segmentStatus().push_back(PaxTypeFare::SegmentStatus());
    return ptf;
  }

  void addPtf(PaxTypeFare* ptf, bool adt = true, bool sky = false)
  {
    if (adt)
      _adtCortege->paxTypeFare() += ptf;
    if (sky)
      _skyCortege->paxTypeFare() += ptf;
    _fareMarket->allPaxTypeFare() += ptf;
  }

  void addPtfs(size_t howMany, bool adt = true, bool sky = false)
  {
    for (size_t i = 0; i < howMany; ++i)
      addPtf(createPtf(), adt, sky);
  }

  void expectVectorsSize(size_t total, size_t adtSize = 0, size_t skySize = 0)
  {
    EXPECT_EQ(total, _fareMarket->allPaxTypeFare().size());
    EXPECT_EQ(adtSize, _adtCortege->paxTypeFare().size());
    EXPECT_EQ(skySize, _skyCortege->paxTypeFare().size());
  }

  bool contains(const std::vector<PaxTypeFare*>& ptfs, PaxTypeFare* ptf)
  {
    return std::find(ptfs.begin(), ptfs.end(), ptf) != ptfs.end();
  }

  void expectExists(PaxTypeFare* ptf, bool all, bool adt, bool sky)
  {
    EXPECT_EQ(all, contains(_fareMarket->allPaxTypeFare(), ptf));
    EXPECT_EQ(adt, contains(_adtCortege->paxTypeFare(), ptf));
    EXPECT_EQ(sky, contains(_skyCortege->paxTypeFare(), ptf));
  }

  void testPtfsWithAsBooked()
  {
    for (size_t i = 0; i < _fareMarket->allPaxTypeFare().size(); ++i)
    {
      PaxTypeFare* ptf = _fareMarket->allPaxTypeFare()[i];
      const bool shouldBeAsBooked = bool(i % 2);
      EXPECT_EQ(shouldBeAsBooked, ptf->iAmAsBookedClone());
    }
  }
};

TEST_F(WpncImprovedTest, testRebook)
{
  addPtfs(3);
  expectVectorsSize(3, 3);

  _fbcVal->callValidateAndSetUp(_fareMarket->allPaxTypeFare());

  expectVectorsSize(3, 3);
}

TEST_F(WpncImprovedTest, testAsBookedClone_SingleCortege)
{
  addPtfs(3);
  expectVectorsSize(3, 3);

  _fbcVal->callHandleClones();

  EXPECT_TRUE(_trx->getRequest()->isLowFareRequested());
  expectVectorsSize(6, 6);
  testPtfsWithAsBooked();
}

TEST_F(WpncImprovedTest, testAsBookedClone_TwoCorteges)
{
  PaxTypeFare* adt = createPtf();
  PaxTypeFare* sky = createPtf();
  PaxTypeFare* common = createPtf();
  addPtf(adt, true, false);
  addPtf(sky, false, true);
  addPtf(common, true, true);

  expectVectorsSize(3, 2, 2);

  _fbcVal->callHandleClones();

  EXPECT_TRUE(_trx->getRequest()->isLowFareRequested());
  expectVectorsSize(6, 4, 4);
  expectExists(adt->getAsBookedClone(), true, true, false);
  expectExists(sky->getAsBookedClone(), true, false, true);
  expectExists(common->getAsBookedClone(), true, true, true);
  testPtfsWithAsBooked();
}
}
