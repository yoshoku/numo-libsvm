/**
 * Copyright (c) 2019-2022 Atsushi Tatsuma
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

#ifndef LIBSVMEXT_HPP
#define LIBSVMEXT_HPP 1

#include <cmath>
#include <cstring>

#include <ruby.h>

#include <numo/narray.h>
#include <numo/template.h>

#include <svm.h>

typedef struct svm_model LibSvmModel;
typedef struct svm_node LibSvmNode;
typedef struct svm_parameter LibSvmParameter;
typedef struct svm_problem LibSvmProblem;

void printNull(const char* s) {}

/** CONVERTERS */
VALUE convertVectorXiToNArray(const int* const arr, const int size) {
  size_t shape[1] = {(size_t)size};
  VALUE vec_val = rb_narray_new(numo_cInt32, 1, shape);
  int32_t* vec_ptr = (int32_t*)na_get_pointer_for_write(vec_val);
  for (int i = 0; i < size; i++) vec_ptr[i] = (int32_t)arr[i];
  return vec_val;
}

int* convertNArrayToVectorXi(VALUE vec_val) {
  if (NIL_P(vec_val)) return NULL;

  narray_t* vec_nary;
  GetNArray(vec_val, vec_nary);
  const size_t n_elements = NA_SHAPE(vec_nary)[0];
  int* arr = ALLOC_N(int, n_elements);
  const int32_t* const vec_ptr = (int32_t*)na_get_pointer_for_read(vec_val);
  for (size_t i = 0; i < n_elements; i++) arr[i] = (int)vec_ptr[i];

  RB_GC_GUARD(vec_val);

  return arr;
}

VALUE convertVectorXdToNArray(const double* const arr, const int size) {
  size_t shape[1] = {(size_t)size};
  VALUE vec_val = rb_narray_new(numo_cDFloat, 1, shape);
  double* vec_ptr = (double*)na_get_pointer_for_write(vec_val);
  for (int i = 0; i < size; i++) vec_ptr[i] = arr[i];
  return vec_val;
}

double* convertNArrayToVectorXd(VALUE vec_val) {
  if (NIL_P(vec_val)) return NULL;

  narray_t* vec_nary;
  GetNArray(vec_val, vec_nary);
  const size_t n_elements = NA_SHAPE(vec_nary)[0];
  double* arr = ALLOC_N(double, n_elements);
  const double* const vec_ptr = (double*)na_get_pointer_for_read(vec_val);
  memcpy(arr, vec_ptr, n_elements * sizeof(double));

  RB_GC_GUARD(vec_val);

  return arr;
}

VALUE convertMatrixXdToNArray(const double* const* mat, const int n_rows, const int n_cols) {
  size_t shape[2] = {(size_t)n_rows, (size_t)n_cols};
  VALUE mat_val = rb_narray_new(numo_cDFloat, 2, shape);
  double* mat_ptr = (double*)na_get_pointer_for_write(mat_val);
  for (int i = 0; i < n_rows; i++) memcpy(&mat_ptr[i * n_cols], mat[i], n_cols * sizeof(double));
  return mat_val;
}

double** convertNArrayToMatrixXd(VALUE mat_val) {
  if (NIL_P(mat_val)) return NULL;

  narray_t* mat_nary;
  GetNArray(mat_val, mat_nary);
  const size_t n_rows = NA_SHAPE(mat_nary)[0];
  const size_t n_cols = NA_SHAPE(mat_nary)[1];
  const double* const mat_ptr = (double*)na_get_pointer_for_read(mat_val);
  double** mat = ALLOC_N(double*, n_rows);
  for (size_t i = 0; i < n_rows; i++) {
    mat[i] = ALLOC_N(double, n_cols);
    memcpy(mat[i], &mat_ptr[i * n_cols], n_cols * sizeof(double));
  }

  RB_GC_GUARD(mat_val);

  return mat;
}

