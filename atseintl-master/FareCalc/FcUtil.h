//----------------------------------------------------------------------------
//  File:        FcUtil - FareCalc Template Util
//  Authors:     Quan Ta
//  Created:
//
//  Description: FareCalc template util
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2006
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

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"

namespace tse
{
namespace FareCalc
{

//////////////////////////////////////////////////////////////////////////////
// Note: g++ 3.2.3 can't handle this code yet, but g++ 3.4 can.
//////////////////////////////////////////////////////////////////////////////

enum Order
{
  EQ,
  GE,
  GT,
  LE,
  LT
};

template <class F, Order o>
class Assert
{
public:
  Assert(const F& f, const typename F::result_type& e) : _f(f), _e(e) {}
  bool operator()(const typename F::argument_type& arg) const
  {
    switch (o)
    {
    case GE:
      return _f(arg) >= _e;
    case GT:
      return _f(arg) > _e;
    case LE:
      return _f(arg) <= _e;
    case LT:
      return _f(arg) < _e;
    case EQ:
    default:
      return _f(arg) == _e;
    }
  }

private:
  const F& _f;
  const typename F::result_type& _e;
};

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

template <class T>
class PassAll
{
public:
  bool operator()(T& t) const { return true; }
};

template <class C, class P = PassAll<typename C::value_type> >
class Collect
{
public:
  Collect(C& c, const P* p = 0) : _c(c), _p(p) {}
  void operator()(typename C::value_type e)
  {
    if (!_p || (*_p)(e))
      _c.insert(_c.end(), e);
  }

private:
  C& _c;
  const P* _p;
};

template <class C, class P>
Collect<C, P>
collect(C& c, const P& p)
{
  return Collect<C, P>(c, &p);
}

template <class C>
Collect<C>
collect(C& c)
{
  return Collect<C>(c, nullptr);
}

//////////////////////////////////////////////////////////////////////////////

template <class T, class F>
void
forEachFareUsage(T& t, const F& f);

template <class F>
void
forEachFareUsage(PricingUnit& pu, const F& f)
{
  std::for_each(pu.fareUsage().begin(), pu.fareUsage().end(), f);
}

template <class F>
void
forEachFareUsage(const PricingUnit& pu, const F& f)
{
  std::for_each(pu.fareUsage().begin(), pu.fareUsage().end(), f);
}

template <class T, class F>
class Iter
{
public:
  Iter(const F& f) : _f(f) {}
  void operator()(T* t) { forEachFareUsage(*t, _f); }
  void operator()(const T* t) { forEachFareUsage(*t, _f); }

private:
  const F& _f;
};

template <class F>
void
forEachFareUsage(FarePath& fp, const F& f)
{
  Iter<PricingUnit, F> iter(f);
  std::for_each(fp.pricingUnit().begin(), fp.pricingUnit().end(), iter);
}

template <class F>
void
forEachFareUsage(const FarePath& fp, const F& f)
{
  Iter<PricingUnit, F> iter(f);
  std::for_each(fp.pricingUnit().begin(), fp.pricingUnit().end(), iter);
}

template <class F>
void
forEachFareUsage(const std::vector<PricingUnit*>& sideTrip, const F& f)
{
  Iter<PricingUnit, F> iter(f);
  std::for_each(sideTrip.begin(), sideTrip.end(), iter);
}

template <class F>
void
forEachFareUsage(Itin& i, const F& f)
{
  Iter<FarePath, F> iter(f);
  std::for_each(i.farePath().begin(), i.farePath().end(), iter);
}

template <class F>
void
forEachFareUsage(const Itin& i, const F& f)
{
  Iter<FarePath, F> iter(f);
  std::for_each(i.farePath().begin(), i.farePath().end(), iter);
}

//////////////////////////////////////////////////////////////////////////////

template <class F>
void
forEachTravelSeg(const FareUsage& fu, const F& f)
{
  std::for_each(fu.travelSeg().begin(), fu.travelSeg().end(), f);
}
template <class F>
void
forEachTravelSeg(FareUsage& fu, const F& f)
{
  std::for_each(fu.travelSeg().begin(), fu.travelSeg().end(), f);
}

template <class T, class F>
class TsIter
{
public:
  TsIter(const F& f) : _f(f) {}
  void operator()(T* t) { forEachTravelSeg(*t, _f); }
  void operator()(const T* t) { forEachTravelSeg(*t, _f); }

private:
  const F& _f;
};

template <class T, class F>
void
forEachTravelSeg(const T& t, const F& f)
{
  TsIter<FareUsage, F> iter(f);
  forEachFareUsage(t, iter);
}
template <class T, class F>
void
forEachTravelSeg(T& t, const F& f)
{
  TsIter<FareUsage, F> iter(f);
  forEachFareUsage(t, iter);
}

//////////////////////////////////////////////////////////////////////////////

template <class T, class Op, class R>
struct EqualOp
{
  EqualOp(const Op& o, const R& e) : _o(o), _e(e) {}
  bool operator()(const T* t) const { return _o(t) == _e; }
  bool operator()(const T& t) const { return _o(t) == _e; }
  const Op& _o;
  const R& _e;
};

template <class T, class Op, class R>
EqualOp<T, Op, R>
Equal(const Op& o, const R& e)
{
  return EqualOp<T, Op, R>(o, e);
}

template <class T, class Op, class R>
struct AccumulateOp
{
  AccumulateOp(const Op& o, R& r) : _o(o), _r(r) {}
  void operator()(const T* t) { _r += _o(t); }
  void operator()(const T& t) { _r += _o(t); }
  const Op& _o;
  R& _r;
};

template <class T, class Op, class R>
AccumulateOp<T, Op, R>
Accumulate(const Op& o, R& r)
{
  return AccumulateOp<T, Op, R>(o, r);
}

} // namespace FareCalc
} // namespace tse

