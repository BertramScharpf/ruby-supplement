#
#  step/date.rb  --  More Date methods
#

require "date"

class Date

  def y ; year  ; end
  def m ; month ; end
  def d ; day   ; end

  def to_a ; [ year, month, day ] ; end

  def to_time *args
    Time.mktime year, month, day, *args
  end

end

