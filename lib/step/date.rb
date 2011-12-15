#
#  step/date.rb  --  More Date methods
#

require "date"

class Date

  def y ; year  ; end
  def m ; month ; end
  def d ; day   ; end

  def to_a ; [ year, month, day ] ; end

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

  class DateTime < Date
    def to_time
      (new_offset 0).instance_eval do
        Time.utc year, mon, mday, hour, min, sec + sec_fraction
      end.getlocal
    end
    def to_date     ; Date.new! Date.jd_to_ajd(jd,0,0), 0, @sg ; end
    def to_datetime ; self ; end
  end

end

