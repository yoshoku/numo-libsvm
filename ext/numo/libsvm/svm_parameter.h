#ifndef NUMO_LIBSVM_SVM_PARAMETER_H
#define NUMO_LIBSVM_SVM_PARAMETER_H 1

#include <svm.h>
#include <ruby.h>
#include <numo/narray.h>
#include <numo/template.h>

#include "converter.h"

struct svm_parameter* rb_hash_to_svm_parameter(VALUE param_hash);
VALUE svm_parameter_to_rb_hash(struct svm_parameter* const param);
void xfree_svm_parameter(struct svm_parameter* param);

#endif /* NUMO_LIBSVM_SVM_PARAMETER_H */
