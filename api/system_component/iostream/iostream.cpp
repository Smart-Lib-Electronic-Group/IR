#include "iostream.hpp"

using namespace OwO;
using namespace system;
using namespace kernel;

O_METAOBJECT(IOStream, Object)

uint32_t IOStream::format_uint(char* buf, uint32_t buffer_size, uint64_t value, const uint8_t base)
{
  static const char digits[] = "0123456789ABCDEF";
  char*             p        = buf;

  do
  {
    *p++   = digits[value % base];
    value /= base;
    if (1 == buffer_size--)
      break;
  } while (value > 0);

  *p = '\0';
  std::reverse(buf, p);
  return std::strlen(buf);
}

uint32_t IOStream::format_int(char* buf, uint32_t buffer_size, int64_t value, const uint8_t base)
{
  static const char digits[] = "0123456789ABCDEF";
  char*             p        = buf;
  bool              negative = false;

  if (value < 0 && base == 10)
  {
    negative = true;
    value    = -value;
    buffer_size--;
  }

  do
  {
    *p++   = digits[value % base];
    value /= base;
    if (1 == buffer_size--)
      break;
  } while (value > 0);

  if (negative)
    *p++ = '-';

  *p = '\0';
  std::reverse(buf, p);
  return std::strlen(buf);
}

uint32_t IOStream::format_double(char* buf, uint32_t buffer_size, double value, const uint8_t precision)
{
  static const char digits[] = "0123456789";
  char*             p        = buf;
  bool              negative = false;

  if (value < 0)
  {
    negative = true;
    value    = -value;
    buffer_size--;
  }

  double rounding = 0.5;
  for (int i = 0; i < precision; ++i)
    rounding /= 10.0;
  value             += rounding;

  long long intPart  = static_cast<long long>(value);
  double    frac     = value - intPart;

  do
  {
    *p++     = digits[intPart % 10];
    intPart /= 10;
    if (1 == buffer_size--)
      break;
  } while (intPart > 0);

  if (negative)
    *p++ = '-';

  std::reverse(buf, p);

  *p++ = '.';

  for (int i = 0; i < precision; ++i)
  {
    frac      *= 10;
    int digit  = static_cast<int>(frac);
    *p++       = '0' + digit;
    frac      -= digit;
    if (1 == buffer_size--)
      break;
  }

  *p = '\0';
  return std::strlen(buf);
}

bool IOStream::parse_uint(uint64_t& output, Redix_Mode base, const uint32_t timeout)
{
  if (!m_is_open)
    return 0;

  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return 0;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return 0;
  }

  uint64_t value = 0;
  char     ch;
  bool     has_value = false;

  skip_char(available, base);

  int mat = match_char('-', available);

  if (-1 == mat)
    goto parse_uint_error;
  else if (1 == mat)
    read_char(ch, available);

  if (Auto_Mode == base)
  {
    if (false == read_char(ch, available))
      goto parse_uint_error;

    if (ch == '0')
    {
      if (false == read_char(ch, available))
        goto parse_uint_error;

      if (ch == 'x' || ch == 'X')
        base = Hex_Mode;
      else
      {
        base = Oct_Mode;
        unget_char(ch, available);
      }
    }
    else
    {
      base = Dec_Mode;
      unget_char(ch, available);
    }
  }

  while (true == read_char(ch, available))
  {
    if (ch >= '0' && ch <= '9')
    {
      value     = value * base + (ch - '0');
      has_value = true;
    }
    else
    {
      unget_char(ch, available);
      break;
    }
  }

  if (!istream_available())
    hard_recv_clean_bit();

  if (has_value)
  {
    output = value;
    return true;
  }
  else
    return false;

parse_uint_error:
  if (!istream_available())
    hard_recv_clean_bit();
  return false;
}

bool IOStream::parse_int(int64_t& output, Redix_Mode base, const uint32_t timeout)
{
  if (!m_is_open)
    return 0;

  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return 0;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return 0;
  }

  long long value = 0;
  int       sign  = 1;
  char      ch;
  bool      has_value = false;

  skip_char(available, base);

  sign = match_char('-', available);

  if (-1 == sign)
    goto parse_int_error;
  else if (0 == sign)
    sign = 1;
  else
    sign = -1;

  if (Auto_Mode == base)
  {
    if (false == read_char(ch, available))
      goto parse_int_error;

    if (ch == '0')
    {
      if (false == read_char(ch, available))
        goto parse_int_error;

      if (ch == 'x' || ch == 'X')
        base = Hex_Mode;
      else
      {
        base = Oct_Mode;
        unget_char(ch, available);
      }
    }
    else
    {
      base = Dec_Mode;
      unget_char(ch, available);
    }
  }

  while (true == read_char(ch, available))
  {
    if (ch >= '0' && ch <= '9')
    {
      value     = value * base + (ch - '0');
      has_value = true;
    }
    else
    {
      unget_char(ch, available);
      break;
    }
  }

  if (!istream_available())
    hard_recv_clean_bit();

  if (has_value)
  {
    output = sign * value;
    return true;
  }
  else
    return false;

parse_int_error:
  if (!istream_available())
    hard_recv_clean_bit();
  return false;
}

