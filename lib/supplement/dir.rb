#
#  supplement/dir.rb  --  More Dir methods
#

class Dir

  class <<self

    # :call-seq:
    #    Dir.nuke!( name)     -> nil
    #
    # Delete a directory, all its contents and all subdirectories.
    # WARNING! This can cause serious damage.
    #
    def nuke! n
      (new n).entries!.each { |e|
        p = File.join n, e
        if File.directory? p then
          nuke! p
        else
          File.unlink p
        end
      }
      rmdir n
      nil
    end

    # :call-seq:
    #    Dir.rmpath( name)     -> nil
    #
    # Delete as many empty directories as possible.
    #
    def rmpath n
      case n
        when "/", "." then
        else
          if (new n).entries!.empty? then
            rmdir n
            d = File.dirname n
            rmpath d
          end
      end
      nil
    end

  end

end

