from ConfigParser import SafeConfigParser
from httplib import HTTPConnection
from StringIO import StringIO

class BSPRemoteException(Exception):
    pass


class BSPRemote:
    """ Remote control for BrandServiceProxy.
    ``Example``:
        remote = BSPRemote(host="atsedbld05a.dev.sabre.com", port=8000)
        # Print configuration in a human-readable format
        print remote.get_config_str()
        # Get the current config, modify and set the new config
        config = remote.get_config()
        config.set("cbas", "dataSource_1", "CBS")
        remote.set_config(config)
        # Verify that the config was updated
        print remote.get_config_str()
        # Wait until the proxy processes one request
        print "Waiting for request..."
        remote.wait_request()
        print "Request was processed"
    """
    def __init__(self, host='127.0.0.1', port=8000):
        self._host = host
        self._port = port

    def bsp_address(self):
        return str(self._host) + ":" + str(self._port)

    def get_config_str(self):
        """ Gets the current configuration as a string from BrandServiceProxy. """
        try:
            connection = HTTPConnection(host=self._host,
                                        port=self._port)
            connection.connect()
        except Exception, e:
            raise BSPRemoteException("Can't connect to BrandServiceProxy at " +
                                     self.bsp_address() + ": " + str(e))

        try:
            connection.request(method='GET',
                               url='/config')
            response = connection.getresponse()
        except Exception, e:
            raise BSPRemoteException("Can't get BS config: " + str(e))

        if response.status == 200:
            response_data = response.read()
            return response_data
        else:
            raise BSPRemoteException("Can\'t get BS config: ERR" +
                                     str(response.status) + ": " + response.read())

    def get_config(self):
        """ Gets the current configuration as a SafeConfigParser from BrandServiceProxy. """
        config = SafeConfigParser()
        config.optionxform = str
        config.readfp(StringIO(self.get_config_str()))
        return config

    def set_config(self, config):
        """ Sets the configuration (it can be a string or a SafeConfigParser
        object with all the configurations added as optuins in sections per plugin """
        if isinstance(config, SafeConfigParser):
            buf = StringIO()
            config.write(buf)
            config = buf.getvalue()
        try:
            connection = HTTPConnection(host=self._host,
                                        port=self._port)
            connection.connect()
        except Exception, e:
            raise BSPRemoteException("Can't connect to BrandServiceProxy at " +
                                     self.bsp_address() + ": " + str(e))
        try:
            connection.request(method='POST',
                               url='/config',
                               body=config)
            response = connection.getresponse()
        except Exception, e:
            raise BSPRemoteException("Can't set BS config: " + str(e))

        if response.status != 200:
            raise BSPRemoteException("Can\'t set BS config: ERR" +
                                     str(response.status) + ": " + response.read())

    def wait_request(self, timeout=None):
        """ Blocks until the proxy successfully parses a request from atsev2 """
        try:
            connection = HTTPConnection(host=self._host,
                                        port=self._port)
            connection.connect()
        except Exception, e:
            raise BSPRemoteException("Can't connect to BrandServiceProxy at " +
                                     self.bsp_address() + ": " + str(e))
        try:
            connection.request(method='GET',
                               url='/wait_request')
            response = connection.getresponse()

        except Exception, e:
            raise BSPRemoteException("Error in wait_request: " + str(e))

        if response.status != 200:
            raise BSPRemoteException("Error in wait_request: ERR" +
                                     str(response.status) + ": " + response.read())