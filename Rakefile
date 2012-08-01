require 'rake'

begin
  require 'jeweler'
  Jeweler::Tasks.new do |gem|
    gem.name        = 'swift'
    gem.summary     = %q{A rational rudimentary database abstraction.}
    gem.description = %q{A rational rudimentary database abstraction.}
    gem.email       = %w{shane.hanna@gmail.com deepfryed@gmail.com}
    gem.homepage    = 'http://github.com/shanna/swift'
    gem.authors     = ["Shane Hanna", "Bharanee 'Barney' Rathna"]
    gem.files.reject!{|f| f =~ %r{\.gitignore|examples|benchmarks|memory|gems/|Gemfile/.*}}

    gem.add_development_dependency 'minitest', '>= 1.7.0'
  end
  Jeweler::GemcutterTasks.new
rescue LoadError
  puts "Jeweler (or a dependency) not available. Install it with: gem install jeweler"
end

require 'rake/testtask'
Rake::TestTask.new(:test) do |test|
  test.libs << 'lib' << 'test'
  test.pattern = 'test/**/test_*.rb'
  test.verbose = true
end

task :default => :test

require 'yard'
YARD::Rake::YardocTask.new do |yard|
  yard.files = Dir["lib/**/*.rb"].reject {|file| %r{eventmachine|synchrony}.match(file)}
end
