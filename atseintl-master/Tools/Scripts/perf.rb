#!/usr/bin/ruby
#-----------------------------------------------------------------------------
#  Copyright Sabre 2005
#
#          The copyright to the computer program(s) herein
#          is the property of Sabre.
#          The program(s) may be used and/or copied only with
#          the written permission of Sabre or in accordance
#          with the terms and conditions stipulated in the
#          agreement/contract under which the program(s)
#          have been supplied.
#
#  This program parses the output from the ATSE metrics and the Hammer
#  program and produces a CSV file for producing a performance summary
#  spreadsheet.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Add methods to built-in Array class for doing regexp matching on an array
# of strings.
#-----------------------------------------------------------------------------
class Array
  # Match the first appearance of a pattern
  def match_once(pattern, &block)
    self.each do
      |elem|
      match = pattern.match(elem)
      return (block ? block.call(match) : match) if match
    end
    return (block ? block.call(nil) : nil)
  end

  # Match a pattern, expecting two floats
  def match_two_floats_once(pattern)
    self.match_once(pattern) do
      |match|
      if match
        [match[1].to_f, match[2].to_f] 
      else
        [0, 0]
      end
    end
  end

  # Match a pattern, expecting one float
  def match_one_float_once(pattern)
    self.match_once(pattern) do
      |match|
      if match
        match[1].to_f
      else
        0
      end
    end
  end

  # Match a pattern, expecting one integer
  def match_one_int_once(pattern)
    self.match_one_float_once(pattern).to_i
  end

  # Match a pattern, expecting two integers
  def match_two_ints_once(pattern)
    self.match_two_floats_once(pattern).map {|x| x.to_i}
  end

  # Match a pattern, expecting one string, with a default value
  def match_string_once(pattern, default)
    self.match_once(pattern) do
      |match|
      if match
        match[1]
      else
        default
      end
    end
  end

  # Match multiple appearances of a pattern
  def match_multi(pattern, &block)
    list = Array.new
    self.each do
      |elem|
      match = pattern.match(elem)
      if match
        list << (block ? block.call(match) : match[1])
      end
    end
    list
  end
end

#-----------------------------------------------------------------------------
# Utilities for dealing with time strings
#-----------------------------------------------------------------------------
module TimeUtil
  # turn a time string in the form YYYY-MM-DD HH:MM:SS or YYYY-MM-DD HHMMSS
  # into a Time object
  def TimeUtil.parse_time(timeStr, dmp)
    timePattern = /(\d+):(\d+):(\d+)/
    if dmp
      timePattern = /(\d\d)(\d\d)(\d\d)/
    end
    timeMatch = timePattern.match(timeStr)
    raise "Unable to parse time value [#{timeStr}]" if not timeMatch
    timeArray = ["2005", "1", "1"] + timeMatch[1..3]
    Time.local(*timeArray)
  end

  # calculate the difference in seconds between the two events.
  def TimeUtil.calc_time_span(beginTimeStr, endTimeStr, dmp=false)
    beginTime = TimeUtil.parse_time(beginTimeStr, dmp)
    endTime = TimeUtil.parse_time(endTimeStr, dmp)
    # check for midnight crossing
    if (beginTime.hour.to_i > 6 && endTime.hour.to_i < 6)
      endTime = endTime + (60 * 60 * 12)      # move ahead twelve hours
    end
    endTime - beginTime
  end
end

#-----------------------------------------------------------------------------
# Utilities for parsing files
#-----------------------------------------------------------------------------
module ParseUtil

  # Split file into groups of lines, divided according to the given pattern.
  # Give each group of lines to the given block, and collect the results in
  # an array.
  def ParseUtil.extract_records(fileName, pattern)
    collectingLines = false
    lineGroup = Array.new
    File.foreach(fileName) do |line|
      if pattern.match(line)
        if collectingLines
          yield(lineGroup)
          lineGroup.clear
        else
          collectingLines = true
        end
      end
      lineGroup << line if collectingLines
    end
    yield(lineGroup)
  end

  # Split file into groups of lines, divided according to the two given patterns.
  # The two patterns should match two consecutive lines to be a valid record
  # separator.
  # Give each group of lines to the given block, and collect the results in
  # an array.
  def ParseUtil.extract_records_dual(fileName, pattern1, pattern2)
    collectingLines = false
    lineGroup = Array.new
    previousLine = ""
    File.foreach(fileName) do |line|
      if pattern1.match(previousLine) && pattern2.match(line)
        if collectingLines
          yield(lineGroup)
          lineGroup.clear
        else
          collectingLines = true
        end
      end
      lineGroup << previousLine if collectingLines
      previousLine = line
    end
    lineGroup << previousLine if collectingLines
    yield(lineGroup)
  end

