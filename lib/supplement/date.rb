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

end


unless DateTime.method_defined? :to_time then

  class Time
    def to_time ; self ; end
    def to_date
      jd = Date.civil_to_jd year, mon, mday, Date::ITALY
      hd = Date.jd_to_ajd jd, 0, 0
      Date.new! hd, 0, Date::ITALY
    end
    def to_datetime
      jd = DateTime.civil_to_jd year, mon, mday, DateTime::ITALY
      fr = DateTime.time_to_day_fraction hour, min, [sec, 59].min
      fr += (Rational usec, 86400*1000000000)
      of = (Rational utc_offset, 86400)
      hd = DateTime.jd_to_ajd jd, fr, of
      DateTime.new! hd, of, DateTime::ITALY
    end
  end

  class Date
    def to_time     ; Time.local year, mon, mday ; end
    def to_date     ; self ; end
    def to_datetime ; DateTime.new! Date.jd_to_ajd(jd,0,0), @of, @sg ; end
  end

  class DateTime
    def to_time
      (new_offset 0).instance_eval do
        Time.utc year, mon, mday, hour, min, sec + sec_fraction
      end.getlocal
    end
    def to_date     ; Date.new! Date.jd_to_ajd(jd,0,0), 0, @sg ; end
    def to_datetime ; self ; end
  end

end

