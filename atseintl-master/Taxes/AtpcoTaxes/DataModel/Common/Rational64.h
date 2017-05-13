// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <boost/rational.hpp>

namespace tax
{

class Rational64 : public boost::rational<int64_t>
{
public:
  Rational64(int64_t nom = 0, int64_t den = 1) : boost::rational<int64_t>(nom, den) {}
  Rational64(int nom) : boost::rational<int64_t>(nom) {}
  Rational64(uint32_t nom) : boost::rational<int64_t>(static_cast<int64_t>(nom)) {}
  Rational64(boost::rational<int64_t> r) : boost::rational<int64_t>(r) {}

private:
  Rational64(double d); // = delete
};

}

