#
#  supplement.gemspec  --  Supplement Gem specification
#

Gem::Specification.new do |s|
  s.name              = "supplement"
  (File.read "README") =~ /^=\s.*?(\d+(?:\.\d+)*)/
  s.version           = $1
  s.license           = "BSD-2-Clause"
  s.summary           = "Simple Ruby extensions"
  s.description       = <<~EOT
                          Simple methods that didn't manage to become
                          part of standard Ruby.
                        EOT
  s.authors           = [ "Bertram Scharpf"]
  s.email             = "<software@bertram-scharpf.de>"
  s.homepage          = "https://github.com/BertramScharpf/ruby-supplement"

  s.required_ruby_version = Gem::Requirement.new ">= 2.0.0"

  s.requirements      = "Ruby and the autorake gem"
  s.add_dependency      "rake", "~>13.0"
  s.add_dependency      "autorake", "~>2.18"

  s.extensions        = "lib/mkrf_conf"
  s.files             = %w(
                          lib/mkrf_conf
                          lib/Rakefile
                          lib/supplement.c
                          lib/supplement.h
                          lib/process.c
                          lib/process.h
                          lib/supplement/locked.c
                          lib/supplement/locked.h
                          lib/supplement/dir.rb
                          lib/supplement/filesys.c
                          lib/supplement/filesys.h
                          lib/supplement/itimer.c
                          lib/supplement/itimer.h
                          lib/supplement/terminal.c
                          lib/supplement/terminal.h
                          lib/supplement/socket.c
                          lib/supplement/socket.h
                          lib/supplement/date.rb
                          lib/supplement/roman.rb
                          examples/teatimer
                        )
  s.executables       = %w(
                        )

  s.rdoc_options.concat %w(--main README)
  s.extra_rdoc_files  = %w(
                          README
                          LICENSE
                        )
end

