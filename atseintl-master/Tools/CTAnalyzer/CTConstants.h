#ifndef CT_CONSTANTS_H
#define CT_CONSTANTS_H

namespace tse
{
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Constants
const std::string CTTCreator         = "creator:";
const std::string CTTVersion         = "version:";
const std::string CTTSummary         = "summary:";
const std::string CTBlank            = "blank";
const std::string CTSpace            = " ";
const std::string CTStartMethod      = "*";
const std::string CTContinueMethod   = ">";
const std::string CTColon            = ":";
const std::string CTCPlusPlusFile    = ".cpp";
const std::string CTCFile            = ".c";
const std::string CTBigCFile         = ".C";
const std::string CTBigHFile         = ".H";
const std::string CTHeaderFile       = ".h";
const std::string CTHPlusPlusFile    = ".hpp";
const std::string CTCxxFile          = ".cxx";
const std::string CTCCFile           = ".cc";
const std::string CTOpenParen        = "(";
const std::string CTCloseParen       = ")";
const std::string CTOpenBracket      = "[";
const std::string CTCloseBracket     = "]";
const std::string CTOpenSBrace       = "{";
const std::string CTCloseSBrace      = "}";
const std::string CTNoData           = "";

const uint32_t INNERBUFFER_SIZE = 2048;

//Header constants
const std::pair<uint32_t,uint32_t> CTHeaderI1Range = std::pair<uint32_t,uint32_t>(3,9);
const std::pair<uint32_t,uint32_t> CTHeaderD1Range = std::pair<uint32_t,uint32_t>(3,9);
const std::pair<uint32_t,uint32_t> CTHeaderL2Range = std::pair<uint32_t,uint32_t>(3,9);
const std::pair<uint32_t,uint32_t> CTHeaderTimeRange = std::pair<uint32_t,uint32_t>(4,5);
const std::pair<uint32_t,uint32_t> CTHeaderTriggerRange = std::pair<uint32_t,uint32_t>(2,3);
const std::pair<uint32_t,uint32_t> CTHeaderTargetDescriptionRange = std::pair<uint32_t,uint32_t>(3,11);
const uint32_t CTHeaderThresholdIndex = 2;
const uint32_t CTHeaderCycleCountIndex = 0;
const uint32_t CTHeaderInitialSkipSize = 3;
const uint32_t CTHeaderSecondSkipSize = 4;

//Method constants
const uint32_t CTMethodMinValidTokenSize = 8;
const uint32_t CTMethodCostIndex = 0;
const uint32_t CTMethodTypeIndex = 1;

//Command line constants
const uint32_t CTComLineMinimum         = 7;
const std::string CTComLineInput        = "-input";
const std::string CTComLineNumFunctions = "-numfunctions";
const std::string CTComLineOutputDir    = "-outputdir";

//Cycle constants
const uint64_t CTCycleEpsilon = static_cast<uint64_t>(20000); 


} //End namespace tse

#endif //CT_CONSTANTS_H