VALUE convertLibSvmNodeToNArray(const LibSvmNode* const* support_vecs, const int n_support_vecs) {
  int n_dimensions = 0;
  for (int i = 0; i < n_support_vecs; i++) {
    for (int j = 0; support_vecs[i][j].index != -1; j++) {
      if (n_dimensions < support_vecs[i][j].index) {
        n_dimensions = support_vecs[i][j].index;
      }
    }
  }

  size_t shape[2] = {(size_t)n_support_vecs, (size_t)n_dimensions};
  VALUE vec_val = rb_narray_new(numo_cDFloat, 2, shape);
  double* vec_ptr = (double*)na_get_pointer_for_write(vec_val);
  memset(vec_ptr, 0, n_support_vecs * n_dimensions * sizeof(double));
  for (int i = 0; i < n_support_vecs; i++) {
    for (int j = 0; support_vecs[i][j].index != -1; j++) {
      vec_ptr[i * n_dimensions + support_vecs[i][j].index - 1] = support_vecs[i][j].value;
    }
  }

  return vec_val;
}

LibSvmNode** convertNArrayToLibSvmNode(VALUE vec_val) {
  if (NIL_P(vec_val)) return NULL;

  narray_t* vec_nary;
  GetNArray(vec_val, vec_nary);
  const size_t n_rows = NA_SHAPE(vec_nary)[0];
  const size_t n_cols = NA_SHAPE(vec_nary)[1];
  const double* const vec_ptr = (double*)na_get_pointer_for_read(vec_val);
  LibSvmNode** support_vecs = ALLOC_N(LibSvmNode*, n_rows);
  for (size_t i = 0; i < n_rows; i++) {
    int n_nonzero_cols = 0;
    for (size_t j = 0; j < n_cols; j++) {
      if (vec_ptr[i * n_cols + j] != 0) {
        n_nonzero_cols++;
      }
    }
    support_vecs[i] = ALLOC_N(LibSvmNode, n_nonzero_cols + 1);
    for (size_t j = 0, k = 0; j < n_cols; j++) {
      if (vec_ptr[i * n_cols + j] != 0) {
        support_vecs[i][k].index = j + 1;
        support_vecs[i][k].value = vec_ptr[i * n_cols + j];
        k++;
      }
    }
    support_vecs[i][n_nonzero_cols].index = -1;
    support_vecs[i][n_nonzero_cols].value = 0.0;
  }

  RB_GC_GUARD(vec_val);

  return support_vecs;
}

LibSvmNode* convertVectorXdToLibSvmNode(const double* const arr, const int size) {
  int n_nonzero_elements = 0;
  for (int i = 0; i < size; i++) {
    if (arr[i] != 0.0) n_nonzero_elements++;
  }

  LibSvmNode* node = ALLOC_N(LibSvmNode, n_nonzero_elements + 1);
  for (int i = 0, j = 0; i < size; i++) {
    if (arr[i] != 0.0) {
      node[j].index = i + 1;
      node[j].value = arr[i];
      j++;
    }
  }
  node[n_nonzero_elements].index = -1;
  node[n_nonzero_elements].value = 0.0;

  return node;
}

LibSvmModel* convertHashToLibSvmModel(VALUE model_hash) {
  LibSvmModel* model = ALLOC(LibSvmModel);
  VALUE el;
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("nr_class")));
  model->nr_class = !NIL_P(el) ? NUM2INT(el) : 0;
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("l")));
  model->l = !NIL_P(el) ? NUM2INT(el) : 0;
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("SV")));
  model->SV = convertNArrayToLibSvmNode(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("sv_coef")));
  model->sv_coef = convertNArrayToMatrixXd(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("rho")));
  model->rho = convertNArrayToVectorXd(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("probA")));
  model->probA = convertNArrayToVectorXd(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("probB")));
  model->probB = convertNArrayToVectorXd(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("sv_indices")));
  model->sv_indices = convertNArrayToVectorXi(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("label")));
  model->label = convertNArrayToVectorXi(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("nSV")));
  model->nSV = convertNArrayToVectorXi(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("free_sv")));
  model->free_sv = !NIL_P(el) ? NUM2INT(el) : 0;
  return model;
}

