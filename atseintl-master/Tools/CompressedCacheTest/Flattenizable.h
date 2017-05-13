//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include <boost/unordered/unordered_map.hpp>
#include <boost/container/string.hpp>

#include "HashKey.h"
#include "DateTime.h"
#include "TseEnums.h"
#include "Code.h"

#include <cstdio>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Macros for use in external object's "flattenize" method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define FLATTENIZE_SAVE( archive, t, ldcMaxBlobSize, name, flatKey )    \
  { \
    FLATTENIZE_PUSH( archive, #t ) ; \
    tse::flattenizer::serialize( archive, t, ldcMaxBlobSize, name, flatKey ) ; \
    FLATTENIZE_POP( archive ) ; \
  }
  /***/

#define FLATTENIZE_RESTORE( archive, t, extBuf, extSz ) \
  { \
    FLATTENIZE_PUSH( archive, #t ) ; \
    flattenizer::deserialize( archive, t, extBuf, extSz ) ; \
    FLATTENIZE_POP( archive ) ; \
  }
  /***/

//
// The __PRETTY_FUNCTION__ built-in returns function signatures.
// We only care about the class name, so this bit of malarky
// strips out the other stuff.  Since we don't use RTTI for
// performance reasons, this is about the best we can do...
//
#define FLATTENIZE_PUSH_CLASS_NAME( archive ) \
  bool theClassNameWasPushed( false ) ; \
  if( archive.isTracing() ) \
  { \
    std::string funcSig( __FUNCTION__ ) ; \
    if( funcSig.find( "static" ) == std::string::npos ) \
    { \
      size_t pos = funcSig.find_first_of( "(" ) ; \
      if( pos != std::string::npos ) \
      { \
        funcSig.erase( pos ) ; \
      } \
      pos = funcSig.find_last_of( " " ) ; \
      if( pos != std::string::npos ) \
      { \
        funcSig.erase( 0, pos+1 ) ; \
      } \
      pos = funcSig.find_last_of( ":" ) ; \
      if( pos != std::string::npos ) \
      { \
        funcSig.erase( pos - 1 ) ; \
      } \
      if( funcSig.substr( 0, 5 ) == "tse::" ) funcSig.erase( 0, 5 ) ; \
      std::string temp( "[" ) ; \
      temp.append( funcSig ) ; \
      temp.append( "]" ) ; \
      FLATTENIZE_PUSH( archive, temp.c_str() ) ; \
      theClassNameWasPushed = true ; \
    } \
  }
  /***/

#define FLATTENIZE( archive, t ) \
  { \
    FLATTENIZE_PUSH_CLASS_NAME( archive ) \
    FLATTENIZE_PUSH( archive, #t ) ; \
    tse::flattenizer::process( archive, t ) ; \
    FLATTENIZE_POP( archive ) ; \
    if( theClassNameWasPushed ) FLATTENIZE_POP( archive ) ; \
  }
  /***/

#define FLATTENIZE_BASE_OBJECT( archive, b ) \
  { \
    FLATTENIZE_PUSH_CLASS_NAME( archive ) \
    b::flattenize( archive ) ; \
    if( theClassNameWasPushed ) FLATTENIZE_POP( archive ) ; \
  }
  /***/

#define FLATTENIZE_ENUM( archive, t ) \
  { \
    FLATTENIZE_PUSH_CLASS_NAME( archive ) \
    FLATTENIZE_PUSH( archive, #t ) ; \
    tse::flattenizer::process_enum( archive, t ) ; \
    FLATTENIZE_POP( archive ) ; \
    if( theClassNameWasPushed ) FLATTENIZE_POP( archive ) ; \
  }
  /***/

#define FLATTENIZE_ENUM_VECTOR( archive, t ) \
  { \
    FLATTENIZE_PUSH_CLASS_NAME( archive ) \
    FLATTENIZE_PUSH( archive, #t ) ; \
    tse::flattenizer::process_enum_vector( archive, t ) ; \
    FLATTENIZE_POP( archive ) ; \
    if( theClassNameWasPushed ) FLATTENIZE_POP( archive ) ; \
  }
  /***/

#define FLATTENIZE_ENUM_KEY_MAP( archive, t ) \
  { \
    FLATTENIZE_PUSH_CLASS_NAME( archive ) \
    FLATTENIZE_PUSH( archive, #t ) ; \
    tse::flattenizer::process_enum_key_map( archive, t ) ; \
    FLATTENIZE_POP( archive ) ; \
    if( theClassNameWasPushed ) FLATTENIZE_POP( archive ) ; \
  }
  /***/

#define FLATTENIZE_SUBITEM_BEGIN( archive ) \
  { \
    if( archive.usingOStream() ) \
    { \
      archive.increaseIndent() ; \
      archive.setNewline() ; \
    } \
  }
  /***/

#define FLATTENIZE_SUBITEM_END( archive ) \
  { \
    if( archive.usingOStream() ) \
    { \
      archive.setNewline() ; \
      archive.decreaseIndent() ; \
    } \
  }
  /***/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Macros for internal (Flattenizable.h) use only
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define FLATTENIZE_PUSH( archive, t ) \
  if( archive.isTracing() )  \
  { \
    archive.pushFieldRef( t ) ; \
  }
  /***/

#define FLATTENIZE_POP( archive ) \
  if( archive.isTracing() ) \
  { \
    archive.popFieldRef() ; \
  }
  /***/

#define FLATTENIZABLE_BASE_TYPE( T ) \
  namespace tse \
  { \
    namespace flattenizer \
    {  \
      inline void flatten( Flattenizable::Archive & archive, const T & t ) \
      { \
        archive.append( &t, sizeof( T ) ) ; \
      } \
      \
      inline void unflatten( Flattenizable::Archive & archive, T & t ) \
      { \
        archive.extract( &t, sizeof( T ) ) ; \
      } \
      \
      inline void calcmem( Flattenizable::Archive & archive, const T & t ) \
      { \
        archive.addSize( sizeof( T ) ) ; \
      } \
    } \
  }
  /***/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Flattenizable::Archive
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  struct Flattenizable
  {
    enum Action
    {
        FLATTEN = 0
      , UNFLATTEN
      , CALCMEM
    } ;

    //--//--//--//--//--//--//--//--//--//--//--//--//--//--//
    //  tse::flattenizable::Archive class implementation    //
    //--//--//--//--//--//--//--//--//--//--//--//--//--//--//

    class Archive
    {
    public:

      Archive()
        : _action( CALCMEM )
        , _buf( NULL )
        , _offset( 0 )
        , _size( 0 )
        , _isExternal( false )
        , _trace( false )
        , _os( NULL )
        , _newline( false )
        , _fieldCount( 0 )
        , _indentLevel( 0 )
        , _stringifyDates( false )
      {
      }

      ~Archive() { reset() ; }

      void setAction( Action action )            { _action         = action ; }
      void setOStream( std::ostringstream * os ) { _os             = os     ; }
      void setNewline()                          { _newline        = true   ; }
      void setStringifyDates()                   { _stringifyDates = true   ; }

      void increaseIndent()                      { ++_indentLevel ; }
      void decreaseIndent()                      { if( _indentLevel > 0 ) --_indentLevel  ; }

      bool   usingOStream()        const { return ( _os != NULL ) ; }
      bool   stringifyDates()      const { return _stringifyDates ; }
      Action action()              const { return _action         ; }
      size_t size()                const { return _size           ; }
      size_t offset()              const { return _offset         ; }
      char * buf()                 const { return _buf            ; }

      void useExternalBuffer( const char * buf, size_t size )
      {
        reset() ;
        _buf        = const_cast<char *>( buf ) ;
        _size       = size ;
        _isExternal = true ;
      }

      void setTrace( bool trace = true ) { _trace = trace ; }
      bool isTracing() const { return _trace ; }

      void resetBufferPointer()
      {
        if( _buf != NULL )
        {
          if( ! _isExternal )
          {
            delete [] _buf ;
          }
          _buf = NULL ;
        }
      }

      void reset()
      {
        resetBufferPointer() ;
        _offset = 0 ;
        _size = 0 ;
        _isExternal = false ;
      }

      void handleIndent()
      {
        if( _newline )
        {
          (*_os) << std::endl ;
          for( size_t i = 0 ; i < _indentLevel ; ++i )
          {
            (*_os) << ' ' ;
          }
          _fieldCount = 0 ;
          _newline = false ;
        }

        ++_fieldCount ;
      }

      void printToStream( const char & t, size_t n )
      {
        if( usingOStream() )
        {
          handleIndent() ;
          char * p( const_cast<char *>( &t ) ) ;
          for( size_t i = 0 ; i < n ; ++i, ++p )
          {
            if( *p == '\0' )
            {
              (*_os) << ' ' ;
            }
            else
            {
              (*_os) << *p ;
            }
          }
          (*_os) << "|" ;
        }
      }

      template <typename T>
      void printToStream( const T & t, size_t n )
      {
        if( usingOStream() )
        {
          handleIndent() ;
          (*_os) << t << "|" ;
        }
      }

      void allocate()
      {
        resetBufferPointer() ;

        if( _size > 0 )
        {
          _buf = new char[_size] ;
        }

        if( isTracing() )
        {
          std::cout << "FLATTENIZE: alloc  - " << _size << " bytes." << std::endl ;
        }
      }

      void rewind()
      {
        _offset = 0 ;
        if( isTracing() )
        {
          std::cout << "FLATTENIZE: rewind - OFFSET=" << _offset << std::endl ;
        }
      }

      void extract( size_t & to ) { extract( &to, sizeof( size_t ) )  ; }

      template <typename T>
      void extract( T * to, size_t n )
      {
        if( _buf != NULL )
        {
          size_t boundary( _offset + n ) ;
          if( boundary <= _size )
          {
            memcpy( to, _buf + _offset, n ) ;

            if( isTracing() )
            {
              std::cout << "FLATTENIZE: xtract - FIELD="
                        << fieldReference()
                        << ", OFFSET="
                        << _offset
                        << ", SIZE="
                        << n
                        << ", VALUE="
                        << *to
                        << std::endl ;
            }

            if( usingOStream() ) printToStream( *to, n ) ;

            _offset = boundary ;
          }
          else
          {
            throw std::out_of_range( "Flattenizable::Archive::extract failed due to boundary violation." ) ;
          }
        }
        else
        {
          throw std::out_of_range( "Flattenizable::Archive::extract failed due to unallocated buffer." ) ;
        }
      }

      template <typename T>
      void direct_assign( T & t, size_t bytes )
      {
        char * ptr = _buf + _offset ;

        t.assign( ptr, bytes ) ;

        if( isTracing() )
        {
          std::cout << "FLATTENIZE: direct - FIELD="
                    << fieldReference()
                    << ", OFFSET="
                    << _offset
                    << ", SIZE="
                    << bytes
                    << ", VALUE="
                    << t
                    << std::endl ;
        }

        if( usingOStream() ) printToStream( t, bytes ) ;

        _offset += bytes ;
      }

      void append( const size_t & from ) { append( &from, sizeof( size_t ) ) ; }

      template <typename T>
      void append( T * from, size_t n )
      {
        if( _buf != NULL )
        {
          size_t boundary( _offset + n ) ;
          if( boundary <= _size )
          {
            memcpy( _buf + _offset, from, n ) ;

            if( isTracing() )
            {
              std::cout << "FLATTENIZE: append - FIELD="
                        << fieldReference()
                        << ", OFFSET="
                        << _offset
                        << ", SIZE="
                        << n
                        << ", VALUE="
                        << *from
                        << std::endl ;
            }

            if( usingOStream() ) printToStream( *from, n ) ;

            _offset = boundary ;
          }
          else
          {
            throw std::out_of_range( "Flattenizable::Archive::append failed due to boundary violation." ) ;
          }
        }
        else
        {
          throw std::out_of_range( "Flattenizable::Archive::append failed due to unallocated buffer." ) ;
        }
      }

      void addSize( size_t n )
      {
        _size += n ;

        if( isTracing() )
        {
          std::cout << "FLATTENIZE: calcsz - FIELD="
                    << fieldReference()
                    << ", SIZE="
                    << n
                    << std::endl ;
        }
      }

      void pushFieldRef( size_t num )
      {
        char buf[16] ;
        sprintf( buf, "%ld", num ) ;
        _stack.push_back( buf ) ;
      }

      void pushFieldRef( const std::string & name ) { _stack.push_back( name ) ; }

      void popFieldRef()
      {
        if( _stack.size() > 0 )
        {
          _stack.pop_back() ;
        }
        else
        {
          throw std::out_of_range( "Flattenizable::Archive::popFieldRef failed due to unexpected stack." ) ;
        }
      }

    private:

      std::string fieldReference()
      {
        std::string retval ;
        for( FIELDNAMESTACK::const_iterator it = _stack.begin() ; it != _stack.end() ; ++it )
        {
          const std::string & ref( *it ) ;
          if( ( ! retval.empty() ) && ( ref[0] != '[' ) )
          {
            retval.append( "." ) ;
          }
          retval.append( ref ) ;
        }
        return retval ;
      }

      typedef std::vector<std::string> FIELDNAMESTACK ;

      Archive( const Archive & rhs ) ;
      Archive & operator=( const Archive & rhs ) ;

      Action               _action         ;
      char   *             _buf            ;
      size_t               _offset         ;
      size_t               _size           ;
      bool                 _isExternal     ;
      bool                 _trace          ;
      FIELDNAMESTACK       _stack          ;
      std::ostringstream * _os             ;
      bool                 _newline        ;
      size_t               _fieldCount     ;
      size_t               _indentLevel    ;
      bool                 _stringifyDates ;


    } ; // class Archive
  } ; // class Flattenizable
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Forward declarations to force lookup of the specialization.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    // tse::DateTime

    inline void flatten  ( tse::Flattenizable::Archive & archive, const tse::DateTime & t ) ;
    inline void unflatten( tse::Flattenizable::Archive & archive, tse::DateTime & t ) ;
    inline void calcmem  ( tse::Flattenizable::Archive & archive, const tse::DateTime & t ) ;

    // Code<n>

    template <size_t n>
    inline void flatten  ( Flattenizable::Archive & archive, const Code<n> & t ) ;
    template <size_t n>
    inline void unflatten( Flattenizable::Archive & archive, Code<n> & t ) ;
    template <size_t n>
    inline void calcmem  ( Flattenizable::Archive & archive, const Code<n> & t ) ;

    // nil_t

    inline void flatten  ( Flattenizable::Archive & archive, const nil_t & f ) ;
    inline void unflatten( Flattenizable::Archive & archive, nil_t & f ) ;
    inline void calcmem  ( Flattenizable::Archive & archive, const nil_t & f ) ;

    // std::string

    inline void flatten  ( Flattenizable::Archive & archive, const std::string & t ) ;
    inline void unflatten( Flattenizable::Archive & archive, std::string & t ) ;
    inline void calcmem  ( Flattenizable::Archive & archive, const std::string & t ) ;

    // std::vector<const std::string*>

    inline void flatten  ( Flattenizable::Archive & archive, const std::vector<const std::string*> & v ) ;
    inline void unflatten( Flattenizable::Archive & archive, std::vector<const std::string*> & v ) ;
    inline void calcmem  ( Flattenizable::Archive & archive, const std::vector<const std::string*> & v ) ;

    // std::map<A,B*>

    template<typename A, typename B>
    inline void flatten  ( Flattenizable::Archive & archive, const std::map<A,B*> & m ) ;
    template<typename A, typename B>
    inline void unflatten( Flattenizable::Archive & archive, std::map<A,B*> & m ) ;
    template<typename A, typename B>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::map<A,B*> & m ) ;

    // std::multimap<A,B*>

    template<typename A, typename B>
    inline void flatten  ( Flattenizable::Archive & archive, const std::multimap<A,B*> & mm ) ;
    template<typename A, typename B>
    inline void unflatten( Flattenizable::Archive & archive, std::multimap<A,B*> & mm ) ;
    template<typename A, typename B>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::multimap<A,B*> & mm ) ;

    //  std::tr1::unordered_map<K,I,F>

    template<typename K, typename I, typename F>
    inline void flatten  ( Flattenizable::Archive & archive, const boost::unordered_map<K,I,F> & hm ) ;
    template<typename K, typename I, typename F>
    inline void unflatten( Flattenizable::Archive & archive, boost::unordered_map<K,I,F> & hm ) ;
    template<typename K, typename I, typename F>
    inline void calcmem  ( Flattenizable::Archive & archive, const boost::unordered_map<K,I,F> & hm ) ;

    // std::tr1::unordered_multimap<const A,B*,C,D>

    template<typename A, typename B, typename C, typename D>
    static void flatten  ( Flattenizable::Archive & archive, const boost::unordered_multimap<const A,B*,C,D> & mm ) ;
    template<typename A, typename B, typename C, typename D>
    static void unflatten( Flattenizable::Archive & archive, boost::unordered_multimap<const A,B*,C,D> & mm ) ;
    template<typename A, typename B, typename C, typename D>
    static void calcmem  ( Flattenizable::Archive & archive, const boost::unordered_multimap<const A,B*,C,D> & mm ) ;

    // std::vector<I*>
    template <typename I>
    inline void flatten  ( Flattenizable::Archive & archive, const std::vector<I*> & v ) ;
    template <typename I>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<I*> & v ) ;
    template <typename I>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::vector<I*> & v ) ;

    // std::vector<const I*>

    template <typename I>
    inline void flatten  ( Flattenizable::Archive & archive, const std::vector<const I*> & v ) ;
    template <typename I>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<const I*> & v ) ;
    template <typename I>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::vector<const I*> & v ) ;

    // std::vector<std::shared_ptr<I>>
    template <typename I>
    inline void
    flatten(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<I>>& v);
    template <typename I>
    inline void
    unflatten(Flattenizable::Archive& archive, std::vector<std::shared_ptr<I>>& v);
    template <typename I>
    inline void
    calcmem(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<I>>& v);

    // std::vector<T>

    template <typename T>
    inline void flatten  ( Flattenizable::Archive & archive, const std::vector<T> & v ) ;
    template <typename T>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<T> & v ) ;
    template <typename T>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::vector<T> & v ) ;

    // std::set<T>

    template <typename T>
    inline void flatten  ( Flattenizable::Archive & archive, const std::set<T> & s ) ;
    template <typename T>
    inline void unflatten( Flattenizable::Archive & archive, std::set<T> & s ) ;
    template <typename T>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::set<T> & s ) ;

    // std::map<A,B>

    template<typename A, typename B>
    inline void flatten  ( Flattenizable::Archive & archive, const std::map<A,B> & m ) ;
    template<typename A, typename B>
    inline void unflatten( Flattenizable::Archive & archive, std::map<A,B> & m ) ;
    template<typename A, typename B>
    inline void calcmem  ( Flattenizable::Archive & archive, const std::map<A,B> & m ) ;

    // array of T

    template <typename T, size_t n>
    inline void flatten  ( Flattenizable::Archive & archive, const T (&a)[n] ) ;
    template <typename T, size_t n>
    inline void unflatten( Flattenizable::Archive & archive, T (&a)[n] ) ;
    template <typename T, size_t n>
    inline void calcmem  ( Flattenizable::Archive & archive, const T (&a)[n] ) ;

    // HashKey<A,B,C,D,E,F,G,H,I,J>

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
    inline void flatten  ( Flattenizable::Archive & archive, const HashKey<A,B,C,D,E,F,G,H,I,J> & t );

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
    inline void unflatten( Flattenizable::Archive & archive, HashKey<A,B,C,D,E,F,G,H,I,J> & t );

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
    inline void calcmem  ( Flattenizable::Archive & archive, const HashKey<A,B,C,D,E,F,G,H,I,J> & t );

    // Things that have a flattenize method

    template <typename F>
    inline void flatten  ( Flattenizable::Archive & archive, const F & f );

    template <typename F>
    inline void unflatten( Flattenizable::Archive & archive, F & f );

    template <typename F>
    inline void calcmem  ( Flattenizable::Archive & archive, const F & f );

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// tse enums
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

