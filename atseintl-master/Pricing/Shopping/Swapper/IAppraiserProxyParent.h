//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

namespace tse
{

namespace swp
{

// Enables score modifications from installed apraisers
template <typename AppraiserType>
class IAppraiserProxyParent
{
public:
  // Informs about a new score from specified appraiser
  // for specified item
  virtual void setScoreFromAppraiser(const AppraiserType* appraiser,
                                     const typename AppraiserType::Item& item,
                                     const typename AppraiserType::Score& score) = 0;

  virtual ~IAppraiserProxyParent() {}
};

} // namespace swp

} // namespace tse

