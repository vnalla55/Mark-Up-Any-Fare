//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Diagnostic/Diag660Collector.h"

#include "test/include/ATSEv2Test.h"
#include "test/include/TestDataBuilders.h"

namespace tse
{

class Diag660CollectorTest : public ATSEv2Test
{
public:
  void SetUp()
  {
    ATSEv2Test::SetUp();
    _diag = getDiagCollector<Diag660Collector>();
  }

  Itin*
  createRoundTripItin(const Loc* origin, const Loc* destination, CarrierCode carrierCode)
  {
    ItinBuilder2 itinBuilder(_memHandle);
    return itinBuilder.addSegment(origin, destination, carrierCode)
                      .addSegment(destination, origin, carrierCode)
                      .setPuType(PricingUnit::Type::ROUNDTRIP)
                      .build();
  }

protected:
  Diag660Collector* _diag;
};

TEST_F(Diag660CollectorTest, testPrint)
{
  _pTrx->itin().push_back(createRoundTripItin(createLoc("MAD"), createLoc("LON"), "LH"));
  _pTrx->itin()[0]->calculationCurrency() = "NUC";

  FareInfo* fareInfo1 = _memHandle.create<FareInfo>();
  FareInfo* fareInfo2 = _memHandle.create<FareInfo>();
  fareInfo1->fareClass() = "Y77";
  fareInfo2->fareClass() = "Y78";
  fareInfo1->directionality() = FROM;
  fareInfo2->directionality() = TO;
  fareInfo1->globalDirection() = GlobalDirection::EH;
  fareInfo2->globalDirection() = GlobalDirection::EH;

  FarePath* farePath = _pTrx->itin()[0]->farePath()[0];
  PaxTypeFare* ptf1 = farePath->pricingUnit()[0]->fareUsage()[0]->paxTypeFare();
  PaxTypeFare* ptf2 = farePath->pricingUnit()[0]->fareUsage()[1]->paxTypeFare();
  ptf1->fare()->setFareInfo(fareInfo1);
  ptf2->fare()->setFareInfo(fareInfo2);
  ptf1->nucFareAmount() = 80;
  ptf2->nucFareAmount() = 90;

  _diag->printHeader();
  *_diag << *(_pTrx->itin()[0]->farePath()[0]);
  ASSERT_EQ("********************* PRICING UNIT ANALYSIS *******************\n"
            " 1--1:MAD LON  2--2:LON MAD \n"
            " MAD-LON    EH O Y77          NUC   80.00 PU01/NORM/    / /  RT\n"
            " LON-MAD    EH I Y78          NUC   90.00 PU02/NORM/    / /  RT\n",
            _diag->str());
}

} // tse
