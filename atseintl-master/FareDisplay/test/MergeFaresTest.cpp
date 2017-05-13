//----------------------------------------------------------------------------
//	File: MergeFaresTest.cpp
//
//	Author: Jeff Hoffman
//	Created:      10/26/2006
//	Description:  This is a unit test class for MergeFaresTest.cpp
//
//  Copyright Sabre 2005
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

#include "Common/FareDisplayUtil.h"
#include "Common/TSEException.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/MergeFares.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

#include <cstdarg>

using namespace std;

namespace
{
constexpr bool IS_DOMESTIC = true;
constexpr bool IS_COPYALL = true;
constexpr int DUP_FARES_IN_MARKET = 1;
constexpr int DIFF_FARES_IN_MARKET = 0;
}

namespace tse
{


class MergeFaresTest : public CppUnit::TestFixture
{
  class FakePaxType : public PaxType
  {
    TestMemHandle _memHandle;

  public:
    FakePaxType(PaxTypeCode ptc = "ADT", Indicator child = 0, Indicator infant = 0) : PaxType()
    {
      PaxTypeInfo* pti = _memHandle.create<PaxTypeInfo>();
      pti->paxType() = ptc;
      pti->childInd() = child;
      pti->infantInd() = infant;
      pti->initPsgType();
      paxType() = ptc;
      paxTypeInfo() = pti;
    };
  };

  class FakePaxTypeFare : public PaxTypeFare
  {
    bool _isValid;
    TestMemHandle _memHandle;

  public:
    FakePaxTypeFare(PaxTypeCode ptc)
    {
      FakePaxTypeFare(true, ptc);
    };
    FakePaxTypeFare(bool valid = true, PaxTypeCode ptc = "ADT") : _isValid(valid)
    {
      setFare(_memHandle.create<Fare>());
      FareInfo* fi = _memHandle.create<FareInfo>();
      fare()->setFareInfo(fi);
      _actualPaxType = _memHandle.create<FakePaxType>();
      _fareDisplayInfo = _memHandle.create<FareDisplayInfo>();

      fi->globalDirection() = GlobalDirection::EU; // arbitrary non-blank dir
    }
    bool isValidForPricing() const
    {
      return _isValid;
    };
    void setValid(bool valid = true) { _isValid = valid; }
    void setInvalid() { setValid(false); }
    void setPaxType(PaxType* pt) { _actualPaxType = pt; }

    // these only exersice one way to make same/different
    // see tests on areEqual for ensuring all ways a exercised
    void makeSame(FakePaxTypeFare* source)
    {
      nucFareAmount() = source->nucFareAmount();
      setPaxType(source->paxType());
    }
    void makeDifferent(FakePaxTypeFare* source) { nucFareAmount() = source->nucFareAmount() + 100; }

    // static so we can create predicate on-the-fly w/o defining a functor class
    // not really a method on Fakes; here because tightly coupled to how same/different defined
    static bool areSame(PaxTypeFare* lhs, PaxTypeFare* rhs)
    {
      return lhs->nucFareAmount() == rhs->nucFareAmount();
    }

    FareInfo* fareInfo() { return const_cast<FareInfo*>(_fare->fareInfo()); }
    virtual string createFareBasisCodeFD(FareDisplayTrx& trx) const
    {
      // NULL is OK here, will bypass db call for trim length in FareCalcConfig
      string fareBasis = createFareBasis(NULL);
      insertBookingCode(fareBasis);
      return fareBasis;
    }
  };

  CPPUNIT_TEST_SUITE(MergeFaresTest);

  CPPUNIT_TEST(areEqual_diffAmt);
  CPPUNIT_TEST(areEqual_diffCreateDate);
  CPPUNIT_TEST(areEqual_diffVendor);
  CPPUNIT_TEST(areEqual_diffCarrier);
  CPPUNIT_TEST(areEqual_diffBasis);
  CPPUNIT_TEST(areEqual_same);

