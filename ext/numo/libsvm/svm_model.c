
#include "svm_model.h"

struct svm_model* rb_hash_to_svm_model(VALUE model_hash)
{
  VALUE el;
  struct svm_model* model = ALLOC(struct svm_model);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("nr_class")));
  model->nr_class = el != Qnil ? NUM2INT(el) : 0;
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("l")));
  model->l = el != Qnil ? NUM2INT(el) : 0;
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("SV")));
  model->SV = nary_to_svm_nodes(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("sv_coef")));
  model->sv_coef = nary_to_dbl_mat(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("rho")));
  model->rho = nary_to_dbl_vec(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("probA")));
  model->probA = nary_to_dbl_vec(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("probB")));
  model->probB = nary_to_dbl_vec(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("sv_indices")));
  model->sv_indices = nary_to_int_vec(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("label")));
  model->label = nary_to_int_vec(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("nSV")));
  model->nSV = nary_to_int_vec(el);
  el = rb_hash_aref(model_hash, ID2SYM(rb_intern("free_sv")));
  model->free_sv = el != Qnil ? NUM2INT(el) : 0;
  return model;
}

VALUE svm_model_to_rb_hash(struct svm_model* const model)
{
  int const n_classes = model->nr_class;
  int const n_support_vecs = model->l;
  VALUE support_vecs = model->SV ? svm_nodes_to_nary(model->SV, n_support_vecs) : Qnil;
  VALUE coefficients = model->sv_coef ? dbl_mat_to_nary(model->sv_coef, n_classes - 1, n_support_vecs) : Qnil;
  VALUE intercepts = model->rho ? dbl_vec_to_nary(model->rho, n_classes * (n_classes - 1) / 2) : Qnil;
  VALUE prob_alpha = model->probA ? dbl_vec_to_nary(model->probA, n_classes * (n_classes - 1) / 2) : Qnil;
  VALUE prob_beta = model->probB ? dbl_vec_to_nary(model->probB, n_classes * (n_classes - 1) / 2) : Qnil;
  VALUE sv_indices = model->sv_indices ? int_vec_to_nary(model->sv_indices, n_support_vecs) : Qnil;
  VALUE labels = model->label ? int_vec_to_nary(model->label, n_classes) : Qnil;
  VALUE n_support_vecs_each_class = model->nSV ? int_vec_to_nary(model->nSV, n_classes) : Qnil;
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

void xfree_svm_model(struct svm_model* model)
{
  int i;
  if (model) {
    if (model->SV) {
      for (i = 0; i < model->l; xfree(model->SV[i++]));
      xfree(model->SV);
      model->SV = NULL;
    }
    if (model->sv_coef) {
      for (i = 0; i < model->nr_class - 1; xfree(model->sv_coef[i++]));
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
