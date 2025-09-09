#ifndef __IOSTREAM_HPP__
#define __IOSTREAM_HPP__

#include "object.hpp"
#include <algorithm>
#include <cmath>

/// @brief 名称空间 库名
namespace OwO
{
/// @brief 名称空间 基础元素
namespace system
{
namespace Class
{
class Auto_Detect
{
};

class Bin
{
};

class Oct
{
};

class Dec
{
};

class Hex
{
};

class Endl
{
};

class Ends
{
};
} /* namespace Class */

const Class::Auto_Detect auto_detect;
const Class::Bin         bin;
const Class::Oct         oct;
const Class::Dec         dec;
const Class::Hex         hex;
const Class::Endl        endl;
const Class::Ends        ends;

class Precision
{
  friend class IOStream;
  uint8_t m_precision;

public:
  explicit Precision(const uint8_t& precision) : m_precision(precision) {}
};

class IOStream : public Object
{
  O_MEMORY
  O_OBJECT
  NO_COPY(IOStream)
  NO_MOVE(IOStream)

private:
  enum Redix_Mode
  {
    Auto_Mode = 0,
    Bin_Mode  = 2,
    Oct_Mode  = 8,
    Dec_Mode  = 10,
    Hex_Mode  = 16,
  };

public:
  enum class Stream_Mode
  {
    No_Buffer   = 0x00,
    Read_Buffer = 0x01,
    Send_Buffer = 0x04,
    Both_Buffer = Read_Buffer | Send_Buffer,
  };

  enum Stream_State
  {
    Good_Bit = 0x00,
    Eof_Bit  = 0x01,
    Over_Bit = 0x02,
  };

private:
  char        m_state;
  Stream_Mode m_stream_mode;

protected:
  Stream_Mode stream_mode() const
  {
    return m_stream_mode;
  }

  virtual bool open(const uint32_t istream_size = 0, const uint32_t ostream_size = 0)
  {
    if (m_is_open)
      return false;

    if (istream_size > 0 && ostream_size > 0)
      m_stream_mode = Stream_Mode::Both_Buffer;
    else if (istream_size > 0)
      m_stream_mode = Stream_Mode::Read_Buffer;
    else if (ostream_size > 0)
      m_stream_mode = Stream_Mode::Send_Buffer;
    else
      m_stream_mode = Stream_Mode::No_Buffer;

    if (Stream_Mode::No_Buffer == m_stream_mode)
      return true;

    bool ret = true;

    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      if (nullptr == m_ostream)
      {
        m_ostream_size = ostream_size;
        m_ostream      = pt_port_os_stream_create(ostream_size);
        if (nullptr == m_ostream)
          ret = false;
      }
    }

    if (Stream_Mode::Read_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      if (nullptr == m_istream)
      {
        m_istream_size = istream_size;
        m_istream      = pt_port_os_stream_create(istream_size);
        if (nullptr == m_istream)
          ret = false;
      }
    }

    m_is_open = true;

    if (!ret)
      close();

    return ret;
  }

  virtual bool close()
  {
    if (!m_is_open)
      return true;

    if (m_istream)
    {
      v_port_os_stream_delete(m_istream);
      m_istream = nullptr;
    }

    if (m_ostream)
    {
      v_port_os_stream_delete(m_ostream);
      m_ostream = nullptr;
    }

    m_is_open = false;
    return true;
  }

public:
  explicit IOStream(const std::string& name, Object* parent = nullptr) : Object(name, parent)
  {
    m_stream_mode  = Stream_Mode::No_Buffer;
    m_state        = Good_Bit;
    m_ostream      = nullptr;
    m_ostream_size = 0;
    m_istream      = nullptr;
    m_istream_size = 0;
    m_is_open      = false;
  }

  bool good() const
  {
    return m_state == Good_Bit;
  }

  bool eof() const
  {
    return m_state & Eof_Bit;
  }

  bool over() const
  {
    return m_state & Over_Bit;
  }

  void clear_bit(Stream_State state = Good_Bit)
  {
    m_state &= !state;
  }

