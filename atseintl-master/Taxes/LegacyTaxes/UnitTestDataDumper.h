#include "DataModel/PricingRequest.h"
#include "DBAccess/TaxCodeReg.h"

#define UT_DIR "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/"

namespace tse
{
class PricingTrx;

class UnitTestDataDumper
{
  static unsigned int dumpCount;

public:
  static void dump(PricingTrx& trx, TaxResponse& taxResponse);
};
}
