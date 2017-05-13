//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Loc.h"

namespace tse
{

// Loc-based predicates
struct AreasNotEqual
{
  bool operator()(const Loc* o, const Loc* d) const { return o->area() != d->area(); }
};

struct SubAreasNotEqual
{
  bool operator()(const Loc* o, const Loc* d) const { return o->subarea() != d->subarea(); }
};

struct ATPReservedZonesNotEqual
{
  bool operator()(const Loc* o, const Loc* d) const;
};

struct NationsNotEqual
{
  bool operator()(const Loc* o, const Loc* d) const { return o->nation() != d->nation(); }
};


}

