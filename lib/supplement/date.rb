#
#  supplement/date.rb  --  More Date methods
#

require "date"


class Date

  def y ; year  ; end
  def m ; month ; end
  def d ; day   ; end

  def to_a ; [ year, month, day ] ; end


  EASTER = "Easter"

  class <<self

    def easter_western year
      # This is after Donald E. Knuth and it works for both
      # Julian (1582 and before) and Gregorian (1583 and after) calendars.
      g = year%19 + 1                   # golden number
      c = year/100 + 1                  # century
      x = (3*c/4) - 12                  # corrections
      z = (8*c+5)/25 - 5
      d = 5*year/4 - x - 10             # March((-d)%7)th is Sunday
      e = (11*g + 20 + z - x) % 30      # moon phase
      e += 1 if e == 25 and g > 11 or e == 24
      n = 44 - e                        # full moon
      n += 30 if n < 21
      month = 3
      day = n+7 - (d+n)%7
      if day > 31 then
        month +=  1
        day   -= 31
      end
      civil year, month, day
    end
    alias easter easter_western

  end


  def workday_after n
    r = self + n
    case r.wday
      when 0 then r += 1
      when 6 then r += 2
    end
    r
  end

  def quarter
    (month-1) / 3 + 1
  end

  def wstart
    self - wday
  end

  def cwstart
    self - (cwday-1)
  end

  def first_of_month
    if day == 1 then
      self
    else
      self.class.civil year, month, 1
    end
  end

end

