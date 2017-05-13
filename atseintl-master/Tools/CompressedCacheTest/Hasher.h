//----------------------------------------------------------------------------
//
//  File:    Hasher.h
//
//  Description:
//     Helper class for calculating hash values.
//     Based on the hashpjw algorithm by PJ Weinberger
//     (see the dragon book for more details).
//
//  Copyright (c) Sabre 2004
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#ifndef HASHER_H
#define HASHER_H

#include <stdint.h>
#include <string>
#include <ctype.h>
//#include "CString.h"

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

namespace tse
{
class Hasher
{
private:

    uint32_t _h;
    uint16_t _method;

    void rehash(uint8_t c)
    {
	switch (_method)
	{
	  case 0:
	  {
               uint32_t p;
               _h = (_h << 4) + c;
               if ((p=_h&0xf0000000) != 0) {
                   _h ^= p>>24;
                   _h ^= p;
               }
	      break;
	  }

	  case 1:
	  {
             _h += c;
             _h += (_h<<10);
             _h ^= (_h>>6); 
	    break;
	  }

	  case 2:
	  {
             _h *= 0x01000193;
             _h ^= (uint32_t) c;
	    break;
	  }

	  case 3:
	  {
             const uint8_t ch = (toupper(c) - 0x1f) & 0x3f;
             uint32_t p=_h & 0xffffffe0;
             _h = (_h << 5) + ch;
             _h ^= p;
	    break;
	  }

	  case 4:
	  {
             uint32_t p;
             const uint8_t ch = (toupper(c) - 0x1f) & 0x3f;
             p=_h&0xf8000000;
             _h = (_h << 5) + ch;
             if (p != 0) 
             {
                 _h ^= p>>22;
             }
	    break;
	  }

	  case 5:
	  {
             uint32_t p;
             const uint8_t ch = (toupper(c) - 0x1f) & 0x3f;
             p=_h&0xe0000000;
             _h = (_h << 3) + ch;
             if (p != 0) 
             {
                _h ^= p>>26;
             }
	    break;
	  }

	  case 6:
	  {
             uint32_t p;
             const uint8_t ch = (toupper(c) - 0x20) & 0x3f;
             p=_h&0xf0000000;
             _h = (_h << 4) + ch;
             if (p != 0)
             {
                 _h ^= p>>24;
             }
	    break;
	  }

	  default:
	  {
               uint32_t p;
               _h = (_h << 4) + c;
               if ((p=_h&0xf0000000) != 0) {
                   _h ^= p>>24;
                   _h ^= p;
               }
	      break;
	  }
	}
    }


    void rehash(const void* p, size_t n)
    {
        const char* s = (const char*) p;
        for (size_t i=0; i < n; i++)
        {
            rehash(*s++);
        }
    }

    void superFastHash (const char * data, size_t len)
    {
        uint32_t tmp;
        int rem;

        rem = len & 3;
        len >>= 2;

        /* Main loop */
        for (;len > 0; len--) {
            _h  += get16bits (data);
            tmp    = (get16bits (data+2) << 11) ^ _h;
            _h   = (_h << 16) ^ tmp;
            data  += 2*sizeof (uint16_t);
            _h  += _h >> 11;
        }

        /* Handle end cases */
        switch (rem) {
            case 3: _h += get16bits (data);
                    _h ^= _h << 16;
                    _h ^= data[sizeof (uint16_t)] << 18;
                    _h += _h >> 11;
                    break;
            case 2: _h += get16bits (data);
                    _h ^= _h << 11;
                    _h += _h >> 17;
                    break;
            case 1: _h += *data;
                    _h ^= _h << 10;
                    _h += _h >> 1;
        }

        /* Force "avalanching" of final 127 bits */
        _h ^= _h << 3;
        _h += _h >> 5;
        _h ^= _h << 4;
        _h += _h >> 17;
        _h ^= _h << 25;
        _h += _h >> 6;
    }


public:

    Hasher()
    :   _h(0), _method(0)
    {
    }

    uint32_t hash()
    {
        return _h;
    }

    void setMethod(const uint16_t method)
    {
       if(method <= 7) 
       {
         _method = method; 
       }
    }

    friend Hasher&
    operator<< (Hasher& hasher, const uint8_t i)
    {
        hasher.rehash(i);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const uint16_t i)
    {
        hasher.rehash(&i, 2);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const uint32_t i)
    {
        hasher.rehash(&i, 4);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const uint64_t i)
    {
        hasher.rehash(&i, 8);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const int8_t i)
    {
        hasher.rehash(&i, 1);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const int16_t i)
    {
        hasher.rehash(&i, 2);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const int32_t i)
    {
        hasher.rehash(&i, 4);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const int64_t i)
    {
        hasher.rehash(&i, 8);
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const char* s)
    {
      if (hasher._method == 7)
      {
        hasher.superFastHash (s, strlen(s));
      }
      else
      {
        while (*s != 0)
        {
            hasher.rehash(*s++);
        }
      }
        return hasher;
    }

    friend Hasher&
    operator<< (Hasher& hasher, const std::string& s)
    {
        const char* p = s.c_str();
        if (hasher._method == 7)
        {
          hasher.superFastHash (p, strlen(p));
        }
        else
        {
          while (*p != 0)
          {
              hasher.rehash(*p++);
          }
	}
        return hasher;
    }

//     template<typename T>
//     friend Hasher&
//     operator<< (Hasher& hasher, const sfc::CString<T>& s)
//     {
//         hasher.rehash(s.value());
//         return hasher;
//     }

};
}// tse
#endif // HASHER_H