  virtual ~IOStream()
  {
    close();
  }

#if 1 /* ----------------------------------------- Out-Stream ----------------------------------------- */
public:
  enum class Send_mode
  {
    Line_Buffer = 0x00, /* 行输出模式   (行尾字符与endl检测) */
    Full_Buffer = 0x01, /* 完整数据模式 (手动) */
    Direct      = 0x02, /* 数据直传模式 */
  };

private:
  char             m_send_precision  = 2;
  char             m_format_ram_size = 24;
  char             m_delimiter       = '\n';
  Send_mode        m_send_mode       = Send_mode::Line_Buffer;
  Redix_Mode       m_send_base       = Dec_Mode;
  uint32_t         m_ostream_size;
  port_os_stream_t m_ostream;
  kernel::Mutex    m_ostream_mutex;
  kernel::Mutex    m_hard_send_mutex;

public:
  static uint32_t format_uint(char* ram, uint32_t ramfer_size, uint64_t value, const uint8_t base);
  static uint32_t format_int(char* ram, uint32_t ramfer_size, int64_t value, const uint8_t base);
  static uint32_t format_double(char* ram, uint32_t ramfer_size, double value, const uint8_t precision);

protected:
  uint32_t ostream_size() const
  {
    return m_ostream_size;
  }

private:
  uint32_t send_ram(const void* ram, uint32_t length, const bool flush)
  {
    uint32_t send_len = send(ram, length, flush);
    if (send_len != length)
      m_state |= Over_Bit;
    return send_len;
  }

  uint32_t send_uint(const uint64_t& value, const Redix_Mode base, const bool flush)
  {
    char     ram[m_format_ram_size];
    uint32_t length = format_uint(ram, m_format_ram_size, value, base);
    return send_ram(ram, length, flush);
  }

  uint32_t send_int(const int64_t& value, const Redix_Mode base, const bool flush)
  {
    char     ram[m_format_ram_size];
    uint32_t length = format_int(ram, m_format_ram_size, value, base);
    return send_ram(ram, length, flush);
  }

  uint32_t send_double(const double& value, const uint8_t precision, const bool flush)
  {
    char     ram[m_format_ram_size];
    uint32_t length = format_double(ram, m_format_ram_size, value, precision);
    return send_ram(ram, length, flush);
  }

  template <typename T>
  uint32_t format_send(const T& value)
  {
    if (std::is_floating_point<T>::value == true)
      return send_double(value, m_send_precision, (Send_mode::Direct == m_send_mode));
    else if (std::is_unsigned<T>::value == true)
      return send_uint(value, m_send_base, (Send_mode::Direct == m_send_mode));
    else
      return send_int(value, m_send_base, (Send_mode::Direct == m_send_mode));
  }

  template <typename T>
  uint32_t format_print(const T& value, const Redix_Mode base, const bool flush)
  {
    if (std::is_unsigned<T>::value == true)
      return send_uint(value, base, flush);
    else
      return send_int(value, base, flush);
  }

  uint32_t flush(const void* data, uint32_t length)
  {
    if (!m_is_open)
      return 0;

    uint32_t available = ul_port_os_stream_available(m_ostream);
    if (available > 0)
    {
      m_hard_send_mutex.lock();
      char ram[available + length];
      ul_port_os_stream_receive(m_ostream, ram, available);
      memcpy(ram + available, data, length);
      send_start();
      return hard_send(ram, length + available);
    }
    else
    {
      m_hard_send_mutex.lock();
      send_start();
      return hard_send(data, length);
    }
  }

protected:
  virtual void     send_start()                                = 0;
  virtual void     send_end()                                  = 0;
  virtual uint32_t hard_send(const void* ram, uint32_t length) = 0;
  void             hard_send_end()
  {
    send_end();
    m_hard_send_mutex.unlock();
  }

public:
  void set_format_ram_size(const uint8_t& size = 24)
  {
    m_format_ram_size = size;
  }

  void set_ostream_send_mode(const Send_mode& mode)
  {
    m_send_mode = mode;
    if (mode == Send_mode::Direct)
      IOStream::flush();
  }

  void set_ostream_delimiter(const char& delimiter = '\n')
  {
    m_delimiter = delimiter;
  }

  void set_ostream_precision(const char& precision)
  {
    m_send_precision = precision;
  }

  void set_ostream_base(const uint8_t base)
  {
    if (base == 2)
      m_send_base = Bin_Mode;
    else if (base == 8)
      m_send_base = Oct_Mode;
    else if (base == 10)
      m_send_base = Dec_Mode;
    else if (base == 16)
      m_send_base = Hex_Mode;
  }

  void set_ostream_base(const Class::Bin&)
  {
    m_send_base = Bin_Mode;
  }

  void set_ostream_base(const Class::Oct&)
  {
    m_send_base = Oct_Mode;
  }

  void set_ostream_base(const Class::Dec&)
  {
    m_send_base = Dec_Mode;
  }

