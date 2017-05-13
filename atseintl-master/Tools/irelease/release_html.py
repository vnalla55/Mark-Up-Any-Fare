#! /usr/bin/env python2.5
import re
import cgi
from HTMLParser import HTMLParser

class MLStripper(HTMLParser):
    def __init__(self):
        self.reset()
        self.fed = []
    def handle_data(self, d):
        self.fed.append(d)
    def get_data(self):
        return ''.join(self.fed)

def strip_tags(html):
    s = MLStripper()
    s.feed(html)
    return s.get_data()


'''
    Generates HTML document for given parsed release notes document (group tree).
    Needs configuration for choosing relevant colors and group visibility.
'''
class HtmlOutputProcessor:
    def _getGroupColor(self, group, configuration):
        if group in configuration.groups:
            return configuration.groups[group].color
        else:
            return r'#d1e4fa' #default color
        
    def _isGroupVisible(self, group, configuration):
        if group in configuration.groups:
            return configuration.groups[group].visibility
        else:
            return True

    '''
        groupTree - parsed document (group tree)
        configuration - Config instance
        errors - list of errors to be written at the bottom of the document.
    '''
    def decorate(self, groupTree, configuration, errors):
        # document header
        header = ""
        if errors:
            header = "Validation errors occured!"

        # all HTML stuff is enclosed in HTMLDocumentBuilder class
        document = HTMLDocumentBuilder()
        document.openDocument(configuration.bgcolor, header)

        for group in groupTree:
            # group can be hidden with proper configuration
            if self._isGroupVisible(group, configuration):
                color = self._getGroupColor(group, configuration)
                document.startGroup(group, color)

                for item in groupTree[group]:
                    document.addItem(item)

                document.endGroup()

        document.addErrors(errors)

        document.closeDocument()
        return document.getDocument()