end

#-----------------------------------------------------------------------------
# Contains metrics information captured from the server process
#-----------------------------------------------------------------------------
class Metric
  attr_reader :pnr, :entry, :pcc, :transId, :lniata, :timeStamp,
    :atseElapsedTime, :atseCpuTime, :ataeTime,
    :toItinElapsedTime, :toItinCpuTime, :toFareCElapsedTime, :toFareCCpuTime,
    :toFareVElapsedTime, :toFareVCpuTime, :toPricingElapsedTime, :toPricingCpuTime,
    :toTaxElapsedTime, :toTaxCpuTime, :toFareCalcElapsedTime, :toFareCalcCpuTime,
    :travelSegs, :fareMarkets, :paxTypes, :paxTypeFares, :virtualMemSize,
    :asv2ElapsedTime, :dssv2ElapsedTime, :baggageElapsedTime, :billingElapsedTime

  def initialize(lines=nil)
    @pnr = @entry = @pcc = @transId = @lniata = "-UDEF-"
    @timeStamp = "00:00:00"
    @atseElapsedTime = @atseCpuTime = @ataeTime = 0.0
    @toItinElapsedTime = @toItinCpuTime = @toFareCElapsedTime = @toFareCCpuTime = 0.0
    @toFareVElapsedTime = @toFareVCpuTime = @toPricingElapsedTime = @toPricingCpuTime = 0.0
    @toTaxElapsedTime = @toTaxCpuTime = @toFareCalcElapsedTime = @toFareCalcCpuTime = 0.0
    @travelSegs = @fareMarkets = @paxTypes = @paxTypeFares = @virtualMemSize = 0
    @asv2ElapsedTime = @dssv2ElapsedTime = @baggageElapsedTime = @billingElapsedTime = 0.0
    @dateTime = "0000-00-00 00:00:00,000"
    parse(lines) if lines
  end

  # Populate object by parsing a text record
  def parse(lines)
    pnrPattern = /PNR: '([A-Z]{6})'/
    entryPattern = /Entry: '([^']*)'/
    pccPattern = /PCC: '([^']*)'/
    transIdPattern = /TransID: '([^']*)'/
    lniataPattern = /LNIATA: '([^']*)'/
    timeStampPattern = /(\d+:\d+:\d+),/
    timePattern = /TSEMANAGERUTIL SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    ataeTimePattern = /ITIN ATAE +(\d+\.\d+) /
    travelSegsPattern = /TRAVEL SEGMENTS: +(\d+)/
    fareMarketsPattern = /FARE MARKETS: +(\d+)/
    paxTypesPattern = /PAX TYPES: +(\d+)/
    paxTypeFaresPattern = /PAXTYPEFARES: +(\d+)/
    virtualMemSizePattern = /VIRTUAL MEM SIZE: +(\d+)/
    toItinPattern = /^- TO ITIN SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareCPattern = /^- TO FARES C SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareVPattern = /^- TO FARES V SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toPricingPattern = /^- TO PRICING SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toTaxPattern = /^- TO TAX SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareCalcPattern = /^- TO FARE CALC SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    asv2Pattern = /^      ASv2 \|  [01] \|  [01] \| *(\d{1,4}\.\d{4})/
    dssv2Pattern = /^     DSSv2 \|  [01] \|  [01] \| *(\d{1,4}\.\d{4})/
    baggagePattern = /^   Baggage \|  [01] \|  [01] \| *(\d{1,4}\.\d{4})/
    billingPattern = /^   Billing \|  [01] \|  [01] \| *(\d{1,4}\.\d{4})/
    dateTimePattern = /(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d+):/

    @pnr = lines.match_string_once(pnrPattern, "-UDEF-")
    @entry = lines.match_string_once(entryPattern, "-UDEF-")
    @pcc = lines.match_string_once(pccPattern, "-UDEF-")
    @transId = lines.match_string_once(transIdPattern, "-UDEF-")
    @lniata = lines.match_string_once(lniataPattern, "-UDEF-")
    @timeStamp = lines.match_string_once(timeStampPattern, "00:00:00")
    @atseElapsedTime, @atseCpuTime = lines.match_two_floats_once(timePattern)
    @ataeTime = lines.match_one_float_once(ataeTimePattern)
    @travelSegs = lines.match_one_int_once(travelSegsPattern)
    @fareMarkets = lines.match_one_int_once(fareMarketsPattern)
    @paxTypes = lines.match_one_int_once(paxTypesPattern)
    @paxTypeFares = lines.match_one_int_once(paxTypeFaresPattern)
    @virtualMemSize = lines.match_one_int_once(virtualMemSizePattern)
    @toItinElapsedTime, @toItinCpuTime = lines.match_two_floats_once(toItinPattern)
    @toFareCElapsedTime, @toFareCCpuTime = lines.match_two_floats_once(toFareCPattern)
    @toFareVElapsedTime, @toFareVCpuTime = lines.match_two_floats_once(toFareVPattern)
    @toPricingElapsedTime, @toPricingCpuTime = lines.match_two_floats_once(toPricingPattern)
    @toTaxElapsedTime, @toTaxCpuTime = lines.match_two_floats_once(toTaxPattern)
    @toFareCalcElapsedTime, @toFareCalcCpuTime = lines.match_two_floats_once(toFareCalcPattern)
    @asv2ElapsedTime = lines.match_one_float_once(asv2Pattern)
    @dssv2ElapsedTime = lines.match_one_float_once(dssv2Pattern)
    @baggageElapsedTime = lines.match_one_float_once(baggagePattern)
    @billingElapsedTime = lines.match_one_float_once(billingPattern)
    @dateTime = lines.match_string_once(dateTimePattern, "0000-00-00 00:00:00,000")
  end

  # One-line string representation of the object
  def to_s
    "#{@pnr}- total:#{@atseElapsedTime} " +
      "cpu:#{@atseCpuTime} atae:#{@ataeTime} ts:#{@travelSegs} " +
      "fm:#{@fareMarkets} pt:#{@paxTypes} ptf:#{@paxTypeFares} " +
      "vm:#{@virtualMemSize}\n"
  end

  def to_csv_1
    "#{@travelSegs},#{@fareMarkets},#{@paxTypes},#{@paxTypeFares},#{@virtualMemSize}"
  end

  def to_csv_2
    "#{@atseElapsedTime},#{@atseCpuTime},#{@ataeTime},#{@atseElapsedTime - @ataeTime}"
  end

  def to_csv_3
    "#{@toItinElapsedTime},#{@toItinCpuTime},#{@toFareCElapsedTime},#{@toFareCCpuTime}," +
      "#{@toFareVElapsedTime},#{@toFareVCpuTime},#{@toPricingElapsedTime},#{@toPricingCpuTime}," +
      "#{@toTaxElapsedTime},#{@toTaxCpuTime},#{@toFareCalcElapsedTime},#{@toFareCalcCpuTime}," +
      "#{@asv2ElapsedTime},#{@dssv2ElapsedTime},#{@baggageElapsedTime},#{@billingElapsedTime}"
  end

  def to_csv_metric_only
    "#{@pcc},#{@pnr},#{@entry},"
  end

  def to_csv_metric_only_end
    ",#{@lniata},\"#{@dateTime}\""
  end

  def print_menu(idx)
    printf("%3d|%8s|%5.1f|%4d|%4d|%4d\n", idx, @timeStamp,
        @atseElapsedTime, @fareMarkets, @paxTypeFares, @virtualMemSize)
  end

  def <=>(other)
    self.timeStamp <=> other.timeStamp
  end

