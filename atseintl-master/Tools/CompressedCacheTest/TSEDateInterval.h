//-------------------------------------------------------------------
// 2005, Sabre Inc.  All rights reserved.
//
// This software/documentation is the confidential and proprietary
// product of Sabre Inc. Any unauthorized use, reproduction, or
// transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system
// or publication, is strictly prohibited
//-------------------------------------------------------------------

#pragma once

#include "Flattenizable.h"

#include "TseTypes.h"

namespace tse {

class TSEDateInterval
{
public:

  // construction / assignment / destruction
  // ============ = ========== = ===========

  TSEDateInterval();

  TSEDateInterval( const TSEDateInterval& rhs );
  const TSEDateInterval& operator = ( const TSEDateInterval& rhs );

  ~TSEDateInterval() {};

  /**
   * This methods populates a given cloneObj to be
   * 'equal' to the current object
   *
   * @param cloneObj - object to populate
   */
  void cloneDateInterval( TSEDateInterval& cloneObj ) const;

  // main interface
  // ==== =========

  bool isEffective( const DateTime tarvelDT ) const;

  bool isEffective( const DateTime ticketingDT,
                    const DateTime tarvelDT    ) const;

  bool isEffective( std::pair< DateTime, DateTime > range ) const;

  bool defineIntersection( const TSEDateInterval& i1,
                           const TSEDateInterval& i2 );

  bool defineIntersectionH( const TSEDateInterval& i1,
                            const TSEDateInterval& i2 );

  bool defineUnion( const TSEDateInterval& i1,
                    const TSEDateInterval& i2 );

  // accessors
  // =========

  DateTime& createDate(){ return _createDate;}
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate(){ return _effDate;}
  const DateTime& effDate() const { return _effDate; }

  DateTime& expireDate(){ return _expireDate;}
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& discDate(){ return _discDate;}
  const DateTime& discDate() const { return _discDate; }

  bool operator<( const TSEDateInterval & rhs ) const
  {
    if ( _createDate != rhs._createDate ) return ( _createDate < rhs._createDate ) ;
    if ( _effDate    != rhs._effDate    ) return ( _effDate    < rhs._effDate    ) ;
    if ( _expireDate != rhs._expireDate ) return ( _expireDate < rhs._expireDate ) ;
    if ( _discDate   != rhs._discDate   ) return ( _discDate   < rhs._discDate   ) ;

    return false ;
  }

  bool operator==( const TSEDateInterval & rhs ) const
  {
    return(    ( _createDate == rhs._createDate )
            && ( _effDate    == rhs._effDate    )
            && ( _expireDate == rhs._expireDate )
            && ( _discDate   == rhs._discDate   )
          ) ;
  }

  bool operator!=( const TSEDateInterval & rhs ) const
  {
    return ( ! ( *this == rhs ) ) ;
  }

  friend inline std::ostream & dumpObject( std::ostream & os, const TSEDateInterval & obj )
  {
    return os << "[" << obj._createDate
              << "|" << obj._effDate
              << "|" << obj._expireDate
              << "|" << obj._discDate
              << "]"
              ;
  }

  static void dummyData( TSEDateInterval & obj )
  {
    static DateTime currentTime(time( NULL ));
    obj._createDate = currentTime;
    obj._effDate    = currentTime;
    obj._expireDate = currentTime;
    obj._discDate   = currentTime;
  }

  void flattenize( Flattenizable::Archive & archive )
  {
    FLATTENIZE( archive, _createDate ) ;
    FLATTENIZE( archive, _effDate    ) ;
    FLATTENIZE( archive, _expireDate ) ;
    FLATTENIZE( archive, _discDate   ) ;
  }

private:

  DateTime  _createDate ;
  DateTime  _effDate    ;
  DateTime  _expireDate ;
  DateTime  _discDate   ;

}; // class TSEDateInterval

} // namespace tse
