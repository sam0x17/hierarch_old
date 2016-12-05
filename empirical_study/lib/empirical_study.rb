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
  JQUERY_FUNC_NAMES = ['$', 'jQuery', 'find'].freeze
  PARSE_TIMEOUT = 30 # seconds
  BAD_JQUERY_CHARS = [':', '[', ']'].freeze # not allowed
  WHITELIST_JQUERY_CHARS = ['-', '_', '.'].freeze # allowed
  DEBUG = false
  UNIQUE_MODE = true

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
    15.times { puts "" }
    processed = 0
    blacklist = Set.new
    uniquelist = Set.new if UNIQUE_MODE
    uniquelist_hits = 0 if UNIQUE_MODE
    blacklist_hits = 0
    start_time = Time.now
    positive_calls = 0
    negative_calls = 0
    total_tokens = 0
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
      if UNIQUE_MODE && uniquelist.include?(sha1)
        uniquelist_hits += 1
        next
      end
      uniquelist.add(sha1) if UNIQUE_MODE
      elapsed = Time.now - start_time
      remaining = (elapsed / processed) * (paths.size - processed)
      total_calls = negative_calls + positive_calls
      print_statistics({
        'time elapsed' => display_time(elapsed),
        'time remaining' => display_time(remaining),
        'paths processed' => "#{processed}/#{paths.size} (#{(safe_div(processed, paths.size) * 100.0).round(3)}%)",
        'files parsed' => parsed_files,
        'parse errors' => num_parse_errors,
        'percent parse errors' => "#{(safe_div(num_parse_errors, paths.size) * 100.0).round(3)}%",
        'blacklist hits' => blacklist_hits,
        'uniquelist hits' => uniquelist_hits,
        'lines parsed' => total_lines,
        'avg lines' => safe_div(total_lines, parsed_files).round,
        'current file' => path.gsub(JS_DATA_DIR, '').rtruncate(TermInfo.screen_size.last - 26),
        'current file lines' => num_lines,
        'positive calls' => "#{positive_calls} (#{(safe_div(positive_calls, total_calls) * 100.0).round(3)})%",
        'negative_calls' => "#{negative_calls} (#{(safe_div(negative_calls, total_calls) * 100.0).round(3)})%",
        'avg nesting level' => (total_tokens.to_f / positive_calls).round(3)
      }) unless DEBUG
      begin
        Timeout::timeout(PARSE_TIMEOUT) do
          has_func_calls = false
          JQUERY_FUNC_NAMES.each { |name| has_func_calls = has_func_calls || source_code.downcase.include?(name.downcase) }
          # optimization: dont bother parsing if there are no jquery calls
          if has_func_calls
            calls = find_jquery_gdbt_calls(source_code)
            calls[:positive].each { |call| total_tokens += call.split(' ').size }
            positive_calls += calls[:positive].size
            negative_calls += calls[:negative].size
          end
        end
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

  def self.find_jquery_gdbt_calls(str, parser = RKelly::Parser.new)
    ast = parser.parse(str)
    positive_calls = []
    negative_calls = []
    ast.pointcut(FunctionCallNode).matches.each do |func_call|
      next unless func_call.value && func_call.value.value && JQUERY_FUNC_NAMES.include?(func_call.value.value)
      call = func_call.arguments.to_ecma
      puts "skip: #{call}" unless call.include?("'") || call.include?('"') if DEBUG
      next unless call.include?("'") || call.include?('"')
      # ^ ignore encapsulation calls like $(elem), as these aren't real DOM queries
      if pure_gdbt_call? call
        puts "good: #{call}" if DEBUG
        positive_calls << call
      else
        puts " bad: #{call}" if DEBUG
        negative_calls << call
      end
    end
    { positive: positive_calls, negative: negative_calls }
  end

  def self.enclosed_in_quotes?(str)
    return false unless (str.start_with?("'") && str.end_with?("'")) ||
                        (str.start_with?('"') && str.end_with?('"'))
    true
  end

  def self.pure_gdbt_call?(str)
    str = str.gsub(',', ' ') # pre-whitelist comma
    str = str.gsub('>', ' ') # pre-whitelist parent > child
    if str.include? '+' # handle concatenation case
      str = "'" + str.gsub('"', '').gsub("'", '').gsub(' + ', '').gsub('+', '') + "'"
      BAD_JQUERY_CHARS.each { |c| str = str.gsub(c, '') }
    end
    return false unless enclosed_in_quotes? str
    BAD_JQUERY_CHARS.each { |c| return false if str.include?(c) }
    WHITELIST_JQUERY_CHARS.each { |c| str = str.gsub(c, '') }
    !!(/\A([#.]{0,1}[a-z]\w*(\s+|\z)(<\s+|\z){0,1})+\z/i.match(str[1..-2]))
  end
end
