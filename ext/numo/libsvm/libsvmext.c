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
 * @return [Hash] The model obtained from the training procedure.
 */
static
VALUE train(VALUE self, VALUE x_val, VALUE y_val, VALUE param_hash)
{
  struct svm_problem* problem;
  struct svm_parameter* param;
  struct svm_model* model;
  char* err_msg;
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

  param = rb_hash_to_svm_parameter(param_hash);
  problem = dataset_to_svm_problem(x_val, y_val);

  err_msg = svm_check_parameter(problem, param);
  if (err_msg) {
    xfree_svm_problem(problem);
    xfree_svm_parameter(param);
    rb_raise(rb_eArgError, "Invalid LIBSVM parameter is given: %s", err_msg);
    return Qnil;
  }

  svm_set_print_string_function(print_null);
  model = svm_train(problem, param);
  model_hash = svm_model_to_rb_hash(model);
  svm_free_and_destroy_model(&model);

  xfree_svm_problem(problem);
  xfree_svm_parameter(param);

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
 * @return [Numo::DFloat] (shape: [n_samples]) The predicted class label or value of each sample.
 */
static
VALUE cross_validation(VALUE self, VALUE x_val, VALUE y_val, VALUE param_hash, VALUE nr_folds)
{
  const int n_folds = NUM2INT(nr_folds);
  size_t t_shape[1];
  VALUE t_val;
  double* t_pt;
  char* err_msg;
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

  svm_set_print_string_function(print_null);
  svm_cross_validation(problem, param, n_folds, t_pt);

  xfree_svm_problem(problem);
  xfree_svm_parameter(param);

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
  x_nodes = ALLOC_N(struct svm_node, n_features + 1);
  x_nodes[n_features].index = -1;
  x_nodes[n_features].value = 0.0;
  for (i = 0; i < n_samples; i++) {
    for (j = 0; j < n_features; j++) {
      x_nodes[j].index = j + 1;
      x_nodes[j].value = (double)x_pt[i * n_features + j];
    }
    y_pt[i] = svm_predict(model, x_nodes);
  }

  xfree(x_nodes);
  xfree_svm_model(model);
  xfree_svm_parameter(param);

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
    x_nodes = ALLOC_N(struct svm_node, n_features + 1);
    x_nodes[n_features].index = -1;
    x_nodes[n_features].value = 0.0;
    for (i = 0; i < n_samples; i++) {
      for (j = 0; j < n_features; j++) {
        x_nodes[j].index = j + 1;
        x_nodes[j].value = (double)x_pt[i * n_features + j];
      }
      svm_predict_values(model, x_nodes, &y_pt[i]);
    }
    xfree(x_nodes);
  } else {
    y_cols = (int)y_shape[1];
    dec_values = ALLOC_N(double, y_cols);
    x_nodes = ALLOC_N(struct svm_node, n_features + 1);
    x_nodes[n_features].index = -1;
    x_nodes[n_features].value = 0.0;
    for (i = 0; i < n_samples; i++) {
      for (j = 0; j < n_features; j++) {
        x_nodes[j].index = j + 1;
        x_nodes[j].value = (double)x_pt[i * n_features + j];
      }
      svm_predict_values(model, x_nodes, dec_values);
      for (j = 0; j < y_cols; j++) {
        y_pt[i * y_cols + j] = dec_values[j];
      }
    }
    xfree(x_nodes);
    xfree(dec_values);
  }

  xfree_svm_model(model);
  xfree_svm_parameter(param);

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
    GetNArray(x_val, x_nary);

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
    x_nodes = ALLOC_N(struct svm_node, n_features + 1);
    x_nodes[n_features].index = -1;
    x_nodes[n_features].value = 0.0;
    for (i = 0; i < n_samples; i++) {
      for (j = 0; j < n_features; j++) {
        x_nodes[j].index = j + 1;
        x_nodes[j].value = (double)x_pt[i * n_features + j];
      }
      svm_predict_probability(model, x_nodes, probs);
      for (j = 0; j < model->nr_class; j++) {
        y_pt[i * model->nr_class + j] = probs[j];
      }
    }
    xfree(x_nodes);
    xfree(probs);
  }

  xfree_svm_model(model);
  xfree_svm_parameter(param);

  return y_val;
}

/**
 * Load the SVM parameters and model from a text file with LIBSVM format.
 *
 * @param filename [String] The path to a file to load.
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
