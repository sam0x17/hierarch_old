class String
  def rtruncate(max_len = 80, omit = '...')
    max_len -= omit.size
    return dup if size < max_len
    "#{omit}#{self[(size - max_len)..-1]}"
  end
end
