//----------------------------------------------------------------------------
//	   Copyright 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include <climits>
#include <map>
#include <sstream>
#include <boost/functional/hash.hpp>
#include "TseTypes.h"
#include "TSEDateInterval.h"

//#include <tr1/unordered_map>

#include "Flattenizable.h"
#include "CompressedDataUtils.h"
//#include "Common/ObjectComparison.h"

#define ADDONCOMBFARECLASSINFO_MEMBERS \

namespace tse
{
  class AddonCombFareClassInfo
  {
  public:

    AddonCombFareClassInfo()
      : _fareTariff     ( 1   )
      , _geoAppl        ( ' ' )
      , _owrt           ( ' ' )
    { }

    ~AddonCombFareClassInfo()
    { }

    TSEDateInterval& effInterval(){ return _effInterval;}
    const TSEDateInterval& effectiveInterval() const { return _effInterval; }

    DateTime& createDate(){ return _effInterval.createDate();}
    const DateTime& createDate() const { return _effInterval.createDate(); }

    DateTime& effDate(){ return _effInterval.effDate();}
    const DateTime& effDate() const { return _effInterval.effDate(); }

    DateTime& expireDate(){ return _effInterval.expireDate(); }
    const DateTime& expireDate() const { return _effInterval.expireDate(); }

    DateTime& discDate(){ return _effInterval.discDate();}
    const DateTime& discDate() const { return _effInterval.discDate(); }

    VendorCode &vendor() { return _vendor;}
    const VendorCode &vendor() const { return _vendor; }

    TariffNumber &fareTariff(){ return _fareTariff;}
    const TariffNumber &fareTariff() const { return _fareTariff; }

    CarrierCode &carrier() { return _carrier;}
    const CarrierCode &carrier() const { return _carrier; }

    FareClassCode &addonFareClass() { return _addonFareClass;}
    const FareClassCode &addonFareClass() const { return _addonFareClass; }

    Indicator &geoAppl() { return _geoAppl;}
    const Indicator &geoAppl() const { return _geoAppl; }

    Indicator &owrt() { return _owrt;}
    const Indicator &owrt() const { return _owrt; }

    FareClassCode &fareClass() { return _fareClass;}
    const FareClassCode &fareClass() const { return _fareClass;}

    bool operator<( const AddonCombFareClassInfo & rhs ) const
    {
      if( _effInterval    != rhs._effInterval    ) return ( _effInterval    < rhs._effInterval    ) ;
      if( _vendor         != rhs._vendor         ) return ( _vendor         < rhs._vendor         ) ;
      if( _fareTariff     != rhs._fareTariff     ) return ( _fareTariff     < rhs._fareTariff     ) ;
      if( _carrier        != rhs._carrier        ) return ( _carrier        < rhs._carrier        ) ;
      if( _addonFareClass != rhs._addonFareClass ) return ( _addonFareClass < rhs._addonFareClass ) ;
      if( _geoAppl        != rhs._geoAppl        ) return ( _geoAppl        < rhs._geoAppl        ) ;
      if( _owrt           != rhs._owrt           ) return ( _owrt           < rhs._owrt           ) ;
      if( _fareClass      != rhs._fareClass      ) return ( _fareClass      < rhs._fareClass      ) ;

      return false ;
    }

    bool operator==( const AddonCombFareClassInfo & rhs ) const
    {
      return(    ( _effInterval    == rhs._effInterval    )
              && ( _vendor         == rhs._vendor         )
              && ( _fareTariff     == rhs._fareTariff     )
              && ( _carrier        == rhs._carrier        )
              && ( _addonFareClass == rhs._addonFareClass )
              && ( _geoAppl        == rhs._geoAppl        )
              && ( _owrt           == rhs._owrt           )
              && ( _fareClass      == rhs._fareClass      )
            ) ;
    }

    friend inline std::ostream & dumpObject( std::ostream & os, const AddonCombFareClassInfo & obj )
    {
      os << "["  ;
      dumpObject( os, obj._effInterval ) ;
      os << "|" << obj._vendor
                << "|" << obj._fareTariff
                << "|" << obj._carrier
                << "|" << obj._addonFareClass
                << "|" << obj._geoAppl
                << "|" << obj._owrt
                << "|" << obj._fareClass
                << "]"
                ;
      return os ;
    }

