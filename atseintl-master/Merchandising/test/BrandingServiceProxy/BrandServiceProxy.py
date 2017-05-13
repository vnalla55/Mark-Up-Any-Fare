#!/usr/bin/env python

import argparse
import functools
import imp
import os
import re
import SocketServer
import sys
import time
import urlparse
import httplib
import xml.etree.cElementTree as ET

from cgi import escape as html_escape
from ConfigParser import SafeConfigParser
from contextlib import closing
from collections import namedtuple
from io import BytesIO
from SimpleHTTPServer import SimpleHTTPRequestHandler
from socket import gethostname
from threading import Event
from threading import Lock
from traceback import format_exc


#   _____                _              _
#  / ____|              | |            | |
# | |     ___  _ __  ___| |_ __ _ _ __ | |_ ___
# | |    / _ \| '_ \/ __| __/ _` | '_ \| __/ __|
# | |___| (_) | | | \__ \ || (_| | | | | |_\__ \
#  \_____\___/|_| |_|___/\__\__,_|_| |_|\__|___/
#
INT_URL = 'http://int.brandedfaresSWS.sabre.com:8280/BrandedFaresService/getBrands'
XMLNS = 'http://stl.sabre.com/Merchandising/v1'
XMLNS_ANCS = 'http://stl.sabre.com/Merchandising/v0'


#  _    _      _                    __                  _   _
# | |  | |    | |                  / _|                | | (_)
# | |__| | ___| |_ __   ___ _ __  | |_ _   _ _ __   ___| |_ _  ___  _ __  ___
# |  __  |/ _ \ | '_ \ / _ \ '__| |  _| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
# | |  | |  __/ | |_) |  __/ |    | | | |_| | | | | (__| |_| | (_) | | | \__ \
# |_|  |_|\___|_| .__/ \___|_|    |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
#               | |
#               |_|
def add_xmlns(tag):
    """Add Branding Service namespace to the tag"""
    return '{' + XMLNS + '}' + tag


def anywhere(tag):
    """Return XPath to a descendant in Branding Service namespace."""
    return './/' + add_xmlns(tag)


def find_or_create(parent, element_name, clear=False):
    """ Returns an element called element_name under parent.
        Clears it if clear=True.
        If an element with name element_name is not found
        under parent - a new element is created and returned """
    element = parent.find(add_xmlns(element_name))
    if element is None:
        return ET.SubElement(parent, add_xmlns(element_name))
    else:
        if clear:
            element.clear()
        return element


#  _____  _             _           _       _             __
# |  __ \| |           (_)         (_)     | |           / _|
# | |__) | |_   _  __ _ _ _ __      _ _ __ | |_ ___ _ __| |_ __ _  ___ ___
# |  ___/| | | | |/ _` | | '_ \    | | '_ \| __/ _ \ '__|  _/ _` |/ __/ _ \
# | |    | | |_| | (_| | | | | |   | | | | | ||  __/ |  | || (_| | (_|  __/
# |_|    |_|\__,_|\__, |_|_| |_|   |_|_| |_|\__\___|_|  |_| \__,_|\___\___|
#                  __/ |
#                 |___/
class XMLTransformerPlugin:
    """ Interface for the XML transformation plugins to implement
        Each plugin should implement plugin_init() function which
        should parse the config and call add_transformation()
        one or more times to register the transformation functions.
        They will be called each time when __call__ is called. """

    def plugin_init(self, config):
        """ Initializes the plugin.
        The plugin should parse the configuration, register all
        the transformation functions or raise an exception """
        raise NotImplementedError('XML Transformer Plugin should implement'
                                  ' config_init(config) function!')

    def default_config(self):
        """ Returns a string with the default configuration for this plugin.
            Each configuration element should be properly documented """
        raise NotImplementedError('XML Transformer Plugin should implement'
                                  ' default_config() function!')

    def __init__(self, config):
        self.transformations = []
        self.plugin_init(config)

    def __call__(self, xml, con_log):
        """ Main function of the plugin.
        Calls all the transformation functions in a sequence
        This function should NOT be overriden"""
        for f, a, k in self.transformations:
            xml = f(xml, con_log, *a, **k)
            yield xml

    def add_transformation(self, func, *args, **kwargs):
        """ Adds a transformation function to the chain
        of functions to be called for each xml.
        This function should NOT be overriden."""
        self.transformations.append((func, args, kwargs))