  CPPUNIT_TEST(mergeFares_Empty);
  CPPUNIT_TEST(mergeFares_EmptyMkts);
  CPPUNIT_TEST(mergeFares_1ptf);
  CPPUNIT_TEST(mergeFares_9Same);
  CPPUNIT_TEST(mergeFares_9Diff);
  CPPUNIT_TEST(mergeFares_AllInvalid);
  CPPUNIT_TEST(mergeFares_SomeInvalid);
  CPPUNIT_TEST(mergeFares_SomeDupe);
  CPPUNIT_TEST(mergeFares_MixInvalid);
  CPPUNIT_TEST(mergeFares_MixInvalid_ShortRD);
  CPPUNIT_TEST(mergeFares_DupInAnotherMarket);

  CPPUNIT_TEST(mergeFares_copyAll);
  CPPUNIT_TEST(mergeFares_copyAllValid_dispPaxType);
  CPPUNIT_TEST(mergeFares_copyAllValid_InclusionCode);

  CPPUNIT_TEST(mergeFares_1NEG);
  CPPUNIT_TEST(mergeFares_1CNN);
  CPPUNIT_TEST(mergeFares_noMergeADT_CNN);
  CPPUNIT_TEST(mergeFares_noMergeUnsupportedPax);
  CPPUNIT_TEST(mergeFares_mixPax);

  CPPUNIT_SKIP_TEST(mergeFares_PerfAllGood);
  CPPUNIT_SKIP_TEST(mergeFares_PerfDupe);
  CPPUNIT_SKIP_TEST(mergeFares_PerfMerge);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memHandle;
  MergeFares* _merge;
  EqualBy* _eq;

  FareDisplayTrx* _trx;
  std::vector<FareMarket*> _markets;
  FareMarket* _market1;
  FakePaxTypeFare* _ptf1, *_ptf2;
  FakePaxType* _ptADT, *_ptCNN, *_ptINF, *_ptNEG, *_ptXAX;

  // keep list of all created, so we don't have to do dynamic cast on
  // pointers from allPaxTypeFare()
  std::vector<FakePaxTypeFare*> _fakePTFs;
  std::vector<PaxTypeFare*>* _resultVec;

  void setUp()
  {
    _market1 = _memHandle.create<FareMarket>(); // one mkt with 0 or more PaxTypeFares
    _markets.push_back(_market1); // usually do requests w/ only one market
    _trx = _memHandle.create<FareDisplayTrx>(); // has output list of PTFs
    _trx->setRequest(_memHandle.create<FareDisplayRequest>());
    _trx->setOptions(_memHandle.create<FareDisplayOptions>());
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
    _resultVec = &_trx->allPaxTypeFare(); // for handy access

    _merge = _memHandle.insert(new MergeFares(*_trx));
    _eq = _memHandle.insert(new EqualBy(_trx));

    _ptADT = _memHandle.create<FakePaxType>();
    _ptCNN = _memHandle.insert(new FakePaxType("CNN", 'Y', 'N')); // childInd == yes
    _ptINF = _memHandle.insert(new FakePaxType("INF", 'N', 'Y')); // infantInd == yes
    _ptNEG = _memHandle.insert(new FakePaxType("NEG"));
    _ptXAX = _memHandle.insert(new FakePaxType("XAX"));

    _ptf1 = _memHandle.create<FakePaxTypeFare>();
    _ptf2 = _memHandle.create<FakePaxTypeFare>();
    _ptf1->setPaxType(_ptADT);
    _ptf2->setPaxType(_ptADT);

  }

  void tearDown()
  {
    _memHandle.clear();
    _markets.clear();
    _fakePTFs.clear();
  }

  //////////////////////////////////////
  //
  // utils
  //
  /////////////////////////////////////
  void usualMergeAssertSize(int i)
  {
    for (vector<FareMarket*>::iterator iter = _markets.begin(); iter != _markets.end(); iter++)
    {
      fillCortege(*iter);
    }

    _merge->run(_markets, !IS_COPYALL, !IS_DOMESTIC);
    CPPUNIT_ASSERT_EQUAL((size_t)i, _resultVec->size());
  }

