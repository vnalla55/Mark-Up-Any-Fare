//----------------------------------------------------------------------------
//
//  File:        PricingEnums.h
//  Created:     3/8/2005
//  Authors:     Mike Lillis
//
//  Description: Enums required for ATSE shopping/pricing.
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
#pragma once

namespace tse
{

/*---------------------------------------------------------------------------
 * Type of threading
 *-------------------------------------------------------------------------*/
enum ThreadingModel
{
  NotThreaded = 0, //  use the calling thread for each task
  Pooled = 1, //  use a thread pool to service a queue of tasks
  NotPooled = 2, //  use a thread for each task
  SFCThread = 3 // Use SFC threads (for timing tests)
};

enum FBR_PRIORITY
{
  FBR_PRIORITY_FBR_ONLY = 1,
  FBR_PRIORITY_MIXED = 2,
  FBR_PRIORITY_PUB_OR_CARRIER_ONLY = 3,
  FBR_PRIORITY_DEFAULT = 4
};

enum PRIORITY
{
  DEFAULT_PRIORITY = 1, // priority higher
  PRIORITY_LOW = 2 // priority low
};

} // end tse namespace

