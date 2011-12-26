#
#  step.gemspec  --  Step Gem specification
#


require "rubygems"


Gem::Specification.new do |s|
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
  s.extra_rdoc_files  = %w(
                          README
                          LICENSE
                        )

  s.extensions        = "lib/mkrf_conf"
  s.files             = %w(
                          lib/mkrf_conf
                          lib/Rakefile
                          lib/step.c
                          lib/step.h
                          lib/sync.c
                          lib/sync.h
                          lib/step/filesys.c
                          lib/step/filesys.h
                          lib/step/itimer.c
                          lib/step/itimer.h
                          lib/step/date.rb
                          examples/teatimer
                        )
  s.executables       = %w(
                        )
end