  void assertMerged(PaxTypeFare* origFare1, PaxTypeFare* origFare2)
  {
    // TODO:update when merged fare has more than just paxtype data and/or how merged data is stored

    // find the merged fare 2 ways
    vector<PaxTypeFare*>::iterator mergedFare1, mergedFare2;

    mergedFare1 = find_if(_resultVec->begin(),
                          _resultVec->end(),
                          bind2nd(ptr_fun(FakePaxTypeFare::areSame), origFare1));
    mergedFare2 = find_if(_resultVec->begin(),
                          _resultVec->end(),
                          bind2nd(ptr_fun(FakePaxTypeFare::areSame), origFare2));

    // all the unmerged fares in the input are assumed distinct & areSame() ignores paxtype, so
    // the 2 found fares should be the same
    CPPUNIT_ASSERT(mergedFare1 != _resultVec->end());
    CPPUNIT_ASSERT(mergedFare1 == mergedFare2);

    // this temp is handy because the paxtypes of a merged fare are in different places
    // (i.e. don't know if actualPaxType came from origFare1 or origFare2)
    set<PaxTypeCode> mergedPaxTypes = (*mergedFare1)->fareDisplayInfo()->passengerTypes();
    mergedPaxTypes.insert((*mergedFare1)->actualPaxType()->paxType());

    PaxTypeCode ptc1 = origFare1->actualPaxType()->paxType();
    PaxTypeCode ptc2 = origFare2->actualPaxType()->paxType();

    CPPUNIT_ASSERT(mergedPaxTypes.find(ptc1) != mergedPaxTypes.end());
    CPPUNIT_ASSERT(mergedPaxTypes.find(ptc2) != mergedPaxTypes.end());
  }

  void assertMerged(int numFares, PaxTypeFare* firstOrigFare, ...)
  {
    va_list allFares;
    va_start(allFares, firstOrigFare);
    PaxTypeFare* targetFare = firstOrigFare;

    vector<PaxTypeFare*>::iterator mergedFare, otherFoundFare;

    mergedFare = find_if(_resultVec->begin(),
                         _resultVec->end(),
                         bind2nd(ptr_fun(FakePaxTypeFare::areSame), targetFare));

    CPPUNIT_ASSERT(mergedFare != _resultVec->end());

    // TODO:update when merged fare has more than just paxtype data and/or how merged data is stored
    // this temp is handy because the paxtypes of a merged fare are in different places
    // (i.e. first fare found in mergedFare and first original fare may have different
    // actualPaxType)
    set<PaxTypeCode> mergedPaxTypes = (*mergedFare)->fareDisplayInfo()->passengerTypes();
    mergedPaxTypes.insert((*mergedFare)->actualPaxType()->paxType());

    for (int i = 0; i < numFares; i++)
    {
      if (i != 0) // no need to check the first fare with itself
      {
        // all the unmerged fares in the input are assumed distinct & areSame() ignores paxtype, so
        // other matching fares should be the same as the first matched fare
        otherFoundFare = find_if(_resultVec->begin(),
                                 _resultVec->end(),
                                 bind2nd(ptr_fun(FakePaxTypeFare::areSame), targetFare));
        CPPUNIT_ASSERT(otherFoundFare != _resultVec->end());
        CPPUNIT_ASSERT(mergedFare == otherFoundFare);
      }

      // verify merged fared has all the goodies from this orginal
      CPPUNIT_ASSERT(mergedPaxTypes.find(targetFare->actualPaxType()->paxType()) !=
                     mergedPaxTypes.end());
      targetFare = va_arg(allFares, PaxTypeFare*);
    }
  }

  void fillSame(int count, FareMarket* mkt = NULL)
  {
    FareMarket* fm = (mkt) ? mkt : _market1;
    for (int i = 0; i < count; i++)
    {
      FakePaxTypeFare* fake = _memHandle.insert(new FakePaxTypeFare("ADT"));
      fake->setPaxType(_ptADT);
      fm->allPaxTypeFare().push_back(fake);
      _fakePTFs.push_back(fake); // so tests can access fake class directly
    }
  }

  void fillDifferent(int count, FareMarket* mkt = NULL)
  {
    fillSame(count, mkt);
    for (int i = 1; i < count; i++)
      _fakePTFs[i]->makeDifferent(_fakePTFs[i - 1]);
  }