end

#-----------------------------------------------------------------------------
# Contains timing information from the Hammer program
#-----------------------------------------------------------------------------
class HammerEntry
  attr_reader :pnr, :entry, :timeStr, :duration, :legacy, :nc

  # Populate object by parsing a text record
  def initialize(pnr="-UDEF-", entry="XX", timeStr="00:00:00", duration=0)
    legacyPattern = /^(WP$)|(WP[^B])/
    ncPattern = /\$NC[$B]?$/
    @pnr = pnr
    @entry = entry.tr("\207", "$") # change Cross of Lorraine to dollar sign
    @timeStr = timeStr
    @duration = duration
    @legacy = (@entry =~ legacyPattern ? true : false)
    @nc = (@entry =~ ncPattern ? "NC" : "")
  end

  # One-line string representation of the object
  def to_s
    "#{@pnr}- #{@timeStr} #{@entry} duration:#{@duration} #{@nc}\n"
  end
end

#-----------------------------------------------------------------------------
# Hold all of the statistics for a particular PNR and entry combination
#-----------------------------------------------------------------------------
class PnrRecord
  attr_reader :metrics, :hammers, :runCount

  def initialize(pnr)
    @pnr = pnr
    @metrics = Array.new
    @hammers = Array.new
    @legacy = HammerEntry.new
    @runCount = 0
  end

  def add_runs(count)
    @runCount += count
  end

  def add_metric(metric)
    @metrics << metric
    nil
  end

  def add_hammer(hammer)
    if (hammer.legacy)
      @legacy = hammer
    else
      @hammers << hammer
    end
  end

  # Simple string representation of the object
  def to_s
    output = ""
    @metrics.each {|m| output += m.to_s}
    @hammers.each {|h| output += h.to_s}
    output
  end

  # CSV representation of the object
  def to_csv(idx)
    metric = @metrics[idx]
    hammer = @hammers[idx]
    return if hammer == nil or metric == nil
    failInd = (metric.pnr == "-FAIL-" ? "FAIL" : "")
    tpfAtseTime = hammer.duration - metric.atseElapsedTime
    ",#{@pnr},#{@legacy.duration},#{metric.to_csv_1}," +
      "#{hammer.duration},#{tpfAtseTime}," +
      "#{metric.to_csv_2},#{failInd},#{hammer.nc},#{metric.to_csv_3}"
  end

  # CSV representation of the object in the case of no metrics file
  def to_csv_hammer(idx)
    hammer = @hammers[idx] 
    return if hammer == nil
    ",#{@pnr},#{@legacy.duration},#{hammer.duration},#{hammer.nc}"
  end
