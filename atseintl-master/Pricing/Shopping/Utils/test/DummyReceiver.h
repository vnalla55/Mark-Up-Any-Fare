//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#ifndef DUMMY_RECEIVER_H
#define DUMMY_RECEIVER_H

#include <string>
#include <sstream>
#include <vector>

namespace tse
{

namespace utils
{

class TruePredicate
{
public:
  bool operator()(unsigned int newCount, bool increased) { return true; }
};

class FalsePredicate
{
public:
  bool operator()(unsigned int newCount, bool increased) { return false; }
};

template <typename T>
class DummyReceiver : public ICountingEventReceiver<T>
{
public:
  void eventForValue(const T& e, bool increased) override
  {
    std::ostringstream out;
    out << e << ":" << increased;
    _v.push_back(out.str());
  }
  std::string dumpLog()
  {
    sort(_v.begin(), _v.end());
    std::ostringstream out;
    for (unsigned int i = 0; i < _v.size(); ++i)
    {
      out << _v[i] << "|";
    }
    _v.clear();
    return out.str();
  }

private:
  std::vector<std::string> _v;
};

} // namespace utils

} // namespace tse

#endif // DUMMY_RECEIVER_H
