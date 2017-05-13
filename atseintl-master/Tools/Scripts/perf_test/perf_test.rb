$: << ".."
require 'test/unit'
ARGV[0]=["-test"]
require 'perf.rb'

class ArrayTest < Test::Unit::TestCase
  def test_match_once
    a = ["abcde", "15", "3.3", "xyz", "abc"]
    assert_equal(a.match_once(/a(..)d/)[1], "bc")
    a.match_once(/x(.)z/) do |match|
      assert_equal("y", match[1])
    end
    assert_nil(a.match_once(/q(.)st/))
  end
  
  def test_match_one_float_once
    a = ["abcde", "15", "3.3", "xyz", "abc"]
    assert_equal(3.3, a.match_one_float_once(/(\d.\d)/))
    assert_equal(0, a.match_one_float_once(/(\d\d.\d\d)/))
  end
  
  def test_match_two_floats_once
    a = ["abcde", "15", "3.3, 95.4", "xyz", "abc"]
    assert_equal([3.3, 95.4], a.match_two_floats_once(/(\d+.\d+), (\d+.\d+)/))
    assert_equal([0, 0], a.match_two_floats_once(/(\d) -- (\d\d)/))
  end

  def test_match_one_int_once
    a = ["abcde", "15", "3.3", "xyz", "abc"]
    assert_equal(15, a.match_one_int_once(/(\d+)/))
    assert_equal(0, a.match_one_int_once(/(-\d+)/))
  end

  def test_match_string_once
    a = ["abcde", "15", "3.3", "xyz", "abc"]
    assert_equal("abc", a.match_string_once(/(ab.)de/, "X"))
    assert_equal("X", a.match_string_once(/(qr.)/, "X"))
  end
end


class TimeTest < Test::Unit::TestCase
  def test_parse_time
    assert_equal(Time.local("2005", "1", "1", "1", "0", "0"),
      TimeUtil.parse_time("1:00:00", false))
    assert_equal(Time.local("2005", "1", "1", "12", "0", "1"),
      TimeUtil.parse_time("12:00:01", false))
    assert_equal(Time.local("2005", "1", "1", "12", "59", "59"),
      TimeUtil.parse_time("12:59:59", false))
    assert_equal(Time.local("2005", "1", "1", "1", "0", "0"),
      TimeUtil.parse_time("010000", true))
    assert_equal(Time.local("2005", "1", "1", "12", "0", "1"),
      TimeUtil.parse_time("120001", true))
    assert_equal(Time.local("2005", "1", "1", "12", "59", "59"),
      TimeUtil.parse_time("125959", true))
  end

  def test_parse_time_input
    assert_raise(RuntimeError) { TimeUtil.parse_time("xxxxx", false) }
    assert_raise(RuntimeError) { TimeUtil.parse_time("125959", false) }
    assert_raise(RuntimeError) { TimeUtil.parse_time("12:59:59", true) }
    assert_nothing_raised() { TimeUtil.parse_time("125959", true) }
    assert_nothing_raised() { TimeUtil.parse_time("12:59:59", false) }
  end

  def test_calc_time_span
    assert_equal(1, TimeUtil.calc_time_span("1:00:15", "1:00:16"))
    assert_equal(1, TimeUtil.calc_time_span("010015", "010016", true))
    assert_equal(10, TimeUtil.calc_time_span("12:59:55", "1:00:05"))
  end
end


class ParseUtilTest < Test::Unit::TestCase
  def test_extract_records
    separator = /=RECORD=/
    lineCount = 4
    recordCount = 0
    ParseUtil.extract_records("extract_records_test_data.txt", separator) do
      |lines|
      assert_equal(lineCount, lines.size)
      lineCount += 1
      recordCount += 1
    end
    assert_equal(3, recordCount)
  end

  def test_extract_records_dual
    separator1 = /=RECORD=/
    separator2 = /--------/
    lineCount = 9
    recordCount = 0
    ParseUtil.extract_records_dual("extract_records_dual_test_data.txt",
                                   separator1, separator2) do
      |lines|
      assert_equal(lineCount, lines.size)
      lineCount += 1
      recordCount += 1
    end
    assert_equal(2, recordCount)
  end

end