  // startIndex=0 to ensure new PTF different from previously created
  // startIndex=1 for duplicte amount sequence  e.g PTF[0] == PTF[start]
  void fillDifferentPax(int count, int startIndex = 1, FareMarket* mkt = NULL)
  {
    // where fillSame() will put these new PTFs
    int offset = _fakePTFs.size();
    // saftey check
    if (offset == 0)
      startIndex = 1;

    fillSame(count, mkt);

    for (int i = startIndex + offset; i < count + offset; i++)
    {
      _fakePTFs[i]->makeDifferent(_fakePTFs[i - 1]);
      switch (i % 10)
      {
      case 2:
        _fakePTFs[i]->setPaxType(_ptCNN);
        break;
      case 3:
        _fakePTFs[i]->setPaxType(_ptCNN);
        break;
      case 4:
        _fakePTFs[i]->setPaxType(_ptNEG);
        break;
      case 5:
        _fakePTFs[i]->setPaxType(_ptXAX);
        break;
      case 6:
        _fakePTFs[i]->setPaxType(_ptINF);
        break;
      default:
        break;
      }
    }
  }

  vector<PaxTypeFare*>& addBucket(PaxType* pt, FareMarket* fm, int index)
  {
    PaxTypeBucket* ptc = &(fm->paxTypeCortege()[index]);
    ptc->requestedPaxType() = pt;
    ptc->actualPaxType().push_back(pt);
    return ptc->paxTypeFare();
  }

  void fillCortege(FareMarket* mkt = NULL)
  {
    FareMarket* fm = (mkt) ? mkt : _market1;
    fm->paxTypeCortege().resize(6);
    vector<PaxTypeFare*>& vecADT = addBucket(_ptADT, fm, 0);
    vector<PaxTypeFare*>& vecCNN = addBucket(_ptCNN, fm, 1);
    vector<PaxTypeFare*>& vecINF = addBucket(_ptINF, fm, 2);
    vector<PaxTypeFare*>& vecNEG = addBucket(_ptNEG, fm, 3);
    vector<PaxTypeFare*>& vecXAX = addBucket(_ptXAX, fm, 4);

    vector<PaxTypeFare*>::iterator iterFare = fm->allPaxTypeFare().begin();
    vector<PaxTypeFare*>::iterator iterEnd = fm->allPaxTypeFare().end();
    for (; iterFare != iterEnd; iterFare++)
    {
      if ((*iterFare)->actualPaxType() == _ptADT)
        vecADT.push_back(*iterFare);
      else if ((*iterFare)->actualPaxType() == _ptCNN)
        vecCNN.push_back(*iterFare);
      else if ((*iterFare)->actualPaxType() == _ptINF)
        vecINF.push_back(*iterFare);
      else if ((*iterFare)->actualPaxType() == _ptNEG)
        vecNEG.push_back(*iterFare);
      else if ((*iterFare)->actualPaxType() == _ptXAX)
        vecXAX.push_back(*iterFare);
    }
  }

  void setInvalidByIndex(int numIndex, int firstIndex, ...)
  {
    va_list allIndexes;
    va_start(allIndexes, firstIndex);
    int thisIndex = firstIndex;
    for (int i = 0; i < numIndex; i++)
    {
      _fakePTFs[thisIndex]->setInvalid();
      thisIndex = va_arg(allIndexes, int);
    }
  };

  void makeDuplicateByIndex(int numIndex, FakePaxTypeFare* source, int firstIndex, ...)
  {
    va_list allIndexes;
    va_start(allIndexes, firstIndex);
    int targetIndex = firstIndex;

    for (int i = 0; i < numIndex; i++)
    {
      _fakePTFs[targetIndex]->makeSame(source);
      targetIndex = va_arg(allIndexes, int);
    }
  };

  void makeMergeCandidate(int index1, int index2, PaxType* pt1, PaxType* pt2)
  {
    makeDuplicateByIndex(1, _fakePTFs[index1], index2);
    _fakePTFs[index1]->setPaxType(pt1);
    _fakePTFs[index2]->setPaxType(pt2);
  }

  //////////////////////////////////////
  //
  // areEqual
  //
  /////////////////////////////////////

