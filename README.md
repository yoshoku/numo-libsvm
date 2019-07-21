# Numo::Libsvm

Numo::Libsvm is a Ruby gem binding to the [LIBSVM](https://github.com/cjlin1/libsvm) library.
LIBSVM is one of the famous libraries that implemented Support Vector Machines,
and provides functions for support vector classifier, regression, and distribution estimation.
Numo::Libsvm makes to use the LIBSVM functions with dataset represented by [Numo::NArray](https://github.com/ruby-numo/numo-narray).

Note: There are other useful Ruby gems binding to LIBSVM:
[rb-libsvm](https://github.com/febeling/rb-libsvm) by C. Florian Ebeling,
[libsvm-ruby-swig](https://github.com/tomz/libsvm-ruby-swig) by Tom Zeng,
and [jrb-libsvm](https://github.com/andreaseger/jrb-libsvm) by Andreas Eger.

## Installation
Numo::Libsvm does not bundle LIBSVM unlike rb-libsvm. You need to install LIBSVM in advance along your environment.

macOS:

    $ brew install libsvm

Ubuntu:

    $ sudo apt-get install libsvm-dev

Add this line to your application's Gemfile:

```ruby
gem 'numo-libsvm'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install numo-libsvm

## Usage

TODO: Write usage instructions here

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/yoshoku/numo-libsvm. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [Contributor Covenant](http://contributor-covenant.org) code of conduct.

## License

The gem is available as open source under the terms of the [BSD-3-Clause License](https://opensource.org/licenses/BSD-3-Clause).

## Code of Conduct

Everyone interacting in the Numo::Libsvm project’s codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/[USERNAME]/numo-libsvm/blob/master/CODE_OF_CONDUCT.md).
