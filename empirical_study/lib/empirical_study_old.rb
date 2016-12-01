require 'empirical_study/version'
require 'capybara'
require 'capybara/dsl'
require 'capybara/poltergeist'
require 'set'

Capybara.register_driver :poltergeist do |app|
  Capybara::Poltergeist::Driver.new(app, js_errors: false)
end

Capybara.default_driver = :poltergeist
Capybara.run_server = false


module EmpiricalStudy
  RATE_LIMIT_MESSAGE = 'Whoa there! You have triggered an abuse detection mechanism.'
  WAIT_TIME = 60.01

  #@@rate_limit_time = 4.0

  def self.increment_requests
    $request_num += 1
    puts $request_num
    if $request_num % 9 == 0
      puts "sleeping for #{WAIT_TIME} seconds due to request limit"
      sleep WAIT_TIME
      puts "woke up"
    end
  end

  def self.num_requests
    $request_num
  end

  def self.start
    $request_num = 0
    session = Capybara::Session.new :poltergeist
    increment_requests
    session.visit 'https://github.com/search/advanced'
    session.select 'JavaScript', from: 'search_language'
    # by default this will sort by stars since there is technically no search string
    increment_requests
    session.all('.btn')[3].click
    prev = 1
    repo_names = Set.new
    until repo_names.size > 999 || prev > 99
      backoff session do
        #session.save_and_open_screenshot(nil, full: true)
        ret = nil
        begin
          wait_until_element_or_rate_limit(session, '.pagination')
          # page is loaded as of here
          unless rate_limited? session
            puts "page: #{prev}"
            session.within '.repo-list' do
              session.all('h3').each do |element|
                puts element.text
                repo_names.add element.text
              end
            end
          end
          session.within('.pagination') { session.click_on('Next') }
          wait_until_element_or_rate_limit(session, '.pagination')
          session.within '.pagination' do
            sleep 0.05 until session.within('.current') { session.text.strip.to_i == prev + 1}
            prev += 1
          end
          ret = true
        rescue
          ret = false
        end
        ret
      end
    end
    puts "reached 1000"
    puts ''
    puts "index\trepo_name"
    repo_names.each_with_index do |name, i|
      puts "#{i}\t#{name}"
    end
    puts "total: #{repo_names.size}"
    nil
  end

  def self.backoff(session = nil, wait_time = 60.0, default_factor = 2.0, &action)
    #sleep @@rate_limit_time
    first = true
    iteration = 0
    retry_url = session.current_url if session
    increment_requests
    until action.call do
      puts "action failed, backoff triggered"
      puts "sleeping for #{wait_time} seconds"
      #@@rate_limit_time *= 1.25
      #puts "setting rate_limit_time to #{@@rate_limit_time}"
      sleep wait_time
      puts "visiting retry url (#{retry_url})" if retry_url
      increment_requests if retry_url
      session.visit retry_url if retry_url
      if session
        # wait until rate limit goes away or 20 seconds pass
        i = 0
        until i * 0.05 > 20 || !rate_limited?(session) do
          i += 1
          sleep 0.05
        end
      end
      puts 'checking for rate limit' if session
      if session && rate_limited?(session)
        puts "rate limited, backing off for #{wait_time} seconds (backoff num = #{iteration})"
      else
        puts 'no more rate limit, performing action'
        next
      end
      wait_time *= default_factor
      iteration += 1
      increment_requests
    end
  end

  def self.wait_until_element_or_rate_limit(session, klass)
    klass = ".#{klass}" unless klass.start_with?('.')
    while true
      sleep 0.05
      raise 'rate_limited' if rate_limited?(session)
      return true if session.all(:css, klass.to_s).size > 0
    end
  end

  def self.wait_until_text_or_rate_limit(session, text)
    while true
      puts "sleeping for 0.05 seconds"
      sleep 0.05
      puts "waiting for text #{text} or rate limit"
      raise 'rate_limited' if rate_limited?(session)
      return true if session.text.include? text
    end
  end

  def self.rate_limited?(session)
    res = session.text.downcase.include? RATE_LIMIT_MESSAGE.downcase
    puts "RATE LIMITED!!!!!!!!!!!" if res
    res
  end

  def self.wait_for_page_num(session, num)
    sleep 0.05 until session.within('.current') { session.text.strip.to_i == num}
  end
end
