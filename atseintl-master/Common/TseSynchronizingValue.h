//-------------------------------------------------------------------
//
//  File:        TseSynchronizingValue.h
//  Created:     April 11, 2008
//  Authors:     Mike Lillis
//
//  Description: Class used to synchronize other objects.  This is an opaque
//               class which hides the actual synchronizing value.  The only
//               operations allowed for these objects are:
//                    Construct a default one
//                    Construct one from another or assign one the value of another
//                    Compare two for equality or inequality
//                    Increment one (this was an arbitrary choice of on operator
//                                   to alter the value)
//
//               There is no sense of "earlier" or "later" with this class.  Two
//               values are either equal or not.  It is up to external logic to
//               determine what should be done if two values are not equal.
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include <limits>

namespace tse
{

class TseSynchronizingValue
{
private:
//  Since equality is all that is necessary it would be okay to merely let
//  the values wrap.  But that might look like a bug, so a value was chosen
//  to avoid implicit wrapping.

#define MAX_SEQUENCE_NUMBER (std::numeric_limits<unsigned long int>::max() - 1)

public:
  //  Construction :  default and copy

  TseSynchronizingValue() : _sequenceNumber(0) {};
  TseSynchronizingValue(const TseSynchronizingValue& rAnother)
    : _sequenceNumber(rAnother._sequenceNumber) {};

  // Assignment of a value from another one so as to synchronize

  TseSynchronizingValue& operator=(const TseSynchronizingValue& another)
  {
    _sequenceNumber = another._sequenceNumber;

    return *this;
  };

  // Altering of the value (just pre and post ++ supported)

  TseSynchronizingValue& operator++()
  {
    if (_sequenceNumber == MAX_SEQUENCE_NUMBER)
    {
      _sequenceNumber = 0;
    }
    else
    {
      _sequenceNumber++;
    }
    return *this;
  };

  TseSynchronizingValue& operator++(int)
  {
    if (_sequenceNumber == MAX_SEQUENCE_NUMBER)
    {
      _sequenceNumber = 0;
    }
    else
    {
      _sequenceNumber++;
    }
    return *this;
  };

  // Equality and inequality operators (the only valid comparisons)

  bool operator==(const TseSynchronizingValue& another)
  {
    return _sequenceNumber == another._sequenceNumber;
  }
  bool operator!=(const TseSynchronizingValue& another)
  {
    return _sequenceNumber != another._sequenceNumber;
  }

private:
  unsigned long int _sequenceNumber;

  // No comparisons other than equality and inequality are valid for this object
  bool operator<(const TseSynchronizingValue&);
  bool operator<=(const TseSynchronizingValue&);
  bool operator>(const TseSynchronizingValue&);
  bool operator>=(const TseSynchronizingValue&);
};
}

