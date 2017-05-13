// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/FarePath.h"

namespace tse
{
class Itin;

class AbstractTaxSplitter
{
public:
  virtual void
  clearTaxMaps() = 0;

  virtual Itin*
  getSubItin(Itin* const primaryItin, const int legId) const = 0;

  virtual void
  setupTaxes(Itin* const primaryItin, Itin* subItin, const int legId) = 0;

  virtual void
  setupTaxesForFarePath(Itin* const primaryItin,
                        Itin* subItin,
                        const int legId,
                        const FarePath* farePath,
                        FarePath::FarePathKey& farePathKey) = 0;

  virtual ~AbstractTaxSplitter() = default;

protected:
  AbstractTaxSplitter() {}
};
} // end of tse namespace