end

#-----------------------------------------------------------------------------
# Collection of statistics for a single Fare Display entry
#-----------------------------------------------------------------------------
class FdEntryRecord

  attr_reader :entry, :duration, :metrics
  attr_writer :metrics

  def initialize(entryItems)
    raise ArgumentError, "size is #{entryItems.size}" if entryItems.size != 3
    @entry = entryItems[0]
    begin
      @duration = TimeUtil.calc_time_span(entryItems[1], entryItems[2])
    rescue
      puts "Unable to calculate duration for entry [#{@entry}]"
      @duration = 0
    end
    @metrics = FdMetric.new
  end

  def to_csv
    "#{@entry},#{@duration}," + (@metrics.nil? ? "nil" : @metrics.to_csv)
  end

end

#-----------------------------------------------------------------------------
# Collection of metrics output for a single Fare Display entry
#-----------------------------------------------------------------------------
class FdMetric
  attr_reader :atseElapsedTime,
    :atseCpuTime,
    :toFareDisplayProcessElapsedTime,
    :toFareDisplayProcessCpuTime,
    :toItinElapsedTime,
    :toItinCpuTime,
    :toFareCElapsedTime,
    :toFareCCpuTime,
    :toFareVElapsedTime,
    :toFareVCpuTime,
    :toFareSelectorElapsedTime,
    :toFareSelectorCpuTime,
    :toTaxElapsedTime,
    :toTaxCpuTime,
    :toFareDisplayElapsedTime,
    :toFareDisplayCpuTime,
    :toReqrspElapsedTime,
    :toReqrspCpuTime,
    :travelSegments,
    :fareMarkets,
    :paxTypes,
    :paxTypeFares,
    :virtualMemSize,
    :requestSize,
    :responseSize,
    :startTime

  def initialize(lines=nil)
    @atseElapsedTime = 0.0
    @atseCpuTime = 0.0
    @toFareDisplayProcessElapsedTime = 0.0
    @toFareDisplayProcessCpuTime = 0.0
    @toItinElapsedTime = 0.0
    @toItinCpuTime = 0.0
    @toFareCElapsedTime = 0.0
    @toFareCCpuTime = 0.0
    @toFareVElapsedTime = 0.0
    @toFareVCpuTime = 0.0
    @toFareSelectorElapsedTime = 0.0
    @toFareSelectorCpuTime = 0.0
    @toTaxElapsedTime = 0.0
    @toTaxCpuTime = 0.0
    @toFareDisplayElapsedTime = 0.0
    @toFareDisplayCpuTime = 0.0
    @toReqrspElapsedTime = 0.0
    @toReqrspCpuTime = 0.0
    @travelSegments = 0.0
    @fareMarkets = 0.0
    @paxTypes = 0.0
    @paxTypeFares = 0.0
    @virtualMemSize = 0.0
    @requestSize = 0.0
    @responseSize = 0.0
    @startTime = "X000-00-00T00:00:00.000000"
    parse(lines) if lines
  end

  # Populate object by parsing a text record
  def parse(lines)
    timePattern = /TSEMANAGERUTIL SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareDisplayProcessPattern = /^- TO FARE DISPLAY PROCESS +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toItinPattern = /^- TO ITIN SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareCPattern = /^- TO FARES C SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareVPattern = /^- TO FARES V SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareSelectorPattern = /^- TO FARE SELECTOR SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toTaxPattern = /^- TO TAX SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toFareDisplayPattern = /^- TO FARE DISPLAY SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    toReqrspPattern = /^- TO REQRSP SERVICE +(\d{1,4}\.\d{2}) *(\d{1,4}\.\d{2})/
    travelSegsPattern = /TRAVEL SEGMENTS: +(\d+)/
    fareMarketsPattern = /FARE MARKETS: +(\d+)/
    paxTypesPattern = /PAX TYPES: +(\d+)/
    paxTypeFaresPattern = /PAXTYPEFARES: +(\d+)/
    virtualMemSizePattern = /VIRTUAL MEM SIZE: +(\d+)/
    requestResponseSizePattern = /Trx \|.+\|.+\|.+\|.+\| +(\d+) \| +(\d+)/
    responseSizePattern = /Response Size  = +(\d+)/
    startTimePattern = /Start Time: '(.+)'/

    @atseElapsedTime, @atseCpuTime = lines.match_two_floats_once(timePattern)
    @toFareDisplayProcessElapsedTime, @toFareDisplayProcessCpuTime = lines.match_two_floats_once(toFareDisplayProcessPattern)
    @toItinElapsedTime, @toItinCpuTime = lines.match_two_floats_once(toItinPattern)
    @toFareCElapsedTime, @toFareCCpuTime = lines.match_two_floats_once(toFareCPattern)
    @toFareVElapsedTime, @toFareVCpuTime = lines.match_two_floats_once(toFareVPattern)
    @toFareSelectorElapsedTime, @toFareSelectorCpuTime = lines.match_two_floats_once(toFareSelectorPattern)
    @toTaxElapsedTime, @toTaxCpuTime = lines.match_two_floats_once(toTaxPattern)
    @toFareDisplayElapsedTime, @toFareDisplayCpuTime = lines.match_two_floats_once(toFareDisplayPattern)
    @toReqrspElapsedTime, @toReqrspCpuTime = lines.match_two_floats_once(toReqrspPattern)
    @travelSegments = lines.match_one_int_once(travelSegsPattern)
    @fareMarkets = lines.match_one_int_once(fareMarketsPattern)
    @paxTypes = lines.match_one_int_once(paxTypesPattern)
    @paxTypeFares = lines.match_one_int_once(paxTypeFaresPattern)
    @virtualMemSize = lines.match_one_int_once(virtualMemSizePattern)
    @requestSize, @responseSize = lines.match_two_ints_once(requestResponseSizePattern)
    @startTime = lines.match_string_once(startTimePattern, "0000-00-00T00:00:00.000000")

  end

  def to_csv
    "#{@atseElapsedTime}," +
    "#{@atseCpuTime}," +
    "#{@toFareDisplayProcessElapsedTime}," +
    "#{@toFareDisplayProcessCpuTime}," +
    "#{@toItinElapsedTime}," +
    "#{@toItinCpuTime}," +
    "#{@toFareCElapsedTime}," +
    "#{@toFareCCpuTime}," +
    "#{@toFareVElapsedTime}," +
    "#{@toFareVCpuTime}," +
    "#{@toFareSelectorElapsedTime}," +
    "#{@toFareSelectorCpuTime}," +
    "#{@toTaxElapsedTime}," +
    "#{@toTaxCpuTime}," +
    "#{@toFareDisplayElapsedTime}," +
    "#{@toFareDisplayCpuTime}," +
    "#{@toReqrspElapsedTime}," +
    "#{@toReqrspCpuTime}," +
    "#{@travelSegments}," +
    "#{@fareMarkets}," +
    "#{@paxTypes}," +
    "#{@paxTypeFares}," +
    "#{@virtualMemSize}," +
    "#{@requestSize}," +
    "#{@responseSize}"
  end

  def <=>(other)
    self.startTime <=> other.startTime
  end

