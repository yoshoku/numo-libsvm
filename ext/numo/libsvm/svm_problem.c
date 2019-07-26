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
  int i, j;
  int n_samples;
  int n_features;

  GetNArray(x_val, x_nary);
  n_samples = (int)NA_SHAPE(x_nary)[0];
  n_features = (int)NA_SHAPE(x_nary)[1];
  x_pt = (double*)na_get_pointer_for_read(x_val);
  y_pt = (double*)na_get_pointer_for_read(y_val);

  problem = ALLOC(struct svm_problem);
  problem->l = n_samples;
  problem->x = ALLOC_N(struct svm_node*, n_samples);
  problem->y = ALLOC_N(double, n_samples);
  for (i = 0; i < n_samples; i++) {
    problem->x[i] = ALLOC_N(struct svm_node, n_features + 1);
    for (j = 0; j < n_features; j++) {
      problem->x[i][j].index = j + 1;
      problem->x[i][j].value = x_pt[i * n_features + j];
    }
    problem->x[i][n_features].index = -1;
    problem->x[i][n_features].value = 0.0;
    problem->y[i] = y_pt[i];
  }

  return problem;
}
