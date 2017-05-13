//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

/**
 * \class SecurityHandshakeValidator
 *
 * \brief Utility method to check agency security handshake
 *
 * Created on: Monday Oct 12 12:07 2015
 */

#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{
class PricingTrx;

class SecurityHandshakeValidator
{
  public:
    /** Retrieves security info for a given SecurityHandshakeProductCode
     * from GETCUSTOMERSECURITYHANDSHAKE table
     * and compare against PCC for security match. Returns true if found.
     */
    static bool validateSecurityHandshake( PricingTrx& trx,
        const SecurityHandshakeProductCode& pc,
        const PseudoCityCode& pcc);
};

} // namespace tse
