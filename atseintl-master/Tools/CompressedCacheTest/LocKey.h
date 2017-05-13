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

#include "Flattenizable.h"

namespace tse {

class LocKey
{
private:

	LocCode	_loc;
	LocTypeCode _locType;

public:

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _loc      ) ;
      FLATTENIZE( archive, _locType  ) ;
    }

    LocKey() : _loc(""), _locType(' ') {}

    LocCode &loc() { return _loc;}
    const LocCode &loc() const { return _loc; }

    LocTypeCode &locType() { return _locType;}
    LocTypeCode locType() const { return _locType; }

    bool isNull() const {return _locType == ' ' && _loc == "";};

    bool operator<( const LocKey & rhs ) const
    {
      if( _loc     != rhs._loc     ) return ( _loc     < rhs._loc     ) ;
      if( _locType != rhs._locType ) return ( _locType < rhs._locType ) ;

      return false ;
    }

    bool operator==( const LocKey & rhs ) const
    {
      return ( ( _loc == rhs._loc ) && ( _locType == rhs._locType ) ) ;
    }

    bool operator!=( const LocKey & rhs ) const
    {
      return ( ! ( *this == rhs ) ) ;
    }

    void dummyData()
    {
      _loc     = "ABCDEFGH" ;
      _locType = 'I'        ;
    }

    friend inline std::ostream & operator<<( std::ostream & os, const LocKey & obj )
    {
      return os << "[" << obj._loc
                << "|" << obj._locType
                << "]"
                ;
    }


};

}