#  _____  _             _           _                     _ _ _
# |  __ \| |           (_)         | |                   | | (_)
# | |__) | |_   _  __ _ _ _ __     | |__   __ _ _ __   __| | |_ _ __   __ _
# |  ___/| | | | |/ _` | | '_ \    | '_ \ / _` | '_ \ / _` | | | '_ \ / _` |
# | |    | | |_| | (_| | | | | |   | | | | (_| | | | | (_| | | | | | | (_| |
# |_|    |_|\__,_|\__, |_|_| |_|   |_| |_|\__,_|_| |_|\__,_|_|_|_| |_|\__, |
#                  __/ |                                               __/ |
#                 |___/                                               |___/
#
PluginPath = namedtuple('PluginPath', 'name, path')
Plugin = namedtuple('Plugin', 'name, instance')


class PluginLoadException(Exception):
    def __init__(self, message, exception=None):
        self.sub_exception = exception
        self.plugin_stacktrace = format_exc()
        super(Exception, self).__init__(message)


def load_plugins(directory, config=None):
    """ Loads all *_bpplg.py files as plugins from given directory """
    # Get the list of available plugins
    plugins = []
    try:
        for fileName in os.listdir(directory):
            match = re.match(r'^(?P<name>.+)_bspplg.py$', fileName)
            if match:
                plugin = PluginPath(match.group('name'),
                                    os.path.join(directory, fileName))
                plugins.append(plugin)
    except EnvironmentError as e:
        raise PluginLoadException("Can't list plugin directory " + directory, e)
    # Try to import the plugins
    imported_plugins = []
    for plugin in plugins:
        try:
            if config and not config.has_section(plugin.name):
                # Skip plugins without configuration
                continue

            if config and config.has_section(plugin.name):
                # Plugin has the configuration defined
                plugin_config = config.items(plugin.name)
            else:
                # No configuration defined at all - use empty config
                plugin_config = []

            print 'Loading plugin "{:s}"'.format(plugin.name)
            plugin_module = imp.load_source(plugin.name, plugin.path)
            plugin_class = getattr(plugin_module, plugin.name)
            plugin_instance = plugin_class(plugin_config)
            imported_plugins.append(Plugin(plugin.path, plugin_instance))
        except Exception as e:
            raise PluginLoadException('Can\'t load plugin "{:s}"'.format(plugin.path), e)
    return imported_plugins


#  _                       _
# | |                     (_)
# | |     ___   __ _  __ _ _ _ __   __ _
# | |    / _ \ / _` |/ _` | | '_ \ / _` |
# | |___| (_) | (_| | (_| | | | | | (_| |
# |______\___/ \__, |\__, |_|_| |_|\__, |
#               __/ | __/ |         __/ |
#              |___/ |___/         |___/
class LogCategory:
    """ Stores the data about a Log Category, is it enabled, its description
    and attributes for HTML generation """
    def __init__(self, color, is_enabled=False, description=None):
        self.color = color
        self.is_enabled = is_enabled
        self.description = description


class LogRecord:
    """ Stores one log record and is able to generate encode it to HTML """
    def __init__(self, log_category, text, long_text=None):
        self.time = time.time()
        self.log_category = log_category
        self.text = text
        self.long_text = long_text

    def to_html(self, out=None):
        if not out:
            out = ""
        if self.long_text:
            out += self.HTML_LONGLOG.format(color=self.log_category.color,
                                            text=html_escape(self.text),
                                            long_text=html_escape(self.long_text))
        else:
            out += self.HTML_LOG.format(color=self.log_category.color,
                                        text=html_escape(self.text))
        return out

    HTML_LOG = '<div style="background-color: {color:s}; width: 100%; text-align: left;"><pre>{text:s}</pre></div>'

    HTML_LONGLOG = """<div style="background-color: {color:s}; width: 100%">
<pre>{text:s}</pre>
<textarea rows=1 style="width: 100%">
{long_text:s}
</textarea></div>"""


