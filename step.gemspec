#
#  step.gemspec  --  Step Gem specification
#


require "rubygems"


SPEC = Gem::Specification.new do |s|
  s.name              = "step"
  (File.read "README") =~ /^=\s.*?(\d+(?:\.\d+)*)/
  s.version           = $1
  s.summary           = "Simple Ruby extensions"
  s.description       = <<EOT
Simple methods that didn't manage to become part of standard Ruby.
EOT
  s.authors           = [ "Bertram Scharpf"]
  s.email             = "<software@bertram-scharpf.de>"
  s.homepage          = "http://www.bertram-scharpf.de"
  s.requirements      = "just Ruby"
  s.add_dependency      "autorake", ">=1.0"
  s.has_rdoc          = true
  s.extensions        = "lib/mkrf_conf"
  s.files             = %w(
                          lib/mkrf_conf
                          lib/Rakefile
                          lib/step.c
                          lib/step.h
                          lib/step/filesys.c
                          lib/step/filesys.h
                          lib/step/itimer.c
                          lib/step/itimer.h
                          lib/step/date.rb
                        )
  s.executables       = %w(
                          teatimer
                        )
  s.extra_rdoc_files  = %w(
                          README
                          LICENSE
                        )
end

if $0 == __FILE__ then
  (Gem::Builder.new SPEC).build
end

