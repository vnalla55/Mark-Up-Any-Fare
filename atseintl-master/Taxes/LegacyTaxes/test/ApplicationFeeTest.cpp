#include <iostream>
#include "Taxes/LegacyTaxes/test/ApplicationFeeTest.h"
#include "Taxes/LegacyTaxes/ApplicationFee.h"
#include "DBAccess/YQYRFees.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ArunkSeg.h"
#include "Taxes/LegacyTaxes/ServiceFeeRec1Validator.h"

using namespace tse;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(ApplicationFeeTest);
void
ApplicationFeeTest::init()
{
  for (int i = 0; i < 10; i++)
  {
    AirSeg* seg = new AirSeg();
    seg->segmentOrder() = i;
    seg->carrier() = "XX";
    _itinSegs.push_back(seg);
  }
}

/*void ApplicationFeeTest::dump(AllAppFees &f)
{
  cout << "\n";
  for (AllAppFees::iterator j=f.begin(); j!=f.end(); j++)
  {
    cout << (*j)->segmentOrderBegin() << " - ";
    cout << (*j)->segmentOrderEnd()   << ((*j)->yqyrFees()==NULL ? " Empty" : " Assigned") << "\n";
  }
}*/

// start with one long empty appFee and assign s1Recs to parts of it
void
ApplicationFeeTest::testOneWay()
{
  YQYRFees s1Dummy;
  bool rc = false;
  ApplicationFee empty, fullA, fullB, fullC, extra1, extra2, extra3, *next;

  // start with 0-9 empty
  init();
  // empty.init(_itinSegs.begin(), _itinSegs.end());
  //_allAppFees.push_back(&empty);
  std::vector<AirSeg*>::iterator pos1 = _itinSegs.begin();
  std::vector<AirSeg*>::iterator pos4 = _itinSegs.begin();
  std::vector<AirSeg*>::iterator pos7 = _itinSegs.begin();
  std::vector<AirSeg*>::iterator pos8 = _itinSegs.begin();

  // appfee ranges to assign: 0-0, 4-6, 8-9
  advance(pos1, 1);
  advance(pos4, 4);
  advance(pos7, 7);
  advance(pos8, 8);
  // fullA.init(_itinSegs.begin(), pos1);
  // fullB.init(pos4,              pos7);
  // fullC.init(pos8,              _itinSegs.end());
  fullA.yqyrFees() = &s1Dummy;
  fullB.yqyrFees() = &s1Dummy;
  fullC.yqyrFees() = &s1Dummy;

  // tough case is when first assign the ones on the end
  // rc = _allAppFees.add(&fullC, &extra3, &next);
  // CPPUNIT_ASSERT_EQUAL(true, rc);
  // CPPUNIT_ASSERT_EQUAL(true, (next==NULL));
  // dump(_allAppFees);

  // rc = _allAppFees.add(&fullB, &extra2, &next);
  // CPPUNIT_ASSERT_EQUAL(true, rc);
  CPPUNIT_ASSERT_EQUAL(true, (next != NULL));
  // dump(_allAppFees);

  // rc = _allAppFees.add(&fullA, &extra1, &next);
  // CPPUNIT_ASSERT_EQUAL(true, rc);
  CPPUNIT_ASSERT_EQUAL(true, (next != NULL));

  // dump(_allAppFees);
}
// test portion split based on carrier, turnaround and ARNK
void
ApplicationFeeTest::testPortion()
{
  std::vector<TravelSeg*> s;
  ArunkSeg arnkSeg;
  arnkSeg.segmentOrder() = 1;
  init();
  Itin it;
  // tweak during copy
  std::vector<AirSeg*>::iterator iter = _itinSegs.begin();
  for (int i = 0; iter != _itinSegs.end(); iter++, i++)
  {
    if (i > 7)
      (*iter)->carrier() = "ZZ";
    if (i == 4)
      (*iter)->furthestPoint(it) = true;

    if (i == 1)
      s.push_back(&arnkSeg);
    else
      s.push_back(*iter);
  }
  ServiceFeeRec1Validator val;
  TaxResponse taxResp;
  FarePath fp;
  //  DataHandle dh;
  PricingTrx trx;

  it.travelSeg() = s;
  fp.itin() = &it;
  taxResp.farePath() = &fp;
  //  trx.dataHandle()  = dh;

  val.init(trx, taxResp);
  // dump (val.allAppFees());
  // CPPUNIT_ASSERT_EQUAL(true, val.allAppFees().size() == 4);
}