class ClientLog:
    """ Stores LogRecords for one client """
    def __init__(self, log):
        self.global_log = log
        self.conn_cnt = 0
        self.log = []

    def new_connection(self, port):
        self.conn_cnt += 1
        self(log_category='conn',
             text='New connection from port {:d}'.format(port))

    def __call__(self, log_category, text, long_text=None):
        if log_category not in self.global_log.lc:
            raise Exception("Undefined log category: " + repr(log_category))
        log_category = self.global_log.lc[log_category]
        if log_category.is_enabled:
            self.log.append(LogRecord(log_category=log_category,
                                      text=text,
                                      long_text=long_text))

    def clear_log(self):
        self.log = []


class Log:
    """ Contains the logs and statistics of client connections and converts
     the gathered data to HTML """
    def __init__(self):
        self.__clients = dict()
        self.lc = dict(err=LogCategory(color='red', is_enabled=True,
                                       description='Error messages'),
                       conn=LogCategory(color='blue', is_enabled=True,
                                        description='Information about the client connections'),
                       http=LogCategory(color='white', is_enabled=False,
                                        description='HTTP info'),
                       req=LogCategory(color='white', is_enabled=False,
                                       description='Request data'),
                       resp=LogCategory(color='white', is_enabled=False,
                                        description='Response data'),
                       resp_int=LogCategory(color='white', is_enabled=False,
                                            description='Response, being modified by plugins'),
                       out=LogCategory(color='out', is_enabled=False,
                                       description='Final modified response sent back to client'),
                       plg=LogCategory(color='grey', is_enabled=False,
                                       description='Plugin calling sequence'),
                       plg_out=LogCategory(color='white', is_enabled=False,
                                           description='Plugin output'),
                       cfg=LogCategory(color='yellow', is_enabled=False,
                                       description='Configuration update'),
                       success=LogCategory(color='green', is_enabled=True))

    def for_client(self, client_address):
        """ Creates a new client statistics object or
        returns the existing one if it exists"""
        return self.__clients.setdefault(client_address, ClientLog(self))

    def __str__(self):
        """ Returns an HTML string ready to be sent back to the browser """
        s = self.HTML_TOP.format(title=gethostname(),
                                 num_entries=self.num_entries())
        for addr, client in self.__clients.iteritems():
            s += self.HTML_CLIENT_START.format(addr=addr,
                                               conn_cnt=client.conn_cnt)
            for log_row in client.log:
                timestamp = time.strftime('%F %H:%M:%S',
                                          time.localtime(log_row.time))
                s += self.HTML_LOG_ROW.format(time=timestamp,
                                              text=log_row.to_html())
            s += self.HTML_CLIENT_END
        s += self.HTML_BOTTOM
        return s

    def num_entries(self):
        num = 0
        for client in self.__clients.itervalues():
            num += len(client.log)
        return num

    HTML_TOP = """<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>BrandServiceProxy@{title:s}</title>
<script>
    function getNumEntries() {{
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.open( "GET", "/num_entries", false );
        xmlHttp.send( null );
        return parseInt(xmlHttp.responseText);
    }}

    function refresh() {{
        try {{ if (getNumEntries() != {num_entries:d}) {{
            window.location.reload(true);
        }} }} catch(err) {{ }}
        setTimeout(refresh, 1000);
    }}
    setTimeout(refresh, 1000);
</script>
<style>
* {{
    margin: 0;
    padding: 0;
}}
div {{
    width: 100%;
}}
</style>
</head>
<body onload="window.scrollTo(0,document.body.scrollHeight);">
<table style="text-align: center; width: 100%">
<tr>
    <td style="width: 0%"><b>Client</b></td>
    <td style="width: 0%"><b>Requests</b></td>
    <td style="width: 100%"><b>Log</b></td>
</tr>
"""

    HTML_CLIENT_START = """<tr>
<td><pre>{addr:s}</pre></td>
<td><pre>{conn_cnt:d}</pre></td>
<td><table style="width: 100%">
"""

    HTML_CLIENT_END = '</table></td></tr>'

    HTML_LOG_ROW = """<tr>
    <td style="width: 0%"><pre>{time:s}</pre></td>
    <td style="width: 100%">{text:s}</td>
</tr>
"""

    HTML_BOTTOM = '</table>\n</body>\n</html>'


