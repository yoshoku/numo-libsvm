# frozen_string_literal: true

RSpec.describe Numo::Libsvm do
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
end
