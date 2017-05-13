//----------------------------------------------------------------------------
//  Description: Common ZLib compression functions
//
//  Copyright Sabre 2006
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

#include <string>
#include <vector>

namespace tse
{

class CompressUtil
{
public:
  /**
  *   @method compress
  *
  *   @Description Compresses a string and replaces it with the
  *                compressed data
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool compress(std::string& buff);

  /**
  *   @method compress
  *
  *   @Description Compresses an input string into
  *                an output string
  *
  *   @param input string
  *   @param output string
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool compress(const std::string& input, std::string& output);

  /**
  *   @method compress
  *
  *   @Description Compresses a char buffer
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool compress(std::vector<char>& buf);

  /**
  *   @method decompress
  *
  *   @Description Deompresses a string and replaces it with the
  *                decompressed data
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool decompress(std::string& buff);

  /**
  *   @method decompress
  *
  *   @Description Decompresses an input string into
  *                an output string
  *
  *   @param input string
  *   @param output string
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool decompress(const std::string& input, std::string& output);

  /**
  *   @method decompress
  *
  *   @Description Decompresses a char buffer
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool decompress(std::vector<char>& buf);

  /**
  *   @method compressBz2
  *
  *   @Description Compresses a string and replaces it with the
  *                compressed data using Bzip2
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool compressBz2(std::string& buf);

  /**
  *   @method compressBz2
  *
  *   @Description Compresses a char buffer using Bzip2
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool compressBz2(std::vector<char>& buf);

  /**
  *   @method decompressBz2
  *
  *   @Description Deompresses a string and replaces it with the
  *                decompressed data using Bzip2
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool decompressBz2(std::string& buf);

  /**
  *   @method decompressBz2
  *
  *   @Description Decompresses a char buffer using Bzip2
  *
  *   @param buffer
  *
  *   @return bool - true if successful, false otherwise
  */
  static bool decompressBz2(std::vector<char>& buf);

private:
  // Placed here so they wont be called
  //
  CompressUtil(const CompressUtil& rhs);
  CompressUtil& operator=(const CompressUtil& rhs);

  // translate Bzip2 compression status code to string
  static const char* xlateBz2StatusCode(int rcode);
};

} // end tse namespace