  void set_ostream_base(const Class::Hex&)
  {
    m_send_base = Hex_Mode;
  }

  bool ostrean_reset()
  {
    if (!m_is_open)
      return false;

    kernel::Mutex_Guard locker(m_ostream_mutex);
    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
      return b_port_os_stream_reset(m_ostream);
    else
      return true;
  }

  uint32_t ostream_available()
  {
    if (!m_is_open)
      return 0;

    kernel::Mutex_Guard locker(m_ostream_mutex);
    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
      return ul_port_os_stream_available(m_ostream);
    else
      return 0;
  }

  uint32_t flush()
  {
    if (!m_is_open)
      return 0;

    kernel::Mutex_Guard locker(m_ostream_mutex);
    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      uint32_t available = ul_port_os_stream_available(m_ostream);
      if (available > 0)
      {
        m_hard_send_mutex.lock();
        char ram[available];
        ul_port_os_stream_receive(m_ostream, ram, available);
        send_start();
        return hard_send(ram, available);
      }
    }
    return 0;
  }

  uint32_t send(const void* data, uint32_t length, const bool flush = true)
  {
    if (!m_is_open)
      return 0;

    kernel::Mutex_Guard locker(m_ostream_mutex);
    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      if (false == flush)
        return ul_port_os_stream_send(m_ostream, data, length);
      else
        return IOStream::flush(data, length);
    }
    else
    {
      m_hard_send_mutex.lock();
      send_start();
      return hard_send(data, length);
    }
  }

  IOStream& operator<<(const Class::Bin&)
  {
    m_send_base = Bin_Mode;
    return *this;
  }

  IOStream& operator<<(const Class::Oct&)
  {
    m_send_base = Oct_Mode;
    return *this;
  }

  IOStream& operator<<(const Class::Dec&)
  {
    m_send_base = Dec_Mode;
    return *this;
  }

  IOStream& operator<<(const Class::Hex&)
  {
    m_send_base = Hex_Mode;
    return *this;
  }

  IOStream& operator<<(const Precision& precision)
  {
    m_send_precision = precision.m_precision;
    return *this;
  }

  IOStream& operator<<(const Class::Endl&)
  {
    *this << m_delimiter;
    if (Send_mode::Full_Buffer != m_send_mode)
      flush();
    return *this;
  }

  IOStream& operator<<(const Class::Ends&)
  {
    flush();
    return *this;
  }

  template <typename T>
  IOStream& operator<<(const T& value)
  {
    format_send(value);
    return *this;
  }

  template <typename T>
  IOStream& operator<<(T* value)
  {
    send_ram(value, sizeof(T), (Send_mode::Direct == m_send_mode));
    return *this;
  }

  IOStream& operator<<(const bool& value)
  {
    if (value)
      *this << "true";
    else
      *this << "false";
    return *this;
  }

  IOStream& operator<<(const char& c)
  {
    if (Send_mode::Line_Buffer == m_send_mode)
      send_ram(&c, 1, (m_delimiter == c));
    else
      send_ram(&c, 1, (Send_mode::Direct == m_send_mode));
    return *this;
  }

  IOStream& operator<<(const std::string& str)
  {
    print(str, (Send_mode::Direct == m_send_mode));
    return *this;
  }

  IOStream& operator<<(const char* str)
  {
    print(str, strlen(str), (Send_mode::Direct == m_send_mode));
    return *this;
  }

  template <typename T>
  uint32_t print(const T& value, bool flush = true)
  {
    return format_print(value, Dec_Mode, flush);
  }

  template <typename T>
  uint32_t print(const T& value, Class::Bin&, bool flush = true)
  {
    return format_print(value, Bin_Mode, flush);
  }

  template <typename T>
  uint32_t print(const T& value, Class::Oct&, bool flush = true)
  {
    return format_print(value, Oct_Mode, flush);
  }

  template <typename T>
  uint32_t print(const T& value, Class::Dec&, bool flush = true)
  {
    return format_print(value, Dec_Mode, flush);
  }

  template <typename T>
  uint32_t print(const T& value, Class::Hex&, bool flush = true)
  {
    return format_print(value, Hex_Mode, flush);
  }

  template <typename T>
  uint32_t print(const T* value, uint8_t base, bool flush = true)
  {
    if (base == 2)
      return format_print(*value, Bin_Mode, flush);
    else if (base == 8)
      return format_print(*value, Oct_Mode, flush);
    else if (base == 10)
      return format_print(*value, Dec_Mode, flush);
    else if (base == 16)
      return format_print(*value, Hex_Mode, flush);
    else
      return 0;
  }

  template <typename T>
  uint32_t print(T* value, bool flush = true)
  {
    return send_ram(value, sizeof(T), flush);
  }

  uint32_t print(const bool& value, bool flush = true)
  {
    if (value)
      return print("true", flush);
    else
      return print("false", flush);
  }

  uint32_t print(const char& c, bool flush = true)
  {
    return send_ram(&c, 1, flush);
  }

  uint32_t print(const float& value, const uint8_t precision = 2, bool flush = true)
  {
    return send_double(value, precision, flush);
  }

  uint32_t print(const double& value, const uint8_t precision = 2, bool flush = true)
  {
    return send_double(value, precision, flush);
  }

  uint32_t print(const std::string& str, bool flush = false)
  {
    if (!m_is_open)
      return 0;

    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      if (flush)
        return send_ram(str.c_str(), str.length(), true);

      if (Send_mode::Line_Buffer == m_send_mode)
        return send_ram(str.c_str(), str.length(), std::string::npos != str.find_first_of(m_delimiter));
    }
    else
      return send_ram(str.c_str(), str.length(), flush);

    return 0;
  }

  uint32_t print(const char* str, uint32_t length, bool flush = true)
  {
    if (!m_is_open)
      return 0;

    if (Stream_Mode::Send_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      if (flush)
        return send_ram(str, length, flush);

      if (Send_mode::Line_Buffer == m_send_mode)
        return send_ram(str, length, strchr(str, m_delimiter));
    }
    else
      return send_ram(str, length, flush);

    return 0;
  }

