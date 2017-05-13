#pragma once

const char * const _NA = "N/A";
const char * const _xmlErrorMsgs[] = { "no error"
                                       , "text element not closed:"
                                       , "incomplete xml:"
                                       , "white space not allowed:"
                                       , "empty element name:"
                                       , "no white space between attributes:"
                                       , "invalid attribute format:"
                                       , "\'=\' expected:"
                                       , "\" or \' expected:"
                                       , "CDATA not closed:"
                                       , "empty xml"
                                       , "garbage after the end"
                                       , "comment not closed:"
                                       , "no matching \';\' for \'&\' in encoding:"
                                       , "cannot translate encoding:"
                                       , "invalid xml name:"
                                       , "duplicate attribute name:"
                                       , "mismatched name in endElement:"
                                       , "not balanced start and end elements"
                                       , "no root element" };
enum IXMLERROR { XMLNOERROR
                 , TEXTELEMENTNOTCLOSED
                 , INCOMPLETEXML
                 , WHITESPACENOTALLOWED
                 , EMPTYELEMENTNAME
                 , NOWHITESPACEBETWEENATTRIBUTES
                 , INVALIDATTRIBUTEFORMAT
                 , EQUALSIGNEXPECTED
                 , QUOTEEXPECTED
                 , CDATANOTCLOSED
                 , EMPTYXML
                 , GARBAGEAFTERTHEEND
                 , COMMENTNOTCLOSED
                 , NOMATCHINGSEMICOLONFORAMPERSANDINENCODING
                 , CANNOTTRANSLATEENCODING
                 , INVALIDXMLNAME
                 , DUPLICATEATTRIBUTENAME
                 , MISMATCHEDNAMEINENDELEMENT
                 , NOTBALANCEDSTARTANDENDELEMENTS
                 , NOROOTELEMENT };