VALUE convertLibSvmModelToHash(const LibSvmModel* const model) {
  const int n_classes = model->nr_class;
  const int n_support_vecs = model->l;
  VALUE support_vecs = model->SV ? convertLibSvmNodeToNArray(model->SV, n_support_vecs) : Qnil;
  VALUE coefficients = model->sv_coef ? convertMatrixXdToNArray(model->sv_coef, n_classes - 1, n_support_vecs) : Qnil;
  VALUE intercepts = model->rho ? convertVectorXdToNArray(model->rho, n_classes * (n_classes - 1) / 2) : Qnil;
  VALUE prob_alpha = model->probA ? convertVectorXdToNArray(model->probA, n_classes * (n_classes - 1) / 2) : Qnil;
  VALUE prob_beta = model->probB ? convertVectorXdToNArray(model->probB, n_classes * (n_classes - 1) / 2) : Qnil;
  VALUE sv_indices = model->sv_indices ? convertVectorXiToNArray(model->sv_indices, n_support_vecs) : Qnil;
  VALUE labels = model->label ? convertVectorXiToNArray(model->label, n_classes) : Qnil;
  VALUE n_support_vecs_each_class = model->nSV ? convertVectorXiToNArray(model->nSV, n_classes) : Qnil;
  VALUE model_hash = rb_hash_new();
  rb_hash_aset(model_hash, ID2SYM(rb_intern("nr_class")), INT2NUM(n_classes));
  rb_hash_aset(model_hash, ID2SYM(rb_intern("l")), INT2NUM(n_support_vecs));
  rb_hash_aset(model_hash, ID2SYM(rb_intern("SV")), support_vecs);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("sv_coef")), coefficients);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("rho")), intercepts);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("probA")), prob_alpha);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("probB")), prob_beta);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("sv_indices")), sv_indices);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("label")), labels);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("nSV")), n_support_vecs_each_class);
  rb_hash_aset(model_hash, ID2SYM(rb_intern("free_sv")), INT2NUM(model->free_sv));
  return model_hash;
}

LibSvmParameter* convertHashToLibSvmParameter(VALUE param_hash) {
  LibSvmParameter* param = ALLOC(LibSvmParameter);
  VALUE el;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("svm_type")));
  param->svm_type = !NIL_P(el) ? NUM2INT(el) : C_SVC;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("kernel_type")));
  param->kernel_type = !NIL_P(el) ? NUM2INT(el) : RBF;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("degree")));
  param->degree = !NIL_P(el) ? NUM2INT(el) : 3;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("gamma")));
  param->gamma = !NIL_P(el) ? NUM2DBL(el) : 1;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("coef0")));
  param->coef0 = !NIL_P(el) ? NUM2DBL(el) : 0;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("cache_size")));
  param->cache_size = !NIL_P(el) ? NUM2DBL(el) : 100;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("eps")));
  param->eps = !NIL_P(el) ? NUM2DBL(el) : 1e-3;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("C")));
  param->C = !NIL_P(el) ? NUM2DBL(el) : 1;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("nr_weight")));
  param->nr_weight = !NIL_P(el) ? NUM2INT(el) : 0;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("nu")));
  param->nu = !NIL_P(el) ? NUM2DBL(el) : 0.5;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("p")));
  param->p = !NIL_P(el) ? NUM2DBL(el) : 0.1;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("shrinking")));
  param->shrinking = RB_TYPE_P(el, T_FALSE) ? 0 : 1;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("probability")));
  param->probability = RB_TYPE_P(el, T_TRUE) ? 1 : 0;
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("weight_label")));
  param->weight_label = NULL;
  if (!NIL_P(el)) {
    param->weight_label = ALLOC_N(int, param->nr_weight);
    memcpy(param->weight_label, (int32_t*)na_get_pointer_for_read(el), param->nr_weight * sizeof(int32_t));
  }
  el = rb_hash_aref(param_hash, ID2SYM(rb_intern("weight")));
  param->weight = NULL;
  if (!NIL_P(el)) {
    param->weight = ALLOC_N(double, param->nr_weight);
    memcpy(param->weight, (double*)na_get_pointer_for_read(el), param->nr_weight * sizeof(double));
  }
  return param;
}

