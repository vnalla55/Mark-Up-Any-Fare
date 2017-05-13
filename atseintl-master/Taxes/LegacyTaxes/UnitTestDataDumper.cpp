#include "Taxes/LegacyTaxes/UnitTestDataDumper.h"
#include "test/testdata/TestTravelSegFactory.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"

namespace tse
{

unsigned int UnitTestDataDumper::dumpCount = 0;

void
UnitTestDataDumper::dump(PricingTrx& trx, TaxResponse& taxResponse)
{
  Itin& itin = *(taxResponse.farePath()->itin());
  std::vector<TravelSeg*>::iterator travelSegI;
  std::stringstream filename;
  travelSegI = itin.travelSeg().begin();
  filename << UT_DIR << dumpCount << "_" << (*travelSegI)->origin()->loc() << "_";
  for (; travelSegI != itin.travelSeg().end(); ++travelSegI)
  {
    filename << (*travelSegI)->destination()->loc() << "_";
  }
  char segIndex = '0';
  for (travelSegI = itin.travelSeg().begin(); travelSegI != itin.travelSeg().end();
       ++travelSegI, ++segIndex)
  {
    TestTravelSegFactory::write(filename.str() + segIndex, **travelSegI);
  }
  ++dumpCount;
}
}
