#ifndef NUMO_LIBSVM_SVM_PROBLEM_H
#define NUMO_LIBSVM_SVM_PROBLEM_H 1

#include <svm.h>
#include <ruby.h>
#include <numo/narray.h>
#include <numo/template.h>

void xfree_svm_problem(struct svm_problem* problem);
struct svm_problem* dataset_to_svm_problem(VALUE x_val, VALUE y_val);

#endif /* NUMO_LIBSVM_SVM_PROBLEM_H */
