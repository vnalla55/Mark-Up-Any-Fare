import argparse
from collections import namedtuple
import csv
from datetime import datetime
from datetime import timedelta
import glob
import itertools
import logging
import os
import random
import re
import select
import signal
import socket
import struct
import sys
import threading
import time
from xml.dom import minidom
import ConfigParser 
import Queue
from urlparse import urlparse
from random import sample

# Some aweful globals
thread_exception_bucket = Queue.Queue()
thread_counter = None   

class FatalError(Exception):
   def __init__(self, error):
      self.error = error
   
   def __str__(self):
      return repr(self.error) 
           
class SharedCounter:
   def __init__(self, initial_value = 0):
      self.__value = initial_value
      self.__value_lock = threading.Lock()

   def incr(self,delta=1):
      with self.__value_lock:
         self.__value += delta

   def decr(self,delta=1):
      with self.__value_lock:
         self.__value -= delta
   
   def value(self):
      return self.__value

class ServerConnection():
    def __init__(self, hostname, server_port, appconsole_port, timeout, strip_ticketing_date=True,
                 add_mip_force_notimeout=False):
      self.__hostname = hostname
      self.__server_port = server_port
      self.__appconsole_port = appconsole_port
      self.__timeout = timeout
      self.__strip_ticketing_date = strip_ticketing_date
      self.__add_mip_force_notimeout = add_mip_force_notimeout
        
    def __str__(self):
      return self.__hostname
    
    def __make_xml2_header(self, command, length):
      return struct.pack('!ll4s8s', 16, length, command, '00010000')
    
    def __send_request(self, port, command, xml_request):
        response = ""
        
        try:
            s = socket.socket()
            s.connect((self.__hostname, port))
            s.send(self.__make_xml2_header(command, len(xml_request)) + xml_request)
            ready = select.select([s], [], [], self.__timeout)
   
            if ready[0]:
                buf = s.recv(20)
                
                if len(buf) == 20:
                    header = struct.unpack('!ll4s8s', buf)
                    
                    payload_len = header[1]
                    bytes_recd = 0
                    
                    while bytes_recd < payload_len:
                        chunk = s.recv(min(payload_len - bytes_recd, 4096))
                        response = response + chunk
                        bytes_recd = bytes_recd + len(chunk)
                    response = response[0:payload_len - 1]
                    
        except (socket.error, select.error):
            raise FatalError('Cannot connect to host {0} on port {1}'.format(self.__hostname, port))
            
        finally:
            try:
                s.shutdown(socket.SHUT_RDWR)
                s.close()
            except socket.error:
                pass
                        
        return response
    
    @staticmethod
    def __get_xml(line):
        try:
            xmlStart, xmlEnd = line.index('<'), line.rindex('>')
        except ValueError:
            return None
        
        return line[xmlStart:xmlEnd + 1]

    def __appconsole_command(self, command):
        return self.__send_request(self.__appconsole_port, command, '')
    
    def request(self, raw_request):
        request = self.__get_xml(raw_request)
        
        if request is None:
            logging.error('Invalid request. Skipping.')
            return
        
#        logging.debug(request)
        
        request = re.sub(r'C20=".*?"', r'C20="SOCKCLIENT"', request)
        
        if self.__add_mip_force_notimeout:
            result = re.sub(r'NTO=".*?"', r'NTO="Y"', request)
            
            if result[1] is 0:
                request = re.sub(r'<ShoppingRequest ', r'<ShoppingRequest NTO="Y" ', request)
            else:
                request = result[0]
                
        if self.__strip_ticketing_date:
            request = re.sub(r'D07=".*?"', '', request)

        response = self.__send_request(self.__server_port, 'REQ', request)

        logging.debug(response)
    
    def get_stats(self):
        response = self.__appconsole_command('RFSH')
        
        if response is -1:
            return None
        else:
           sample = StatSample()
           
           return sample if sample.parse(minidom.parseString(response)) else None
    
    def reset_stats(self):
        self.__appconsole_command('CNRT')
        
    def get_config(self):
        return self.__appconsole_command('CNFG')