class MetricsTest < Test::Unit::TestCase

  def test_new_metric
    lines = File.readlines("metric_block.txt")
    metric = Metric.new(lines)
    assert_equal("MZVTAG", metric.pnr)
    assert_equal("WPBET-ITL$S3/8$Q/*499/FCBLWAP3M+B*", metric.entry)
    assert_equal("E1N0", metric.pcc)
    assert_equal("191920347254021571", metric.transId)
    assert_equal("546BF7", metric.lniata)
    assert_equal("15:04:51", metric.timeStamp)
    assert_equal(4.86, metric.atseElapsedTime)
    assert_equal(0.3, metric.atseCpuTime)
    assert_equal(0.13, metric.ataeTime)
    assert_equal(0.17, metric.toItinElapsedTime)
    assert_equal(0.0, metric.toItinCpuTime)
    assert_equal(4.48, metric.toFareCElapsedTime)
    assert_equal(0.0, metric.toFareCCpuTime)
    assert_equal(0.21, metric.toFareVElapsedTime)
    assert_equal(0.0, metric.toFareVCpuTime)
    assert_equal(0.0, metric.toPricingElapsedTime)
    assert_equal(0.0, metric.toPricingCpuTime)
    assert_equal(0.0, metric.toTaxElapsedTime)
    assert_equal(0.0, metric.toTaxCpuTime)
    assert_equal(0.0, metric.toFareCalcElapsedTime)
    assert_equal(0.0, metric.toFareCalcCpuTime)
    assert_equal(2, metric.travelSegs)
    assert_equal(2, metric.fareMarkets)
    assert_equal(1, metric.paxTypes)
    assert_equal(164, metric.paxTypeFares)
    assert_equal(1112, metric.virtualMemSize)
    assert_equal(0.01, metric.asv2ElapsedTime)
    assert_equal(0.01, metric.dssv2ElapsedTime)
    assert_equal(0.1099, metric.baggageElapsedTime)
    assert_equal(0.0, metric.billingElapsedTime)
  end

  def test_metric_to_csv
    lines = File.readlines("metric_block.txt")
    metric = Metric.new(lines)
    expectedCsv1 = "2,2,1,164,1112"
    expectedCsv2 = "4.86,0.3,0.13,4.73"
    expectedCsv3 = "0.17,0.0,4.48,0.0,0.21,0.0,0,0,0,0,0,0,0.01,0.01,0.1099,0.0"
    assert_equal(expectedCsv1, metric.to_csv_1)
    assert_equal(expectedCsv2, metric.to_csv_2)
    assert_equal(expectedCsv3, metric.to_csv_3)
  end

end


class HammerEntryTest < Test::Unit::TestCase

  def test_hammer_entry_new
    entry = HammerEntry.new("ABCDEF", "WP", "15:04:15", 4)
    assert_equal("ABCDEF- 15:04:15 WP duration:4 \n", entry.to_s)
    assert(entry.legacy)
    entry = HammerEntry.new("GHIJKL", "WPBET-ITL", "01:02:03", 5)
    assert_equal("GHIJKL- 01:02:03 WPBET-ITL duration:5 \n", entry.to_s)
    assert(!entry.legacy)
    entry = HammerEntry.new("MNOPQR", "WPBET-ITL‡NC", "07:30:55", 7.2)
    assert_equal("MNOPQR- 07:30:55 WPBET-ITL$NC duration:7.2 NC\n", entry.to_s)
    assert(!entry.legacy)
  end

end


class PnrRecordTest < Test::Unit::TestCase

  def test_pnr_entry_record_new
    record = PnrRecord.new("ABCDEF")
    assert_equal(0, record.metrics.size)
    assert_equal(0, record.hammers.size)
    assert_equal(0, record.runCount)
    assert_nil(record.to_csv(1))
  end

  def test_add_run
    record = PnrRecord.new("ABCDEF")
    assert_equal(0, record.metrics.size)
    assert_equal(0, record.hammers.size)
    assert_equal(0, record.runCount)
    assert_nil(record.to_csv(1))
    record.add_runs(1)
    record.add_metric(Metric.new(File.readlines("metric_block.txt")))
    record.add_hammer(HammerEntry.new("ABCDEF", "WP", "15:04:15", 4))
    record.add_hammer(HammerEntry.new("ABCDEF", "WPBET-ITL", "15:04:56", 6))
    assert_equal(1, record.metrics.size)
    assert_equal(1, record.hammers.size)
    assert_equal(1, record.runCount)
    expectedCsv = ",ABCDEF,4,2,2,1,164,1112,6,1.14,4.86,0.3,0.13,4.73,,,0.17,0.0,4.48,0.0,0.21,0.0,0,0,0,0,0,0,0.01,0.01,0.1099,0.0"
    assert_equal(expectedCsv, record.to_csv(0))
  end
    

