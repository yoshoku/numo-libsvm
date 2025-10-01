require 'mkmf'
require 'numo/narray'

$LOAD_PATH.each do |lp|
  if File.exist?(File.join(lp, 'numo/numo/narray.h'))
    $INCFLAGS = "-I#{lp}/numo #{$INCFLAGS}"
    break
  end
end

abort 'numo/narray.h not found.' unless have_header('numo/narray.h')

if RUBY_PLATFORM =~ /mswin|cygwin|mingw/
  $LOAD_PATH.each do |lp|
    if File.exist?(File.join(lp, 'numo/libnarray.a'))
      $LDFLAGS = "-L#{lp}/numo #{$LDFLAGS}"
      break
    end
  end
  abort 'libnarray.a not found.' unless have_library('narray', 'nary_new')
end

if RUBY_PLATFORM.match?(/darwin/) && Gem::Version.new('3.1.0') <= Gem::Version.new(RUBY_VERSION)
  if try_link('int main(void){return 0;}', '-Wl,-undefined,dynamic_lookup')
    $LDFLAGS << ' -Wl,-undefined,dynamic_lookup'
  end
end

have_libcpp = false
if RUBY_PLATFORM.include?('darwin')
  if have_library('c++')
    have_libcpp = true
  else
    warn 'libc++ is not found.'
  end
end

if !have_libcpp && !RUBY_PLATFORM.include?('mswin')
  warn 'libstdc++ is not found.' unless have_library('stdc++')
end

$srcs = Dir.glob("#{$srcdir}/**/*.cpp").map { |path| File.basename(path) }
$INCFLAGS << " -I$(srcdir)/src"
$VPATH << "$(srcdir)/src"

create_makefile('numo/libsvm/libsvmext')
