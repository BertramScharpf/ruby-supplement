#
#  supplement.gemspec  --  Supplement Gem specification
#

Gem::Specification.new do |s|
  s.name              = "supplement"
  (File.read "README") =~ /^=\s.*?(\d+(?:\.\d+)*)/
  s.version           = $1
  s.summary           = "Simple Ruby extensions"
  s.description       = <<EOT
Simple methods that didn't manage to become part of standard Ruby.
EOT
  s.authors           = [ "Bertram Scharpf"]
  s.email             = "<software@bertram-scharpf.de>"
  s.homepage          = "http://www.bertram-scharpf.de/software/supplement"

  s.rubyforge_project = "NONE"

  s.requirements      = "Ruby and the autorake gem"
  s.add_dependency      "autorake", ">=2.0"

  s.has_rdoc          = true
  s.extra_rdoc_files  = %w(
                          README
                          LICENSE
                        )

  s.extensions        = "lib/mkrf_conf"
  s.files             = %w(
                          lib/mkrf_conf
                          lib/Rakefile
                          lib/supplement.c
                          lib/supplement.h
                          lib/sync.c
                          lib/sync.h
                          lib/supplement/filesys.c
                          lib/supplement/filesys.h
                          lib/supplement/itimer.c
                          lib/supplement/itimer.h
                          lib/supplement/terminal.c
                          lib/supplement/terminal.h
                          lib/supplement/date.rb
                          examples/teatimer
                        )
  s.executables       = %w(
                        )
end