end



class FdMetricsTest < Test::Unit::TestCase

  def test_extract_hammer_out
    stats = FdStats.new
    numHammers = stats.extract_hammer_out("fd_hammers.out")
    assert_equal(2, numHammers)
  end

  def test_new_entry_record
    entryRecord = FdEntryRecord.new(["FQB*TYOOSAAD-JL/JPY", "14:36:16", "14:36:16"])
    assert_equal("FQB*TYOOSAAD-JL/JPY", entryRecord.entry)
    assert_equal(0, entryRecord.duration)
    entryRecord = FdEntryRecord.new(["FQB*TYOOSA-JL/JPY", "14:36:11", "14:36:16"])
    assert_equal("FQB*TYOOSA-JL/JPY", entryRecord.entry)
    assert_equal(5, entryRecord.duration)
    entryRecord = FdEntryRecord.new(["FQB*DXBLON-EK‡BM", "14:37:50", "14:38:33"])
    assert_equal("FQB*DXBLON-EK‡BM", entryRecord.entry)
    assert_equal(43, entryRecord.duration)
  end

  def test_extract_metrics
    stats = FdStats.new
    numMetrics = stats.extract_metrics("fd_metrics.txt")
    assert_equal(2, numMetrics)
  end

  def test_new_metric
    lines = File.readlines("fd_metric_block.txt")
    metric = FdMetric.new(lines)
    assert_equal(3.56, metric.atseElapsedTime)
    assert_equal(0.01, metric.atseCpuTime)
    assert_equal(3.56, metric.toFareDisplayProcessElapsedTime)
    assert_equal(0.0, metric.toItinElapsedTime)
    assert_equal(2.29, metric.toFareCElapsedTime)
    assert_equal(0.0, metric.toFareSelectorElapsedTime)
    assert_equal(0.01, metric.toFareVElapsedTime)
    assert_equal(0.0, metric.toTaxElapsedTime)
    assert_equal(1.26, metric.toFareDisplayElapsedTime)
    assert_equal(0.0, metric.toReqrspElapsedTime)
    assert_equal(1, metric.travelSegments)
    assert_equal(1, metric.fareMarkets)
    assert_equal(77, metric.paxTypes)
    assert_equal(34, metric.paxTypeFares)
    assert_equal(613, metric.virtualMemSize)
    assert_equal(351, metric.requestSize)
    assert_equal(970, metric.responseSize)
  end

  def test_metric_to_csv
    lines = File.readlines("fd_metric_block.txt")
    metric = FdMetric.new(lines)
    expectedCsv = "3.56,0.01,3.56,0.01,0.0,0.0,2.29,0.01,0.01,0.0,0.0,0.0,0.0,0.0,1.26,0.0,0.0,0.0,1,1,77,34,613,351,970"
    assert_equal(expectedCsv, metric.to_csv)
  end

  def test_to_csv
    stats = FdStats.new
    numHammers = stats.extract_hammer_out("fd_hammers.out")
    assert_equal(2, numHammers)
    numMetrics = stats.extract_metrics("fd_metrics.txt")
    assert_equal(2, numMetrics)
    csv = stats.to_csv
    expectedCsv = "FQB*TYOOSA-JL/JPY,0.0,3.56,0.01,3.56,0.01,0.0,0.0,2.29,0.01,0.01,0.0,0.0,0.0,0.0,0.0,1.26,0.0,0.0,0.0,1,1,77,34,613,351,970"
    assert_equal(expectedCsv, csv[0])
    expectedCsv = "RD3,3.0,4.48,0.0,4.48,0.0,0.0,0.0,0,0,0,0,0.0,0.0,0,0,4.48,0.0,0.0,0.0,1,1,1,0,623,360,7970"
    assert_equal(expectedCsv, csv[1])
  end

end