#  _    _ _______ _______ _____      _                     _ _ _
# | |  | |__   __|__   __|  __ \    | |                   | | (_)
# | |__| |  | |     | |  | |__) |   | |__   __ _ _ __   __| | |_ _ __   __ _
# |  __  |  | |     | |  |  ___/    | '_ \ / _` | '_ \ / _` | | | '_ \ / _` |
# | |  | |  | |     | |  | |        | | | | (_| | | | | (_| | | | | | | (_| |
# |_|  |_|  |_|     |_|  |_|        |_| |_|\__,_|_| |_|\__,_|_|_|_| |_|\__, |
#                                                                       __/ |
#                                                                      |___/
class SharedData:
    """ Contains the data shared between different HTTP connection handlers """
    def __init__(self):
        self.__lock = Lock()
        self.__plugin_dir = None
        self.__plugins = None
        self.__config = None
        self.events = SharedEvents()

    def set(self, plugin_dir=None, config=None, plugins=None):
        with self.__lock:
            if plugin_dir is not None:
                self.__plugin_dir = plugin_dir
            if config is not None:
                self.__config = config
            if plugins is not None:
                self.__plugins = plugins

    def plugin_dir(self):
        with self.__lock:
            plugin_dir = self.__plugin_dir
        return plugin_dir

    def config(self):
        with self.__lock:
            config = self.__config
        return config

    def plugins(self):
        with self.__lock:
            plugins = self.__plugins
        return plugins


class SharedEvents:
    """ Contains the events shared between different HTTP connection handlers """
    def __init__(self):
        self.__event_request_processed = Event()

    def request_processed(self):
        self.__event_request_processed.set()

    def wait_request_processed(self, timeout=None):
        self.__event_request_processed.clear()
        self.__event_request_processed.wait(timeout)


