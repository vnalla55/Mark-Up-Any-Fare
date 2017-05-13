// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/Diversity/DiversityModelFactory.h"

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag941Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/Diversity/DiversityModelAltDates.h"
#include "Pricing/Shopping/Diversity/DiversityModelBasic.h"
#include "Pricing/Shopping/Diversity/DiversityModelPriceOnly.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"

namespace tse
{

DiversityModelFactory::DiversityModelFactory(ShoppingTrx& trx,
                                             ItinStatistic& stats,
                                             DiagCollector* dc)
  : _trx(trx), _stats(stats), _dc(dc)
{
}

DiversityModel*
DiversityModelFactory::createModel(shpq::SoloTrxData& soloTrxData)
{
  DataHandle& dh(_trx.dataHandle());

  DiversityModel* model;
  if (_trx.isAltDates())
  {
    if (_dc)
      *_dc << "Using alt. dates diversity model\n";
    _trx.diversity().setModel(DiversityModelType::ALTDATES);
    model = &dh.safe_create<DiversityModelAltDates>(_trx, _stats, _dc);
  }
  else if (_trx.diversity().getModel() == DiversityModelType::BASIC)
  {
    if (_dc)
      *_dc << "Using basic diversity model\n";
    model = createBasicModel(soloTrxData);
  }
  else if (_trx.diversity().getModel() == DiversityModelType::PRICE)
  {
    if (_dc)
      *_dc << "Using price-only diversity model\n";
    model = &dh.safe_create<DiversityModelPriceOnly>(_trx, _stats, _dc);
  }
  else
  {
    // TODO: Change it to Basic
    if (_dc)
      *_dc << "Using default (price-only) diversity model\n";
    model = &dh.safe_create<DiversityModelPriceOnly>(_trx, _stats, _dc);
  }

  return model;
}

bool
DiversityModelFactory::checkModelType(const ShoppingTrx& trx, DiversityModelType::Enum dmt)
{
  if (dmt != DiversityModelType::PRICE && dmt != DiversityModelType::BASIC &&
      dmt != DiversityModelType::DEFAULT)
  {
    return false;
  }

  if (trx.isAltDates() && dmt != DiversityModelType::PRICE && dmt != DiversityModelType::DEFAULT)
  {
    return false;
  }

  return true;
}

DiversityModel*
DiversityModelFactory::createBasicModel(shpq::SoloTrxData& soloTrxData)
{
  DiversityModel* model;
  DataHandle& dh(_trx.dataHandle());

  DiagCollector* shoppingDC = &(soloTrxData.getShoppingDC());
  model = &dh.safe_create<DiversityModelBasic>(_trx, _stats, shoppingDC);

  return model;
}

} // ns tse
