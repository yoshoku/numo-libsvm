#include "svm_problem.h"

void xfree_svm_problem(struct svm_problem* problem)
{
  int i;
  if (problem) {
    if (problem->x) {
      for (i = 0; i < problem->l; i++) {
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

struct svm_problem* dataset_to_svm_problem(VALUE x_val, VALUE y_val)
{
  struct svm_problem* problem;
  narray_t* x_nary;
  double* x_pt;
  double* y_pt;
  int i, j, k;
  int n_samples;
  int n_features;
  int n_nonzero_features;
  int is_padded;
  int last_feature_id;

  GetNArray(x_val, x_nary);
  n_samples = (int)NA_SHAPE(x_nary)[0];
  n_features = (int)NA_SHAPE(x_nary)[1];
  x_pt = (double*)na_get_pointer_for_read(x_val);
  y_pt = (double*)na_get_pointer_for_read(y_val);

  problem = ALLOC(struct svm_problem);
  problem->l = n_samples;
  problem->x = ALLOC_N(struct svm_node*, n_samples);
  problem->y = ALLOC_N(double, n_samples);

  is_padded = 0;
  for (i = 0; i < n_samples; i++) {
    n_nonzero_features = 0;
    for (j = 0; j < n_features; j++) {
      if (x_pt[i * n_features + j] != 0.0) {
        n_nonzero_features += 1;
        last_feature_id = j + 1;
      }
    }
    if (is_padded == 0 && last_feature_id == n_features) {
      is_padded = 1;
    }
    if (is_padded == 1) {
      problem->x[i] = ALLOC_N(struct svm_node, n_nonzero_features + 1);
    } else {
      problem->x[i] = ALLOC_N(struct svm_node, n_nonzero_features + 2);
    }
    for (j = 0, k = 0; j < n_features; j++) {
      if (x_pt[i * n_features + j] != 0.0) {
        problem->x[i][k].index = j + 1;
        problem->x[i][k].value = (double)x_pt[i * n_features + j];
        k++;
      }
    }
    if (is_padded == 1) {
      problem->x[i][n_nonzero_features].index = -1;
      problem->x[i][n_nonzero_features].value = 0.0;
    } else {
      problem->x[i][n_nonzero_features].index = n_features;
      problem->x[i][n_nonzero_features].value = 0.0;
      problem->x[i][n_nonzero_features + 1].index = -1;
      problem->x[i][n_nonzero_features + 1].value = 0.0;
    }
    problem->y[i] = y_pt[i];
  }

  RB_GC_GUARD(x_val);
  RB_GC_GUARD(y_val);

  return problem;
}
