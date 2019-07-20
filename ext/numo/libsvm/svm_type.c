#include "svm_type.h"

RUBY_EXTERN VALUE mLibsvm;

void rb_init_svm_type_module()
{
  /**
   * Document-module: Numo::Libsvm::SvmType
   * The module consisting of constants for SVM algorithm type that used for parameter of LIBSVM.
   */
  VALUE mSvmType = rb_define_module_under(mLibsvm, "SvmType");
  /* C-SVM classification */
  rb_define_const(mSvmType, "C_SVC", INT2NUM(C_SVC));
  /* nu-SVM classification */
  rb_define_const(mSvmType, "NU_SVC", INT2NUM(NU_SVC));
  /* one-class-SVM */
  rb_define_const(mSvmType, "ONE_CLASS", INT2NUM(ONE_CLASS));
  /* epsilon-SVM regression */
  rb_define_const(mSvmType, "EPSILON_SVR", INT2NUM(EPSILON_SVR));
  /* nu-SVM regression */
  rb_define_const(mSvmType, "NU_SVR", INT2NUM(NU_SVR));
}
