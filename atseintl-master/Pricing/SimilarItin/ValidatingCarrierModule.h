/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/TseCodeTypes.h"

#include <vector>

namespace tse
{
class FPPQItem;
class FarePath;
class Itin;
class Diag990Collector;

namespace similaritin
{
class Context;

class ValidatingCarrierModule
{
public:
  explicit ValidatingCarrierModule(Context& context, Diag990Collector* diag990)
    : _context(context), _diag990(diag990)
  {
  }
  bool buildCarrierLists(Itin& estItin, FarePath* cloned);
  bool processValidatingCarriers(const Itin& motherItin,
                                 FarePath& farePath,
                                 const Itin& childItin,
                                 bool& revalidationRequired);

private:
  void showDiag(const std::vector<CarrierCode>& farePathCxrVec);
  Context& _context;
  Diag990Collector* _diag990;
};
}
}
