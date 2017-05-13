//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace tse
{

namespace tools
{


template <class T>
struct PprintAsClass
{
  void operator()(std::ostream& out, const T& t) const
  {
    pprint_impl(out, t);
  }
};

template <class T>
struct PprintAsArithmetic
{
  void operator()(std::ostream& out, const T& a) const
  {
    out << a;
  }
};

template <class T>
struct Pprint : std::conditional<std::is_arithmetic<T>::value,
                                PprintAsArithmetic<T>,
                                PprintAsClass<T>>::type {};



template <class T>
void pprint(std::ostream& out, const T& v)
{
  Pprint<T>{}(out, v);
}



template <class Collection>
void ojoin(std::ostream& out, const Collection& c, const std::string& sep = ", ")
{
  size_t pos = 0;
  const size_t end = c.size() - 1;
  for (const auto& e: c)
  {
    pprint<typename Collection::value_type>(out, e);
    if (pos != end)
    {
      out << sep;
    }
    ++pos;
  }
}

template <class Collection>
void pprint_collection(std::ostream& out, const Collection& c,
                      const std::string& otag, const std::string& ctag)
{
  out << otag;
  ojoin(out, c);
  out << ctag;
}

template <class T>
struct Pprint<T*>
{
  void operator()(std::ostream& out, T* p) const
  {
    if (p == nullptr)
    {
      out << "None";
    }
    else
    {
      out << p << " -> ";
      pprint(out, *p);
    }
  }
};

template <class T>
struct Pprint<std::vector<T>>
{
  void operator()(std::ostream& out, const std::vector<T>& v)
  {
    tse::tools::pprint_collection(out, v, "[", "]");
  }
};


// Get a string immedately

template <class T>
std::string pformat(const T& t)
{
  std::ostringstream out;
  pprint(out, t);
  return out.str();
}


} // namespace tools

} // namespace tse