VALUE convertLibSvmParameterToHash(const LibSvmParameter* const param) {
  VALUE param_hash = rb_hash_new();
  rb_hash_aset(param_hash, ID2SYM(rb_intern("svm_type")), INT2NUM(param->svm_type));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("kernel_type")), INT2NUM(param->kernel_type));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("degree")), INT2NUM(param->degree));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("gamma")), DBL2NUM(param->gamma));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("coef0")), DBL2NUM(param->coef0));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("cache_size")), DBL2NUM(param->cache_size));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("eps")), DBL2NUM(param->eps));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("C")), DBL2NUM(param->C));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("nr_weight")), INT2NUM(param->nr_weight));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("nu")), DBL2NUM(param->nu));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("p")), DBL2NUM(param->p));
  rb_hash_aset(param_hash, ID2SYM(rb_intern("shrinking")), param->shrinking == 1 ? Qtrue : Qfalse);
  rb_hash_aset(param_hash, ID2SYM(rb_intern("probability")), param->probability == 1 ? Qtrue : Qfalse);
  rb_hash_aset(param_hash, ID2SYM(rb_intern("weight_label")),
               param->weight_label ? convertVectorXiToNArray(param->weight_label, param->nr_weight) : Qnil);
  rb_hash_aset(param_hash, ID2SYM(rb_intern("weight")),
               param->weight ? convertVectorXdToNArray(param->weight, param->nr_weight) : Qnil);
  return param_hash;
}

LibSvmProblem* convertDatasetToLibSvmProblem(VALUE x_val, VALUE y_val) {
  narray_t* x_nary;
  GetNArray(x_val, x_nary);
  const int n_samples = (int)NA_SHAPE(x_nary)[0];
  const int n_features = (int)NA_SHAPE(x_nary)[1];
  const double* const x_ptr = (double*)na_get_pointer_for_read(x_val);
  const double* const y_ptr = (double*)na_get_pointer_for_read(y_val);

  LibSvmProblem* problem = ALLOC(LibSvmProblem);
  problem->l = n_samples;
  problem->x = ALLOC_N(LibSvmNode*, n_samples);
  problem->y = ALLOC_N(double, n_samples);

  int last_feature_id = 0;
  bool is_padded = false;
  for (int i = 0; i < n_samples; i++) {
    int n_nonzero_features = 0;
    for (int j = 0; j < n_features; j++) {
      if (x_ptr[i * n_features + j] != 0.0) {
        n_nonzero_features += 1;
        last_feature_id = j + 1;
      }
    }
    if (!is_padded && last_feature_id == n_features) is_padded = true;
    if (is_padded) {
      problem->x[i] = ALLOC_N(LibSvmNode, n_nonzero_features + 1);
    } else {
      problem->x[i] = ALLOC_N(LibSvmNode, n_nonzero_features + 2);
    }
    for (int j = 0, k = 0; j < n_features; j++) {
      if (x_ptr[i * n_features + j] != 0.0) {
        problem->x[i][k].index = j + 1;
        problem->x[i][k].value = x_ptr[i * n_features + j];
        k++;
      }
    }
    if (is_padded) {
      problem->x[i][n_nonzero_features].index = -1;
      problem->x[i][n_nonzero_features].value = 0.0;
    } else {
      problem->x[i][n_nonzero_features].index = n_features;
      problem->x[i][n_nonzero_features].value = 0.0;
      problem->x[i][n_nonzero_features + 1].index = -1;
      problem->x[i][n_nonzero_features + 1].value = 0.0;
    }
    problem->y[i] = y_ptr[i];
  }

  RB_GC_GUARD(x_val);
  RB_GC_GUARD(y_val);

  return problem;
}

/** UTILITIES */
bool isSignleOutputModel(LibSvmModel* model) {
  return (model->param.svm_type == ONE_CLASS || model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR);
}