'''
Builder for HTML document.
All HTML markups are enclosed in this class.

HTML markups from items are generally escaped, but you can some of allowed markups:
    * <br />, <br>                              - new line enforcement
    * <b> TEXT </b>                             - bold
    * <i> TEXT </i>                             - italics
    * <u> TEXT </u>                             - underline
    * <code> TEXT </code>                       - monospaced font (like source code)
    * <color=yellow> TEXT </color>              - text color
      <color=#fa893a> TEXT </color>
    * <bgcolor=yellow> TEXT </color>            - text background color
      <bgcolor=#fa893a> TEXT </color>
    * <a>http://google.com/ </a>                - hyperlink
      <a href=http://google.com/> TEXT </a>
'''
class HTMLDocumentBuilder():
    def __init__(self):
        self._document = ""
        self._currentGroupColor = ""
        self._fieldsInSummary = ['Title', 'Developer']
        self._index = ""
        self._anchorCounter = 0

    def openDocument(self, bgcolor, header):
        self._header = str(r"""
            <html>  
                <head>
                    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
                    <style type="text/css">
                        body {background-color: """ + bgcolor + """}
                        .group_header { text-align: justify; color: navy;font: bold 18px verdana, arial, helvetica, sans-serif; }
                        .rd { width:71.05pt;border:solid orange 1.0pt; padding: 1.5pt; font-family: sans-serif; background: #ff9; text-align: right; font-weight: bold; } 
                        .rc { width:549.0pt;border:solid orange 1.0pt; border-left:none;padding: 1.5pt; background: #ffb}                        
                        table { max-width: 800px; margin-top:.5pt;margin-left:.5pt;border-collapse:collapse }
                        .c { color: black;font: normal 11px verdana, arial, helvetica, sans-serif; }
                        .errorlist p { font-size:15.0pt;font-family:Arial;color:red; font-weight: bold;}

                        .index_group {font-size: 8pt; font-family: sans-serif; margin-bottom: 5px; padding:0; font-weight: bold; }
                        .index_link {font-size: 8pt; font-family: sans-serif; }
                        .index_link a { font-size: 8pt; font-family: sans-serif; text-decoration: none; color: navy; }
                    </style>
                </head>
                <body lang=EN-US link=blue vlink=purple>
                    <div class=Section1>
                        <p>
                            <span style="font-size:20.0pt;font-family:Arial;color:red; font-weight: bold;">
                                <o:p>""" + header + """&nbsp;</o:p>
                            </span>
                        </p>
                    </div>

        """)

    def closeDocument(self):
        self._footer = str(r'</body></html>')

    def getDocument(self):
        return self._header + self._index + self._document + self._footer

    def addItem(self , item):
        #specific for html: '\n' => <br /> and escaping HTML
        def convertContentEndlineChars(content): 
            return "".join(
                        map(lambda line: cgi.escape(line.strip()) + '<br />', content.splitlines())
                    )
    
        def decorateHTML(line):
            # for prettier checkboxes
            line = re.sub(cgi.escape(r"\[(X|x| )\]"), r"<code>[\g<1>]</code>", line)

            # BB Code
            line = re.sub(cgi.escape(r"<br\s?/?>"), r"<br />", line)
            line = re.sub(cgi.escape(r"<b>(.*?)</b>"), r"<b>\g<1></b>", line)
            line = re.sub(cgi.escape(r"<i>(.*?)</i>"), r"<i>\g<1></i>", line)
            line = re.sub(cgi.escape(r"<u>(.*?)</u>"), r"<u>\g<1></u>", line)
            line = re.sub(cgi.escape(r"<code>(.*?)</code>"), r"<code>\g<1></code>", line)
            line = re.sub(cgi.escape(r"<color=(\w+|#[0-9a-fA-F]{3,6})>(.*?)</color>"), r'<span style="color: \g<1>;">\g<2></span>', line)
            line = re.sub(cgi.escape(r"<bgcolor=(\w+|#[0-9a-fA-F]{3,6})>(.*?)</bgcolor>"), r'<span style="background-color: \g<1>;">\g<2></span>', line)
            line = re.sub(cgi.escape(r"<a>(.*?)</a>"), r'<a href="\g<1>">\g<1></a>', line)
            line = re.sub(cgi.escape(r"<a href=(.*?)>(.*?)</a>"), r'<a href="\g<1>">\g<2></a>', line)

            return line

        '''
        Converts one release note table row to HTML.
        '''
        def itemRowToHTML(itemRow):
            return str(r"""<tr>
                    <td width=95 valign=top class='rd'>
                        <p class='c'>{0}</p>
                    </td>
                    <td width=732 valign=top class='rc'>
                        <p class='c'>{1}</p>
                    </td>
                </tr>""".format(itemRow[0], decorateHTML(convertContentEndlineChars(itemRow[1]))))

        '''
        Split item for rows.
        Rows ends when (optional) indented lines are finished.
        '''
        def rowsFromItem(item):
            rows = []
            for line in item.splitlines(True):
                # if starting from whitespace character
                if re.match('^\s.*', line): 
                    if rows:
                        rows[-1] = (rows[-1][0], rows[-1][1] + line)    # only append next line
                    else:   # if no unindented line before (should not happen ordinarily)
                        rows.append(('', line))
                else:
                    # split line with semicolon
                    match = re.match('^([^:]*):(.*)', line)
                    if match:
                        rows.append((match.groups()[0], match.groups()[1]))
                    else:
                        rows.append((line, ""))

            return rows

        def itemRowToIndexHTML(item):
            """Converts a release note table row to HTML for note's index"""
            return "{0}".format(strip_tags(item[1]))

        self._anchorCounter += 1

        indexLinkText = ' - '.join(map(itemRowToIndexHTML, filter(lambda itemRow: itemRow[0] in self._fieldsInSummary,
                                                            rowsFromItem(item))))

        self._index += """<li class="index_link"><a href="#delivery_{0}">{1}</a></li>""".format(self._anchorCounter,  indexLinkText)
        

        # convert item to HTML
        htmlItems = "".join(map(itemRowToHTML, rowsFromItem(item)))
       
        # insert item into table
        self._document += str(r"""
            <a name="delivery_{0}"></a>
            <table border=0 cellspacing=0 cellpadding=0 style='background-color:{0}; margin-left:.5pt;border-collapse:collapse'>
                {2}
            </table>&nbsp
        """.format(self._anchorCounter, self._currentGroupColor, htmlItems))

    '''
    Append errors list at the bottom of document.
    '''
    def addErrors(self, errors):
        errorsHTML = ""

        for error in errors:
            errorsHTML += """<li> {0} </li> """.format(error)

        if errors:
            self._document += """<div class="errorlist"><p>Please refine release note.</p><p>Error list: </p><ol> {0} </ol></div>""" .format(errorsHTML)

    '''
    Use this when new group is starting.
    '''
    def startGroup(self, groupname, color):
        self._currentGroupColor = color
        self._document += str(r"<div class='group_header'><p> {0} ".format(groupname))
        self._index += r"<p class='index_group'>{0}</p>".format(groupname)
        
    '''
    Use this always after adding all group items.
    '''
    def endGroup(self):
        self._document += str(r"</p></div>")

            
    
