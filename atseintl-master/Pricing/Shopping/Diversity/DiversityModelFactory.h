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
#pragma once

#include "DataModel/DiversityModelType.h"

#include <string>

namespace tse
{

class ShoppingTrx;
class ItinStatistic;
class DiversityModel;
class DiagCollector;

namespace shpq
{
class SoloTrxData;
}

class DiversityModelFactory
{
public:
  DiversityModelFactory(ShoppingTrx& trx, ItinStatistic& stats, DiagCollector* dc);

  DiversityModel* createModel(shpq::SoloTrxData& soloTrxData);

  static bool checkModelType(const ShoppingTrx& trx, DiversityModelType::Enum dmt);

private:
  DiversityModel* createBasicModel(shpq::SoloTrxData& soloTrxData);

  ShoppingTrx& _trx;
  ItinStatistic& _stats;
  DiagCollector* _dc;
};
}

