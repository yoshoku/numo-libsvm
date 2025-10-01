# [[3.0.0](https://github.com/yoshoku/numo-libsvm/compare/v2.3.0...v3.0.0)] - 2025-10-01

**Breaking chage**

- Change dependency from numo-narray to [numo-narray-alt](https://github.com/yoshoku/numo-narray-alt).

# 2.3.0
- Update bundled LIBSVM to 3.35.

# 2.2.0
- Update bundled LIBSVM to 3.31.

# 2.1.1
- Fix build failure with Xcode 14 and Ruby 3.1.x.

# 2.1.0
- Update bundled LIBSVM to 3.30.
- Support for probilistic outputs of one-class SVM.

# 2.0.1
- Chnage to use memcpy to copy vector.
- Refactor codes and configs with RuboCop.

# 2.0.0
- Redesign native extension codes.
- Change not ot use git submodule for LIBSVM codes bundle.
- Introduce conventional commits.

# 1.1.2
- Remove dependent gem's type declaration file from installation files.

# 1.1.1
- Fix version specifier of runtime dependencies.

# 1.1.0
- Add type declaration file: sig/numo/libsvm.rbs

# 1.0.2
- Add GC guard to model saving and loading methods.
- Fix size specification to memcpy function.

# 1.0.1
- Add GC guard codes.
- Fix some configuration files.

# 1.0.0
## Breaking change
- For easy installation, Numo::LIBSVM bundles LIBSVM codes.
There is no need to install LIBSVM in advance to use Numo::LIBSVM.

# 0.5.0
- Fix to use LIBSVM sparce vector representation for internal processing.

# 0.4.0
- Add verbose parameter to output learning process messages.
- Several documentation improvements.

# 0.3.0
- Add random_seed parameter for specifying seed to give to srand function.
- Several documentation improvements.

# 0.2.0
- Add validation of method parameters.
- Several documentation improvements.

# 0.1.0
- First release.