class StatSample:
   def __init__(self):
      self.timestamp = 0
      self.trx_total_count = 0
      self.trx_ok_count = 0
      self.trx_err_count = 0
      self.total_elapsed_time = 0

   def parse(self, dom):
      self.timestamp = int(time.time())
      
      try:
         root_element = dom.getElementsByTagName('STATS')[0]
         
         for service_stat_element in dom.getElementsByTagName('SP'):
             if service_stat_element.attributes['NM'].value == 'TRX':
                 self.trx_ok_count += int(service_stat_element.attributes['OK'].value) 
                 self.trx_err_count += int(service_stat_element.attributes['ER'].value)
                 self.total_elapsed_time += float(service_stat_element.attributes['ET'].value)
       
         self.trx_th_count = int(root_element.attributes['TH'].value)
         self.trx_total_count = self.trx_ok_count + self.trx_err_count
         self.total_cpu_time = float(root_element.attributes['CP'].value)
         self.load_last_5min = float(root_element.attributes['L2'].value)        
         self.resident_memory = int(root_element.attributes['RS'].value)
         self.virtual_memory = int(root_element.attributes['VM'].value)
         self.concurrent_trx = int(root_element.attributes['CT'].value)
      except IndexError:
         return False
      
      return True
           
   def __str__(self):
       return '''timestamp={0}, trx_ok_count={1}, trx_err_count={2},
               total_cpu_time={3} total_elapsed_time={4} virtual_memory={5},
               resident_memory={6} load_last 5min={7} concurrent_trx={8}'''.format(self.timestamp, self.trx_ok_count, self.trx_err_count,
                                                               self.total_cpu_time, self.total_elapsed_time, self.virtual_memory,
                                                               self.resident_memory, self.load_last_5min, self.concurrent_trx)

class RequestSender(threading.Thread):
   def __init__(self, server_cnx, request, barrier):
      threading.Thread.__init__(self)
      self.__server_cnx = server_cnx
      self.__request = request
      self.__barrier = barrier

   def run(self):
      global thread_counter
      
      thread_counter.incr()
            
      try:
         self.__server_cnx.request(self.__request)
      except FatalError as exception:
         global thread_exception_bucket
         thread_exception_bucket.put(exception)
      
      thread_counter.decr()
      self.__barrier.release()
            
class StatsCollector(threading.Thread):
   def __init__(self, interval, stats_output_file, server_cnx, delimiter, reset):
      threading.Thread.__init__(self)
      self.__stop = threading.Event()
      self.__stop.clear()
      self.__interval = interval
      self.__output_file = stats_output_file
      self.__writer = csv.writer(stats_output_file, delimiter=delimiter)
      self.__server_cnx = server_cnx
      self.__reset = reset
      
   def stop(self):
      self.__stop.set()
   
   def is_running(self):
      return not self.__stop.is_set()
   
   def write_header(self):
      self.__writer.writerow(['trx_total_count', 'trx_err_count', 'trx_per_sec', 'avg_cpu_time', 
                              'avg_elapsed_time', 'resident_memory', 'load_last_5min', 'concurrent_trx'])
      self.__output_file.flush()
         
   def write(self, prev_sample, current_sample):
      delta_trx_count = current_sample.trx_total_count - prev_sample.trx_total_count
      delta_time = current_sample.timestamp - prev_sample.timestamp
                                                                    
      if delta_trx_count > 0 and delta_time > 0:
         self.__writer.writerow([current_sample.trx_total_count, current_sample.trx_err_count,
                                 delta_trx_count / delta_time,
                                 (current_sample.total_cpu_time - prev_sample.total_cpu_time) / delta_trx_count,
                                 (current_sample.total_elapsed_time - prev_sample.total_elapsed_time) / delta_trx_count,
                                 current_sample.resident_memory, current_sample.load_last_5min,
                                 current_sample.concurrent_trx])
         self.__output_file.flush()
      
      
   def run(self):
      prev_sample = None
      
      try:              
         self.write_header()
         
         if self.__reset:
            logging.warning('Resetting stats of server {0}'.format(self.__server_cnx))
            self.__server_cnx.reset_stats()
            
         self.__stop.wait(self.__interval)
          
         while (not self.__stop.is_set()):
            logging.debug('Collecting stats from server {0}'.format(self.__server_cnx)) 
            
            time_start = time.time()     
            sample = self.__server_cnx.get_stats()
            
            if sample is not None:
               if prev_sample is not None:
                  self.write(prev_sample, sample)
                  
               prev_sample = sample
                  
            time_end = time.time()
            elapsed_time = time_end - time_start
            
            if elapsed_time < self.__interval:
              self.__stop.wait(self.__interval - elapsed_time)
                
      except FatalError as exception:
         global thread_exception_bucket
         thread_exception_bucket.put(exception)
   
