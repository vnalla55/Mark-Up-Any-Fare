//----------------------------------------------------------------------------
//  File:         DCFactoryBase.h
//  Description:  Base class for the DCFactory which resides in Diagnostics.
//                This object exists so as to avoid a link time coupling.
//  Authors:      Mike Lillis
//  Created:      May 2005
//
//  Updates:
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Common/DCFactoryBase.h"

namespace tse
{
// NOTE:  This is the instance of the static data member for the
//        DCFactoryBase class.  The class is actually implemented in the
//        include file, but this has to appear in a cpp file so
//        that only one instance is created.

DCFactoryBase* DCFactoryBase::_baseInstance = nullptr;

DCFactoryBase*
DCFactoryBase::baseInstance()
{
  return _baseInstance;
}
};
