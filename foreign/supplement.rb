#
#  foreign/supplement.rb  --  Additional useful Ruby functions
#

# The purpose of this is simply to reduce dependencies.

begin
  require "supplement"
rescue LoadError
  class NilClass ; def nonzero?  ;                      end ; end
  class NilClass ; def notempty? ;                      end ; end
  class String   ; def notempty? ; self unless empty? ; end ; end
  class Array    ; def notempty? ; self unless empty? ; end ; end
  class Array    ; def first= val ; self[ 0] = val ; end ; end
  class Array    ; def last=  val ; self[-1] = val ;
                    rescue IndexError ; a.push val ; end ; end
  class NilClass   ; def to_bool ; false ; end ; end
  class FalseClass ; def to_bool ; false ; end ; end
  class Object     ; def to_bool ; true  ; end ; end
  class <<Struct ; alias [] new ; end
  class Module
    def plain_name
      sep = "::"
      n = name.dup
      i = n.rindex sep
      n.slice! 0, i+sep.length if i
      n
    end
  end
  class Integer
    def length base = nil
      base ||= 10
      y, n = abs, 0
      until y.zero? do y /= base ; n += 1 end
      n
    end
  end
  class String
    ELLIPSE = :"..."
    def axe n = 80
      if n < length then
        l = ELLIPSE.length
        if n >= l then
          n -= l
        else
          n, l = 0, n
        end
        self[ 0, n] << ELLIPSE[0,l]
      else
        self
      end
    end
    def axe! n = 80
      if n < length then
        l = ELLIPSE.length
        if n >= l then
          n -= l
        else
          n, l = 0, n
        end
        slice! n...nil
        self << ELLIPSE[0,l]
      end
    end
    def starts_with? oth ; o = oth.to_str ; o.length          if start_with? o ; end
    def ends_with?   oth ; o = oth.to_str ; length - o.length if end_with?   o ; end
    alias starts_with starts_with?
    alias ends_with   ends_with?
  end
  class Array
    def lstrip! ; shift while first and yield first ; self if empty? ; end
    def rstrip! ; pop   while last  and yield last  ; self if empty? ; end
    def find_nth num
      find do |e|
        next unless yield e
        begin num.zero? ensure num -= 1 end
      end
    end
    def delete_nth num, &block
      f = find_nth num, &block
      delete f if f
    end
  end
  class Dir ; def entries! ; entries - %w(. ..) ; end ; end
end

