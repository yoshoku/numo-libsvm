# frozen_string_literal: true

require 'bundler/setup'
require 'numo/libsvm'

if defined?(GC.verify_compaction_references) == 'method'
  GC.verify_compaction_references(double_heap: true, toward: :empty)
end

def accuracy(y_true, y_pred)
  y_pred.eq(y_true).count.fdiv(y_true.size)
end

def r2_score(y_true, y_pred)
  numerator = ((y_true - y_pred)**2).sum
  denominator = ((y_true - y_true.mean)**2).sum
  denominator.zero? ? 0.0 : 1.0 - numerator / denominator
end

RSpec.configure do |config|
  # Enable flags like --only-failures and --next-failure
  config.example_status_persistence_file_path = '.rspec_status'

  # Disable RSpec exposing methods globally on `Module` and `main`
  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end
end