  void areEqual_diffAmt()
  {
    _ptf2->nucFareAmount() = 99;
    CPPUNIT_ASSERT(!_eq->areEqual(_ptf1, _ptf2));
  }
  void areEqual_diffCreateDate()
  {
    _ptf1->fareInfo()->createDate() = DateTime(2005, 2, 2);
    _ptf2->fareInfo()->createDate() = DateTime::localTime();

    CPPUNIT_ASSERT(!_eq->areEqual(_ptf1, _ptf2));
  }
  void areEqual_diffVendor()
  {
    _ptf2->fareInfo()->vendor() = Vendor::SITA;
    CPPUNIT_ASSERT(!_eq->areEqual(_ptf1, _ptf2));
  }
  void areEqual_diffCarrier()
  {
    _ptf2->fareInfo()->carrier() = INDUSTRY_CARRIER;
    Fare* fare(_ptf2->fare());
    fare->setFareInfo(_ptf2->fareInfo());
    // have to call setFare to update cache
    _ptf2->setFare(fare);
    CPPUNIT_ASSERT(!_eq->areEqual(_ptf1, _ptf2));
  }
  void areEqual_diffBasis()
  {
    _ptf2->fareInfo()->fareClass() = "DIFF";
    CPPUNIT_ASSERT(!_eq->areEqual(_ptf1, _ptf2));
  }

  void areEqual_same() { CPPUNIT_ASSERT(_eq->areEqual(_ptf1, _ptf2)); }

  //////////////////////////////////////
  //
  // mergeFares - simple
  //
  /////////////////////////////////////
  void mergeFares_Empty()
  {
    _markets.empty(); // undo setup
    usualMergeAssertSize(0);
  }

  void mergeFares_EmptyMkts()
  {
    _markets.push_back(_memHandle.insert(new FareMarket())); // 2nd market
    usualMergeAssertSize(0);
  }

  void mergeFares_1ptf()
  {
    _market1->allPaxTypeFare().push_back(_ptf1);
    usualMergeAssertSize(1);
  }

  void mergeFares_9Same()
  {
    fillSame(9);
    usualMergeAssertSize(1);
  }

  void mergeFares_9Diff()
  {
    int size = 9;
    fillDifferent(size);
    usualMergeAssertSize(size);
  }
  //////////////////////////////////////
  //
  // mergeFares - dupes and invalid
  //
  /////////////////////////////////////

  void mergeFares_AllInvalid()
  {
    fillDifferent(30);
    for (int i = 0; i < 30; i++)
      _fakePTFs[i]->setInvalid();

    usualMergeAssertSize(0);
  }

  void mergeFares_SomeInvalid()
  {
    int size = 30;
    int numInvalid = 4;

    fillDifferent(size);
    setInvalidByIndex(numInvalid, 4, 14, 9, 23); // some arbitrary indexes

    usualMergeAssertSize(size - numInvalid);
  }

  void mergeFares_SomeDupe()
  {
    int size = 30;
    int numDupe = 6;

    fillDifferent(size);
    FakePaxTypeFare* fareToDuplicate = _fakePTFs[11]; // any index not used below will do
    makeDuplicateByIndex(numDupe, fareToDuplicate, 0, 1, 2, 27, 28, 29); // indexes 'on the edge'

    usualMergeAssertSize(size - numDupe);
  }

  void mergeFares_MixInvalid()
  {
    int size = 30;
    int numInvalid = 4;
    int numDupe = 5;
    int numInvalidAndDupe = 2; // index 9 , 27

    fillDifferent(size);
    FakePaxTypeFare* fareToDuplicate = _fakePTFs[0]; // any index not used below will do

    setInvalidByIndex(numInvalid, 9, 27, 18, 4); // more arbitrary indexes
    makeDuplicateByIndex(numDupe, fareToDuplicate, 9, 27, 6, 12, 25);

    // don't account twice for fares that are both invalid and dupe
    usualMergeAssertSize(size - numInvalid - numDupe + numInvalidAndDupe);
  }
  void mergeFares_MixInvalid_ShortRD()
  {
    int size = 30;
    int numInvalid = 4;
    int numDupe = 5;
    int numInvalidAndDupe = 2; // index 9 , 27

    fillDifferent(size);
    FakePaxTypeFare* fareToDuplicate = _fakePTFs[0]; // any index not used below will do

    setInvalidByIndex(numInvalid, 9, 27, 18, 4); // more arbitrary indexes
    makeDuplicateByIndex(numDupe, fareToDuplicate, 9, 27, 6, 12, 25);

    _trx->getRequest()->requestType() = "RD";
    _trx->getOptions()->lineNumber() = 11; // arbitrary; e.g. RD11$H request
    // don't account twice for fares that are both invalid and dupe
    usualMergeAssertSize(size - numInvalid - numDupe + numInvalidAndDupe);
  }

