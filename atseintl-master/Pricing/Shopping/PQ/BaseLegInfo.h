// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         03-10-2011
//! \file         BaseLegInfo.h
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

#pragma once

#include "Common/Assert.h"

#include <string>

namespace tse
{
namespace shpq
{
namespace details
{

template <typename T, typename Iterator>
class BaseLegInfo
{
public:
  BaseLegInfo(const T& dFm, const Iterator& dFmIt) : _dFm(dFm), _dFmIt(dFmIt) {}

  BaseLegInfo(const BaseLegInfo& other, const bool getNext)
    : _dFm(other._dFm), _dFmIt(getNext ? other._dFmIt + 1 : other._dFmIt)
  {
  }

  ~BaseLegInfo() {}

  bool isValid() const { return _dFm.get(); }
  bool hasCurrent() const { return isValid() && _dFmIt != _dFm->end(); }
  bool hasNext() const { return hasCurrent() && _dFmIt + 1 != _dFm->end(); }

  const typename Iterator::value_type operator->() const
  {
    TSE_ASSERT(hasCurrent());
    return *_dFmIt;
  }

  const typename Iterator::value_type& operator*() const
  {
    TSE_ASSERT(hasCurrent());
    return *_dFmIt;
  }

  const T getFM() const { return _dFm; }

private:
  const T _dFm;
  const Iterator _dFmIt;
};
} /* namespace details */
} /* namespace shpq */
} /* namespace tse */