bool isProbabilisticModel(LibSvmModel* model) {
  return ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) && model->probA != NULL && model->probB != NULL);
}

void xfreeLibSvmModel(LibSvmModel* model) {
  if (model) {
    if (model->SV) {
      for (int i = 0; i < model->l; i++) xfree(model->SV[i]);
      xfree(model->SV);
      model->SV = NULL;
    }
    if (model->sv_coef) {
      for (int i = 0; i < model->nr_class - 1; i++) xfree(model->sv_coef[i]);
      xfree(model->sv_coef);
      model->sv_coef = NULL;
    }
    xfree(model->rho);
    model->rho = NULL;
    xfree(model->probA);
    model->probA = NULL;
    xfree(model->probB);
    model->probB = NULL;
    xfree(model->sv_indices);
    model->sv_indices = NULL;
    xfree(model->label);
    model->label = NULL;
    xfree(model->nSV);
    model->nSV = NULL;
    xfree(model);
    model = NULL;
  }
}

void xfreeLibSvmParameter(LibSvmParameter* param) {
  if (param) {
    if (param->weight_label) {
      xfree(param->weight_label);
      param->weight_label = NULL;
    }
    if (param->weight) {
      xfree(param->weight);
      param->weight = NULL;
    }
    xfree(param);
    param = NULL;
  }
}

void xfreeLibSvmProblem(LibSvmProblem* problem) {
  if (problem) {
    if (problem->x) {
      for (int i = 0; i < problem->l; i++) {
        if (problem->x[i]) {
          xfree(problem->x[i]);
          problem->x[i] = NULL;
        }
      }
      xfree(problem->x);
      problem->x = NULL;
    }
    if (problem->y) {
      xfree(problem->y);
      problem->y = NULL;
    }
    xfree(problem);
    problem = NULL;
  }
}

/** MODULE FUNCTIONS */
static VALUE numo_libsvm_train(VALUE self, VALUE x_val, VALUE y_val, VALUE param_hash) {
  if (CLASS_OF(x_val) != numo_cDFloat) x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  if (CLASS_OF(y_val) != numo_cDFloat) y_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, y_val);
  if (!RTEST(nary_check_contiguous(x_val))) x_val = nary_dup(x_val);
  if (!RTEST(nary_check_contiguous(y_val))) y_val = nary_dup(y_val);

  narray_t* x_nary;
  narray_t* y_nary;
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

  VALUE random_seed = rb_hash_aref(param_hash, ID2SYM(rb_intern("random_seed")));
  if (!NIL_P(random_seed)) srand(NUM2UINT(random_seed));

  LibSvmParameter* param = convertHashToLibSvmParameter(param_hash);
  LibSvmProblem* problem = convertDatasetToLibSvmProblem(x_val, y_val);

  const char* err_msg = svm_check_parameter(problem, param);
  if (err_msg) {
    xfreeLibSvmProblem(problem);
    xfreeLibSvmParameter(param);
    rb_raise(rb_eArgError, "Invalid LIBSVM parameter is given: %s", err_msg);
    return Qnil;
  }

  VALUE verbose = rb_hash_aref(param_hash, ID2SYM(rb_intern("verbose")));
  if (!RTEST(verbose)) svm_set_print_string_function(printNull);

  LibSvmModel* model = svm_train(problem, param);
  VALUE model_hash = convertLibSvmModelToHash(model);
  svm_free_and_destroy_model(&model);

  xfreeLibSvmProblem(problem);
  xfreeLibSvmParameter(param);

  RB_GC_GUARD(x_val);
  RB_GC_GUARD(y_val);

  return model_hash;
}

