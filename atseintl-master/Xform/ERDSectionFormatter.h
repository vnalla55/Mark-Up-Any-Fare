#include "FareCalc/CalcTotals.h"

class XMLConstruct;

namespace tse
{

class PricingTrx;
class PaxTypeFare;

class ERDSectionFormatter
{
  friend class ERDSectionFormatterTest;

public:
  ERDSectionFormatter(PricingTrx& pricingTrx,
                      CalcTotals& calcTotals,
                      const FareUsage& fareUsage,
                      const CalcTotals::FareCompInfo& fareCompInfo,
                      XMLConstruct& construct);

  void prepareERD();
  void prepareERDCat25();
  void prepareERDCat35(const PaxTypeFare* netFare);
  void prepareERDDiscount();
  void prepareERDConstructedFareInfo();
  void prepareERDOriginAddOn();
  void prepareERDDestAddOn();

private:
  PricingTrx& _pricingTrx;
  CalcTotals& _calcTotals;
  const FareUsage& _fareUsage;
  const CalcTotals::FareCompInfo& _fareCompInfo;
  XMLConstruct& _construct;
};
}
