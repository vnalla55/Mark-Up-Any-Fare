// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Rules/ApplyGroup.h"
#include "Rules/FillerGroup.h"
#include "Rules/FinalGroup.h"
#include "Rules/GeoPathGroup.h"
#include "Rules/ItinGroup.h"
#include "Rules/LimitGroup.h"
#include "Rules/MiscGroup.h"
#include "Rules/OptionalServiceGroup.h"
#include "Rules/ProcessingOptionGroup.h"
#include "Rules/SaleGroup.h"
#include "Rules/TaxPointBeginGroup.h"
#include "Rules/TaxPointEndGroup.h"
#include "Rules/TicketGroup.h"

#include <utility>

namespace tax
{
struct ValidatorsGroups
{
  ProcessingOptionGroup _processingOptionGroup;
  TicketGroup _ticketGroup;
  SaleGroup _saleGroup;
  FillerGroup _fillerGroup;
  GeoPathGroup _geoPathGroup;
  OptionalServiceGroup _optionalServiceGroup;
  TaxPointBeginGroup _taxPointBeginGroup;
  TaxPointEndGroup _taxPointEndGroup;
  ItinGroup _itinGroup;
  MiscGroup _miscGroup;

  template <template <class> class Functor, class ...Args>
  bool foreach(bool foundItinApplication,
               bool foundYqYrApplication,
               Args&&... args) const
  {
    return _processingOptionGroup.foreach<Functor>(foundItinApplication,
                                                   foundYqYrApplication,
                                                   std::forward<Args>(args)...) &&
           _ticketGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _saleGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _fillerGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _geoPathGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _optionalServiceGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _taxPointBeginGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _taxPointEndGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _itinGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _miscGroup.foreach<Functor>(std::forward<Args>(args)...);
    return true;
  }
};

struct CalculatorsGroups
{
  ApplyGroup _applyGroup;
  FinalGroup _finalGroup;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return _applyGroup.foreach<Functor>(std::forward<Args>(args)...) &&
           _finalGroup.foreach<Functor>(std::forward<Args>(args)...);
  }
};
}
