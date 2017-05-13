#ifndef PRINT_COLLECTION_H
#define PRINT_COLLECTION_H

#include <iosfwd>
#include <algorithm>
#include <iterator>

namespace print
{

template <typename T>
inline
std::ostream&
toStream(const T& cont, std::ostream& os)
{
  std::copy(cont.begin(), cont.end(), std::ostream_iterator<typename T::value_type>(os, " "));
  return os;
}

} // print

template <typename T, typename U>
inline
std::ostream&
operator<<(std::ostream& os, const std::vector<T, U>& cont)
{
  return print::toStream(cont, os);
}

template <typename T, typename U>
inline
std::ostream&
operator<<(std::ostream& os, const std::set<T, U>& cont)
{
  return print::toStream(cont, os);
}

template <typename T, typename U, typename W>
inline
std::ostream&
operator<<(std::ostream& os, const std::map<T, U, W>& cont)
{
  return print::toStream(cont, os);
}

template <typename T, typename U>
inline
std::ostream&
operator<<(std::ostream& os, const std::list<T, U>& cont)
{
  return print::toStream(cont, os);
}

template <typename T, typename U>
inline
std::ostream&
operator<<(std::ostream& os, const std::multiset<T, U>& cont)
{
  return print::toStream(cont, os);
}

#endif
