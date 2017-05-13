// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         27-10-2011
//! \file         CommonSoloPQItem.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#include "Pricing/Shopping/PQ/CommonSoloPQItem.h"

#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include <iostream>

namespace tse
{
namespace shpq
{

void
CommonSoloPQItem::printBasicStr(std::ostream& stream, const char* const level) const
{
  stream.setf(std::ios::fixed, std::ios::floatfield);
  stream << level << " ";
  stream << _solutionPattern.getSPIdStr() << " " << std::setw(7) << std::setprecision(2)
         << getScore();
}

void
CommonSoloPQItem::printPlaceHolderStr(std::ostream& stream,
                                      const LegPosition legPosition,
                                      const bool hasNext) const
{
  if (isPlaceholderLeg(legPosition))
  {
    stream << (hasNext ? " _" : " *");
  }
}
}
}
