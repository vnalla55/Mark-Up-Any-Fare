//-------------------------------------------------------------------
//
//  File:        BaseTrx.h
//  Created:     October 2, 2009
//
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

#ifndef BaseTrx_H
#define BaseTrx_H

#include <boost/cstdint.hpp>

namespace tse
{
  class BaseTrx
  {
  public:
    BaseTrx ();
    virtual ~BaseTrx ();
    boost::int64_t getBaseIntId () const { return _baseIntId; }
    void setBaseIntId (boost::int64_t id) { _baseIntId = id; }
  protected:
  private:
    boost::int64_t _baseIntId;
    // not impelemented
    BaseTrx (const BaseTrx &);
    BaseTrx &operator = (const BaseTrx &);
  };

}// namespace tse
#endif// BaseTrx_H