FLATTENIZABLE_BASE_TYPE( tse::GlobalDirection      ) ;
FLATTENIZABLE_BASE_TYPE( tse::Directionality       ) ;
FLATTENIZABLE_BASE_TYPE( tse::RecordScope          ) ;
FLATTENIZABLE_BASE_TYPE( tse::MATCHTYPE            ) ;
FLATTENIZABLE_BASE_TYPE( tse::FCASegDirectionality ) ;
FLATTENIZABLE_BASE_TYPE( tse::RoundingRule         ) ;
FLATTENIZABLE_BASE_TYPE( tse::LocType              ) ;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// tse::DateTime
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    inline void flatten( tse::Flattenizable::Archive & archive, const tse::DateTime & t )
    {
      if( archive.stringifyDates() )
      {
        // this option is only activated when dumping objects for diagnostics
        std::ostringstream temp ;
        temp << t ;
        flatten( archive, temp.str() ) ;
      }
      else
      {
        int64_t temp( t.get64BitRep() ) ;
        archive.append(&temp, sizeof( int64_t ) ) ;
      }
    }

    inline void unflatten( tse::Flattenizable::Archive & archive, tse::DateTime & t )
    {
      if( archive.stringifyDates() )
      {
        // this option is only activated when dumping objects for diagnostics
        std::string temp ;
        unflatten( archive, temp ) ;
        t = DateTime( temp ) ;
      }
      else
      {
        int64_t temp( 0 ) ;
        archive.extract( &temp, sizeof( int64_t ) ) ;
        t.set64BitRep( temp ) ;
      }
    }

    inline void calcmem( tse::Flattenizable::Archive & archive, const tse::DateTime & t )
    {
      if( archive.stringifyDates() )
      {
        // this option is only activated when dumping objects for diagnostics
        std::ostringstream temp ;
        temp << t ;
        calcmem( archive, temp.str() ) ;
      }
      else
      {
        archive.addSize( sizeof( int64_t ) ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Code<n>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <size_t n>
    inline void flatten( Flattenizable::Archive & archive, const Code<n> & t )
    {
      archive.append(t.c_str(), n ) ;
    }

    template <size_t n>
    inline void unflatten( Flattenizable::Archive & archive, Code<n> & t )
    {
      archive.direct_assign( t, n ) ;
    }

    template <size_t n>
    inline void calcmem( Flattenizable::Archive & archive, const Code<n> & t )
    {
      archive.addSize( n ) ;
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// nil_t
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    inline void flatten( Flattenizable::Archive & archive, const nil_t & f )
    {
      // do nothing
    }

    inline void unflatten( Flattenizable::Archive & archive, nil_t & f )
    {
      // do nothing
    }

    inline void calcmem( Flattenizable::Archive & archive, const nil_t & f )
    {
      // do nothing
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    inline void flatten( Flattenizable::Archive & archive, const std::string & t )
    {
      size_t sz( t.size() ) ;
      archive.append(sz ) ;
      if( sz > 0 )
      {
        archive.append(t.c_str(), sz ) ;
      }
    }

    inline void unflatten( Flattenizable::Archive & archive, std::string & t )
    {
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      if( sz > 0 )
      {
        archive.direct_assign( t, sz ) ;
      }
    }

    inline void calcmem( Flattenizable::Archive & archive, const std::string & t )
    {
      archive.addSize( sizeof( size_t ) ) ;
      archive.addSize( t.size() ) ;
    }

  } // namespace flattenizer
} // namespace tse
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// boost::container::string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    inline void flatten( Flattenizable::Archive & archive, const boost::container::string & t )
    {
      size_t sz( t.size() ) ;
      archive.append(sz ) ;
      if( sz > 0 )
      {
        archive.append(t.c_str(), sz ) ;
      }
    }

    inline void unflatten( Flattenizable::Archive & archive, boost::container::string & t )
    {
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      if( sz > 0 )
      {
        archive.direct_assign( t, sz ) ;
      }
    }

    inline void calcmem( Flattenizable::Archive & archive, const boost::container::string & t )
    {
      archive.addSize( sizeof( size_t ) ) ;
      archive.addSize( t.size() ) ;
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// integral types
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

FLATTENIZABLE_BASE_TYPE( float              ) ;
FLATTENIZABLE_BASE_TYPE( double             ) ;
FLATTENIZABLE_BASE_TYPE( short              ) ;
FLATTENIZABLE_BASE_TYPE( unsigned short     ) ;
FLATTENIZABLE_BASE_TYPE( uint8_t            ) ;
FLATTENIZABLE_BASE_TYPE( int                ) ;
FLATTENIZABLE_BASE_TYPE( unsigned int       ) ;
FLATTENIZABLE_BASE_TYPE( long               ) ;
FLATTENIZABLE_BASE_TYPE( unsigned long      ) ;
FLATTENIZABLE_BASE_TYPE( long long          ) ;
FLATTENIZABLE_BASE_TYPE( unsigned long long ) ;
FLATTENIZABLE_BASE_TYPE( bool               ) ;
FLATTENIZABLE_BASE_TYPE( char               ) ;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::vector<const std::string*>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    inline void flatten( Flattenizable::Archive & archive, const std::vector<const std::string*> & v )
    {
      size_t item_counter( 0 ) ;
      archive.append(v.size() ) ;
      for( std::vector<const std::string*>::const_iterator i = v.begin() ; i != v.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, *(const_cast<const std::string*>( *i )) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    inline void unflatten( Flattenizable::Archive & archive, std::vector<const std::string*> & v )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      v.clear() ;
      v.reserve( sz ) ;
      while( sz-- )
      {
        std::string * info( new std::string ) ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, *info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        v.push_back( info ) ;
      }
    }

    inline void calcmem( Flattenizable::Archive & archive, const std::vector<const std::string*> & v )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( std::vector<const std::string*>::const_iterator i = v.begin() ; i != v.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, *(const_cast<const std::string*>( *i )) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::map<A,B*>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template<typename A, typename B>
    inline void flatten( Flattenizable::Archive & archive, const std::map<A,B*> & m )
    {
      size_t item_counter( 0 ) ;
      archive.append(m.size() ) ;
      for( typename std::map<A,B*>::const_iterator i = m.begin() ; i != m.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*i).first ) ;
        flatten( archive, *(const_cast<B*>((*i).second)) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template<typename A, typename B>
    inline void unflatten( Flattenizable::Archive & archive, std::map<A,B*> & m )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      m.clear() ;
      while( sz-- )
      {
        A key ;
        B * info( new B ) ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, const_cast<A&>( key ) ) ;
        unflatten( archive, *info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        m.insert( typename std::map<A,B*>::value_type( key, info ) ) ;
      }
    }

    template<typename A, typename B>
    inline void calcmem( Flattenizable::Archive & archive, const std::map<A,B*> & m )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::map<A,B*>::const_iterator i = m.begin() ; i != m.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*i).first ) ;
        calcmem( archive, *(const_cast<B*>((*i).second)) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::multimap<A,B*>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template<typename A, typename B>
    inline void flatten( Flattenizable::Archive & archive, const std::multimap<A,B*> & mm )
    {
      size_t item_counter( 0 ) ;
      archive.append(mm.size() ) ;
      for( typename std::multimap<A,B*>::const_iterator i = mm.begin() ; i != mm.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*i).first ) ;
        flatten( archive, *(const_cast<B*>((*i).second)) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template<typename A, typename B>
    inline void unflatten( Flattenizable::Archive & archive, std::multimap<A,B*> & mm )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      mm.clear() ;
      while( sz-- )
      {
        A key ;
        B * info( new B ) ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, const_cast<A&>( key ) ) ;
        unflatten( archive, *info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        mm.insert( typename std::multimap<A,B*>::value_type( key, info ) ) ;
      }
    }

    template<typename A, typename B>
    inline void calcmem( Flattenizable::Archive & archive, const std::multimap<A,B*> & mm )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::multimap<A,B*>::const_iterator i = mm.begin() ; i != mm.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*i).first ) ;
        calcmem( archive, *(const_cast<B*>((*i).second)) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  std::tr1::unordered_map<K,I,F>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template<typename K, typename I, typename F>
    inline void flatten( Flattenizable::Archive & archive, const boost::unordered_map<K,I,F> & hm )
    {
      size_t item_counter( 0 ) ;
      archive.append(hm.size() ) ;
      for( typename boost::unordered_map<K,I,F>::const_iterator i = hm.begin() ; i != hm.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*i).first ) ;
        flatten( archive, (*i).second ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template<typename K, typename I, typename F>
    inline void unflatten( Flattenizable::Archive & archive, boost::unordered_map<K,I,F> & hm )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      hm.clear() ;
      while( sz-- )
      {
        K key ;
        I info ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, key ) ;
        unflatten( archive, info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        hm.insert( typename boost::unordered_map<K,I,F>::value_type( key, info ) ) ;
      }
    }

    template<typename K, typename I, typename F>
    inline void calcmem( Flattenizable::Archive & archive, const boost::unordered_map<K,I,F> & hm )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename boost::unordered_map<K,I,F>::const_iterator i = hm.begin() ; i != hm.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*i).first ) ;
        calcmem( archive, (*i).second ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::tr1::unordered_multimap<const A,B*,C,D>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template<typename A, typename B, typename C, typename D>
    static void flatten( Flattenizable::Archive & archive, const boost::unordered_multimap<const A,B*,C,D> & mm )
    {
      size_t item_counter( 0 ) ;
      archive.append(mm.size() ) ;
      for( typename boost::unordered_multimap<const A,B*,C,D>::const_iterator i = mm.begin() ; i != mm.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*i).first ) ;
        flatten( archive, *(const_cast<B*>((*i).second)) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template<typename A, typename B, typename C, typename D>
    static void unflatten( Flattenizable::Archive & archive, boost::unordered_multimap<const A,B*,C,D> & mm )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      mm.clear() ;
      while( sz-- )
      {
        A key ;
        B * info( new B ) ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, key ) ;
        unflatten( archive, *info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        mm.insert( typename boost::unordered_multimap<const A,B*,C,D>::value_type( key, info ) ) ;
      }
    }

    template<typename A, typename B, typename C, typename D>
    static void calcmem( Flattenizable::Archive & archive, const boost::unordered_multimap<const A,B*,C,D> & mm )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename boost::unordered_multimap<const A,B*,C,D>::const_iterator i = mm.begin() ; i != mm.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*i).first ) ;
        calcmem( archive, *(const_cast<B*>((*i).second)) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::vector<I*>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename I>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<I*> & v )
    {
      size_t item_counter( 0 ) ;
      archive.append(v.size() ) ;
      for( typename std::vector<I*>::const_iterator i = v.begin() ; i != v.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, *(const_cast<I*>( *i )) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename I>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<I*> & v )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      v.clear() ;
      while( sz-- )
      {
        I * info( new I ) ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, *info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        v.push_back( info ) ;
      }
    }

    template <typename I>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<I*> & v )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::vector<I*>::const_iterator i = v.begin() ; i != v.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, *(const_cast<I*>( *i )) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::vector<const I*>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename I>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<const I*> & v )
    {
      size_t item_counter( 0 ) ;
      archive.append(v.size() ) ;
      for( typename std::vector<const I*>::const_iterator i = v.begin() ; i != v.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, *(const_cast<const I*>( *i )) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename I>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<const I*> & v )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      v.clear() ;
      v.reserve( sz ) ;
      while( sz-- )
      {
        I * info( new I ) ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, *info ) ;
        v.push_back( info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename I>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<const I*> & v )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::vector<const I*>::const_iterator i = v.begin() ; i != v.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, *(const_cast<const I*>( *i )) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::vector<std::shared_ptr<I>>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
  template <typename I>
  inline void
  flatten(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<I>>& v)
    {
      archive.append( v.size() ) ;
      for (typename std::vector<std::shared_ptr<I>>::const_iterator i = v.begin(); i != v.end();
           ++i)
      {
        flatten( archive, *( *i ) ) ;
      }
    }

    template <typename I>
    inline void
    unflatten(Flattenizable::Archive& archive, std::vector<std::shared_ptr<I>>& v)
    {
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      v.clear() ;
      v.reserve( sz ) ;
      while( sz-- )
      {
        I * info( new I ) ;
        unflatten( archive, *info ) ;
        v.push_back(std::shared_ptr<I>(info));
      }
    }

    template <typename I>
    inline void
    calcmem(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<I>>& v)
    {
      archive.addSize( sizeof( size_t ) ) ;
      for (typename std::vector<std::shared_ptr<I>>::const_iterator i = v.begin(); i != v.end();
           ++i)
      {
        calcmem( archive, *( *i ) ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::vector<T>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename T>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<T> & v )
    {
      size_t item_counter( 0 ) ;
      archive.append(v.size() ) ;
      for( typename std::vector<T>::const_iterator it = v.begin() ; it != v.end() ; ++it )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*it) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename T>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<T> & v )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      v.clear() ;
      v.reserve( sz ) ;
      while( sz-- )
      {
        T thing ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, thing ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        v.push_back( thing ) ;
      }
    }

    template <typename T>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<T> & v )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::vector<T>::const_iterator it = v.begin() ; it != v.end() ; ++it )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*it) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::set<T>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename T>
    inline void flatten( Flattenizable::Archive & archive, const std::set<T> & s )
    {
      size_t item_counter( 0 ) ;
      archive.append(s.size() ) ;
      for( typename std::set<T>::const_iterator it = s.begin() ; it != s.end() ; ++it )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*it) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename T>
    inline void unflatten( Flattenizable::Archive & archive, std::set<T> & s )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      s.clear() ;
      while( sz-- )
      {
        T thing ;
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, thing ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
        s.insert( thing ) ;
      }
    }

    template <typename T>
    inline void calcmem( Flattenizable::Archive & archive, const std::set<T> & s )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::set<T>::const_iterator it = s.begin() ; it != s.end() ; ++it )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*it) ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// std::map<A,B>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template<typename A, typename B>
    inline void flatten( Flattenizable::Archive & archive, const std::map<A,B> & m )
    {
      size_t item_counter( 0 ) ;
      archive.append(m.size() ) ;
      for( typename std::map<A,B>::const_iterator i = m.begin() ; i != m.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, (*i).first  ) ;
        flatten( archive, (*i).second ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template<typename A, typename B>
    inline void unflatten( Flattenizable::Archive & archive, std::map<A,B> & m )
    {
      size_t item_counter( 0 ) ;
      size_t sz( 0 ) ;
      archive.extract( sz ) ;
      m.clear() ;
      while( sz-- )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        A key ;
        unflatten( archive, key ) ;

        B & info = m[key] ;
        unflatten( archive, info ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template<typename A, typename B>
    inline void calcmem( Flattenizable::Archive & archive, const std::map<A,B> & m )
    {
      size_t item_counter( 0 ) ;
      archive.addSize( sizeof( size_t ) ) ;
      for( typename std::map<A,B>::const_iterator i = m.begin() ; i != m.end() ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, (*i).first  ) ;
        calcmem( archive, (*i).second ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// array of T
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename T, size_t n>
    inline void flatten( Flattenizable::Archive & archive, const T (&a)[n] )
    {
      size_t item_counter( 0 ) ;
      for( size_t i = 0 ; i < n ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        flatten( archive, a[i] ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename T, size_t n>
    inline void unflatten( Flattenizable::Archive & archive, T (&a)[n] )
    {
      size_t item_counter( 0 ) ;
      for( size_t i = 0 ; i < n ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        unflatten( archive, a[i] ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

    template <typename T, size_t n>
    inline void calcmem( Flattenizable::Archive & archive, const T (&a)[n] )
    {
      size_t item_counter( 0 ) ;
      for( size_t i = 0 ; i < n ; ++i )
      {
        FLATTENIZE_SUBITEM_BEGIN( archive ) ;
        FLATTENIZE_PUSH( archive, item_counter++ ) ;
        calcmem( archive, a[i] ) ;
        FLATTENIZE_POP( archive ) ;
        FLATTENIZE_SUBITEM_END( archive ) ;
      }
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HashKey<A,B,C,D,E,F,G,H,I,J>
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
    inline void flatten( Flattenizable::Archive & archive, const HashKey<A,B,C,D,E,F,G,H,I,J> & t )
    {
      flatten( archive, t.initialized ) ;
      flatten( archive, t._a ) ;
      flatten( archive, t._b ) ;
      flatten( archive, t._c ) ;
      flatten( archive, t._d ) ;
      flatten( archive, t._e ) ;
      flatten( archive, t._f ) ;
      flatten( archive, t._g ) ;
      flatten( archive, t._h ) ;
      flatten( archive, t._i ) ;
      flatten( archive, t._j ) ;
    }

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
    inline void unflatten( Flattenizable::Archive & archive, HashKey<A,B,C,D,E,F,G,H,I,J> & t )
    {
      unflatten( archive, t.initialized ) ;
      unflatten( archive, t._a ) ;
      unflatten( archive, t._b ) ;
      unflatten( archive, t._c ) ;
      unflatten( archive, t._d ) ;
      unflatten( archive, t._e ) ;
      unflatten( archive, t._f ) ;
      unflatten( archive, t._g ) ;
      unflatten( archive, t._h ) ;
      unflatten( archive, t._i ) ;
      unflatten( archive, t._j ) ;
    }

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
    inline void calcmem( Flattenizable::Archive & archive, const HashKey<A,B,C,D,E,F,G,H,I,J> & t )
    {
      calcmem( archive, t.initialized ) ;
      calcmem( archive, t._a ) ;
      calcmem( archive, t._b ) ;
      calcmem( archive, t._c ) ;
      calcmem( archive, t._d ) ;
      calcmem( archive, t._e ) ;
      calcmem( archive, t._f ) ;
      calcmem( archive, t._g ) ;
      calcmem( archive, t._h ) ;
      calcmem( archive, t._i ) ;
      calcmem( archive, t._j ) ;
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Things that have a flattenize method
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename F>
    inline void flatten( Flattenizable::Archive & archive, const F & f )
    {
      const_cast<F&>(f).flattenize( archive ) ;
    }

    template <typename F>
    inline void unflatten( Flattenizable::Archive & archive, F & f )
    {
      const_cast<F&>(f).flattenize( archive ) ;
    }

    template <typename F>
    inline void calcmem( Flattenizable::Archive & archive, const F & f )
    {
      const_cast<F&>(f).flattenize( archive ) ;
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Top level serialization calls
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename OBJ>
    inline void serialize( Flattenizable::Archive & archive
                         , const OBJ   &            obj
                         , size_t                   ldcMaxBlobSize
                         , const std::string &      name
                         , const std::string &      flatKey
                         )
    {
      archive.reset() ;
      archive.setAction( Flattenizable::CALCMEM ) ;
      process( archive, obj ) ;

      if ( ldcMaxBlobSize && ldcMaxBlobSize <= archive.size() )
      {
        std::ostringstream msg ;
        msg << "LDC skipping write of oversize object ["
            << name
            << "] key ["
            << flatKey
            << "] object size ["
            << archive.size()
            << "] greater than ldcMaxBlobSize ["
            << ldcMaxBlobSize
            << "]" ;
        archive.reset() ;
      }

      if ( archive.size() )
      {
        archive.allocate() ;
        archive.setAction( Flattenizable::FLATTEN ) ;
        process( archive, obj ) ;
      }
    }

    template <typename OBJ>
    inline void deserialize( Flattenizable::Archive & archive
                           , const OBJ & obj
                           , const char * externalBuffer
                           , size_t externalBufferSize
                           )
    {
      archive.setAction( Flattenizable::UNFLATTEN ) ;
      if( externalBuffer != NULL )
      {
        archive.useExternalBuffer( externalBuffer, externalBufferSize ) ;
      }
      archive.rewind() ;
      process( archive, obj ) ;
    }

  } // namespace flattenizer
} // namespace tse

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// process functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace tse
{
  namespace flattenizer
  {
    template <typename T>
    inline void process( Flattenizable::Archive & archive, const T & t )
    {
      switch( archive.action() )
      {
        case Flattenizable::FLATTEN :
          flatten( archive, t ) ;
          break ;

        case Flattenizable::UNFLATTEN :
          unflatten( archive, const_cast<T&>(t) ) ;
          break ;

        case Flattenizable::CALCMEM :
          calcmem( archive, t ) ;
          break ;
      }
    }

    template <typename T>
    inline void process( Flattenizable::Archive & archive, const T *& t )
    {
      process( archive, const_cast<T*&>( t ) ) ;
    }

    template <typename T>
    inline void process( Flattenizable::Archive & archive, T *& t )
    {
      switch( archive.action() )
      {
        case Flattenizable::FLATTEN :
          {
            size_t sz( 0 ) ;
            if( t != NULL )
            {
              Flattenizable::Archive temp ;
              temp.setAction( Flattenizable::CALCMEM ) ;
              calcmem( temp, *t ) ;
              sz = temp.size() ;
            }
            archive.append(sz ) ;
            if( sz > 0 )
            {
              flatten( archive, *t ) ;
            }
          }
          break ;

        case Flattenizable::UNFLATTEN :
          {
            size_t sz( 0 ) ;
            if( t != NULL )
            {
              delete t ;
              t = NULL ;
            }
            archive.extract( sz ) ;
            if( sz > 0 )
            {
              t = new T ;
              unflatten( archive, const_cast<T&>(*t) ) ;
            }
          }
          break ;

        case Flattenizable::CALCMEM :
          {
            archive.addSize( sizeof( size_t ) ) ;
            if( t != NULL )
            {
              calcmem( archive, *t ) ;
            }
          }
          break ;
      }
    }

    template <typename T>
    inline void process_enum( Flattenizable::Archive & archive, const T & t )
    {
      switch( archive.action() )
      {
        case Flattenizable::FLATTEN :
          {
            int temp( static_cast<int>( t ) ) ;
            flatten( archive, temp ) ;
          }
          break ;

        case Flattenizable::UNFLATTEN :
          {
            int temp( 0 ) ;
            unflatten( archive, const_cast<int&>(temp) ) ;
            (const_cast<T&>(t)) = static_cast<T>( temp ) ;
          }
          break ;

        case Flattenizable::CALCMEM :
          archive.addSize( static_cast<size_t>( sizeof( int ) ) ) ;
          break ;
      }
    }

    template <typename T>
    inline void process_enum_vector( Flattenizable::Archive & archive, const std::vector<T> & v )
    {
      switch( archive.action() )
      {
        case Flattenizable::FLATTEN :
          {
            archive.append(v.size() ) ;
            int temp( 0 ) ;
            for( typename std::vector<T>::const_iterator it = v.begin()
               ; it != v.end()
               ; ++it
               )
            {
              temp = static_cast<int>( (*it) ) ;
              flatten( archive, temp ) ;
            }
          }
          break ;

        case Flattenizable::UNFLATTEN :
          {
            size_t sz( 0 ) ;
            archive.extract( sz ) ;
            (const_cast< std::vector<T> & >( v )).clear() ;
            int temp( 0 ) ;
            while( sz-- )
            {
              unflatten( archive, temp ) ;
              (const_cast< std::vector<T> & >( v )).push_back( static_cast<T>( temp ) ) ;
            }
          }
          break ;

        case Flattenizable::CALCMEM :
          archive.addSize( sizeof( size_t ) ) ;
          archive.addSize( static_cast<size_t>( v.size() * sizeof( int ) ) ) ;
          break ;
      }
    }

    template <typename K, typename V>
    inline void process_enum_key_map( Flattenizable::Archive & archive, const std::map<K,V> & m )
    {
      switch( archive.action() )
      {
        case Flattenizable::FLATTEN :
          {
            archive.append(m.size() ) ;
            int tempKey( 0 ) ;
            V tempValue ;
            for( typename std::map<K,V>::const_iterator it = m.begin()
               ; it != m.end()
               ; ++it
               )
            {
              tempKey = static_cast<int>( (*it).first ) ;
              V tempValue = (*it).second ;
              flatten( archive, tempKey ) ;
              flatten( archive, tempValue ) ;
            }
          }
          break ;

        case Flattenizable::UNFLATTEN :
          {
            size_t sz( 0 ) ;
            archive.extract( sz ) ;
            (const_cast< std::map<K,V> & >( m )).clear() ;
            while( sz-- )
            {
              int key( 0 ) ;
              V value ;
              unflatten( archive, key ) ;
              unflatten( archive, value ) ;
              (const_cast< std::map<K,V> & >(m)).insert( typename std::map<K,V>::value_type( static_cast<K>(key), value ) ) ;
            }
          }
          break ;

        case Flattenizable::CALCMEM :
          archive.addSize( sizeof( size_t ) ) ;
          archive.addSize( static_cast<size_t>( m.size() * sizeof( int ) ) ) ;
          for( typename std::map<K,V>::const_iterator it = m.begin()
             ; it != m.end()
             ; ++it
             )
          {
            calcmem( archive, (*it).second ) ;
          }
          break ;
      }
    }

  } // namespace flattenizer
} // namespace tse