  // for performance, don't look for dupes between markets
  // the case with YY and ALL is handled during creation
  void mergeFares_DupInAnotherMarket()
  {
    int size = 20;
    fillDifferentPax(size);

    FareMarket mkt2;
    _markets.push_back(&mkt2);
    fillDifferentPax(size, DUP_FARES_IN_MARKET, &mkt2);

    usualMergeAssertSize(size * 2);
  }

  //////////////////////////////////////
  //
  // mergeFares - option bits
  //
  /////////////////////////////////////
  void mergeFares_copyAll()
  {
    int size = 40;
    int numInvalid = 4;
    int numDupe = 5;
    //  int numInvalidAndDupe = 2;  // index 9 , 27

    fillDifferent(size);
    FakePaxTypeFare* fareToDuplicate = _fakePTFs[0]; // any index not used below will do

    setInvalidByIndex(numInvalid, 9, 27, 18, 4); // more arbitrary indexes
    makeDuplicateByIndex(numDupe, fareToDuplicate, 9, 27, 6, 12, 25);

    // copy all bit forces all fares copied
    _merge->run(_markets, IS_COPYALL, !IS_DOMESTIC);
    CPPUNIT_ASSERT_EQUAL((size_t)size, _trx->allPaxTypeFare().size());
  }

  void mergeFares_copyAllValid_dispPaxType()
  {
    int size = 40;
    int numInvalid = 4;
    int numDupe = 5;
    //  int numInvalidAndDupe = 2;  // index 9 , 27

    fillDifferent(size);
    FakePaxTypeFare* fareToDuplicate = _fakePTFs[0]; // any index not used below will do

    setInvalidByIndex(numInvalid, 9, 27, 18, 4); // more arbitrary indexes
    makeDuplicateByIndex(numDupe, fareToDuplicate, 9, 27, 6, 12, 25);

    // copy valid fares without merging
    _trx->getRequest()->displayPassengerTypes().push_back("ADT"); // arbitrary paxtypes
    _trx->getRequest()->displayPassengerTypes().push_back("NEG");

    // TODO: error ?!?!?!
    // should still mark/remove dupe when no merge?
    usualMergeAssertSize(size - numInvalid);
  }

  void mergeFares_copyAllValid_InclusionCode()
  {
    int size = 40;
    int numInvalid = 4;
    int numDupe = 5;
    //  int numInvalidAndDupe = 2;  // index 9 , 27

    fillDifferent(size);
    FakePaxTypeFare* fareToDuplicate = _fakePTFs[0]; // any index not used below will do

    setInvalidByIndex(numInvalid, 9, 27, 18, 4); // more arbitrary indexes
    makeDuplicateByIndex(numDupe, fareToDuplicate, 9, 27, 6, 12, 25);

    // copy valid fares without merging
    _trx->getRequest()->requestedInclusionCode() = ALL_FARES;
    usualMergeAssertSize(size - numInvalid);
  }

  //////////////////////////////////////
  //
  // mergeFares - paxtypes
  //
  /////////////////////////////////////

  void mergeFares_1NEG()
  {
    int size = 20;
    int indexADTtoMerge = 4;
    int indexNEGtoMerge = 14;

    fillDifferent(size);
    makeMergeCandidate(indexADTtoMerge, indexNEGtoMerge, _ptADT, _ptNEG);

    // merged fares are altered, so copy original for validating proper merge
    // TODO how to refactor this out (couple makeMergeCandidate & assertMerged)
    FakePaxTypeFare fareADT, fareNEG;
    _fakePTFs[indexADTtoMerge]->clone(_trx->dataHandle(), fareADT);
    _fakePTFs[indexNEGtoMerge]->clone(_trx->dataHandle(), fareNEG);

    usualMergeAssertSize(size - 1);
    assertMerged(2, &fareADT, &fareNEG);
    assertMerged(&fareADT, &fareNEG);
  }