bool IOStream::parse_double(double& output, const uint32_t timeout)
{
  if (!m_is_open)
    return 0.0;

  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return 0.0;

  Mutex_Guard locker(m_istream_mutex);
  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return 0.0;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return 0.0;
  }

  double value     = 0.0;
  double fraction  = 0.1;
  int    sign      = 1;
  bool   has_point = false;
  bool   has_value = false;
  char   ch;

  skip_char(available, 20);

  sign = match_char('-', available);
  if (-1 == sign)
  {
    if (!istream_available())
      hard_recv_clean_bit();
    return 0.0;
  }
  else if (0 == sign)
    sign = 1;
  else
    sign = -1;

  while (true == read_char(ch, available))
  {
    if (ch == '.')
    {
      if (has_value && !has_point)
        has_point = true;
      else
      {
        unget_char(ch, available);
        break;
      }
    }

    else if (ch >= '0' && ch <= '9')
    {
      if (false == has_point)
        value = value * 10.0f + (ch - '0');
      else
      {
        value    += (ch - '0') * fraction;
        fraction *= 0.1f;
      }

      has_value = true;
    }
    else
    {
      unget_char(ch, available);
      break;
    }
  }

  if (read_char(ch, available) && has_value)
  {
    if (ch == 'e' || ch == 'E')
    {
      int64_t exp = 0;
      if (parse_int(exp, Dec_Mode, timeout))
        value *= std::pow(10.0, exp);
    }
    else
    {
      unget_char(ch, available);
    }
  }

  if (!istream_available())
    hard_recv_clean_bit();

  if (has_value)
  {
    output = sign * value;
    return true;
  }
  else
    return false;
}

bool IOStream::parse_char(char& ch, const uint32_t timeout)
{
  if (!m_is_open)
    return 0;

  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return false;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return false;
  }

  bool ret = read_char(ch, available);

  if (!istream_available())
    hard_recv_clean_bit();

  return ret;
}

bool IOStream::parse_char(uint8_t& ch, const uint32_t timeout)
{
  if (!m_is_open)
    return 0;

  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (false == hard_recv_wait_bit(timeout))
  {
    ch       = 0;
    m_state |= Eof_Bit;
    return false;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    ch       = 0;
    m_state |= Eof_Bit;
    return false;
  }

  bool ret = read_char(ch, available);

  if (!istream_available())
    hard_recv_clean_bit();

  return ret;
}

uint32_t IOStream::parse_string(std::string& str, const uint32_t length, const uint32_t timeout)
{
  if (!m_is_open)
    return 0;

  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return 0;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return 0;
  }

  if (0 != length)
    available = std::min(available, length);

  skip_char(available, 1);

  char ch;

  while (true == read_char(ch, available))
  {
    if (ch == '\n' || ch == '\r')
    {
      break;
    }

    if (!isprint(ch))
    {
      unget_char(ch, available);
      break;
    }

    str += ch;
  }

  if (0 == str.length())
    m_state |= Eof_Bit;

  if (!istream_available())
    hard_recv_clean_bit();

  return str.length();
}

bool IOStream::hard_recv_input(void* buf, uint32_t length)
{
  if (nullptr == buf || 0 == length)
    return false;

  if (Stream_Mode::Read_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    return ul_port_os_stream_send(m_istream, buf, length) == length;
  else
    return false;
}

uint32_t IOStream::recv(void* buf, uint32_t length, const uint32_t timeout)
{
  if (!m_is_open)
    return 0;

  if (0 == length || nullptr == buf)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return hard_recv(buf, length, timeout);

  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return 0;
  }

  uint32_t available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return 0;
  }

  if (true == m_is_has_ungot_char)
  {
    memcpy(buf, &m_ungot_char, 1);
    length--;
    if (0 == length)
    {
      if (!istream_available())
        hard_recv_clean_bit();
      return 1;
    }
  }

  available = std::min(available, length);

  if (true == m_is_has_ungot_char)
  {
    m_is_has_ungot_char = false;
    length              = ul_port_os_stream_receive(m_istream, static_cast<char*>(buf) + 1, available) + 1;
  }
  else
    length = ul_port_os_stream_receive(m_istream, buf, available);

  if (0 == length)
    m_state |= Eof_Bit;

  if (!istream_available())
    hard_recv_clean_bit();

  return length;
}

int IOStream::recv(void* buf, int length, const int timeout)
{
  if (!m_is_open)
    return 0;

  if (0 == length || nullptr == buf)
    return 0;

  Mutex_Guard locker(m_istream_mutex);
  if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
    return hard_recv(buf, length, timeout);

  if (false == hard_recv_wait_bit(timeout))
  {
    m_state |= Eof_Bit;
    return 0;
  }

  int available = istream_available();
  if (0 == available)
  {
    m_state |= Eof_Bit;
    return 0;
  }

  if (true == m_is_has_ungot_char)
  {
    memcpy(buf, &m_ungot_char, 1);
    length--;
    if (0 == length)
    {
      if (!istream_available())
        hard_recv_clean_bit();
      return 1;
    }
  }

  available = std::min(available, length);

  if (true == m_is_has_ungot_char)
  {
    m_is_has_ungot_char = false;
    length              = ul_port_os_stream_receive(m_istream, static_cast<char*>(buf) + 1, available) + 1;
  }
  else
    length = ul_port_os_stream_receive(m_istream, buf, available);

  if (0 == length)
    m_state |= Eof_Bit;

  if (!istream_available())
    hard_recv_clean_bit();

  return length;
}