end

#-----------------------------------------------------------------------------
# Collection of all of the statistics for Fare Display
#-----------------------------------------------------------------------------
class FdStats

  def initialize
    @entryArray = Array.new
  end

  # Merge metrics into entry records
  def merge_metrics(metricsArray)
    index = 0
    metricsArray.sort.each do
      |metric|
      if index < @entryArray.size
        while @entryArray[index].entry[0..2] == "AAA"
          @entryArray[index].metrics = FdMetric.new
	  index += 1
	end
	@entryArray[index].metrics = metric
	index += 1
      end
    end
    index
  end

  # Extract Metric objects from a file
  def extract_metrics(fileName)
    metricsArray = Array.new
    measurementPattern = /TRANSACTION MEASUREMENTS/
    ParseUtil.extract_records(fileName, measurementPattern) do
      |metricLines|
      metricsArray << FdMetric.new(metricLines)
    end
    self.merge_metrics(metricsArray)
  end

  # Extract HammerEntry objects from a file
  def extract_hammer_out(fileName)
    File.open(fileName) do
      |hammerFile|
      hammerFile.gets # skip the header
      pattern = /^([AFR][^,]+).*,(\d+:\d\d:\d\d),(\d+:\d\d:\d\d)/
      while hammerFile.gets
        match = pattern.match($_)
        next if match.nil?
        entry = match[1..3]
        entryRecord = FdEntryRecord.new(entry)
        @entryArray << entryRecord
      end
    end
    @entryArray.size
  end

  # Output the information in csv format for importing into spreadsheet
  def to_csv
    @entryArray.map do
      |entry|
      entry.to_csv
    end
  end

  def create_csv_file
    File.open("fd_performance.csv", "w") do |file|
      file.puts("Entry,ETE Duration,ATSE Elapsed Time,ATSE CPU Time," +
                "FD Process Elapsed,FD Process CPU,Itin Elapsed,Itin CPU," +
                "Fare C Elapsed,Fare C CPU,Fare V Elapsed,Fare V CPU," +
                "Fare Selector Elapsed,Fare Selector CPU,Tax Elapsed,Tax CPU," +
                "Fare Display Elapsed,Fare Display CPU,Reqrsp Elapsed,Reqrsp CPU," +
                "Travel Segs,Fare Markets,Pax Types,Pax Type Fares," +
                "Virtual Mem Size,Request Size,Response Size")
      file.puts(self.to_csv.join("\n"))
    end
  end

