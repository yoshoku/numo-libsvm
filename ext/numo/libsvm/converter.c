
#include "converter.h"

VALUE int_vec_to_nary(int* const arr, int const size)
{
  int i;
  size_t shape[1] = { size };
  VALUE v = rb_narray_new(numo_cInt32, 1, shape);
  int32_t* vp = (int32_t*)na_get_pointer_for_write(v);
  for (i = 0; i < size; i++) { vp[i] = (int32_t)arr[i]; }
  return v;
}

int* nary_to_int_vec(VALUE vec_val)
{
  int i;
  int n_elements;
  narray_t* vec_nary;
  int32_t* vec_pt;
  int* vec;

  if (vec_val == Qnil) return NULL;

  GetNArray(vec_val, vec_nary);
  n_elements = (int)NA_SHAPE(vec_nary)[0];

  vec = ALLOC_N(int, n_elements);
  vec_pt = (int32_t*)na_get_pointer_for_read(vec_val);
  for (i = 0; i < n_elements; i++) { vec[i] = (int)vec_pt[i]; }

  return vec;
}

VALUE dbl_vec_to_nary(double* const arr, int const size)
{
  int i;
  size_t shape[1] = { size };
  VALUE v = rb_narray_new(numo_cDFloat, 1, shape);
  double* vp = (double*)na_get_pointer_for_write(v);
  for (i = 0; i < size; i++) { vp[i] = arr[i]; }
  return v;
}

double* nary_to_dbl_vec(VALUE vec_val)
{
  int n_elements;
  narray_t* vec_nary;
  double* vec_pt;
  double* vec;

  if (vec_val == Qnil) return NULL;

  GetNArray(vec_val, vec_nary);
  n_elements = (int)NA_SHAPE(vec_nary)[0];

  vec = ALLOC_N(double, n_elements);
  vec_pt = (double*)na_get_pointer_for_read(vec_val);
  memcpy(vec, vec_pt, n_elements * sizeof(double));

  return vec;
}

VALUE dbl_mat_to_nary(double** const mat, int const n_rows, int const n_cols)
{
  int i, j;
  size_t shape[2] = { n_rows, n_cols };
  VALUE v = rb_narray_new(numo_cDFloat, 2, shape);
  double* vp = (double*)na_get_pointer_for_write(v);

  for (i = 0; i < n_rows; i++) {
    for (j = 0; j < n_cols; j++) {
      vp[i * n_cols + j] = mat[i][j];
    }
  }

  return v;
}

double** nary_to_dbl_mat(VALUE mat_val)
{
  int i, j;
  int n_rows, n_cols;
  narray_t* mat_nary;
  double* mat_pt;
  double** mat;

  if (mat_val == Qnil) return NULL;

  GetNArray(mat_val, mat_nary);
  n_rows = (int)NA_SHAPE(mat_nary)[0];
  n_cols = (int)NA_SHAPE(mat_nary)[1];

  mat_pt = (double*)na_get_pointer_for_read(mat_val);
  mat = ALLOC_N(double*, n_rows);
  for (i = 0; i < n_rows; i++) {
    mat[i] = ALLOC_N(double, n_cols);
    for (j = 0; j < n_cols; j++) {
      mat[i][j] = mat_pt[i * n_cols + j];
    }
  }

  return mat;
}

VALUE svm_nodes_to_nary(struct svm_node** const support_vecs, const int n_support_vecs)
{
  int i, j;
  int n_dimensions = 0;
  size_t shape[2] = { n_support_vecs, 1 };
  VALUE v;
  double* vp;

  for (i = 0; i < n_support_vecs; i++) {
    for (j = 0; support_vecs[i][j].index != -1; j++) {
      if (n_dimensions < support_vecs[i][j].index) {
        n_dimensions = support_vecs[i][j].index;
      }
    }
  }

  shape[1] = n_dimensions;
  v = rb_narray_new(numo_cDFloat, 2, shape);
  vp = (double*)na_get_pointer_for_write(v);
  memset(vp, 0, n_support_vecs * n_dimensions * sizeof(double));

  for (i = 0; i < n_support_vecs; i++) {
    for (j = 0; support_vecs[i][j].index != -1; j++) {
      vp[i * n_dimensions + support_vecs[i][j].index - 1] = support_vecs[i][j].value;
    }
  }

  return v;
}

struct svm_node** nary_to_svm_nodes(VALUE model_val)
{
  int i, j;
  int n_rows, n_cols;
  narray_t* model_nary;
  double* model_pt;
  struct svm_node** support_vecs;

  if (model_val == Qnil) return NULL;

  GetNArray(model_val, model_nary);
  n_rows = (int)NA_SHAPE(model_nary)[0];
  n_cols = (int)NA_SHAPE(model_nary)[1];

  model_pt = (double*)na_get_pointer_for_read(model_val);
  support_vecs = ALLOC_N(struct svm_node*, n_rows);
  for (i = 0; i < n_rows; i++) {
    support_vecs[i] = ALLOC_N(struct svm_node, n_cols + 1);
    for (j = 0; j < n_cols; j++) {
      support_vecs[i][j].index = j + 1;
      support_vecs[i][j].value = model_pt[i * n_cols + j];
    }
    support_vecs[i][n_cols].index = -1;
    support_vecs[i][n_cols].value = 0.0;
  }

  return support_vecs;
}
