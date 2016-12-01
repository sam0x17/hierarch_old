require 'empirical_study/version'
require 'find'
require 'rkelly'
require 'awesome_print'

module EmpiricalStudy
  JS_DATA_DIR = './data/javascript_150k'

  def self.parse_js_files
    unless Dir.exist? JS_DATA_DIR
      raise '150K JS dataset is missing. make sure you run get_data.sh first'
    end
    puts "scanning for .js files..."
    paths = []
    Find.find(JS_DATA_DIR) do |path|
      paths << path if !File.directory?(path) && path.downcase.end_with?('.js')
      print "\rfound #{paths.size} .js files"
    end
    print "\n"
    puts "parsing individual files..."
    paths.each do |path|
      File.open(path, 'r') do |file|
        file.each_line do |line|
          puts line
          sleep 0.001
        end
      end
    end
  end
end
