//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include <boost/random.hpp>
#include "CompressionTestCommon.h"

#include <mutex>

namespace tse
{
std::mutex _mutexRand;

boost::mt19937 _eng(static_cast<unsigned int>(std::time(nullptr)));

boost::uniform_int<int> _unif(0, MAXNUMBERKEYS);

float _mean(MAXNUMBERKEYS / 2);
float _variance(MAXNUMBERKEYS / 32);
boost::normal_distribution<float> _distr(_mean, _variance); 
boost::variate_generator<boost::mt19937, boost::normal_distribution<float>> _gen(_eng, _distr);

Key getRandomKey ()
{
  std::unique_lock<std::mutex> lock(_mutexRand);
  Key key(Key(_unif(_eng)));
  //Key key(static_cast<int>(_gen()));
  if (key < 0)
  {
    key._a = -key._a;
  }
  return key;
}

}// tse
