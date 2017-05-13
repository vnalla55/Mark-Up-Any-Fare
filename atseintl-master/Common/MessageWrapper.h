//
// Copyright Sabre 2015
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace tse
{
namespace MessageWrapper
{
using const_iterator = std::string::const_iterator;

template <typename Predicate>
const_iterator
getTokenEnd(const_iterator begin, const_iterator end, Predicate predicate)
{
  if (begin == end)
    return end;
  auto it = std::find_if(begin, end, predicate);
  return (it == begin) ? ++it : it;
}

template <typename Predicate>
std::vector<std::string>
wrap(const std::string& str, Predicate predicate, std::string::size_type width = 63)
{
  std::vector<std::string> result;
  std::string line;
  line.reserve(width);
  auto left = str.cbegin();

  while (left != str.cend())
  {
    std::string::size_type remaining = width - line.size();
    auto right = getTokenEnd(left, str.cend(), predicate);
    std::string::size_type distance = std::distance(left, right);

    if (distance > remaining)
    {
      if (distance > width)
      {
        right = left + remaining;
        line.append(left, right);
        left = right;
      }

      result.push_back(line);
      line.clear();
    }

    if (right > left)
    {
      line.append(left, right);
      left = right;
    }
  }
  if (line.size() > 0)
  {
    result.push_back(line);
  }
  return result;
}
} // namespace MessageWrapper
} // namespace tse