static VALUE numo_libsvm_cross_validation(VALUE self, VALUE x_val, VALUE y_val, VALUE param_hash, VALUE nr_folds) {
  if (CLASS_OF(x_val) != numo_cDFloat) x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  if (CLASS_OF(y_val) != numo_cDFloat) y_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, y_val);
  if (!RTEST(nary_check_contiguous(x_val))) x_val = nary_dup(x_val);
  if (!RTEST(nary_check_contiguous(y_val))) y_val = nary_dup(y_val);

  narray_t* x_nary;
  narray_t* y_nary;
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

  VALUE random_seed = rb_hash_aref(param_hash, ID2SYM(rb_intern("random_seed")));
  if (!NIL_P(random_seed)) srand(NUM2UINT(random_seed));

  LibSvmParameter* param = convertHashToLibSvmParameter(param_hash);
  LibSvmProblem* problem = convertDatasetToLibSvmProblem(x_val, y_val);

  const char* err_msg = svm_check_parameter(problem, param);
  if (err_msg) {
    xfreeLibSvmProblem(problem);
    xfreeLibSvmParameter(param);
    rb_raise(rb_eArgError, "Invalid LIBSVM parameter is given: %s", err_msg);
    return Qnil;
  }

  size_t t_shape[1] = {(size_t)(problem->l)};
  VALUE t_val = rb_narray_new(numo_cDFloat, 1, t_shape);
  double* t_pt = (double*)na_get_pointer_for_write(t_val);

  VALUE verbose = rb_hash_aref(param_hash, ID2SYM(rb_intern("verbose")));
  if (!RTEST(verbose)) svm_set_print_string_function(printNull);

  const int n_folds = NUM2INT(nr_folds);
  svm_cross_validation(problem, param, n_folds, t_pt);

  xfreeLibSvmProblem(problem);
  xfreeLibSvmParameter(param);

  RB_GC_GUARD(x_val);
  RB_GC_GUARD(y_val);

  return t_val;
}

static VALUE numo_libsvm_predict(VALUE self, VALUE x_val, VALUE param_hash, VALUE model_hash) {
  if (CLASS_OF(x_val) != numo_cDFloat) x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  if (!RTEST(nary_check_contiguous(x_val))) x_val = nary_dup(x_val);

  narray_t* x_nary;
  GetNArray(x_val, x_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }

  LibSvmParameter* param = convertHashToLibSvmParameter(param_hash);
  LibSvmModel* model = convertHashToLibSvmModel(model_hash);
  model->param = *param;

  const int n_samples = (int)NA_SHAPE(x_nary)[0];
  const int n_features = (int)NA_SHAPE(x_nary)[1];
  size_t y_shape[1] = {(size_t)n_samples};
  VALUE y_val = rb_narray_new(numo_cDFloat, 1, y_shape);
  double* y_ptr = (double*)na_get_pointer_for_write(y_val);
  const double* const x_ptr = (double*)na_get_pointer_for_read(x_val);
  for (int i = 0; i < n_samples; i++) {
    LibSvmNode* x_nodes = convertVectorXdToLibSvmNode(&x_ptr[i * n_features], n_features);
    y_ptr[i] = svm_predict(model, x_nodes);
    xfree(x_nodes);
  }

  xfreeLibSvmModel(model);
  xfreeLibSvmParameter(param);

  RB_GC_GUARD(x_val);

  return y_val;
}

static VALUE numo_libsvm_decision_function(VALUE self, VALUE x_val, VALUE param_hash, VALUE model_hash) {
  if (CLASS_OF(x_val) != numo_cDFloat) x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  if (!RTEST(nary_check_contiguous(x_val))) x_val = nary_dup(x_val);

  narray_t* x_nary;
  GetNArray(x_val, x_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }

  LibSvmParameter* param = convertHashToLibSvmParameter(param_hash);
  LibSvmModel* model = convertHashToLibSvmModel(model_hash);
  model->param = *param;

  const int n_samples = (int)NA_SHAPE(x_nary)[0];
  const int n_features = (int)NA_SHAPE(x_nary)[1];
  const int y_cols = isSignleOutputModel(model) ? 1 : model->nr_class * (model->nr_class - 1) / 2;
  size_t y_shape[2] = {(size_t)n_samples, (size_t)y_cols};
  const int n_dims = isSignleOutputModel(model) ? 1 : 2;
  VALUE y_val = rb_narray_new(numo_cDFloat, n_dims, y_shape);
  const double* const x_ptr = (double*)na_get_pointer_for_read(x_val);
  double* y_ptr = (double*)na_get_pointer_for_write(y_val);

  for (int i = 0; i < n_samples; i++) {
    LibSvmNode* x_nodes = convertVectorXdToLibSvmNode(&x_ptr[i * n_features], n_features);
    svm_predict_values(model, x_nodes, &y_ptr[i * y_cols]);
    xfree(x_nodes);
  }

  xfreeLibSvmModel(model);
  xfreeLibSvmParameter(param);

  RB_GC_GUARD(x_val);

  return y_val;
}

