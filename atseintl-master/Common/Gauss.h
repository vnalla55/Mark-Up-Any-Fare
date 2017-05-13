#pragma once

#include <cmath>
#include "Util/BranchPrediction.h"

namespace tse
{

template <class Type>
class Gauss
{
public:
  Gauss();
  Gauss(const Gauss<Type>& gauss);

  // Incremental analysis
  //
  void include(Type value);
  void exclude(Type value);
  void clear();

  // Regression coefficients
  //
  long num() const { return long(_num); }
  Type min() const { return _min; }
  Type max() const { return _max; }

  Type mean() const { return _tv / _num; }
  Type stddev() const { return std::sqrt(variance()); }
  Type variance() const { return (_num * _tvv - (_tv * _tv)) / (_num * (_num - 1)); }

  const Gauss<Type>& operator=(const Gauss<Type>& gauss);

private:
  // Distribution
  //
  Type _num;
  Type _min;
  Type _max;

  // Running totals
  //
  Type _tv;
  Type _tvv;
};

template <class Type>
Gauss<Type>::Gauss()
  : _num(0), _min(0), _max(0), _tv(0), _tvv(0)
{
}

template <class Type>
Gauss<Type>::Gauss(const Gauss<Type>& gauss)
  : _num(gauss._num), _min(gauss._min), _max(gauss._max), _tv(gauss._tv), _tvv(gauss._tvv)
{
}

template <class Type>
void
Gauss<Type>::include(Type value)
{
  _num += 1;
  _tv += value;
  _tvv += value * value;

  if (_num == 1)
  {
    _min = value;
    _max = value;
  }
  else
  {
    if (UNLIKELY(value < _min))
      _min = value;
    if (UNLIKELY(value > _max))
      _max = value;
  }
}

template <class Type>
void
Gauss<Type>::exclude(Type value)
{
  if (!_num)
    return;

  _num -= 1;

  if (_num == 0)
  {
    clear(); // re-establish default state
  }
  else
  {
    _tv -= value;
    _tvv -= value * value;
  }
}

template <class Type>
void
Gauss<Type>::clear()
{
  _num = 0;
  _min = 0;
  _max = 0;
  _tv = 0;
  _tvv = 0;
}

template <class Type>
const Gauss<Type>&
Gauss<Type>::
operator=(const Gauss<Type>& gauss)
{
  _num = gauss._num;
  _min = gauss._min;
  _max = gauss._max;
  _tv = gauss._tv;
  _tvv = gauss._tvv;

  return *this;
}

} // namespace tse

