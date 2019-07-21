# frozen_string_literal: true

RSpec.describe Numo::Libsvm do
  let(:pendigits) { Marshal.load(File.read(__dir__ + '/../pendigits.dat')) }
  let(:x) { pendigits[0] }
  let(:y) { pendigits[1] }
  let(:x_test) { pendigits[2] }
  let(:y_test) { pendigits[3] }
  let(:classes) { Numo::Int32[*y.to_a.uniq] }
  let(:n_classes) { classes.size }
  let(:n_samples) { x_test.shape[0] }
  let(:c_svc_param) do
    { svm_type: Numo::Libsvm::SvmType::C_SVC,
      kernel_type: Numo::Libsvm::KernelType::RBF,
      gamma: 0.0001,
      C: 10,
      shrinking: true,
      probability: true }
  end
  let(:c_svc_model) { Numo::Libsvm.train(x, y, c_svc_param) }

  it 'has some version numbers' do
    expect(Numo::Libsvm::VERSION).not_to be nil
    expect(Numo::Libsvm::LIBSVM_VERSION).not_to be nil
  end

  it 'has some constant values', aggregate_failures: true do
    expect(Numo::Libsvm::SvmType::C_SVC).to eq(0)
    expect(Numo::Libsvm::SvmType::NU_SVC).to eq(1)
    expect(Numo::Libsvm::SvmType::ONE_CLASS).to eq(2)
    expect(Numo::Libsvm::SvmType::EPSILON_SVR).to eq(3)
    expect(Numo::Libsvm::SvmType::NU_SVR).to eq(4)
    expect(Numo::Libsvm::KernelType::LINEAR).to eq(0)
    expect(Numo::Libsvm::KernelType::POLY).to eq(1)
    expect(Numo::Libsvm::KernelType::RBF).to eq(2)
    expect(Numo::Libsvm::KernelType::SIGMOID).to eq(3)
    expect(Numo::Libsvm::KernelType::PRECOMPUTED).to eq(4)
  end

  it 'performs 5-cross validation with C-SVC' do
    predicted = Numo::Libsvm.cv(x, y, c_svc_param, 5)
    accuracy = predicted.eq(y).count.fdiv(y.size)
    expect(accuracy).to be_within(0.05).of(0.95)
  end

  it 'calculates decision function with C-SVC' do
    func_vals = Numo::Libsvm.decision_function(x_test, c_svc_param, c_svc_model)
    expect(func_vals.class).to eq(Numo::DFloat)
    expect(func_vals.shape[0]).to eq(n_samples)
    expect(func_vals.shape[1]).to eq(n_classes * (n_classes - 1) / 2)
  end

  it 'predicts probabilities with C-SVC' do
    probs = Numo::Libsvm.predict_proba(x_test, c_svc_param, c_svc_model)
    predicted = Numo::Int32[*(Array.new(n_samples) { |n| classes[probs[n, true].max_index] })]
    accuracy = predicted.eq(y_test).count.fdiv(n_samples)
    expect(probs.class).to eq(Numo::DFloat)
    expect(probs.shape[0]).to eq(n_samples)
    expect(probs.shape[1]).to eq(n_classes)
    expect(accuracy).to be_within(0.05).of(0.95)
  end

  it 'predicts labels with C-SVC' do
    predicted = Numo::Libsvm.predict(x_test, c_svc_param, c_svc_model)
    accuracy = predicted.eq(y_test).count.fdiv(n_samples)
    expect(predicted.class).to eq(Numo::DFloat)
    expect(predicted.shape[0]).to eq(n_samples)
    expect(predicted.shape[1]).to be_nil
    expect(accuracy).to be_within(0.05).of(0.95)
  end
end
