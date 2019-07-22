# Numo::Libsvm

[![Build Status](https://travis-ci.org/yoshoku/numo-libsvm.svg?branch=master)](https://travis-ci.org/yoshoku/numo-libsvm)
[![Gem Version](https://badge.fury.io/rb/numo-libsvm.svg)](https://badge.fury.io/rb/numo-libsvm)
[![BSD 3-Clause License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://github.com/yoshoku/numo-libsvm/blob/master/LICENSE.txt)

Numo::Libsvm is a Ruby gem binding to the [LIBSVM](https://github.com/cjlin1/libsvm) library.
LIBSVM is one of the famous libraries that implemented Support Vector Machines,
and provides functions for support vector classifier, regression, and distribution estimation.
Numo::Libsvm makes to use the LIBSVM functions with dataset represented by [Numo::NArray](https://github.com/ruby-numo/numo-narray).

Note: There are other useful Ruby gems binding to LIBSVM:
[rb-libsvm](https://github.com/febeling/rb-libsvm) by C. Florian Ebeling,
[libsvm-ruby-swig](https://github.com/tomz/libsvm-ruby-swig) by Tom Zeng,
and [jrb-libsvm](https://github.com/andreaseger/jrb-libsvm) by Andreas Eger.

## Installation
Numo::Libsvm does not bundle LIBSVM unlike rb-libsvm. You need to install LIBSVM in advance along your environment.

macOS:

    $ brew install libsvm

Ubuntu:

    $ sudo apt-get install libsvm-dev

Add this line to your application's Gemfile:

```ruby
gem 'numo-libsvm'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install numo-libsvm

## Usage

### Preparation

In the following examples, we use [red-datasets](https://github.com/red-data-tools/red-datasets) to download dataset.

    $ gem install red-datasets-numo-narray

### Example 1. Cross-validation

We conduct cross validation of support vector classifier on [Iris dataset](https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass.html#iris).

```ruby
require 'numo/narray'
require 'numo/libsvm'
require 'datasets-numo-narray'

# Download Iris dataset.
puts 'Download dataset.'
iris = Datasets::LIBSVM.new('iris').to_narray
x = iris[true, 1..-1]
y = iris[true, 0]

# Define parameters of C-SVC with RBF Kernel.
param = {
  svm_type: Numo::Libsvm::SvmType::C_SVC,
  kernel_type: Numo::Libsvm::KernelType::RBF,
  gamma: 1.0,
  C: 1
}

# Perform 5-cross validation.
puts 'Perform cross validation.'
n_folds = 5
predicted = Numo::Libsvm.cv(x, y, param, n_folds)

# Print mean accuracy.
mean_accuracy = y.eq(predicted).count.fdiv(y.size)
puts "Accuracy: %.1f %%" % (100 * mean_accuracy)
```

Execution result in the following:

```sh
Download dataset.
Perform cross validation.
Accuracy: 96.0 %
```

### Example 2. Pendigits dataset classification

We first train the support vector classifier with RBF kernel using training [pendigits dataset](https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass.html#pendigits).

```ruby
require 'numo/narray'
require 'numo/libsvm'
require 'datasets-numo-narray'

# Download pendigits training dataset.
puts 'Download dataset.'
pendigits = Datasets::LIBSVM.new('pendigits').to_narray
x = pendigits[true, 1..-1]
y = pendigits[true, 0]

# Define parameters of C-SVC with RBF Kernel.
param = {
  svm_type: Numo::Libsvm::SvmType::C_SVC,
  kernel_type: Numo::Libsvm::KernelType::RBF,
  gamma: 0.0001,
  C: 10,
  shrinking: true
}

# Perform 5-cross validation.
puts 'Training support vector machine.'
model = Numo::Libsvm.train(x, y, param)

# Save parameters and trained model.
puts 'Save parameters and model with Marshal.'
File.open('pendigits.dat', 'wb') { |f| f.write(Marshal.dump([param, model])) }
```

```sh
$ ruby train.rb
Download dataset.
Training support vector machine.
Save paramters and model with Marshal.
```

We then predict labels of testing dataset, and evaluate the classifier.

```ruby
require 'numo/narray'
require 'numo/libsvm'
require 'datasets-numo-narray'

# Download pendigits testing dataset.
puts 'Download dataset.'
pendigits_test = Datasets::LIBSVM.new('pendigits', note: 'testing').to_narray
x = pendigits_test[true, 1..-1]
y = pendigits_test[true, 0]

# Load parameter and model.
puts 'Load parameter and model.'
param, model = Marshal.load(File.binread('pendigits.dat'))

# Predict labels.
puts 'Predict labels.'
predicted = Numo::Libsvm.predict(x, param, model)

# Evaluate classification results.
mean_accuracy = y.eq(predicted).count.fdiv(y.size)
puts "Accuracy: %.1f %%" % (100 * mean_accuracy)
```

```sh
$ ruby test.rb
Download dataset.
Load parameter and model.
Predict labels.
Accuracy: 98.3 %
```

### Note
The hyperparameter of SVM is given with Ruby Hash on Numo::Libsvm.
The hash key of hyperparameter and its meaning match the struct svm_parameter of LIBSVM.
The svm_parameter is detailed in [LIBSVM README](https://github.com/cjlin1/libsvm/blob/master/README).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/yoshoku/numo-libsvm. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [Contributor Covenant](http://contributor-covenant.org) code of conduct.

## License

The gem is available as open source under the terms of the [BSD-3-Clause License](https://opensource.org/licenses/BSD-3-Clause).

## Code of Conduct

Everyone interacting in the Numo::Libsvm project’s codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/[USERNAME]/numo-libsvm/blob/master/CODE_OF_CONDUCT.md).
