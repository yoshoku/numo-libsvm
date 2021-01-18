/**
 * LIBSVM interface for Numo::NArray
 */
#include "libsvmext.h"

VALUE mNumo;
VALUE mLibsvm;

void print_null(const char *s) {}

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
static
VALUE train(VALUE self, VALUE x_val, VALUE y_val, VALUE param_hash)
{
  struct svm_problem* problem;
  struct svm_parameter* param;
  struct svm_model* model;
  narray_t* x_nary;
  narray_t* y_nary;
  char* err_msg;
  VALUE random_seed;
  VALUE verbose;
  VALUE model_hash;

  if (CLASS_OF(x_val) != numo_cDFloat) {
    x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  }
  if (CLASS_OF(y_val) != numo_cDFloat) {
    y_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, y_val);
  }
  if (!RTEST(nary_check_contiguous(x_val))) {
    x_val = nary_dup(x_val);
  }
  if (!RTEST(nary_check_contiguous(y_val))) {
    y_val = nary_dup(y_val);
  }

  GetNArray(x_val, x_nary);
  GetNArray(y_val, y_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }
  if (NA_NDIM(y_nary) != 1) {
    rb_raise(rb_eArgError, "Expect label or target values to be 1-D arrray.");
    return Qnil;
  }
  if (NA_SHAPE(x_nary)[0] != NA_SHAPE(y_nary)[0]) {
    rb_raise(rb_eArgError, "Expect to have the same number of samples for samples and labels.");
    return Qnil;
  }

  random_seed = rb_hash_aref(param_hash, ID2SYM(rb_intern("random_seed")));
  if (!NIL_P(random_seed)) {
    srand(NUM2UINT(random_seed));
  }

  param = rb_hash_to_svm_parameter(param_hash);
  problem = dataset_to_svm_problem(x_val, y_val);

  err_msg = svm_check_parameter(problem, param);
  if (err_msg) {
    xfree_svm_problem(problem);
    xfree_svm_parameter(param);
    rb_raise(rb_eArgError, "Invalid LIBSVM parameter is given: %s", err_msg);
    return Qnil;
  }

  verbose = rb_hash_aref(param_hash, ID2SYM(rb_intern("verbose")));
  if (verbose != Qtrue) {
    svm_set_print_string_function(print_null);
  }

  model = svm_train(problem, param);
  model_hash = svm_model_to_rb_hash(model);
  svm_free_and_destroy_model(&model);

  xfree_svm_problem(problem);
  xfree_svm_parameter(param);

  RB_GC_GUARD(x_val);
  RB_GC_GUARD(y_val);

  return model_hash;
}

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
static
VALUE cross_validation(VALUE self, VALUE x_val, VALUE y_val, VALUE param_hash, VALUE nr_folds)
{
  const int n_folds = NUM2INT(nr_folds);
  size_t t_shape[1];
  VALUE t_val;
  double* t_pt;
  narray_t* x_nary;
  narray_t* y_nary;
  char* err_msg;
  VALUE random_seed;
  VALUE verbose;
  struct svm_problem* problem;
  struct svm_parameter* param;

  if (CLASS_OF(x_val) != numo_cDFloat) {
    x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  }
  if (CLASS_OF(y_val) != numo_cDFloat) {
    y_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, y_val);
  }
  if (!RTEST(nary_check_contiguous(x_val))) {
    x_val = nary_dup(x_val);
  }
  if (!RTEST(nary_check_contiguous(y_val))) {
    y_val = nary_dup(y_val);
  }

  GetNArray(x_val, x_nary);
  GetNArray(y_val, y_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }
  if (NA_NDIM(y_nary) != 1) {
    rb_raise(rb_eArgError, "Expect label or target values to be 1-D arrray.");
    return Qnil;
  }
  if (NA_SHAPE(x_nary)[0] != NA_SHAPE(y_nary)[0]) {
    rb_raise(rb_eArgError, "Expect to have the same number of samples for samples and labels.");
    return Qnil;
  }

  random_seed = rb_hash_aref(param_hash, ID2SYM(rb_intern("random_seed")));
  if (!NIL_P(random_seed)) {
    srand(NUM2UINT(random_seed));
  }

  param = rb_hash_to_svm_parameter(param_hash);
  problem = dataset_to_svm_problem(x_val, y_val);

  err_msg = svm_check_parameter(problem, param);
  if (err_msg) {
    xfree_svm_problem(problem);
    xfree_svm_parameter(param);
    rb_raise(rb_eArgError, "Invalid LIBSVM parameter is given: %s", err_msg);
    return Qnil;
  }

  t_shape[0] = problem->l;
  t_val = rb_narray_new(numo_cDFloat, 1, t_shape);
  t_pt = (double*)na_get_pointer_for_write(t_val);

  verbose = rb_hash_aref(param_hash, ID2SYM(rb_intern("verbose")));
  if (verbose != Qtrue) {
    svm_set_print_string_function(print_null);
  }

  svm_cross_validation(problem, param, n_folds, t_pt);

  xfree_svm_problem(problem);
  xfree_svm_parameter(param);

  RB_GC_GUARD(x_val);
  RB_GC_GUARD(y_val);

  return t_val;
}

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
static
VALUE predict(VALUE self, VALUE x_val, VALUE param_hash, VALUE model_hash)
{
  struct svm_parameter* param;
  struct svm_model* model;
  struct svm_node* x_nodes;
  narray_t* x_nary;
  double* x_pt;
  size_t y_shape[1];
  VALUE y_val;
  double* y_pt;
  int i, j, k;
  int n_samples;
  int n_features;
  int n_nonzero_features;

  /* Obtain C data structures. */
  if (CLASS_OF(x_val) != numo_cDFloat) {
    x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  }
  if (!RTEST(nary_check_contiguous(x_val))) {
    x_val = nary_dup(x_val);
  }

  GetNArray(x_val, x_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }

  param = rb_hash_to_svm_parameter(param_hash);
  model = rb_hash_to_svm_model(model_hash);
  model->param = *param;

  /* Initialize some variables. */
  n_samples = (int)NA_SHAPE(x_nary)[0];
  n_features = (int)NA_SHAPE(x_nary)[1];
  y_shape[0] = n_samples;
  y_val = rb_narray_new(numo_cDFloat, 1, y_shape);
  y_pt = (double*)na_get_pointer_for_write(y_val);
  x_pt = (double*)na_get_pointer_for_read(x_val);

  /* Predict values. */
  for (i = 0; i < n_samples; i++) {
    x_nodes = dbl_vec_to_svm_node(&x_pt[i * n_features], n_features);
    y_pt[i] = svm_predict(model, x_nodes);
    xfree(x_nodes);
  }

  xfree_svm_model(model);
  xfree_svm_parameter(param);

  RB_GC_GUARD(x_val);

  return y_val;
}

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
static
VALUE decision_function(VALUE self, VALUE x_val, VALUE param_hash, VALUE model_hash)
{
  struct svm_parameter* param;
  struct svm_model* model;
  struct svm_node* x_nodes;
  narray_t* x_nary;
  double* x_pt;
  size_t y_shape[2];
  VALUE y_val;
  double* y_pt;
  double* dec_values;
  int y_cols;
  int i, j;
  int n_samples;
  int n_features;

  /* Obtain C data structures. */
  if (CLASS_OF(x_val) != numo_cDFloat) {
    x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  }
  if (!RTEST(nary_check_contiguous(x_val))) {
    x_val = nary_dup(x_val);
  }

  GetNArray(x_val, x_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }

  param = rb_hash_to_svm_parameter(param_hash);
  model = rb_hash_to_svm_model(model_hash);
  model->param = *param;

  /* Initialize some variables. */
  n_samples = (int)NA_SHAPE(x_nary)[0];
  n_features = (int)NA_SHAPE(x_nary)[1];

  if (model->param.svm_type == ONE_CLASS || model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) {
    y_shape[0] = n_samples;
    y_shape[1] = 1;
    y_val = rb_narray_new(numo_cDFloat, 1, y_shape);
  } else {
    y_shape[0] = n_samples;
    y_shape[1] = model->nr_class * (model->nr_class - 1) / 2;
    y_val = rb_narray_new(numo_cDFloat, 2, y_shape);
  }

  x_pt = (double*)na_get_pointer_for_read(x_val);
  y_pt = (double*)na_get_pointer_for_write(y_val);

  /* Predict values. */
  if (model->param.svm_type == ONE_CLASS || model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) {
    for (i = 0; i < n_samples; i++) {
      x_nodes = dbl_vec_to_svm_node(&x_pt[i * n_features], n_features);
      svm_predict_values(model, x_nodes, &y_pt[i]);
      xfree(x_nodes);
    }
  } else {
    y_cols = (int)y_shape[1];
    dec_values = ALLOC_N(double, y_cols);
    for (i = 0; i < n_samples; i++) {
      x_nodes = dbl_vec_to_svm_node(&x_pt[i * n_features], n_features);
      svm_predict_values(model, x_nodes, dec_values);
      xfree(x_nodes);
      for (j = 0; j < y_cols; j++) {
        y_pt[i * y_cols + j] = dec_values[j];
      }
    }
    xfree(dec_values);
  }

  xfree_svm_model(model);
  xfree_svm_parameter(param);

  RB_GC_GUARD(x_val);

  return y_val;
}

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
static
VALUE predict_proba(VALUE self, VALUE x_val, VALUE param_hash, VALUE model_hash)
{
  struct svm_parameter* param;
  struct svm_model* model;
  struct svm_node* x_nodes;
  narray_t* x_nary;
  double* x_pt;
  size_t y_shape[2];
  VALUE y_val = Qnil;
  double* y_pt;
  double* probs;
  int i, j;
  int n_samples;
  int n_features;

  GetNArray(x_val, x_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }

  param = rb_hash_to_svm_parameter(param_hash);
  model = rb_hash_to_svm_model(model_hash);
  model->param = *param;

  if ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) && model->probA != NULL && model->probB != NULL) {
    /* Obtain C data structures. */
    if (CLASS_OF(x_val) != numo_cDFloat) {
      x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
    }
    if (!RTEST(nary_check_contiguous(x_val))) {
      x_val = nary_dup(x_val);
    }

    /* Initialize some variables. */
    n_samples = (int)NA_SHAPE(x_nary)[0];
    n_features = (int)NA_SHAPE(x_nary)[1];
    y_shape[0] = n_samples;
    y_shape[1] = model->nr_class;
    y_val = rb_narray_new(numo_cDFloat, 2, y_shape);
    x_pt = (double*)na_get_pointer_for_read(x_val);
    y_pt = (double*)na_get_pointer_for_write(y_val);

    /* Predict values. */
    probs = ALLOC_N(double, model->nr_class);
    for (i = 0; i < n_samples; i++) {
      x_nodes = dbl_vec_to_svm_node(&x_pt[i * n_features], n_features);
      svm_predict_probability(model, x_nodes, probs);
      xfree(x_nodes);
      for (j = 0; j < model->nr_class; j++) {
        y_pt[i * model->nr_class + j] = probs[j];
      }
    }
    xfree(probs);
  }

  xfree_svm_model(model);
  xfree_svm_parameter(param);

  RB_GC_GUARD(x_val);

  return y_val;
}

