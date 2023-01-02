/**
 * Copyright (c) 2019-2023 Atsushi Tatsuma
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "libsvmext.hpp"

extern "C" void Init_libsvmext(void) {
  rb_require("numo/narray");

  /**
   * Document-module: Numo
   * Numo is the top level namespace of NUmerical MOdules for Ruby.
   */

  /**
   * Document-module: Numo::Libsvm
   * Numo::Libsvm is a binding library for LIBSVM that handles dataset with Numo::NArray.
   */
  VALUE mLibsvm = rb_define_module_under(mNumo, "Libsvm");

  /* The version of LIBSVM used in backgroud library. */
  rb_define_const(mLibsvm, "LIBSVM_VERSION", INT2NUM(LIBSVM_VERSION));

  /**
   * Document-module: Numo::Libsvm::SvmType
   * The module consisting of constants for SVM algorithm type that used for parameter of LIBSVM.
   */
  VALUE mSvmType = rb_define_module_under(mLibsvm, "SvmType");
  /* C-SVM classification */
  rb_define_const(mSvmType, "C_SVC", INT2NUM(C_SVC));
  /* nu-SVM classification */
  rb_define_const(mSvmType, "NU_SVC", INT2NUM(NU_SVC));
  /* one-class-SVM */
  rb_define_const(mSvmType, "ONE_CLASS", INT2NUM(ONE_CLASS));
  /* epsilon-SVM regression */
  rb_define_const(mSvmType, "EPSILON_SVR", INT2NUM(EPSILON_SVR));
  /* nu-SVM regression */
  rb_define_const(mSvmType, "NU_SVR", INT2NUM(NU_SVR));

  /**
   * Document-module: Numo::Libsvm::KernelType
   * The module consisting of constants for kernel type that used for parameter of LIBSVM.
   */
  VALUE mKernelType = rb_define_module_under(mLibsvm, "KernelType");
  /* Linear kernel; u' * v */
  rb_define_const(mKernelType, "LINEAR", INT2NUM(LINEAR));
  /* Polynomial kernel; (gamma * u' * v + coef0)^degree */
  rb_define_const(mKernelType, "POLY", INT2NUM(POLY));
  /* RBF kernel; exp(-gamma * ||u - v||^2) */
  rb_define_const(mKernelType, "RBF", INT2NUM(RBF));
  /* Sigmoid kernel; tanh(gamma * u' * v + coef0) */
  rb_define_const(mKernelType, "SIGMOID", INT2NUM(SIGMOID));
  /* Precomputed kernel */
  rb_define_const(mKernelType, "PRECOMPUTED", INT2NUM(PRECOMPUTED));

  /**
   * Train the SVM model according to the given training data.
   *
   * @overload train(x, y, param) -> Hash
   *   @param x [Numo::DFloat] (shape: [n_samples, n_features]) The samples to be used for training the model.
   *   @param y [Numo::DFloat] (shape: [n_samples]) The labels or target values for samples.
   *   @param param [Hash] The parameters of an SVM model.
   *
   * @example
   *   require 'numo/libsvm'
   *
   *   # Prepare XOR data.
   *   x = Numo::DFloat[[-0.8, -0.7], [0.9, 0.8], [-0.7, 0.9], [0.8, -0.9]]
   *   y = Numo::Int32[-1, -1, 1, 1]
   *
   *   # Train C-Support Vector Classifier with RBF kernel.
   *   param = {
   *     svm_type: Numo::Libsvm::SvmType::C_SVC,
   *     kernel_type: Numo::Libsvm::KernelType::RBF,
   *     gamma: 2.0,
   *     C: 1,
   *     random_seed: 1
   *   }
   *   model = Numo::Libsvm.train(x, y, param)
   *
   *   # Predict labels of test data.
   *   x_test = Numo::DFloat[[-0.4, -0.5], [0.5, -0.4]]
   *   result = Numo::Libsvm.predict(x_test, param, model)
   *   p result
   *   # Numo::DFloat#shape=[2]
   *   # [-1, 1]
   *
   * @raise [ArgumentError] If the sample array is not 2-dimensional, the label array is not 1-dimensional,
   *   the sample array and label array do not have the same number of samples, or
   *   the hyperparameter has an invalid value, this error is raised.
   * @return [Hash] The model obtained from the training procedure.
   */
  rb_define_module_function(mLibsvm, "train", RUBY_METHOD_FUNC(numo_libsvm_train), 3);
  /**
   * Perform cross validation under given parameters. The given samples are separated to n_fols folds.
   * The predicted labels or values in the validation process are returned.
   *
   * @overload cv(x, y, param, n_folds) -> Numo::DFloat
   *   @param x [Numo::DFloat] (shape: [n_samples, n_features]) The samples to be used for training the model.
   *   @param y [Numo::DFloat] (shape: [n_samples]) The labels or target values for samples.
   *   @param param [Hash] The parameters of an SVM model.
   *   @param n_folds [Integer] The number of folds.
   *
   * @example
   *   require 'numo/libsvm'
   *
   *   # x: samples
   *   # y: labels
   *
   *   # Define parameters of C-SVC with RBF Kernel.
   *   param = {
   *     svm_type: Numo::Libsvm::SvmType::C_SVC,
   *     kernel_type: Numo::Libsvm::KernelType::RBF,
   *     gamma: 1.0,
   *     C: 1,
   *     random_seed: 1,
   *     verbose: true
   *   }
   *
   *   # Perform 5-cross validation.
   *   n_folds = 5
   *   res = Numo::Libsvm.cv(x, y, param, n_folds)
   *
   *   # Print mean accuracy.
   *   mean_accuracy = y.eq(res).count.fdiv(y.size)
   *   puts "Accuracy: %.1f %%" % (100 * mean_accuracy)
   *
   * @raise [ArgumentError] If the sample array is not 2-dimensional, the label array is not 1-dimensional,
   *   the sample array and label array do not have the same number of samples, or
   *   the hyperparameter has an invalid value, this error is raised.
   * @return [Numo::DFloat] (shape: [n_samples]) The predicted class label or value of each sample.
   */
  rb_define_module_function(mLibsvm, "cv", RUBY_METHOD_FUNC(numo_libsvm_cross_validation), 4);
  /**
   * Predict class labels or values for given samples.
   *
   * @overload predict(x, param, model) -> Numo::DFloat
   *   @param x [Numo::DFloat] (shape: [n_samples, n_features]) The samples to calculate the scores.
   *   @param param [Hash] The parameters of the trained SVM model.
   *   @param model [Hash] The model obtained from the training procedure.
   *
   * @raise [ArgumentError] If the sample array is not 2-dimensional, this error is raised.
   * @return [Numo::DFloat] (shape: [n_samples]) The predicted class label or value of each sample.
   */
  rb_define_module_function(mLibsvm, "predict", RUBY_METHOD_FUNC(numo_libsvm_predict), 3);
  /**
   * Calculate decision values for given samples.
   *
   * @overload decision_function(x, param, model) -> Numo::DFloat
   *   @param x [Numo::DFloat] (shape: [n_samples, n_features]) The samples to calculate the scores.
   *   @param param [Hash] The parameters of the trained SVM model.
   *   @param model [Hash] The model obtained from the training procedure.
   *
   * @raise [ArgumentError] If the sample array is not 2-dimensional, this error is raised.
   * @return [Numo::DFloat] (shape: [n_samples, n_classes * (n_classes - 1) / 2]) The decision value of each sample.
   */
  rb_define_module_function(mLibsvm, "decision_function", RUBY_METHOD_FUNC(numo_libsvm_decision_function), 3);
  /**
   * Predict class probability for given samples. The model must have probability information calcualted in training procedure.
   * The parameter ':probability' set to 1 in training procedure.
   *
   * @overload predict_proba(x, param, model) -> Numo::DFloat
   *   @param x [Numo::DFloat] (shape: [n_samples, n_features]) The samples to predict the class probabilities.
   *   @param param [Hash] The parameters of the trained SVM model.
   *   @param model [Hash] The model obtained from the training procedure.
   *
   * @raise [ArgumentError] If the sample array is not 2-dimensional, this error is raised.
   * @return [Numo::DFloat] (shape: [n_samples, n_classes]) Predicted probablity of each class per sample.
   */
  rb_define_module_function(mLibsvm, "predict_proba", RUBY_METHOD_FUNC(numo_libsvm_predict_proba), 3);
  /**
   * Load the SVM parameters and model from a text file with LIBSVM format.
   *
   * @overload load_svm_model(filename) -> Array
   *   @param filename [String] The path to a file to load.
   *
   * @raise [IOError] This error raises when failed to load the model file.
   * @return [Array] Array contains the SVM parameters and model.
   */
  rb_define_module_function(mLibsvm, "load_svm_model", RUBY_METHOD_FUNC(numo_libsvm_load_model), 1);
  /**
   * Save the SVM parameters and model as a text file with LIBSVM format. The saved file can be used with the libsvm tools.
   * Note that the svm_save_model saves only the parameters necessary for estimation with the trained model.
   *
   * @overload save_svm_model(filename, param, model) -> Boolean
   *   @param filename [String] The path to a file to save.
   *   @param param [Hash] The parameters of the trained SVM model.
   *   @param model [Hash] The model obtained from the training procedure.
   *
   * @raise [IOError] This error raises when failed to save the model file.
   * @return [Boolean] true on success, or false if an error occurs.
   */
  rb_define_module_function(mLibsvm, "save_svm_model", RUBY_METHOD_FUNC(numo_libsvm_save_model), 3);
}