    void dummyData()
    {
      TSEDateInterval::dummyData( _effInterval ) ;

      _vendor         = "ABCD"     ;
      _fareTariff     = 1          ;
      _carrier        = "EFG"      ;
      _addonFareClass = "HIJKLMNO" ;
      _geoAppl        = 'P'        ;
      _owrt           = 'Q'        ;
      _fareClass      = "RSTUVWXY" ;
    }

    static void dummyData2( AddonCombFareClassInfo & obj )
    {
      TSEDateInterval::dummyData( obj._effInterval ) ;

      obj._vendor         = "abcd"     ;
      obj._fareTariff     = 2          ;
      obj._carrier        = "efg"      ;
      obj._addonFareClass = "hijklmno" ;
      obj._geoAppl        = 'p'        ;
      obj._owrt           = 'q'        ;
      obj._fareClass      = "rstuvwxy" ;
    }

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _effInterval    ) ;
      FLATTENIZE( archive, _vendor         ) ;
      FLATTENIZE( archive, _fareTariff     ) ;
      FLATTENIZE( archive, _carrier        ) ;
      FLATTENIZE( archive, _addonFareClass ) ;
      FLATTENIZE( archive, _geoAppl        ) ;
      FLATTENIZE( archive, _owrt           ) ;
      FLATTENIZE( archive, _fareClass      ) ;
    }

    WBuffer &write (WBuffer &os) const
    {
      return convert(os, this);
    }

    RBuffer &read (RBuffer &is)
    {
      return convert(is, this);
    }
  protected:

      TSEDateInterval _effInterval;
      VendorCode      _vendor;
      TariffNumber    _fareTariff;
      CarrierCode     _carrier;
      FareClassCode   _addonFareClass;
      Indicator       _geoAppl;
      Indicator       _owrt;
      FareClassCode   _fareClass;
    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
    return buffer
           & ptr->_effInterval
           & ptr->_vendor
           & ptr->_fareTariff
           & ptr->_carrier
           & ptr->_addonFareClass
           & ptr->_geoAppl
           & ptr->_owrt
           & ptr->_fareClass;
    }
 };

  class AddonCombFareClassInfoKey
  {
  public:

    // construction/destruction
    // ========================

    AddonCombFareClassInfoKey()
      : _geoAppl( ' ' ),
        _owrt(' ')
    {}

    AddonCombFareClassInfoKey(const FareClassCode& addonFareClass,
                              const Indicator    geoAppl,
                              const Indicator    owrt,
                              const FareClassCode& fareClass )
      : _addonFareClass(addonFareClass),
        _geoAppl( geoAppl ),
        _owrt(owrt),
        _fareClass( fareClass )
    {}

    AddonCombFareClassInfoKey( const AddonCombFareClassInfoKey& rhs )
      : _addonFareClass(rhs._addonFareClass),
        _geoAppl( rhs._geoAppl ),
        _owrt(rhs._owrt),
        _fareClass( rhs._fareClass )
    {}

    // Assignment
    // ==========

    AddonCombFareClassInfoKey& operator = ( const AddonCombFareClassInfoKey& rhs )
    {
      if ( this != &rhs )
      {

        _addonFareClass = rhs._addonFareClass;
        _geoAppl        = rhs._geoAppl;
        _owrt           = rhs._owrt;
        _fareClass      = rhs._fareClass;
      }

      return *this;
    }

    // Comparison
    // ==========

    friend bool operator < ( const AddonCombFareClassInfoKey& x,
                             const AddonCombFareClassInfoKey& y )
    {
      if( x._geoAppl < y._geoAppl )
        return true;
      if( x._geoAppl > y._geoAppl )
        return false;

      if( x._owrt < y._owrt )
        return true;
      if( x._owrt > y._owrt )
        return false;

      if( x._addonFareClass < y._addonFareClass )
        return true;
      if( x._addonFareClass > y._addonFareClass )
        return false;

      return x._fareClass < y._fareClass;
    }

    bool operator == ( const AddonCombFareClassInfoKey& rhs ) const
    {
      return _geoAppl         == rhs._geoAppl &&
             _owrt            == rhs._owrt &&
             _addonFareClass  == rhs._addonFareClass &&
             _fareClass       == rhs._fareClass;

    }

    friend inline std::ostream & dumpObject( std::ostream & os, const AddonCombFareClassInfoKey & obj )
    {
      return os << "|" << obj._addonFareClass
                << "|" << obj._geoAppl
                << "|" << obj._owrt
                << "|" << obj._fareClass
                << "]"
                ;
    }

    static void dummyData( AddonCombFareClassInfoKey & obj )
    {
      obj._addonFareClass = "aaaaaaaa" ;
      obj._geoAppl        = 'A'        ;
      obj._owrt           = '1'        ;
      obj._fareClass      = "bbbbbbbb" ;
    }

    static void dummyData2( AddonCombFareClassInfoKey & obj )
    {
      obj._addonFareClass = "cccccccc" ;
      obj._geoAppl        = 'B'        ;
      obj._owrt           = '2'        ;
      obj._fareClass      = "dddddddd" ;
    }

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _addonFareClass ) ;
      FLATTENIZE( archive, _geoAppl        ) ;
      FLATTENIZE( archive, _owrt      ) ;
      FLATTENIZE( archive, _fareClass      ) ;
    }

    const FareClassCode& addonFareClass() const {return _addonFareClass;}
    Indicator    geoAppl() const { return _geoAppl;}
    Indicator    owrt() const { return _owrt;}

    const FareClassCode& fareClass() const {return _fareClass;}

  protected:

    FareClassCode _addonFareClass ;
    Indicator     _geoAppl        ;
    Indicator     _owrt           ;
    FareClassCode _fareClass      ;
  };

  class AddonCombFareClassInfoKey1
  {
  public:

    // construction/destruction
    // ========================

    AddonCombFareClassInfoKey1()
      : _fareTariff( 0 )
    {}

    AddonCombFareClassInfoKey1( const VendorCode &  vendor,
                                const TariffNumber& fareTariff,
				const CarrierCode & carrier )
      : _vendor( vendor ),
        _fareTariff( fareTariff ),
        _carrier( carrier )
    {}

    AddonCombFareClassInfoKey1( const AddonCombFareClassInfoKey1& rhs )
      : _vendor( rhs._vendor ),
        _fareTariff( rhs._fareTariff ),
        _carrier( rhs._carrier )
    {};

    // Assignment
    // ==========

    AddonCombFareClassInfoKey1& operator = ( const AddonCombFareClassInfoKey1& rhs )
    {
      if ( this != &rhs )
      {
        _vendor  = rhs._vendor;
        _fareTariff  = rhs._fareTariff;
        _carrier = rhs._carrier;
      }

      return *this;
    }

    // Comparison
    // ==========

    friend bool operator < ( const AddonCombFareClassInfoKey1& x,
                             const AddonCombFareClassInfoKey1& y )
    {
      if( x._fareTariff < y._fareTariff )
        return true;
      if( x._fareTariff > y._fareTariff )
        return false;

     if( x._vendor < y._vendor )
        return true;
     if( x._vendor > y._vendor )
        return false;

     return x._carrier < y._carrier;
    }

    bool operator==( const AddonCombFareClassInfoKey1 & rhs ) const
    {
      return _fareTariff == rhs._fareTariff
             && _vendor == rhs._vendor
             && _carrier == rhs._carrier;
    }

    friend inline std::ostream & dumpObject( std::ostream & os, const AddonCombFareClassInfoKey1 & obj )
    {
      return os << "[" << obj._vendor
                << "|" << obj._fareTariff
                << "|" << obj._carrier
                << "]"
                ;
    }

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _vendor  ) ;
      FLATTENIZE( archive, _fareTariff  ) ;
      FLATTENIZE( archive, _carrier ) ;
    }

  protected:

    VendorCode   _vendor;
    TariffNumber _fareTariff;
    CarrierCode  _carrier;
  };

  struct addonCombFareClassHashEqual
  {
    bool operator()(const AddonCombFareClassInfoKey & first, const AddonCombFareClassInfoKey & second) const
    {
      return first == second;
    }
  };

  struct P05Hash
  {
      size_t operator () (const AddonCombFareClassInfoKey &key) const
      {
	std::size_t hash(boost::hash_value(*key.addonFareClass().c_str()));
	boost::hash_combine(hash, key.geoAppl());
	boost::hash_combine(hash, key.owrt());
	boost::hash_combine(hash, key.fareClass());
	return hash;
      }

  private:

      static const int RADIX = 37;
      static const int DIV = 393241;

      static int hashAlphanum(int x)
      {
          if(isupper(x)) x -= 'A';
          else x -= '0' - 'Z' + 'A' - 1;
          return x;
      }
  };

  typedef boost::unordered::unordered_multimap< const AddonCombFareClassInfoKey
                                  , AddonCombFareClassInfo *
                                  , P05Hash
                                  , addonCombFareClassHashEqual
                                  > ADDONHASHMULTI ;


}
