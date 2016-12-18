class StatsFinder
  def initialize
    @sum = 0.0
    @array = []
  end

  def observe_value(value)
    @sum += value
    @array << value
    @array.sort!
    true
  end

  def total
    @count
  end

  def average
    @sum.to_f / @array.size.to_f
  end

  def median
    @array[@array.size / 2]
  end

  def standard_deviation
    mean = average
    sd = 0.0
    @array.each do |value|
      diff = value - mean
      sd += diff * diff
    end
    sd /= @array.size.to_f
    Math.sqrt(sd)
  end
end