class BrandingServiceProxy(SimpleHTTPRequestHandler):
    def __init__(self, request, client_address, server, cli_args, log, shared_data):
        self._cli_args = cli_args
        self._log = log
        self._shared_data = shared_data
        # We need to call base ctor last because it never returns.
        SimpleHTTPRequestHandler.__init__(self, request,
                                          client_address,
                                          server)

    def do_HEAD(self):
        self.send_error(405)

    def do_GET(self):
        """ Handle HTTP GET request:
        /num_entries - returns the number of entries in the log
        /config - returns the current configuration as a string
        /wait_request - locks until the proxy processes a request"""
        if self.path == '/num_entries':
            self.send_response(200)
            self.end_headers()
            self.wfile.write(self._log.num_entries())
        elif self.path == '/config':
            self.send_response(200)
            self.end_headers()
            self._shared_data.config().write(self.wfile)
        elif self.path == '/wait_request':
            self._shared_data.events.wait_request_processed()
            self.send_response(200)
            self.end_headers()
        else:
            self.send_response(200)
            self.end_headers()
            self.wfile.write(self._log)

    def do_POST(self):
        """ Handle HTTP POST request:
        /config - set the configuration
        * - Send the request to the Brand Service, run the response through
            the chain of plugins according to the current configuration,
            send the modified response back"""
        connection_log = self._log.for_client(self.client_address[0])
        connection_log.new_connection(port=self.client_address[1])
        connection_log(log_category='http',
                       text='HTTP request headers',
                       long_text=str(self.headers))
        try:
            contentlen = int(self.headers.get('content-length') or 0)
            if contentlen:
                request = self.rfile.read(contentlen)
            else:
                request = ''
            connection_log(log_category='req',
                           text='Request[{:d}:{:d}]:'.format(contentlen,
                                                             len(request)),
                           long_text=request)
            # Check what is requested
            if self.path == '/config':
                # Dynamic confguration request was received
                self.process_dynamic_config_request(request, connection_log)
            else:
                # Send all the other requests to the branding service
                self.process_brand_service_request(request, connection_log)
        except Exception as e:
            connection_log(log_category='err',
                           text=str(e),
                           long_text=format_exc())

    def process_brand_service_request(self, request, connection_log):
        """ Processes the request to Brand Service - sends the request to the actual
        Brand Service, processes the response using all the enabled plugins ans
        sends the modified response back to the client """
        try:
            self.send_response(200)
            self.end_headers()
            response = self.query_branding_service(request)
            connection_log(log_category='resp',
                           text='Response[{:d}]:'.format(len(response)),
                           long_text=response)
            # Call all loaded plugins in a sequence
            response = self.modify_response_using_plugins(response,
                                                          connection_log)
            # Send the modified response back to the client
            connection_log(log_category='out',
                           text='Output[{:d}]:'.format(len(response)),
                           long_text=response)
            self.wfile.write(response)
            connection_log(log_category='success',
                           text='Sent response: {:d}B'.format(len(response)))
            self._shared_data.events.request_processed()
        except ET.ParseError as e:
            connection_log(log_category='err',
                           text='XML parse error: ' + str(e),
                           long_text=format_exc())
        except httplib.HTTPException as e:
            connection_log(log_category='err',
                           text='BrandingService connection error: ' + str(e),
                           long_text=format_exc())

    def query_branding_service(self, request):
        up = urlparse.urlparse(self._cli_args.url)
        with closing(httplib.HTTPConnection(up.netloc)) as conn:
            conn.request('POST', up.path, request)
            response = conn.getresponse()
            return response.read()

    def process_dynamic_config_request(self, request, connection_log):
        """ Processes a configuration string, reloads and reconfigures the plugins """
        try:
            connection_log(log_category='cfg',
                           text='Received new configuration',
                           long_text=request)
            config = SafeConfigParser()
            config.optionxform = str
            config.readfp(BytesIO(request))
            plugins = load_plugins(self._shared_data.plugin_dir(), config)
            self._shared_data.set(config=config, plugins=plugins)
            connection_log(log_category='cfg',
                           text='Configuration updated')
            self.send_response(200)
            self.end_headers()
        except Exception as e:
            self.send_error(400, "Configuration parse error: " + format_exc())
            self.end_headers()
            raise


    def log_response(self, response, connection_log):
        if connection_log.global_log.lc['resp_int'].is_enabled:
            if not isinstance(response, str):
                response = ET.tostring(response)
            connection_log(log_category='resp_int',
                           text='Response[{:d}]:'.format(len(response)),
                           long_text=response)


    def modify_response_using_plugins(self, response, connection_log):
        """ Calls all the loaded plugin in a sequence, modifying the request.
        Returns the request, modified by the plugins as a string """
        xml = ET.XML(response)
        plugins = self._shared_data.plugins()
        for plugin in plugins:
            connection_log(log_category='plg',
                           text='Calling plugin "{:s}"...'.format(plugin.name))
            # Try to call a plugin and catch all its exceptions
            try:
                for response in plugin.instance(xml, connection_log):
                    if isinstance(response, str):
                        self.log_response(response, connection_log)
                        return response
                xml = response
                self.log_response(xml, connection_log)
            except Exception as e:
                connection_log(log_category='err',
                               text=plugin.name + ': ' + format_exc())
        return ET.tostring(xml)


#                                                _
#     /\                                        | |
#    /  \   _ __ __ _ _   _ _ __ ___   ___ _ __ | |_ ___
#   / /\ \ | '__/ _` | | | | '_ ` _ \ / _ \ '_ \| __/ __|
#  / ____ \| | | (_| | |_| | | | | | |  __/ | | | |_\__ \
# /_/    \_\_|  \__, |\__,_|_| |_| |_|\___|_| |_|\__|___/
#                __/ |
#               |___/
class EnableLogCategoryAction(argparse.Action):
    """ If the argument is specified - enables Log Categories dest[2] in
    logger dest[1]. Defalt dest should be specified as dest[0]. """
    def __init__(self,
                 option_strings,
                 dest,
                 default=None,
                 required=False,
                 help=None,
                 metavar=None):
        self.__log = dest[1]
        self.__log_categories = dest[2]
        super(EnableLogCategoryAction, self).__init__(
            option_strings=option_strings,
            dest=dest[0],
            nargs=0,
            const=None,
            default=default,
            type=bool,
            choices=None,
            required=required,
            help=help,
            metavar=metavar)

    def __call__(self, parser, namespace, values, option_string=None):
        """ This function gets called by ArgumentParser when the argument is found """
        if isinstance(self.__log_categories, list):
            for log_category in self.__log_categories:
                self.__log.lc[log_category].is_enabled = True
        else:
            self.__log.lc[self.__log_categories].is_enabled = True