end




#-----------------------------------------------------------------------------
# Collection of contents of the metrics file for Pricing
#-----------------------------------------------------------------------------
class PricingMetricsStats
  def initialize
    @metrics = Array.new
    @numUnrecognized = 0
  end

  # Extract Metric objects from a file
  def extract_metrics(fileName, lniata)
    measurementPattern1 = /^\d\d\d\d-.+Metrics - .?$/
    measurementPattern2 = /\*{58}/
    ParseUtil.extract_records_dual(fileName, measurementPattern1, measurementPattern2) do
      |metricLines|
      m = Metric.new(metricLines)
      if lniata.nil? || m.lniata == lniata
        @metrics << m
      else
        @numUnrecognized += 1
      end
    end
  end

  # Output the information in csv format for importing into spreadsheet
  def create_csv
    File.open("performance.csv", "w") do |file|
      file.puts("PCC,PNR,Entry,TRAVEL SEGMENTS," +
		"FARE MARKETS,PAX TYPES,PAX TYPE FARES,VIRTUAL MEM SIZE (MB)," +
		"TPF Exist Time,TPF-ATSE Time,ATSE V2 ELAPSED Time," +
		"ATSE V2 CPU Time,ATAE Time,ATSE V2 Time,Fail,NC," +
		"TO ITIN Time,TO ITIN CPU,TO FARE C Time,TO FARE C CPU," +
		"TO FARE V Time,TO FARE V CPU,TO PRICING Time,TO PRICING CPU," +
		"TO TAX Time,TO TAX CPU,TO FARE CALC Time,TO FARE CALC CPU," +
		"ASv2 Time,DSSv2 Time,Baggage Time,Billing Time,LNIATA,Date/Time")
      @metrics.each do |m|
          file.puts(m.to_csv_metric_only +  m.to_csv_1 + ",0,0," + m.to_csv_2 + ",,," +
		    m.to_csv_3 + m.to_csv_metric_only_end)
      end
      file.puts
    end
  end

end



