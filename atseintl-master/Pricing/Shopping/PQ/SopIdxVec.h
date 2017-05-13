// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "Common/Assert.h"

#include <algorithm>
#include <vector>

#include <tr1/array>

namespace tse
{
namespace shpq
{

const uint8_t MAX_NUMBER_LEGS = 2;

class SopIdxVec : private std::tr1::array<int, MAX_NUMBER_LEGS>
{
  typedef std::vector<int> PricingSopIdxVec;
  typedef std::tr1::array<int, MAX_NUMBER_LEGS> raw;

public:
  typedef SopIdxVec PassArgType; // enforce pass-by-value semantic

  typedef raw::iterator iterator;
  typedef raw::const_iterator const_iterator;

  SopIdxVec() { resize(0); }
  SopIdxVec(std::size_t sz) { resize(sz); }
  SopIdxVec(const PricingSopIdxVec& prSopVec)
  {
    resize(prSopVec.size());
    std::copy(prSopVec.begin(), prSopVec.end(), raw::begin());
  }

  reference operator[](std::size_t off) { return raw::operator[](off); }
  value_type operator[](std::size_t off) const { return raw::operator[](off); }

  bool operator==(PassArgType rhs) const
  {
    const raw& raw_lhs = *this;
    const raw& raw_rhs = rhs;
    return raw_lhs == raw_rhs;
  }

  /**
   * To avoid SopIdxVec to PricingSopIdxVec casting for the comparison
   */
  friend bool operator==(const PricingSopIdxVec& sopIdxVec, PassArgType rhs)
  {
    SopIdxVec lhs(sopIdxVec);
    return (lhs == rhs);
  }

  /**
   * A lot of time is spent in this call while looking up for existing combination
   */
  bool operator<(PassArgType rhs) const
  {
    const raw& raw_lhs = *this;
    const raw& raw_rhs = rhs;
    return raw_lhs < raw_rhs;
  }
  /**
   * To avoid SopIdxVec to PricingSopIdxVec casting for the comparison
   *
   * A lot of time is spent in this call while looking up for existing combination
   */
  friend bool operator<(const PricingSopIdxVec& sopIdxVec, PassArgType rhs)
  {
    SopIdxVec lhs(sopIdxVec);
    return (lhs < rhs);
  }

  iterator begin() { return raw::begin(); }
  const_iterator begin() const { return raw::begin(); }

  iterator end() { return std::find(raw::begin(), raw::end(), grayPlaceholder()); }
  const_iterator end() const { return std::find(raw::begin(), raw::end(), grayPlaceholder()); }

  bool empty() const { return ((*this)[0] == grayPlaceholder()); }

  uint32_t size() const
  {
    return static_cast<uint32_t>(end() - begin());
  }

  /**
   * The sops initialization _must_ follow resize(),
   * otherwise subsequent size() query result can be invalid
   */
  void resize(std::size_t sz)
  {
    TSE_ASSERT(raw::begin() + sz <= raw::end() || !"Buffer overrun prevention");

    //    std::fill(raw::begin(),
    //              raw::begin() + sz,
    //              NOT_INITIALIZED_PLACEHOLDER);

    std::fill(raw::begin() + sz, raw::end(), grayPlaceholder());
  }

  void clear() { resize(0); }

  /**
   * It is important while profiling to find out, how often this operator is called.
   *
   * Normally, it has to be called only for diagnostic and for adding sop vec to
   * shoppingTrx flight matrix.
   */
  operator PricingSopIdxVec() const { return PricingSopIdxVec(begin(), end()); }

private:
  // Array element, that "virtually" does not exist
  static value_type grayPlaceholder() { return static_cast<int32_t>(0xdeadbeef); }

  // Just for a guarantee, that size() is reported properly,
  // even if the sops are not initialized
  //  static const value_type NOT_INITIALIZED_PLACEHOLDER = 0;
};

typedef SopIdxVec::PassArgType SopIdxVecArg; // enforce pass-by-value semantic
}
} // ns tse::shpq

