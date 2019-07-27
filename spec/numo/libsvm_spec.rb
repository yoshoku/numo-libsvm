# frozen_string_literal: true

RSpec.describe Numo::Libsvm do
  describe 'constant values' do
    it 'has version numbers', aggregate_failures: true do
      expect(Numo::Libsvm::VERSION).not_to be nil
      expect(Numo::Libsvm::LIBSVM_VERSION).not_to be nil
    end

    it 'has svm and kernel type values', aggregate_failures: true do
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
  end

  describe 'classification' do
    let(:dataset) { Marshal.load(File.read(__dir__ + '/../iris.dat')) }
    let(:x) { dataset[0] }
    let(:y) { dataset[1] }
    let(:x_test) { dataset[2] }
    let(:y_test) { dataset[3] }
    let(:classes) { Numo::Int32[*y.to_a.uniq] }
    let(:n_classes) { classes.size }
    let(:n_test_samples) { x_test.shape[0] }
    let(:c_svc_model) { Numo::Libsvm.train(x, y, c_svc_param) }
    let(:c_svc_param) do
      { svm_type: Numo::Libsvm::SvmType::C_SVC,
        kernel_type: Numo::Libsvm::KernelType::SIGMOID,
        gamma: 0.1,
        coef0: 1,
        C: 10,
        shrinking: true,
        probability: true }
    end

    it 'performs 5-cross validation with C-SVC' do
      pr = Numo::Libsvm.cv(x, y, c_svc_param, 5)
      expect(accuracy(y, pr)).to be_within(0.05).of(0.95)
    end

    it 'calculates decision function with C-SVC' do
      df = Numo::Libsvm.decision_function(x_test, c_svc_param, c_svc_model)
      expect(df.class).to eq(Numo::DFloat)
      expect(df.shape[0]).to eq(n_test_samples)
      expect(df.shape[1]).to eq(n_classes * (n_classes - 1) / 2)
    end

    it 'predicts probabilities with C-SVC' do
      pb = Numo::Libsvm.predict_proba(x_test, c_svc_param, c_svc_model)
      pr = Numo::Int32[*(Array.new(n_test_samples) { |n| classes[pb[n, true].max_index] })]
      expect(pb.class).to eq(Numo::DFloat)
      expect(pb.shape[0]).to eq(n_test_samples)
      expect(pb.shape[1]).to eq(n_classes)
      expect(accuracy(y_test, pr)).to be_within(0.05).of(0.95)
    end

    it 'predicts labels with C-SVC' do
      pr = Numo::Libsvm.predict(x_test, c_svc_param, c_svc_model)
      expect(pr.class).to eq(Numo::DFloat)
      expect(pr.shape[0]).to eq(n_test_samples)
      expect(pr.shape[1]).to be_nil
      expect(accuracy(y_test, pr)).to be_within(0.05).of(0.95)
    end
  end

  describe 'regression' do
    let(:dataset) { Marshal.load(File.read(__dir__ + '/../housing.dat')) }
    let(:x) { dataset[0] }
    let(:y) { dataset[1] }
    let(:x_test) { dataset[2] }
    let(:y_test) { dataset[3] }
    let(:n_test_samples) { x_test.shape[0] }
    let(:svr_model) { Numo::Libsvm.train(x, y, svr_param) }
    let(:svr_param) do
      { svm_type: Numo::Libsvm::SvmType::EPSILON_SVR,
        kernel_type: Numo::Libsvm::KernelType::RBF,
        gamma: 0.0001,
        C: 10,
        p: 0.0001,
        shrinking: true }
    end

    it 'predicts target values with SVR', aggregate_failures: true do
      pr = Numo::Libsvm.predict(x_test, svr_param, svr_model)
      expect(pr.class).to eq(Numo::DFloat)
      expect(pr.shape[0]).to eq(n_test_samples)
      expect(pr.shape[1]).to be_nil
      expect(r2_score(y_test, pr)).to be >= 0.5
    end

    it 'calculates decision function with SVR', aggregate_failures: true do
      df = Numo::Libsvm.decision_function(x_test, svr_param, svr_model)
      pr = Numo::Libsvm.predict(x_test, svr_param, svr_model)
      err = (df - pr).abs.mean
      expect(df.class).to eq(Numo::DFloat)
      expect(df.shape[0]).to eq(n_test_samples)
      expect(df.shape[1]).to be_nil
      expect(err).to be <= 1e-8
    end

    it 'performs 5-cross validation with SVR' do
      pr = Numo::Libsvm.cv(x, y, svr_param, 5)
      expect(r2_score(y, pr)).to be >= 0.1
    end
  end
end