#-----------------------------------------------------------------------------
# Collection of all of the statistics for Pricing
#-----------------------------------------------------------------------------
class Stats
  def initialize
    @pnrIndex = Array.new
    @pnrRecords = Hash.new
    @numHammers = 0
    @numMetrics = 0
    @numUnrecognized = 0
  end

  # Extract a PNR string from a group of lines from a hammer script file
  def extract_pnr(lines)
    lines.match_string_once(/^\*([A-Z]{6})/, "******")
  end

  # Find how many international pricing entries there are for a PNR
  def extract_num_runs(lines)
    intlEntryPattern = /^WPB/
    runCount = 0
    lines.each do
      |line|
      runCount += 1 if line =~ intlEntryPattern
    end
    runCount
  end

  # Find the maximum number of international pricing entries across all of the PNR
  def calc_max_runs
    max = 0
    @pnrIndex.each do
      |pnr|
      runs = @pnrRecords[pnr].runCount
      max = runs if runs > max
    end
    max
  end

  # Extract PNR strings from a hammer script file
  def extract_pnr_list(fileName)
    headerPattern = /^-+(\d+)-+/
    ParseUtil.extract_records(fileName, headerPattern) do |lines|
      pnr = extract_pnr(lines)
      if not @pnrRecords.has_key?(pnr)
        @pnrIndex << pnr
        @pnrRecords[pnr] = PnrRecord.new(pnr)
      end
      @pnrRecords[pnr].add_runs(extract_num_runs(lines))
    end
  end

  # Extract Metric objects from a file
  def extract_metrics(fileName, lniata)
    measurementPattern1 = /^\d\d\d\d-.+Metrics - .?$/
    measurementPattern2 = /\*{58}/
    ParseUtil.extract_records_dual(fileName, measurementPattern1, measurementPattern2) do
      |metricLines|
      m = Metric.new(metricLines)
      if m.lniata == lniata && @pnrRecords.has_key?(m.pnr)
        @pnrRecords[m.pnr].add_metric(m)
        @numMetrics += 1
      else
        @numUnrecognized += 1
      end
    end
  end

  # Extract HammerEntry objects from a file
  def extract_hammer_dmp(fileName)
    headerPattern = /^-+(\d+)-+/
    pnrPattern = /^> +\*([A-Z]{6})/
    entryPattern = /^> +(WP.*)\r/
    timePattern = /^ \d\d\/\d\d (\d{6})/
    
    ParseUtil.extract_records(fileName, headerPattern) do
      |lines|
      state = :start
      pnr = nil
      entry = nil
      beginTimeStr = nil
      endTimeStr = nil
      lines.each do
        |line|
        if line =~ pnrPattern
          pnr = $1
        elsif line =~ timePattern
          if state == :start
            beginTimeStr = $1
            state = :beginFound
          elsif state == :entryFound
            endTimeStr = $1
            duration = TimeUtil.calc_time_span(beginTimeStr, endTimeStr, true)
            state = :start
            hammer = HammerEntry.new(pnr, entry, beginTimeStr, duration)
            @pnrRecords[pnr].add_hammer(hammer)
            @numHammers += 1
          else
            puts "Illegal state #{state} for timePattern match"
            state = :start
          end
        elsif line =~ entryPattern
          if state == :start
            # use end time from previous entry as begin time of this entry
            beginTimeStr = endTimeStr
          elsif state != :beginFound
            puts "Illegal state #{state} for entryPattern match"
            state = :start
          end
          entry = $1
          state = :entryFound
        end #pattern check
      end #lines iterator
    end #extract_records
  end

  # Extract HammerEntry objects from a file
  def extract_hammer_out(fileName)
    headerPattern = /^-+(\d+)-+/
    pnrPattern = /^\*([A-Z]{6}),/
    entryPattern = /^(WP.*),.*?(\d+:\d+:\d+),(\d+:\d+:\d+)/
    
    ParseUtil.extract_records(fileName, headerPattern) do
      |lines|
      pnr = lines.match_string_once(pnrPattern, "******")
      entries = lines.match_multi(entryPattern) do
        |match|
        entry, beginTimeStr, endTimeStr = match[1..3]
        duration = TimeUtil.calc_time_span(beginTimeStr, endTimeStr)
        HammerEntry.new(pnr, entry, beginTimeStr, duration)
      end
      entries.each do
        |entry|
        record = @pnrRecords[pnr]
        if record.nil?
          puts "No record found for PNR: #{pnr}."
        else
          @pnrRecords[pnr].add_hammer(entry)
          @numHammers += 1
        end
      end
    end
  end

  # Print sizes of internal data
  def print_sizes
    numRuns = 0
    @pnrRecords.each do
      |pnrRecordPair|
      numRuns += pnrRecordPair[1].runCount
    end
    puts
    puts "********************"
    puts "Data Statistics"
    puts "********************"
    puts "Num PNRs: #{@pnrIndex.length}"
    puts "Num expected runs: #{numRuns}"
    puts "Num hammer entries: #{@numHammers}"
    puts "Num metrics: #{@numMetrics}"
    puts "Num unrecognized metrics: #{@numUnrecognized}"
  end
  
  # Sort metrics and fill in missing hammer entries and metrics
  def clean_up
    self.print_sizes
    numRuns = self.calc_max_runs
    @pnrIndex.each do |pnr|
      pnrRecord = @pnrRecords[pnr]
      pnrRecord.metrics.sort!
    end
  end

  # Output the information in text format
  def create_output
    counter = 1
    File.open("performance.out", "w") do |file|
      @pnrIndex.each do |pnr|
        file.puts "-----#{counter}-----\n#{@pnrRecords[pnr].to_s}"
        counter += 1
      end
    end
  end

  # Output the information in csv format for importing into spreadsheet
  def create_csv
    numRuns = self.calc_max_runs
    File.open("performance.csv", "w") do |file|
      numRuns.times do |idx|
        file.puts("Run #{idx + 1}\n,PNR,TPF Legacy Time,TRAVEL SEGMENTS," +
            "FARE MARKETS,PAX TYPES,PAX TYPE FARES,VIRTUAL MEM SIZE (MB)," +
            "TPF Exist Time,TPF-ATSE Time,ATSE V2 ELAPSED Time," +
            "ATSE V2 CPU Time,ATAE Time,ATSE V2 Time,Fail,NC," +
            "TO ITIN Time,TO ITIN CPU,TO FARE C Time,TO FARE C CPU," +
            "TO FARE V Time,TO FARE V CPU,TO PRICING Time,TO PRICING CPU," +
            "TO TAX Time,TO TAX CPU,TO FARE CALC Time,TO FARE CALC CPU," +
            "ASv2 Time,DSSv2 Time,Baggage Time,Billing Time")
        @pnrIndex.each do |pnr|
          if !@pnrRecords[pnr].nil?
            row = @pnrRecords[pnr].to_csv(idx)
            file.puts row if !row.nil?
          end
        end
        file.puts
      end
    end
  end

  # Output the information for a hammer file in csv format for importing into spreadsheet
  def create_csv_hammer
    numRuns = self.calc_max_runs
    File.open("performance.csv", "w") do |file|
      numRuns.times do |idx|
        file.puts("Run #{idx + 1}\n,PNR,TPF Legacy Time,TPF Exist Time,NC")
        @pnrIndex.each do |pnr|
          file.puts @pnrRecords[pnr].to_csv_hammer(idx)
        end
        file.puts
      end
    end
  end

