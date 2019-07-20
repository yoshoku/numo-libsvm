#include "kernel_type.h"

RUBY_EXTERN VALUE mLibsvm;

void rb_init_kernel_type_module()
{
  /**
   * Document-module: Numo::Libsvm::KernelType
   * The module consisting of constants for kernel type that used for parameter of LIBSVM.
   */
  VALUE mKernelType = rb_define_module_under(mLibsvm, "KernelType");
  /* Linear kernel; u' * v */
  rb_define_const(mKernelType, "LINEAR", INT2NUM(LINEAR));
  /* Polynomial kernel; (gamma * u' * v + coef0)^degree */
  rb_define_const(mKernelType, "POLY", INT2NUM(POLY));
  /* RBF kernel; exp(-gamma * ||u - v||^2) */
  rb_define_const(mKernelType, "RBF", INT2NUM(RBF));
  /* Sigmoid kernel; tanh(gamma * u' * v + coef0) */
  rb_define_const(mKernelType, "SIGMOID", INT2NUM(SIGMOID));
  /* Precomputed kernel */
  rb_define_const(mKernelType, "PRECOMPUTED", INT2NUM(PRECOMPUTED));
}