#endif /* ---------------------------------------- Out-Stream ----------------------------------------- */

#if 1 /* ------------------------------------------ In-Stream ----------------------------------------- */
private:
  uint32_t         m_recv_time_out     = WAIT_FOREVER;
  Redix_Mode       m_recv_base         = Auto_Mode;
  bool             m_is_has_ungot_char = false;
  char             m_ungot_char;
  port_os_stream_t m_istream;
  uint32_t         m_istream_size;
  kernel::Mutex    m_istream_mutex;

protected:
  port_os_stream_t istream()
  {
    return m_istream;
  }

  uint32_t istream_size() const
  {
    return m_istream_size;
  }

private:
  bool get_char(void* ch)
  {
    if (Stream_Mode::Read_Buffer == m_stream_mode || Stream_Mode::Both_Buffer == m_stream_mode)
    {
      if (0 == ul_port_os_stream_receive(m_istream, ch, 1))
        return false;
      else
        return true;
    }
    else
    {
      if (0 == hard_recv(ch, 1, 0))
        return false;
      else
        return true;
    }
  }

  bool read_char(uint8_t& uch, uint32_t& length)
  {
    if (0 == length)
      return false;

    if (true == m_is_has_ungot_char)
    {
      uch                 = m_ungot_char;
      m_is_has_ungot_char = false;
      length--;
      return true;
    }

    if (get_char(&uch))
    {
      length--;
      return true;
    }
    else
      return false;
  }

  bool read_char(char& ch, uint32_t& length)
  {
    if (0 == length)
      return false;

    if (true == m_is_has_ungot_char)
    {
      ch                  = m_ungot_char;
      m_is_has_ungot_char = false;
      length--;
      return true;
    }

    if (get_char(&ch))
    {
      length--;
      return true;
    }
    else
      return false;
  }

  void unget_char(const char& c, uint32_t& length)
  {
    m_is_has_ungot_char = true;
    m_ungot_char        = c;
    length++;
  }

  /**
   * @brief IOStream 跳过不合法字符
   *
   * @param length 剩余长度
   * @param type   类型(0-自动进制,1-字符,2-二进制,8-八进制,10-十进制,16-十六进制,20-小数)
   */
  void skip_char(uint32_t& length, const uint8_t type)
  {
    char ch;
    while (read_char(ch, length))
    {
      if (type == 0)
      {
        if (isxdigit(ch) || '-' == ch)
        {
          unget_char(ch, length);
          return;
        }
      }
      else if (type == 1)
      {
        if (isprint(ch))
        {
          unget_char(ch, length);
          return;
        }
      }
      else if (type == 2)
      {
        if (0 == ch || 1 == ch)
        {
          unget_char(ch, length);
          return;
        }
      }
      else if (type == 8)
      {
        if (0 == ch || 1 == ch || 2 == ch || 3 == ch || 4 == ch || 5 == ch || 6 == ch || 7 == ch)
        {
          unget_char(ch, length);
          return;
        }
      }
      else if (type == 10)
      {
        if (isdigit(ch) || '-' == ch)
        {
          unget_char(ch, length);
          return;
        }
      }
      else if (type == 16)
      {
        if (isxdigit(ch))
        {
          unget_char(ch, length);
          return;
        }
      }
      else if (type == 20)
      {
        if (isdigit(ch) || '-' == ch || '.' == ch || 'e' == ch || 'E' == ch)
        {
          unget_char(ch, length);
          return;
        }
      }
    }
  }

  int match_char(const char expected, uint32_t& length)
  {
    char actual;
    if (false == read_char(actual, length))
      return -1;

    if (expected == actual)
      return 1;
    else
      unget_char(actual, length);

    return 0;
  }

  bool     parse_uint(uint64_t& output, Redix_Mode base, const uint32_t timeout);
  bool     parse_int(int64_t& output, Redix_Mode base, const uint32_t timeout);
  bool     parse_double(double& output, const uint32_t timeout);
  bool     parse_char(char& ch, const uint32_t timeout);
  bool     parse_char(uint8_t& ch, const uint32_t timeout);
  uint32_t parse_string(std::string& str, uint32_t length, const uint32_t timeout);

  template <typename T>
  bool parse_recv(T& value, const uint32_t& timeout)
  {
    if (std::is_floating_point<T>::value == true)
    {
      double d_val = 0;
      if (false == parse_double(d_val, timeout))
        return false;
      value = d_val;
      return true;
    }
    else if (std::is_unsigned<T>::value == true)
    {
      uint64_t u_val = 0;
      if (false == parse_uint(u_val, m_recv_base, timeout))
        return false;
      value = u_val;
      return true;
    }
    else
    {
      int64_t s_val = 0;
      if (false == parse_int(s_val, m_recv_base, timeout))
        return false;
      value = s_val;
      return true;
    }
  }

  template <typename T>
  bool parse_scan(T& value, const Redix_Mode& base, const uint32_t& timeout)
  {
    if (std::is_unsigned<T>::value == true)
    {
      uint64_t u_val = 0;
      if (false == parse_uint(u_val, base, timeout))
        return false;
      value = u_val;
      return true;
    }
    else
    {
      int64_t s_val = 0;
      if (false == parse_int(s_val, base, timeout))
        return false;
      value = s_val;
      return true;
    }
  }

