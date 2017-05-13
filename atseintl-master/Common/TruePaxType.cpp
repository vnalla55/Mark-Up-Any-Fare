#include "Common/TruePaxType.h"

#include "Common/PaxTypeUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "FareCalc/FcUtil.h"

namespace tse
{
void
TruePaxType::init()
{
  std::vector<const FareUsage*> fus;
  FareCalc::forEachFareUsage(_fp, FareCalc::collect(fus));

  const PaxType* paxType = _fp.paxType();
  PaxTypeCode farePaxType("");
  unsigned privateFareCount = 0;

  for (const auto fareUsage : fus)
  {
    farePaxType = fareUsage->paxTypeFare()->fcasPaxType();

    if (farePaxType.empty())
      farePaxType = ADULT;

    if (_paxType.empty())
    {
      _paxType = farePaxType;
    }
    else
    {
      if (farePaxType != _paxType)
        _allFareSamePaxType = false;
    }

    if (farePaxType == paxType->paxType())
      _atLeastOneMatch = true;

    if (paxType->vendorCode() == Vendor::SABRE)
    {
      if (PaxTypeUtil::sabreVendorPaxType(_trx, *paxType, *fareUsage->paxTypeFare()))
      {
        if (_oneInGroup.empty())
          _oneInGroup = farePaxType;
      }
      else
      {
        _allInGroup = false;
      }
    }

    if (fareUsage->paxTypeFare()->isNegotiated())
      _negFareUsed = true;

    if (fareUsage->paxTypeFare()->tcrTariffCat() == 1 /* RuleConst::PRIVATE_TARIFF */)
    {
      _privateFareUsed = true;
      privateFareCount++;
    }
  }

  if (paxType->vendorCode() == Vendor::SABRE)
  {
    if (!_allFareSamePaxType)
    {
      if (_allInGroup)
      {
        _paxType = paxType->paxType();
      }
      else
      {
        if (!_oneInGroup.empty())
          _paxType = _oneInGroup;
        else
          _paxType = paxType->paxType();

        _mixedPaxType = true;
      }
    }
  }
  else
  {
    if (!_allFareSamePaxType)
    {
      _paxType = paxType->paxType();
      _mixedPaxType = true;
    }
  }

  if ((_paxType[1] == _paxType[2] && _paxType[1] == 'N')) // pax type ended with NN
  {
    if (paxType->age() > 0 && paxType->age() < 100)
    {
      _paxType[1] = char('0' + paxType->age() / 10);
      _paxType[2] = char('0' + paxType->age() % 10);
    }
  }
}
}
