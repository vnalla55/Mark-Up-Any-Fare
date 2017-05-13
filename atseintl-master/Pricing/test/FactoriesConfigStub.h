// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         18-11-2011
//! \file         FactoriesConfigStub.h
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

#ifndef FACTORIESCONFIGSTUB_H_
#define FACTORIESCONFIGSTUB_H_

#include "Pricing/FactoriesConfig.h"

namespace tse
{
namespace test
{

class FactoriesConfigStub : public FactoriesConfig
{
public:
  void setSearchAlwaysLowToHigh(const bool value)
  {
    FactoriesConfig::setSearchAlwaysLowToHighForUnitTestsOnly(value);
  }
};
}
}

#endif /* FACTORIESCONFIGSTUB_H_ */
