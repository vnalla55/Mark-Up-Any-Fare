//----------------------------------------------------------------------------
//   Copyright 2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include <cstdio>
#include "DateTime.h"

#include "Code.h"

//#define _DEBUGFLATKEY

namespace tse
{
  const int _FLATKEYMAXSZ(256);

  class KeyStream
  {
  public:
    explicit KeyStream (int)
      : _pointer(0)
    {
      _buffer[0] = '\0';
    }
    KeyStream (const std::string &name,
               int version)
      : _pointer(0)
    {
      char ch(0);
      const char *pchar = name.c_str();
      while ((ch = *pchar++) != 0)
      {
        _buffer[_pointer++] = ch;
      }
      _buffer[_pointer++] = '.';
      char versionStr[20] = "";
      sprintf(versionStr, "%d", version);
      pchar = versionStr;
      while ((ch = *pchar++) != 0)
      {
        _buffer[_pointer++] = ch;
      }
      _buffer[_pointer++] = '/';
    }
    int size () const
    {
      return _pointer;
    }
    operator const char * () const
    {
      return _buffer;
    }

    friend inline void copy( const char* pchar, KeyStream& stream )
    {
      char ch ;
      while ((ch = *pchar++) != 0)
      {
        stream._buffer[stream._pointer++] = ch;
      }
      stream._buffer[stream._pointer++] = '|';
      stream._buffer[stream._pointer] = 0 ;
    }

    friend inline bool elementToFlatKey (int element,
                                         KeyStream &stream)
    {
      char intStr[20] = "";
      sprintf(intStr, "%d", element);
      copy( intStr, stream ) ;
      return true;
    }
    friend inline bool elementToFlatKey (unsigned long element,
                                         KeyStream &stream)
    {
      char intStr[20] = "";
      sprintf(intStr, "%lu", element);
      copy( intStr, stream ) ;
      return true;
    }
    friend inline bool elementToFlatKey (char element,
                                         KeyStream &stream)
    {
      stream._buffer[stream._pointer++] = element;
      stream._buffer[stream._pointer++] = '|';
      return true;
    }

    friend inline bool elementToFlatKey (bool element,
                                         KeyStream &stream)
    {
      char ch(element ? '1' : '0');
      stream._buffer[stream._pointer++] = ch;
      stream._buffer[stream._pointer++] = '|';
      return true;
    }
    friend inline bool elementToFlatKey (long long element,
                                         KeyStream &stream)
    {
      char longLongStr[20] = "";
      sprintf(longLongStr, "%Ld", element);
      copy( longLongStr, stream ) ;
      return true;
    }

    friend inline bool elementToFlatKey (const DateTime &element, KeyStream &stream)
    {
#if 0 // LDC_DEBUG_DATETIME
      if ( element.isOpenDate() )
      {
        std::cout << "<Open Date[" << element << "]" << std::endl ;
        copy( "*open date*", stream ) ;
      }
      else if ( element.isInfinity() )
      {
        std::cout << "<Infinity Date[" << element << "]" << std::endl ;
        copy( "*infinity date*", stream ) ;
      }
      else if ( element.isEmptyDate() )
      {
        std::cout << "<Empty Date[" << element << "]" << std::endl ;
        copy( "*empty date*", stream ) ;
      }
      else
#endif
      {

#if 1
        /// 12345678901234567890
        /// 2010-Dec-31^23:59:59
        char buffer[ 32 ] ;
        memset( buffer, 0, sizeof( buffer ) ) ;
      std::ostringstream os;
        os.rdbuf()->pubsetbuf( buffer, sizeof( buffer ) ) ;
      os << element;
        copy( os.str().c_str(), stream ) ;
#else // not LDC_USE_READABLE_DATETIME_STRING [julian]
      ////
      //char longLongStr[20] = "";
      //sprintf(longLongStr, "%ll", element.get64BitRep());
      ////
        char string[22] ;
        memset( string, 0, sizeof( string ) ) ;
        snprintf(string, sizeof( string ), "%ld", element.get64BitRep() );
        copy( string, stream ) ;
#endif
      }
      return true;
    }
    friend inline bool elementToFlatKey (const char *element,
                                         KeyStream &stream)
    {
      if (element)
      {
        copy( element, stream ) ;
      }
      return true;
    }
    // stream._pointer == string length
    friend inline void endKey (KeyStream &stream)
    {
      // replace characters
      // use the same replacements as in REPLACE_CHAR_WITH_CHAR (HashKey.h)
      for (int i = 0; i < stream._pointer; ++i)
      {
        char &ch(stream._buffer[i]);
        switch (ch)
        {
        case ' ':
          ch = '^';
          break;
        case '\0':
          ch = '~';
          break;
        default:
          break;
        }
      }
      if ('|' == stream._buffer[stream._pointer - 1])
      {
        stream._buffer[--stream._pointer] = '\0';
      }
      else
      {
        stream._buffer[stream._pointer] = '\0';
      }
    }
  private:
    int _pointer;
    char _buffer[_FLATKEYMAXSZ];
    // not implemented
    KeyStream (const KeyStream &);
    KeyStream &operator = (const KeyStream &);
  };

  inline bool elementToFlatKey (const struct nil_t &,
                                KeyStream &)
  {
    return false;
  }
  inline bool elementToFlatKey (const std::string &element,
                                KeyStream &stream)
  {
    return elementToFlatKey(element.c_str(), stream);
  }
  template<size_t n, typename T>
    bool elementToFlatKey (const Code<n, T> &element,
                           KeyStream &stream)
  {
    return elementToFlatKey(element.c_str(), stream);
  }
  inline KeyStream &operator << (KeyStream &stream,
                                 int key)
  {
    elementToFlatKey(key, stream);
    endKey(stream);
    return stream;
  }
  inline KeyStream &operator << (KeyStream &stream,
                                 const std::string &key)
  {
    elementToFlatKey(key.c_str(), stream);
    endKey(stream);
    return stream;
  }
  template<size_t n, typename T>
    KeyStream &operator << (KeyStream &stream,
                            const Code<n, T> &key)
  {
    elementToFlatKey(key.c_str(), stream);
    endKey(stream);
    return stream;
  }
}// tse
