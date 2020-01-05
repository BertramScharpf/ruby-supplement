#
#  roman.rb  --  Roman numerals
#

class Integer

  ROMANS = {
       1 => 'I',
       5 => 'V',
      10 => 'X',
      50 => 'L',
     100 => 'C',
     500 => 'D',
    1000 => 'M'
  }
  ROMANSUB = [
    [1000, 100],
    [ 500, 100],
    [ 100,  10],
    [  50,  10],
    [  10,   1],
    [   5,   1],
    [   1,   0]
  ]

  # :call-seq:
  #   int.roman()   -> str or nil
  #
  # Return the roman numeral as uppercase if possible.
  #
  #   1994.roman          #=> "MCMXCIV"
  #   -1.roman            #=> nil
  #
  def roman
    return ROMANS[ self]  if ROMANS.has_key? self
    ROMANSUB.each do |cut,sub|
      return cut.roman + (self - cut).roman  if self >  cut
      return sub.roman + (self + sub).roman  if self >= cut - sub and self < cut
    end
    nil
  end

end

