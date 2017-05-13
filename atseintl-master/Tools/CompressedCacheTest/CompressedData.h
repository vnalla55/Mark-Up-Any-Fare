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

#pragma once

#include <boost/noncopyable.hpp>

#include <memory>
#include <vector>

namespace sfc
{

class CompressedData;
typedef std::shared_ptr<CompressedData> CompressedDataPtr;

class CompressedData : boost::noncopyable
{
public:
  CompressedData()
  {
  }

  CompressedData(std::vector<char>& deflated)
  {
    _deflated.swap(deflated);
  }

  CompressedData(char* deflated,
                 size_t deflatedSz)
    : _deflated(deflated, deflated + deflatedSz)
  {
  }

  void swap(CompressedData& other)
  {
    _deflated.swap(other._deflated);
  }

  bool empty() const
  {
    return _deflated.empty();
  }

  std::vector<char> _deflated;
};

}// sfc