  void mergeFares_1CNN()
  {
    int size = 40;
    int index1toMerge = 33;
    int index2toMerge = 10;

    fillDifferent(size);
    makeMergeCandidate(index1toMerge, index2toMerge, _ptCNN, _ptINF);

    FakePaxTypeFare fare1, fare2;
    _fakePTFs[index1toMerge]->clone(_trx->dataHandle(), fare1);
    _fakePTFs[index2toMerge]->clone(_trx->dataHandle(), fare2);

    usualMergeAssertSize(size - 1);
    assertMerged(2, &fare1, &fare2);
  }
  void mergeFares_noMergeADT_CNN()
  {
    int size = 55;
    int index1toMerge = 39;
    int index2toMerge = 9;

    fillDifferent(size);
    makeMergeCandidate(index1toMerge, index2toMerge, _ptCNN, _ptADT);
    usualMergeAssertSize(size);
  }

  void mergeFares_noMergeUnsupportedPax()
  {
    int size = 2;
    int indexUnsuported = 0;

    fillSame(size);
    _fakePTFs[indexUnsuported]->setPaxType(_ptXAX); // unsupported, so un-mergeable
    _fakePTFs[indexUnsuported]->fareDisplayInfo()->unsupportedPaxType() = true;

    usualMergeAssertSize(size);
  }

  void mergeFares_mixPax()
  {
    fillSame(5);
    _fakePTFs[1]->setPaxType(_ptINF);
    _fakePTFs[2]->setPaxType(_ptCNN);
    _fakePTFs[3]->setPaxType(_ptNEG);
    _fakePTFs[4]->setPaxType(_ptXAX); // unsupported, so un-mergeable
    _fakePTFs[4]->fareDisplayInfo()->unsupportedPaxType() = true;

    // CNN & INF merge, ADT & NEG merge, XAX no merge => 3 fares
    usualMergeAssertSize(3);
  }
  //////////////////////////////////////
  //
  // mergeFares - performance
  //
  /////////////////////////////////////
  void mergeFares_PerfAllGood()
  {
    // this testcase is not a blind copy, gobs of comparisons are done
    int size = 3000;
    fillDifferentPax(size);

    FareMarket mkt2;
    _markets.push_back(&mkt2);
    // 0 ensures mkt2's fares are distinct from mkt1's
    fillDifferentPax(size, DIFF_FARES_IN_MARKET, &mkt2);

    usualMergeAssertSize(size * 2);
  }

  // NOTE: the input list is not expected to have two entries with the same pointer
  // such degenerate lists (like null pointers) should be handled upstream during creation
  void mergeFares_PerfDupe()
  {
    int size = 5000;
    fillDifferentPax(size);

    FareMarket mkt2;
    _markets.push_back(&mkt2);
    fillDifferentPax(size, DIFF_FARES_IN_MARKET, &mkt2);

    // make a fifth of the fares duplicate
    // note that duplicate checking between markets is NOT required
    for (int i = size * 2; i > 0; i -= 5)
      makeDuplicateByIndex(1, _fakePTFs[i - 1], i - 2);

    usualMergeAssertSize((size + size) * 4 / 5);
  }
  void mergeFares_PerfMerge()
  {
    int size = 10000;
    int numMergedPairs = 2;
    int numNotMergedPairs = 2;

    int totPairs = numMergedPairs + numNotMergedPairs;
    int numCandidatesPerPair = size / (totPairs * 2);

    fillDifferent(size);
    for (int i = 0; i < size; i += totPairs * 2)
    {
      makeMergeCandidate(i, i + 1, _ptCNN, _ptINF); // merged
      makeMergeCandidate(i + 2, i + 3, _ptNEG, _ptADT); // merged
      makeMergeCandidate(i + 4, i + 5, _ptXAX, _ptADT); // NOT merged
      makeMergeCandidate(i + 6, i + 7, _ptINF, _ptADT); // NOT merged
      _fakePTFs[i + 4]->fareDisplayInfo()->unsupportedPaxType() = true;
    }

    usualMergeAssertSize(size - (numMergedPairs * numCandidatesPerPair));
  }

}; // end class

CPPUNIT_TEST_SUITE_REGISTRATION(MergeFaresTest);
}
