// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include <tuple>
#include <type_traits>

namespace tse
{
namespace helper
{
// ----------------------------------------------------------------
// IndexOf<SearchedType, TypeList>::value
//   An index of SearchedType in a TypeList (parameter pack)
// ----------------------------------------------------------------
template <class...>
struct IndexOf;

template <class SearchedType, class... Tail>
struct IndexOf<SearchedType, SearchedType, Tail...> : std::integral_constant<size_t, 0u>
{};

template <class SearchedType, class Head, class... Tail>
struct IndexOf<SearchedType, Head, Tail...>
    : std::integral_constant<size_t, 1u + IndexOf<SearchedType, Tail...>::value>
{};


} // ns helper

// ----------------------------------------------------------------
// Tag<UnderlyingType>
//   A base class for all tags used in TaggedTuple.
//   Tags act as both tuple's elements' identifers and descriptors.
//   E.g. we can define a tag "Distance" of type "size_t" using the following construct:
//     struct Distance : public Tag<size_t> {};
// ----------------------------------------------------------------
template <class UnderlyingType>
struct Tag
{
  using underlying_type = UnderlyingType;
};

// ----------------------------------------------------------------
// TaggedTuple<Tags...>
//   TaggedTuple is meant to mimic std::tuple in many respects but it uses tags
//   described above instead of plain types which makes it slightly more powerful.
//   Note that all tags within a single tuple should be unique to take advantage
//   of some of its features (e.g. get<Tag>()).
//
// Example:
//   struct Name : Tag<std::string> {};
//   struct Age : Tag<uint16_t> {};
//   TaggedTuple<Name, Age> t;
//   std::cout << "Name: " << t.get<Name>() << " Age: " << t.get<Age>();
// ----------------------------------------------------------------
template <class... Tags>
class TaggedTuple : public std::tuple<typename Tags::underlying_type...>
{
  using RawTuple = std::tuple<typename Tags::underlying_type...>;

public:
  using RawTuple::RawTuple;

  template <class SearchedTag>
  constexpr decltype(auto) get()
  {
    return std::get<helper::IndexOf<SearchedTag, Tags...>::value>(*this);
  }

  template <class SearchedTag>
  constexpr decltype(auto) get() const
  {
    return std::get<helper::IndexOf<SearchedTag, Tags...>::value>(*this);
  }
};

} // ns tse
