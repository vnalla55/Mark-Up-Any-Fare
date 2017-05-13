//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

namespace sfc
{
class CompressedData;
}

namespace tse
{

namespace CompressedDataImpl
{

sfc::CompressedData* compress(const char* input,
                              size_t inputSz);

bool uncompress(const sfc::CompressedData& compressedData,
                std::vector<char>& uncompressed);

}// CompressedDataImpl

}// tse