/**
 * Load the SVM parameters and model from a text file with LIBSVM format.
 *
 * @param filename [String] The path to a file to load.
 * @raise [IOError] This error raises when failed to load the model file.
 * @return [Array] Array contains the SVM parameters and model.
 */
static
VALUE load_svm_model(VALUE self, VALUE filename)
{
  char* filename_ = StringValuePtr(filename);
  struct svm_model* model = svm_load_model(filename_);
  VALUE res = rb_ary_new2(2);
  VALUE param_hash = Qnil;
  VALUE model_hash = Qnil;

  if (model == NULL) {
    rb_raise(rb_eIOError, "Failed to load file '%s'", filename_);
    return Qnil;
  }

  if (model) {
    param_hash = svm_parameter_to_rb_hash(&(model->param));
    model_hash = svm_model_to_rb_hash(model);
    svm_free_and_destroy_model(&model);
  }

  rb_ary_store(res, 0, param_hash);
  rb_ary_store(res, 1, model_hash);

  RB_GC_GUARD(filename);

  return res;
}

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
static
VALUE save_svm_model(VALUE self, VALUE filename, VALUE param_hash, VALUE model_hash)
{
  char* filename_ = StringValuePtr(filename);
  struct svm_parameter* param = rb_hash_to_svm_parameter(param_hash);
  struct svm_model* model = rb_hash_to_svm_model(model_hash);
  int res;

  model->param = *param;
  res = svm_save_model(filename_, model);

  xfree_svm_model(model);
  xfree_svm_parameter(param);

  if (res < 0) {
    rb_raise(rb_eIOError, "Failed to save file '%s'", filename_);
    return Qfalse;
  }

  RB_GC_GUARD(filename);

  return Qtrue;
}

void Init_libsvmext()
{
  rb_require("numo/narray");

  /**
   * Document-module: Numo
   * Numo is the top level namespace of NUmerical MOdules for Ruby.
   */
  mNumo = rb_define_module("Numo");

  /**
   * Document-module: Numo::Libsvm
   * Numo::Libsvm is a binding library for LIBSVM that handles dataset with Numo::NArray.
   */
  mLibsvm = rb_define_module_under(mNumo, "Libsvm");

  /* The version of LIBSVM used in backgroud library. */
  rb_define_const(mLibsvm, "LIBSVM_VERSION", INT2NUM(LIBSVM_VERSION));

  rb_define_module_function(mLibsvm, "train", train, 3);
  rb_define_module_function(mLibsvm, "cv", cross_validation, 4);
  rb_define_module_function(mLibsvm, "predict", predict, 3);
  rb_define_module_function(mLibsvm, "decision_function", decision_function, 3);
  rb_define_module_function(mLibsvm, "predict_proba", predict_proba, 3);
  rb_define_module_function(mLibsvm, "load_svm_model", load_svm_model, 1);
  rb_define_module_function(mLibsvm, "save_svm_model", save_svm_model, 3);

  rb_init_svm_type_module();
  rb_init_kernel_type_module();
}