protected:
  virtual bool     hard_recv_wait_bit(uint32_t timeout)                    = 0;
  virtual bool     hard_recv_clean_bit()                                   = 0;
  virtual uint32_t hard_recv(void* buf, uint32_t length, uint32_t timeout) = 0;
  bool             hard_recv_input(void* buf, uint32_t length);

public:
  uint32_t recv(void* buf, uint32_t length, const uint32_t timeout = WAIT_FOREVER);
  int      recv(void* buf, int length, const int timeout = WAIT_FOREVER);

  void set_istream_base(uint8_t base)
  {
    if (2 == base)
      m_recv_base = Bin_Mode;
    else if (8 == base)
      m_recv_base = Oct_Mode;
    else if (10 == base)
      m_recv_base = Dec_Mode;
    else if (16 == base)
      m_recv_base = Hex_Mode;
    else
      return;
  }

  void set_istream_base(const Class::Auto_Detect&)
  {
    m_recv_base = Auto_Mode;
  }

  void set_istream_base(const Class::Bin&)
  {
    m_recv_base = Bin_Mode;
  }

  void set_istream_base(const Class::Oct&)
  {
    m_recv_base = Oct_Mode;
  }

  void set_istream_base(const Class::Dec&)
  {
    m_recv_base = Dec_Mode;
  }

  void set_istream_base(const Class::Hex&)
  {
    m_recv_base = Hex_Mode;
  }

  void set_istream_time_out(const uint32_t time_out)
  {
    m_recv_time_out = time_out;
  }

  uint32_t istream_available()
  {
    if (!is_open())
      return 0;

    if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
      return 0;

    if (true == m_is_has_ungot_char)
      return ul_port_os_stream_available(m_istream) + 1;
    else
      return ul_port_os_stream_available(m_istream);
  }

  bool istream_reset()
  {
    if (!is_open())
      return true;

    if (Stream_Mode::No_Buffer == m_stream_mode || Stream_Mode::Send_Buffer == m_stream_mode)
      return true;

    kernel::Mutex_Guard locker(m_istream_mutex);
    hard_recv_clean_bit();
    return b_port_os_stream_reset(m_istream);
  }

  IOStream& operator>>(const Class::Bin&)
  {
    m_recv_base = Bin_Mode;
    return *this;
  }

  IOStream& operator>>(const Class::Oct&)
  {
    m_recv_base = Oct_Mode;
    return *this;
  }

  IOStream& operator>>(const Class::Dec&)
  {
    m_recv_base = Dec_Mode;
    return *this;
  }

  IOStream& operator>>(const Class::Hex&)
  {
    m_recv_base = Hex_Mode;
    return *this;
  }

  IOStream& operator>>(const Class::Auto_Detect&)
  {
    m_recv_base = Auto_Mode;
    return *this;
  }

  template <typename T>
  IOStream& operator>>(T& value)
  {
    parse_recv(value, m_recv_time_out);
    return *this;
  }

  IOStream& operator>>(char& value)
  {
    parse_char(value, m_recv_time_out);
    return *this;
  }

  IOStream& operator>>(uint8_t& value)
  {
    parse_char(value, m_recv_time_out);
    return *this;
  }

  template <typename T>
  IOStream& operator>>(T* value)
  {
    recv(value, sizeof(T), m_recv_time_out);
    return *this;
  }

  IOStream& operator>>(char* value)
  {
    parse_char(*value, m_recv_time_out);
    return *this;
  }

  IOStream& operator>>(uint8_t* value)
  {
    parse_char(*value, m_recv_time_out);
    return *this;
  }

  IOStream& operator>>(std::string& str)
  {
    parse_string(str, 0, m_recv_time_out);
    return *this;
  }

  template <typename T>
  bool scan(T& value, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_scan(value, Auto_Mode, timeout);
  }

  template <typename T>
  bool scan(T& value, const Class::Bin&, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_scan(value, Bin_Mode, timeout);
  }

  template <typename T>
  bool scan(T& value, const Class::Oct&, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_scan(value, Oct_Mode, timeout);
  }

  template <typename T>
  bool scan(T& value, const Class::Dec&, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_scan(value, Dec_Mode, timeout);
  }

  template <typename T>
  bool scan(T& value, const Class::Hex&, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_scan(value, Hex_Mode, timeout);
  }

  template <typename T>
  bool scan(T& value, const Class::Auto_Detect&, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_scan(value, Auto_Mode, timeout);
  }

  template <typename T>
  bool scan(T* value, const uint8_t base, const uint32_t timeout = WAIT_FOREVER)
  {
    if (2 == base)
      return parse_scan(*value, Bin_Mode, timeout);
    else if (8 == base)
      return parse_scan(*value, Oct_Mode, timeout);
    else if (10 == base)
      return parse_scan(*value, Dec_Mode, timeout);
    else if (16 == base)
      return parse_scan(*value, Hex_Mode, timeout);
    else
      return false;
  }

  bool scan(char& value, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_char(value, timeout);
  }

  bool scan(uint8_t& value, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_char(value, m_recv_time_out);
  }

  bool scan(float& value, const uint32_t timeout = WAIT_FOREVER)
  {
    double d_val = 0;
    if (false == parse_double(d_val, timeout))
      return false;
    value = d_val;
    return true;
  }

  bool scan(double& value, const uint32_t timeout = WAIT_FOREVER)
  {
    return parse_double(value, timeout);
  }

  template <typename T>
  bool scan(T* value, const uint32_t timeout = WAIT_FOREVER)
  {
    recv(value, sizeof(T), timeout);
    if (Eof_Bit == m_state)
      return false;
    else
      return true;
  }

  bool scan(char* value, const uint32_t& timeout = WAIT_FOREVER)
  {
    return parse_char(*value, timeout);
  }

  bool scan(uint8_t* value, const uint32_t& timeout = WAIT_FOREVER)
  {
    return parse_char(*value, timeout);
  }

  uint32_t scan(char* str, uint32_t length, const uint32_t& timeout = WAIT_FOREVER)
  {
    return recv(str, length, timeout);
  }

  uint32_t scan(std::string& str, uint32_t length, const uint32_t& timeout = WAIT_FOREVER)
  {
    return parse_string(str, length, timeout);
  }

#endif /* ----------------------------------------- In-Stream ----------------------------------------- */
};
} /* namespace system */
} /* namespace OwO */

#endif /* __IOSTREAM_HPP__ */