def create_argument_parser(log):
    parser = argparse.ArgumentParser(description='Mock new functionality of Branding Service',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('-p', '--port', type=int, default=8000,
                        help="""HTTP port for this server. The POST requests
                                are forwarded to the BrandingService while
                                GET requests are handled locally - the
                                current connection statistics is returned.""")
    parser.add_argument('-u', '--url', default=INT_URL,
                        help='URL of the real Branding Service')

    parser.add_argument('-d', '--default-config', action='store_true',
                        help='Show the default configuration for all plugins')

    parser.add_argument('config_file', type=argparse.FileType('r'),
                        default=None,
                        nargs='?', help='Configuration file')

    debugGroup = parser.add_argument_group('Debugging')
    debugGroup.add_argument('--dump-all',
                            default=False,
                            dest=('dump_all', log, log.lc.keys()),
                            action=EnableLogCategoryAction,
                            help='Enable all printouts')

    for lc_name, lc in log.lc.iteritems():
        arg_name = '--dump-' + lc_name
        debugGroup.add_argument(arg_name,
                                default=lc.is_enabled,
                                dest=(arg_name, log, lc_name),
                                action=EnableLogCategoryAction,
                                help=lc.description)

    return parser


#  ______       _                              _       _
# |  ____|     | |                            (_)     | |
# | |__   _ __ | |_ _ __ _   _     _ __   ___  _ _ __ | |_
# |  __| | '_ \| __| '__| | | |   | '_ \ / _ \| | '_ \| __|
# | |____| | | | |_| |  | |_| |   | |_) | (_) | | | | | |_
# |______|_| |_|\__|_|   \__, |   | .__/ \___/|_|_| |_|\__|
#                         __/ |   | |
#                        |___/    |_|
#
if __name__ == '__main__':
    shared_data = SharedData()
    app_dir = os.path.dirname(sys.argv[0])
    shared_data.set(plugin_dir=os.path.join(app_dir, 'plugins'))
    log = Log()
    parser = create_argument_parser(log)
    args = parser.parse_args()

    ET.register_namespace('', XMLNS)
    ET.register_namespace('ANCS', XMLNS_ANCS)
    ET.register_namespace('omsg2', 'http://opentravel.org/common/message/v02')
    ET.register_namespace('oc2', 'http://opentravel.org/common/v02')
    ET.register_namespace('stlp21', 'http://services.sabre.com/STL_Payload/v02_01')
    ET.register_namespace('m0', 'http://stl.sabre.com/Merchandising/v0')

    try:
        if args.default_config:
            # Print default configuration of all plugins
            plugins = load_plugins(directory=shared_data.plugin_dir())
            print '\nDefault configuration:'
            for plugin in plugins:
                print plugin.instance.default_config()
            sys.exit(0)

        # Parse configuration file
        config = SafeConfigParser()
        if args.config_file:
            config.optionxform = str
            config.readfp(args.config_file)

        shared_data.set(config=config,
                        plugins=load_plugins(shared_data.plugin_dir(),
                                             config))
    except PluginLoadException as e:
        print ('Plugin load error: {:s}: '
               '{:s}').format(str(e), e.plugin_stacktrace)
        sys.exit(-1)

    make_proxy = functools.partial(BrandingServiceProxy,
                                   cli_args=args,
                                   log=log,
                                   shared_data=shared_data)

    SocketServer.TCPServer.allow_reuse_address = True
    httpd = SocketServer.ThreadingTCPServer(("", args.port), make_proxy)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        httpd.server_close()
        sys.exit(0)
