//----------------------------------------------------------------------------
//   Copyright 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include "DateTime.h"
#include <sstream>
#include <boost/tokenizer.hpp>
#include "TseEnums.h"
#include "KeyStream.h"
#include "Code.h"
#include "Hasher.h"

#define MAP_STRING_TO_COMPONENT(a,b) \
  { \
    switch( b ) \
    { \
      case 0: initialized = elementFromString( a, _a ) ; break ; \
      case 1:               elementFromString( a, _b ) ; break ; \
      case 2:               elementFromString( a, _c ) ; break ; \
      case 3:               elementFromString( a, _d ) ; break ; \
      case 4:               elementFromString( a, _e ) ; break ; \
      case 5:               elementFromString( a, _f ) ; break ; \
      case 6:               elementFromString( a, _g ) ; break ; \
      case 7:               elementFromString( a, _h ) ; break ; \
      case 8:               elementFromString( a, _i ) ; break ; \
      case 9:               elementFromString( a, _j ) ; break ; \
      default:                                           break ; \
    } \
  }

#define REPLACE_CHAR_WITH_CHAR(a,b,c) \
  { \
    for( size_t s = 0 ; s < a.size() ; ++s ) \
    { \
      if( a[s] == b ) \
      { \
        a[s] = c ; \
      } \
    } \
  }

namespace tse
{
  //
  // dummy key component
  //
  struct nil_t
  {
      friend inline bool operator< (const nil_t&, const nil_t&)
      {
          return false;
      }

      friend inline bool operator> (const nil_t& , const nil_t&)
      {
          return false;
      }

      friend inline bool operator== (const nil_t& , const nil_t& )
      {
          return true;
      }

      friend inline tse::Hasher& operator<< (tse::Hasher& hasher, const nil_t&)
      {
          return hasher;
      }

      friend inline std::ostream & operator<<( std::ostream & os, const nil_t&)
      {
          return os ;
      }
  };

  inline tse::Hasher & operator<<( tse::Hasher & hasher, const tse::DateTime & dt )
  {
      return hasher << dt.get64BitRep() ;
  }

  //
  // hash table key template.
  // Can be used for keys with up to seven sub keys.
  //
  template<typename A, typename B = nil_t, typename C = nil_t, typename D = nil_t,
           typename E = nil_t, typename F = nil_t, typename G = nil_t, typename H = nil_t,
           typename I = nil_t, typename J = nil_t >
  class HashKey
  {
  public:

    bool initialized ;
    A _a ;
    B _b ;
    C _c ;
    D _d ;
    E _e ;
    F _f ;
    G _g ;
    H _h ;
    I _i ;
    J _j ;

    HashKey() : initialized( false ) { }

    ~HashKey() { }

    HashKey( const HashKey & other )
    {
      copyFrom( other ) ;
    }

    HashKey(const A &a
           , const B &b = B()
           , const C &c = C()
           , const D &d = D()
           , const E &e = E()
           , const F &f = F()
           , const G &g = G()
           , const H &h = H()
           , const I &i = I()
           , const J &j = J()
           )
      : _a( a )
      , _b( b )
      , _c( c )
      , _d( d )
      , _e( e )
      , _f( f )
      , _g( g )
      , _h( h )
      , _i( i )
      , _j( j )
    {
      initialized = true ;
    }

    HashKey & operator=( const HashKey & other )
    {
      copyFrom( other ) ;
      return *this ;
    }

    template<class T>
    HashKey & operator=( const T & other );

    friend bool operator< (const HashKey& key, const HashKey& key2)
    {
      if (key._a < key2._a) return true  ;
      if (key._a > key2._a) return false ;

      if (key._b < key2._b) return true  ;
      if (key._b > key2._b) return false ;

      if (key._c < key2._c) return true  ;
      if (key._c > key2._c) return false ;

      if (key._d < key2._d) return true  ;
      if (key._d > key2._d) return false ;

      if (key._e < key2._e) return true  ;
      if (key._e > key2._e) return false ;

      if (key._f < key2._f) return true  ;
      if (key._f > key2._f) return false ;

      if (key._g < key2._g) return true  ;
      if (key._g > key2._g) return false ;

      if (key._h < key2._h) return true  ;
      if (key._h > key2._h) return false ;

      if (key._i < key2._i) return true  ;
      if (key._i > key2._i) return false ;

      if (key._j < key2._j) return true  ;
      if (key._j > key2._j) return false ;

      return false ;
    }