static VALUE numo_libsvm_predict_proba(VALUE self, VALUE x_val, VALUE param_hash, VALUE model_hash) {
  narray_t* x_nary;
  GetNArray(x_val, x_nary);
  if (NA_NDIM(x_nary) != 2) {
    rb_raise(rb_eArgError, "Expect samples to be 2-D array.");
    return Qnil;
  }

  LibSvmParameter* param = convertHashToLibSvmParameter(param_hash);
  LibSvmModel* model = convertHashToLibSvmModel(model_hash);
  model->param = *param;

  if (!isProbabilisticModel(model)) {
    xfreeLibSvmModel(model);
    xfreeLibSvmParameter(param);
    return Qnil;
  }

  if (CLASS_OF(x_val) != numo_cDFloat) x_val = rb_funcall(numo_cDFloat, rb_intern("cast"), 1, x_val);
  if (!RTEST(nary_check_contiguous(x_val))) x_val = nary_dup(x_val);

  const int n_samples = (int)NA_SHAPE(x_nary)[0];
  const int n_features = (int)NA_SHAPE(x_nary)[1];
  size_t y_shape[2] = {(size_t)n_samples, (size_t)(model->nr_class)};
  VALUE y_val = rb_narray_new(numo_cDFloat, 2, y_shape);
  const double* const x_ptr = (double*)na_get_pointer_for_read(x_val);
  double* y_ptr = (double*)na_get_pointer_for_write(y_val);
  for (int i = 0; i < n_samples; i++) {
    LibSvmNode* x_nodes = convertVectorXdToLibSvmNode(&x_ptr[i * n_features], n_features);
    svm_predict_probability(model, x_nodes, &y_ptr[i * model->nr_class]);
    xfree(x_nodes);
  }

  xfreeLibSvmModel(model);
  xfreeLibSvmParameter(param);

  RB_GC_GUARD(x_val);

  return y_val;
}

static VALUE numo_libsvm_load_model(VALUE self, VALUE filename) {
  const char* const filename_ = StringValuePtr(filename);
  LibSvmModel* model = svm_load_model(filename_);
  if (model == NULL) {
    rb_raise(rb_eIOError, "Failed to load file '%s'", filename_);
    return Qnil;
  }

  VALUE param_hash = convertLibSvmParameterToHash(&(model->param));
  VALUE model_hash = convertLibSvmModelToHash(model);
  svm_free_and_destroy_model(&model);

  VALUE res = rb_ary_new2(2);
  rb_ary_store(res, 0, param_hash);
  rb_ary_store(res, 1, model_hash);

  RB_GC_GUARD(filename);

  return res;
}

static VALUE numo_libsvm_save_model(VALUE self, VALUE filename, VALUE param_hash, VALUE model_hash) {
  LibSvmParameter* param = convertHashToLibSvmParameter(param_hash);
  LibSvmModel* model = convertHashToLibSvmModel(model_hash);
  model->param = *param;

  const char* const filename_ = StringValuePtr(filename);
  const int res = svm_save_model(filename_, model);

  xfreeLibSvmModel(model);
  xfreeLibSvmParameter(param);

  if (res < 0) {
    rb_raise(rb_eIOError, "Failed to save file '%s'", filename_);
    return Qfalse;
  }

  RB_GC_GUARD(filename);

  return Qtrue;
}

#endif /* LIBSVMEXT_HPP */
