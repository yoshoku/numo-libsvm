
#include "svm_parameter.h"

struct svm_parameter* rb_hash_to_svm_parameter(VALUE param_hash)
{
  VALUE el;
  struct svm_parameter* param = ALLOC(struct svm_parameter);
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
  param->weight       = NULL;
  if (!NIL_P(el)) {
    param->weight = ALLOC_N(double, param->nr_weight);
    memcpy(param->weight, (double*)na_get_pointer_for_read(el), param->nr_weight * sizeof(double));
  }
  return param;
}

VALUE svm_parameter_to_rb_hash(struct svm_parameter* const param)
{
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
  rb_hash_aset(param_hash, ID2SYM(rb_intern("shrinking")),
    param->shrinking == 1 ? Qtrue : Qfalse);
  rb_hash_aset(param_hash, ID2SYM(rb_intern("probability")),
    param->probability == 1 ? Qtrue : Qfalse);
  rb_hash_aset(param_hash, ID2SYM(rb_intern("weight_label")),
    param->weight_label ? int_vec_to_nary(param->weight_label, param->nr_weight) : Qnil);
  rb_hash_aset(param_hash, ID2SYM(rb_intern("weight")),
    param->weight ? dbl_vec_to_nary(param->weight, param->nr_weight) : Qnil);
  return param_hash;
}

void xfree_svm_parameter(struct svm_parameter* param)
{
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