class RequestsFilePool():
  def __init__(self, path):
      self.__path = path
      
  def init(self):
      file_paths = []
      self.__current_file = 0
      self.__files_number = 0
      
      for path_entry in glob.iglob(self.__path):
          if os.path.isfile(path_entry):
              logging.debug("Adding request file to pool: {0}".format(path_entry))
              file_paths.append(path_entry)
      
      self.__files_number = len(file_paths)
      logging.debug("Number of request files in pool: {0}".format(self.__files_number))
      
      if self.__files_number is 0:
        raise FatalError("No request files in given path")
      
      file_paths.sort()
      self.__file_paths = itertools.cycle(file_paths)
      
  def next(self):
      next_path = self.__file_paths.next()
      
      logging.debug("Requested next request file from pool")
      try:
          if not self.__current_file:
              self.__current_file = open(next_path, 'r')
          else:
              if self.__files_number is 1:
                  self.__current_file.seek(0)
                  logging.debug("  Seek to begining of the current requests file: {0}".format(next_path))
              elif self.__files_number > 1:
                  self.__current_file.close()
                  logging.debug("  Opening next requests file from pool: {0}".format(next_path))
                  self.__current_file = open(next_path, 'r')
      except IOError as error:
          self.__current_file = 0
          
          if len(self.__file_paths) is 1:
              raise FatalError("Error while opening requests file \'{0}\' for reading: {1} ".format(next_path, error.strerror))
          else:
              raise
                       
      return self.__current_file

def parse_arguments():
  parser = argparse.ArgumentParser()
  parser.add_argument('config_file', help='path to configuration file')
  parser.add_argument('-l', '--log-level', choices=['debug', 'info', 'warning', 'error', 'critical'],
                      default='critical', help='logging level')
    
  return parser.parse_args()

