#ifndef NUMO_LIBSVM_CONVERTER_H
#define NUMO_LIBSVM_CONVERTER_H 1

#include <string.h>
#include <svm.h>
#include <ruby.h>
#include <numo/narray.h>
#include <numo/template.h>

VALUE int_vec_to_nary(int* const arr, int const size);
int* nary_to_int_vec(VALUE vec_val);
VALUE dbl_vec_to_nary(double* const arr, int const size);
double* nary_to_dbl_vec(VALUE vec_val);
VALUE dbl_mat_to_nary(double** const mat, int const n_rows, int const n_cols);
double** nary_to_dbl_mat(VALUE mat_val);
VALUE svm_nodes_to_nary(struct svm_node** const support_vecs, const int n_support_vecs);
struct svm_node** nary_to_svm_nodes(VALUE nary_val);
struct svm_node* dbl_vec_to_svm_node(double* const arr, int const size);

#endif /* NUMO_LIBSVM_CONVERTER_H */
