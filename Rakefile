# frozen_string_literal: true

require 'bundler/gem_tasks'
require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(:spec)

require 'rubocop/rake_task'

RuboCop::RakeTask.new

require 'rake/extensiontask'

task build: :compile # rubocop:disable Rake/Desc

Rake::ExtensionTask.new('libsvmext') do |ext|
  ext.ext_dir = 'ext/numo/libsvm'
  ext.lib_dir = 'lib/numo/libsvm'
end

task default: %i[clobber compile rubocop spec]
