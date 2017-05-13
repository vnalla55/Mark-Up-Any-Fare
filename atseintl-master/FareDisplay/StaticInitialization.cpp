/**
 * Static initializers related to MP should be placed in this file
 * in appropriate order, i.e. dependent definitions after those
 * on which they depend.
 */

#include "FareDisplay/LongMPChooser.h"
#include "FareDisplay/NoMarketMPChooser.h"
#include "FareDisplay/ShortMPChooser.h"

namespace tse
{
MPChooser::ChooserMap MPChooser::_chooserMap;

const bool ShortMPChooser::_registered =
    MPChooser::registerChooser(SHORT_MP, ShortMPChooser::proxy);

const bool LongMPChooser::_registered = MPChooser::registerChooser(LONG_MP, LongMPChooser::proxy);

const bool NoMarketMPChooser::_registered =
    MPChooser::registerChooser(NO_MARKET_MP, NoMarketMPChooser::proxy);
}
