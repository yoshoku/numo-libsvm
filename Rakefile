# frozen_string_literal: true

require 'bundler/gem_tasks'
require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(spec: :compile)
if ENV['BUNDLE_WITH'] == 'memcheck'
  require 'ruby_memcheck'
  require 'ruby_memcheck/rspec/rake_task'

  namespace :spec do
    RubyMemcheck::RSpec::RakeTask.new(valgrind: :compile)
  end
end

require 'rake/extensiontask'

task build: :compile # rubocop:disable Rake/Desc

desc 'Run clang-format'
task 'clang-format' do
  sh 'clang-format -style=file -Werror --dry-run ext/numo/libsvm/*.cpp ext/numo/libsvm/*.hpp'
end

Rake::ExtensionTask.new('libsvmext') do |ext|
  ext.ext_dir = 'ext/numo/libsvm'
  ext.lib_dir = 'lib/numo/libsvm'
end

task default: %i[clobber compile spec]
