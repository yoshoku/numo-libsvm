#ifndef NUMO_LIBSVM_SVM_MODEL_H
#define NUMO_LIBSVM_SVM_MODEL_H 1

#include <svm.h>
#include <ruby.h>
#include <numo/narray.h>
#include <numo/template.h>

#include "converter.h"

struct svm_model* rb_hash_to_svm_model(VALUE model_hash);
VALUE svm_model_to_rb_hash(struct svm_model* const model);
void xfree_svm_model(struct svm_model* model);

#endif /* NUMO_LIBSVM_SVM_MODEL_H */