    friend bool operator== (const HashKey& key, const HashKey& key2)
    {
      return(    ( key.initialized == key2.initialized )
              && ( key._a          == key2._a          )
              && ( key._b          == key2._b          )
              && ( key._c          == key2._c          )
              && ( key._d          == key2._d          )
              && ( key._e          == key2._e          )
              && ( key._f          == key2._f          )
              && ( key._g          == key2._g          )
              && ( key._h          == key2._h          )
              && ( key._i          == key2._i          )
              && ( key._j          == key2._j          )
            ) ;
    }

    friend tse::Hasher& operator<< (tse::Hasher& hasher, const HashKey& key)
    {
      return( hasher << key._a
                     << key._b
                     << key._c
                     << key._d
                     << key._e
                     << key._f
                     << key._g
                     << key._h
                     << key._i
                     << key._j
             ) ;
    }

    void copyFrom( const HashKey & other )
    {
      if( this != &other )
      {
        initialized = other.initialized ;
        _a = other._a ;
        _b = other._b ;
        _c = other._c ;
        _d = other._d ;
        _e = other._e ;
        _f = other._f ;
        _g = other._g ;
        _h = other._h ;
        _i = other._i ;
        _j = other._j ;
      }
    }

    friend inline std::ostream & dumpObject( std::ostream & os, const HashKey & obj )
    {
      KeyStream flatKey(0);
      flatKey << obj;
      return os << "[" << flatKey << "]" ;
    }
#ifdef _DEBUGFLATKEY
    // Separate fields with a vertical bar.
    std::string toString() const
    {
      std::ostringstream os ;
      bool nill = streamElement( os, _a ) ;
      if( ! nill )
      {
        os << '|' ;
        nill = streamElement( os, _b ) ;
        if( ! nill )
        {
          os << '|' ;
          nill = streamElement( os, _c ) ;
          if( ! nill )
          {
            os << '|' ;
            nill = streamElement( os, _d ) ;
            if( ! nill )
            {
              os << '|' ;
              nill = streamElement( os, _e ) ;
              if( ! nill )
              {
                os << '|' ;
                nill = streamElement( os, _f ) ;
                if( ! nill )
                {
                  os << '|' ;
                  nill = streamElement( os, _g ) ;
                  if( ! nill )
                  {
                    os << '|' ;
                    nill = streamElement( os, _h ) ;
                    if( ! nill )
                    {
                      os << '|' ;
                      nill = streamElement( os, _i ) ;
                      if( ! nill )
                      {
                        os << '|' ;
                        nill = streamElement( os, _j ) ;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

      std::string retval( os.str() ) ;

      if( nill )
      {
        size_t sz = retval.size() ;
        if( ( sz > 0 ) && ( retval[sz-1] == '|' ) )
        {
          retval.erase( sz-1 ) ;
        }
      }

      REPLACE_CHAR_WITH_CHAR( retval, ' ' , '^' ) ;
      REPLACE_CHAR_WITH_CHAR( retval, '\0', '~' ) ;

      return retval ;
    }

    bool streamElement( std::ostream & os, const nil_t & ref ) const
    {
      return true ;
    }

    bool streamElement( std::ostream & os, const bool & ref ) const
    {
      os << ( ref ? "1" : "0" ) ;
      return false ;
    }

    template<typename T>
    bool streamElement( std::ostream & os, const T & ref ) const
    {
      os << ref ;
      return false ;
    }

    friend inline std::ostream & operator<<( std::ostream & os, const HashKey & obj )
    {
      return os << obj.toString();
    }
#else
    std::string toString() const
    {
      KeyStream stream(0);
      stream << *this;
      std::string result(stream);
      return result;
    }
#endif// _DEBUGFLATKEY
    void fromString( const std::string & keyString )
    {
      static std::string emptyString ;
      size_t num = 0 ;

      if( keyString.empty() )
      {
        MAP_STRING_TO_COMPONENT( emptyString, num ) ;
      }
      else
      {
        static std::size_t numComponents = 10 ;
        static boost::char_separator<char> fieldSep( "|", "", boost::keep_empty_tokens ) ;

        std::string str( keyString ) ;

        REPLACE_CHAR_WITH_CHAR( str, '^', ' '  ) ;
        REPLACE_CHAR_WITH_CHAR( str, '~', '\0' ) ;

        boost::tokenizer<boost::char_separator<char>> fields( str, fieldSep ) ;
        for( boost::tokenizer<boost::char_separator<char>>::const_iterator f = fields.begin()
           ; f != fields.end()
           ; ++f, ++num
           )
        {
          MAP_STRING_TO_COMPONENT( *f, num ) ;
        }

        while( num < numComponents )
        {
          MAP_STRING_TO_COMPONENT( emptyString, num++ ) ;
        }
      }
    }

    template<size_t n>
    bool elementFromString( const std::string & str, Code<n> & ref )
    {
      ref = str ;
      return true ;
    }

    bool elementFromString( const std::string & str, std::string & ref )
    {
      ref = str ;
      return true ;
    }

    bool elementFromString( const std::string & str, bool & ref )
    {
      ref = ( str == "0" ? false : true ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, int & ref )
    {
      ref = atoi( str.c_str() ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, char & ref )
    {
      ref = ( str.length() > 0 ? str[0] : ' ' ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, nil_t & ref )
    {
      return false ;
    }

    bool elementFromString( const std::string & str, long long int & ref )
    {
      ref = atoll( str.c_str() ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, RecordScope & ref )
    {
      ref = static_cast<RecordScope>( atoi( str.c_str() ) ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, GlobalDirection & ref )
    {
      ref = static_cast<GlobalDirection>( atoi( str.c_str() ) ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, uint64_t & ref )
    {
      ref = atoll( str.c_str() ) ;
      return true ;
    }

    bool elementFromString( const std::string & str, tse::DateTime & ref )
    {
#if 0 // LDC_DEBUG_DATETIME Support debugging if we run into datetime object problems.
      if ( str == "*open date*" )
      {
        std::cout << ">Open Date[" << str << "]" << std::endl ;
        ref = DateTime::openDate() ;
      }
      else if ( str == "*infinity date*" )
      {
        std::cout << ">Infinity Date[" << str << "]" << std::endl ;
        ref = boost::date_time::pos_infin ;
      }
      else if ( str == "*empty date*"  )
      {
        std::cout << ">Empty Date[" << str << "]" << std::endl ;
        ref = tse::DateTime::emptyDate() ;
      }
      else
#endif
      {
#if 1
      ref = DateTime( const_cast<std::string &>( str ) ) ;
#else // not LDC_USE_READABLE_DATETIME_STRING [julian]
        long date ;
        sscanf( str.c_str(), "%ld", &date ) ;
        ref.set64BitRep( date ) ;
#endif
      }
      return true ;
    }

    friend KeyStream &operator << (KeyStream &stream,
                                   const HashKey &key)
    {
      elementToFlatKey(key._a, stream)
      && elementToFlatKey(key._b, stream)
      && elementToFlatKey(key._c, stream)
      && elementToFlatKey(key._d, stream)
      && elementToFlatKey(key._e, stream)
      && elementToFlatKey(key._f, stream)
      && elementToFlatKey(key._g, stream)
      && elementToFlatKey(key._h, stream)
      && elementToFlatKey(key._i, stream)
      && elementToFlatKey(key._j, stream);
      endKey(stream);
      return stream;
    }

    size_t boostHash() const
    {
      size_t hash(0);
      hash_combine(hash, _a);
      hash_combine(hash, _b);
      hash_combine(hash, _c);
      hash_combine(hash, _d);
      hash_combine(hash, _e);
      hash_combine(hash, _f);
      hash_combine(hash, _g);
      hash_combine(hash, _h);
      hash_combine(hash, _i);
      hash_combine(hash, _j);
      return hash;
    }
  private:
    static void hash_combine(size_t& hash,
                             const nil_t&)
    {
    }
    template <size_t n> static void hash_combine(size_t& hash,
                                                 const Code<n>& str)
    {
      typename Code<n>::const_iterator it(str.begin());
      char c(0);
      for (; (c = *it) != 0; ++it)
      {
        boost::hash_combine(hash, c);
      }
    }
    template <typename T> static void hash_combine(size_t& hash,
                                                   const T& field)
    {
      boost::hash_combine(hash, field);
    }
  };

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G,
            typename H,
            typename I,
            typename J> std::ostream& operator << (std::ostream &stream,
                                                   HashKey<A, B, C, D, E, F, G, H, I, J>& key)
  {
    return stream << key._a
                  << key._b
                  << key._c
                  << key._d
                  << key._e
                  << key._f
                  << key._g
                  << key._h
                  << key._i
                  << key._j;
  }

}// tse
