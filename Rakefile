require 'bundler/gem_tasks'
require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(:spec)

require 'rake/extensiontask'

task :build => :compile

Rake::ExtensionTask.new('libsvm') do |ext|
  ext.ext_dir = 'ext/numo/libsvm'
  ext.lib_dir = 'lib/numo/libsvm'
end

task :default => [:clobber, :compile, :spec]
