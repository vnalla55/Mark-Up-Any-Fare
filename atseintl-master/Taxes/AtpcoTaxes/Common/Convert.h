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
#include <limits>
#include <stdexcept>
#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>

#include "Util/Utility.h"

namespace tax {

struct Convert
{
  static char intToCharAsText(int i)
  {
    if(LIKELY(i >= 0 && i < 10))
      return static_cast<char>(i + '0');
    else
      throw std::logic_error("conversion error in intToCharAsText");
  }

  static short longToShort(long i)
  {
    if(LIKELY(inRange<short>(i)))
      return static_cast<short>(i);
    else
      throw std::logic_error("conversion error in longToShort");
  }

  static short intToShort(int i)
  {
    if(LIKELY(inRange<short>(i)))
      return static_cast<short>(i);
    else
       throw std::logic_error("conversion error in intToShort");
  }

  static unsigned short intToUshort(int i)
  {
    if (LIKELY(isSignedInUnsignedRange<unsigned short>(i)))
      return static_cast<unsigned short>(i);
    else
      throw std::logic_error("conversion error in intToUshort");
  }

  static unsigned short ulongToUshort(unsigned long i)
  {
    if (LIKELY(inUnsignedRange<unsigned short>(i)))
      return static_cast<unsigned short>(i);
    else
      throw std::logic_error("conversion error in ulongToUshort");
  }

private:
  template<typename Target, typename Source>
  static bool inRange(Source i)
  {
    BOOST_STATIC_ASSERT(sizeof(Target) <= sizeof(Source));
    static const Source lo = std::numeric_limits<Target>::min();
    static const Source hi = std::numeric_limits<Target>::max();
    return lo <= i && i <= hi;
  }

  template<typename Target, typename Source>
  static bool inUnsignedRange(Source i)
  {
    BOOST_STATIC_ASSERT(sizeof(Target) <= sizeof(Source));
    static const Source hi = std::numeric_limits<Target>::max();
    return i <= hi;
  }

  template<typename Target, typename Source>
  static
  typename boost::enable_if_c<(sizeof(Target) >= sizeof(Source)), bool>::type
  isSignedInUnsignedRange(Source i)
  {
    return i >= Source(0);
  }

  template<typename Target, typename Source>
  static
  typename boost::enable_if_c<(sizeof(Target) < sizeof(Source)), bool>::type
  isSignedInUnsignedRange(Source i)
  {
    return i >= Source(0) &&
           i <= static_cast<Source>(std::numeric_limits<Target>::max());
  }
};

} // namespace tax