def parse_config(config_file):
  raw_config = ConfigParser.ConfigParser()
  raw_config.read(config_file)
    
  Config = namedtuple('Config', ['strip_ticketing_date', 'mip_force_notimeout', 'timeout',
    'warmup_duration_time', 'test_duration_time', 'tps', 'stats_sample_interval', 'stats_csv_delimiter', 'reset_server_stats',
    'requests_path',
    'server1_hostname', 'server1_port', 'server1_appconsole_port', 'server1_stats_output_file',
    'server2_hostname', 'server2_port', 'server2_appconsole_port', 'server2_stats_output_file']
  )
  
  if raw_config.has_option('General', 'strip_ticketing_date'):
    strip_ticketing_date = raw_config.getboolean('General', 'strip_ticketing_date')
  else:
    strip_ticketing_date = True
    
  if raw_config.has_option('General', 'mip_force_notimeout'):
    add_mip_force_notimeout = raw_config.getboolean('General', 'mip_force_notimeout')
  else:
    add_mip_force_notimeout = False
    
  if raw_config.has_option('General', 'timeout'):
    timeout = raw_config.getint('General', 'timeout')
  else:
    timeout = 120
    
  if raw_config.has_option('General', 'warmup_phase_duration'):
    warmup_duration_time = raw_config.getint('General', 'warmup_phase_duration')
  else:
    warmup_duration_time = 0
    
  if raw_config.has_option('General', 'test_phase_duration'):
    test_duration_time = raw_config.getint('General', 'test_phase_duration')
  else:
    test_duration_time = 43200
  
  if raw_config.has_option('General', 'tps'):
    tps = raw_config.getint('General', 'tps')
    
    if tps is 0:
      tps = None
  else:
    tps = 25    
      
  if raw_config.has_option('General', 'stats_sample_interval'):
    stats_sample_interval = raw_config.getint('General', 'stats_sample_interval')
  else:
    stats_sample_interval = 30
    
  if raw_config.has_option('General', 'stats_csv_delimiter'):
    csv_delimiter = raw_config.get('General', 'stats_csv_delimiter')
    
    if csv_delimiter is '#':
       csv_delimiter = ' '
  else:
    csv_delimiter = ';'

  if raw_config.has_option('General', 'reset_server_stats'):
    reset_stats = raw_config.getboolean('General', 'reset_server_stats')
  else:
    reset_stats = True
      
  try:   
    return Config(strip_ticketing_date, add_mip_force_notimeout, timeout, warmup_duration_time, test_duration_time,
      tps, stats_sample_interval, csv_delimiter, reset_stats,
      raw_config.get('General', 'requests_path'),
      raw_config.get('ReferenceServer', 'hostname'), raw_config.getint('ReferenceServer', 'port'),
      raw_config.getint('ReferenceServer', 'appconsole_port'), raw_config.get('ReferenceServer', 'stats_output_file'),
      raw_config.get('TestServer', 'hostname'), raw_config.getint('TestServer', 'port'),
      raw_config.getint('TestServer', 'appconsole_port'), raw_config.get('TestServer', 'stats_output_file')
    )
  except ConfigParser.Error as error:
    logging.error('Error in configuration file: {0}'.format(error))
    raise FatalError('Error in configuration file: {0}'.format(error))      
  
def do_phase(interrupt, requests_reader, duration_time, tps, server1_cnx, server2_cnx):   
   global thread_exception_bucket
   global thread_counter
   
   stop = threading.Event()
   sem1 = threading.Semaphore(50)
   sem2 = threading.Semaphore(50)
  
   request_number = 0
   requests_reader.init()
   requests_file = requests_reader.next()
  
   test_start_time = time.time() 
   logging.debug('Phase start time: {0}'.format(datetime.fromtimestamp(test_start_time)))
   shuffle = False
  
   while not stop.is_set() and not interrupt.is_set():
      try: 
         if requests_file is 0:
            raise StopIteration
         
         request = requests_file.next()                     
         request_number = request_number + 1
         current_duration_time = time.time() - test_start_time
         
         sys.stdout.write('Processing request #{0} (Phase duration: {1}, Worker threads count: {2: >3}, Current file: {3})...\r'.format(request_number,
                         timedelta(seconds=int(current_duration_time)),
                         thread_counter.value(),
                         os.path.basename(requests_file.name)))
         sys.stdout.flush()
         
         sem1.acquire()
         sem2.acquire()
        
         try:
            server1_thread = RequestSender(server1_cnx, request, sem1)
            server2_thread = RequestSender(server2_cnx, request, sem2)
            
            if not shuffle:
               server1_thread.start()
               server2_thread.start()
            else:
               server2_thread.start()
               server1_thread.start()
                
         except threading.ThreadError as error:
             logging.warning('Too many working threads. Throtlling...')
            
         shuffle = not shuffle
            
         if tps is not None:
            stop.wait(random.expovariate(tps))
         else:
            server1_thread.join()
            server2_thread.join()
                    
         if duration_time > 0:
            if current_duration_time >= duration_time:
               print '\nTime is up.'
               stop.set()
      
         try:
            thread_exception = thread_exception_bucket.get(block=False)
         except Queue.Empty:
            pass
         else:
            raise thread_exception
          
      except StopIteration:
          requests_file = requests_reader.next()
      
      except IOError as error:
          logging.error(error)
          pass
          
      except FatalError as error:
         print '\n'
         logging.fatal(error)
         stop.set()
         interrupt.set()
       
   test_end_time = time.time()    
   logging.debug('Phase end time: {0}'.format(test_end_time))

   return (request_number, timedelta(seconds=int(test_end_time - test_start_time)))