end # class Stats

#-----------------------------------------------------------------------------
# Main Program
#-----------------------------------------------------------------------------

# Run Fare Display performance summary
if ARGV.length == 3 and ARGV[0] == "-fd"
  metricsFileName, hammerFileName = ARGV[1..2]
  stats = FdStats.new
  stats.extract_hammer_out(hammerFileName)
  stats.extract_metrics(metricsFileName)
  stats.create_csv_file

# Run Pricing performance summary
elsif ARGV.length == 4
  pnrFileName, metricsFileName, hammerFileName, lniata = ARGV
  stats = Stats.new
  stats.extract_pnr_list(pnrFileName)
  stats.extract_metrics(metricsFileName, lniata)
  if hammerFileName[-3..-1] == "dmp"
    stats.extract_hammer_dmp(hammerFileName)
  else
    stats.extract_hammer_out(hammerFileName)
  end
  stats.clean_up
  stats.create_csv

# Run Pricing performance summary with missing metrics file
elsif ARGV.length == 2 && ARGV[0] != "-m"
  pnrFileName, hammerFileName = ARGV
  stats = Stats.new
  stats.extract_pnr_list(pnrFileName)
  if hammerFileName[-3..-1] == "dmp"
    stats.extract_hammer_dmp(hammerFileName)
  else
    stats.extract_hammer_out(hammerFileName)
  end
  stats.create_csv_hammer

# Run Pricing performance summary with only a metrics file
elsif ARGV.length > 1 && ARGV[0] == "-m"
  metricsFileName, lniata = ARGV[1..2]
  stats = PricingMetricsStats.new
  stats.extract_metrics(metricsFileName, lniata)
  stats.create_csv
  
elsif ARGV.length != 1 || ARGV[0] != ["-test"]
  puts "Did not receive expected command-line arguments."
  puts "* For pricing performance, provide three filenames:"
  puts "  pnr-file metrics-output hammer-output"
  puts "  followed by the LNIATA for the test."
  puts "  (if the metrics file is not available, use only two"
  puts "  arguments: pnr-file hammer-output)"
  puts "* For fare display performance, first use an argument of"
  puts '  "-fd" followed by two filenames:'
  puts "   metrics-output hammer-output"
end
