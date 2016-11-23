require 'empirical_study/version'
require 'capybara'
require 'capybara/dsl'
require 'capybara/poltergeist'

Capybara.register_driver :poltergeist do |app|
  Capybara::Poltergeist::Driver.new(app, js_errors: false)
end

Capybara.default_driver = :poltergeist
Capybara.run_server = false


module EmpiricalStudy
  def self.start
    session = Capybara::Session.new :poltergeist
    session.visit 'https://github.com'
    session.save_and_open_screenshot
  end
end
