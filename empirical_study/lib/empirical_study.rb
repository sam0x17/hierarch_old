require 'empirical_study/version'
require 'find'
require 'rkelly'
require 'awesome_print'
require 'rtruncate'
require 'terminfo'
require 'timeout'
require 'set'

module EmpiricalStudy
  include RKelly::Nodes
  JS_DATA_DIR = './data/javascript_150k'
  JQUERY_FUNC_NAMES = ['$', 'jQuery'].freeze
  PARSE_TIMEOUT = 15 # seconds
  BAD_JQUERY_CHARS = [':', '[', ']'].freeze

  def self.parse_js_files
    unless Dir.exist? JS_DATA_DIR
      raise '150K JS dataset is missing. make sure you run get_data.sh first'
    end
    puts "scanning for .js files..."
    paths = []
    Find.find(JS_DATA_DIR) do |path|
      paths << path if File.file?(path) && path.downcase.end_with?('.js')
      print "\rfound #{paths.size} .js files"
    end
    print "\n"
    puts "parsing individual files..."
    parser = RKelly::Parser.new
    parsed_files = 0
    max_lines = 0
    total_lines = 0
    num_parse_errors = 0
    puts ""
    11.times { puts "" }
    processed = 0
    blacklist = Set.new
    blacklist_hits = 0
    start_time = Time.now
    paths.each do |path|
      processed += 1
      source_code = nil
      File.open(path, 'r') { |file| source_code = file.read }
      num_lines = source_code.lines.size
      sha1 = Digest::SHA1.base64digest source_code
      if blacklist.include? sha1
        num_parse_errors += 1
        blacklist_hits += 1
        next
      end
      elapsed = Time.now - start_time
      remaining = (elapsed / processed) * (paths.size - processed)
      # "#{(remaining / 60).floor} minutes #{(remaining - (remaining / 60).floor).round} seconds",
=begin
      print_statistics({
        'time elapsed' => display_time(elapsed),
        'time remaining' => display_time(remaining),
        'paths processed' => "#{processed}/#{paths.size} (#{(safe_div(processed, paths.size) * 100.0).round(3)}%)",
        'files parsed' => parsed_files,
        'parse errors' => num_parse_errors,
        'percent parse errors' => "#{(safe_div(num_parse_errors, paths.size) * 100.0).round(3)}%",
        'blacklist hits' => blacklist_hits,
        'lines parsed' => total_lines,
        'avg lines' => safe_div(total_lines, parsed_files).round,
        'current file' => path.gsub(JS_DATA_DIR, '').rtruncate(TermInfo.screen_size.last - 26),
        'current file lines' => num_lines
      })
=end
      begin
        Timeout::timeout(PARSE_TIMEOUT) { find_jquery_calls(source_code) }
      rescue NoMethodError, ArgumentError, RKelly::SyntaxError, RuntimeError, Timeout::Error
        num_parse_errors += 1
        blacklist.add sha1
        next
      end
      parsed_files += 1
      total_lines += num_lines
    end
    ''
  end

  def self.safe_div(numerator, divisor)
    numerator == 0 ? 0.0 : numerator.to_f / divisor.to_f
  end

  def self.print_statistics(stats={}, indent = 3)
    output = ''
    min_size = stats.keys.map { |k| k.size }.max
    (stats.size).times { output += "\r\e[A\e[K"} # move cursor up
    stats.each do |k, v|
      output += "#{" " * indent}#{" " * (min_size - k.size)}#{k}: #{v}\n"
    end
    print output
  end

  def self.display_time(seconds)
    "#{(seconds / 60).floor} minutes #{(seconds - (60 * (seconds / 60).floor)).round} seconds"
  end

  def self.find_jquery_calls(str, parser = RKelly::Parser.new)
    ast = parser.parse(str)
    calls = []
    ast.pointcut(FunctionCallNode).matches.each do |func_call|
      next unless func_call.value && func_call.value.value && JQUERY_FUNC_NAMES.include?(func_call.value.value)
      call = func_call.arguments.to_ecma
      puts "false: #{call}" if !pure_gdbt_call?(call) && candidate?(str)
      #puts " true: #{call}" if pure_gdbt_call?(call)
    end
    calls
  end

  def self.candidate?(str)
    return false unless (str.start_with?("'") && str.end_with?("'")) ||
                        (str.start_with?('"') && str.end_with?('"'))
    true
  end

  def self.pure_gdbt_call?(str)
    return false unless candidate?(str)
    BAD_JQUERY_CHARS.each { |c| return false if str.include?(c) }
    return false if str.include? ':'
    !!(/\A([#.]{0,1}[a-z_-]\w*(\s+|\z)(<\s+|\z){0,1})+\z/i.match(str[1..-2]))
  end
end