def join_working_threads():
   global thread_counter
   
   main_thread = threading.current_thread()
   
   for thread in threading.enumerate():
      sys.stdout.write('  Waiting for worker threads to finish (Remaining: {0})...\r'.format(thread_counter.value()))
      sys.stdout.flush()
      
      if thread != main_thread and thread.is_alive():
         thread.join()
                                    
def main():
   global thread_counter

   interrupt = threading.Event()

   def signal_handler(signal, frame):
      print '\nStopping by user request...'
      interrupt.set()
      
   signal.signal(signal.SIGINT, signal_handler)  
  
   thread_counter = SharedCounter()  
   arguments = parse_arguments()
  
   logging.basicConfig(format='%(asctime)s %(levelname)s: %(message)s',
                       level=getattr(logging, arguments.log_level.upper(), None))
  
   try:
      config = parse_config(arguments.config_file)
      requests_reader = RequestsFilePool(config.requests_path)              
   except FatalError as error:
      logging.fatal(error)
      return
    
   server1_cnx = ServerConnection(config.server1_hostname, config.server1_port, config.server1_appconsole_port,
                                  config.timeout, config.strip_ticketing_date, config.mip_force_notimeout)      
   server2_cnx = ServerConnection(config.server2_hostname, config.server2_port, config.server2_appconsole_port,
                                  config.timeout, config.strip_ticketing_date, config.mip_force_notimeout)
   try:     
      server1_stats_output_file = open(config.server1_stats_output_file, 'w')
      server2_stats_output_file = open(config.server2_stats_output_file, 'w')
   except IOError as error:
      logging.fatal(error)
      return
  
   try:
      server1_stats_collector = StatsCollector(config.stats_sample_interval, server1_stats_output_file,
                                               server1_cnx, config.stats_csv_delimiter, config.reset_server_stats)          
      server2_stats_collector = StatsCollector(config.stats_sample_interval, server2_stats_output_file,
                                               server2_cnx, config.stats_csv_delimiter, config.reset_server_stats)
   except FatalError as error:
      logging.fatal(error)
      return

   # Warmup phase    
   if config.warmup_duration_time > 0:
      print 'Starting cache warmup phase...'
      do_phase(interrupt, requests_reader, config.warmup_duration_time, config.tps, server1_cnx, server2_cnx)
      print 'Stopping cache warmup phase...'
      join_working_threads()     
      print '\nCache warmup phase completed.\n'

   # Test phase
   if not interrupt.is_set():
      print 'Starting test phase...'
      
      server1_stats_collector.start()
      server2_stats_collector.start()
      
      (request_count, duration) = do_phase(interrupt, requests_reader, config.test_duration_time, config.tps, server1_cnx, server2_cnx)
      
      print 'Stopping test phase...'
      
      server1_stats_collector.stop()
      server1_stats_collector.join()    
      server1_stats_output_file.close() 
      server2_stats_collector.stop()
      server2_stats_collector.join()
      server2_stats_output_file.close()
      join_working_threads()     
      
      print '\nTest phase completed.'
      print 'Processed {0} requests in {1}.'.format(request_count, duration)          
     
   print 'All Done.'
    
if __name__ == '__main__':
   main()
