// Copyright (c) 2013 Thom Chiovoloni.
// This file is distributed under the terms of the Boost Software License.
// See LICENSE.md at the root of this distribution for details.
#ifndef LOGGO_LOGGO_HH
#define LOGGO_LOGGO_HH
#include <string>
#include <iostream>

/// \file Some basic logging.
///
/// You can disable logging by calling `loggo::Logger::get().disable()`,
/// or, equivalently, `loggo::Logger::disableLogging()`
///
/// Avoid including this file in headers, as it includes `<iostream>`
///

// This isn't 100% thread safe, but it won't deadlock or get corrupted or anything horrifying.
// The worst thing that can happen is that the output gets a bit garbled or that it writes
// shortly after being disabled.

namespace loggo {

  enum LogLevel : unsigned { Debug, Trace, Info, Warn, Error, Fatal, };

  inline bool compareLevels(LogLevel min, LogLevel cur) {
    return unsigned(min) <= unsigned(cur);
  }

  inline std::ostream &writeLevel(std::ostream &o, LogLevel lvl, bool color) {
    if (color) {
      switch (lvl) {
        case Debug: return o << "\033[30m"   "Debug " "\033[0m";
        case Trace: return o << "\033[30;1m" "Trace " "\033[0m";
        case Info:  return o << "\033[33m"   "Info " "\033[0m";
        case Warn:  return o << "\033[33;1m" "Warn " "\033[0m";
        case Error: return o << "\033[31m"   "Error " "\033[0m";
        case Fatal: return o << "\033[31;1m" "Fatal " "\033[0m";
        default:    return o << "?????";
      }
    } else {
      switch (lvl) {
        case Debug: return o << "DEBUG";
        case Trace: return o << "TRACE";
        case Info:  return o << "INFO ";
        case Warn:  return o << "WARN ";
        case Error: return o << "ERROR";
        case Fatal: return o << "FATAL";
        default:    return o << "?????";
      }
    }
  }

  struct StreamOut {


    template <class T>
    StreamOut &operator<<(T &&t) {
      if (write_) {
        ostream_ << std::forward<T>(t);
      }
      return *this;
    }

    template <class T>
    StreamOut &operator<<(T const &t) {
      if (write_) {
        ostream_ << t;
      }
      return *this;
    }


    ~StreamOut() {
      if (write_) {
        ostream_ << std::endl;
      }
    }

  private:

    friend class Logger;

    explicit StreamOut(std::ostream &o, bool write, bool color, LogLevel l, char const *file, int line, char const *func)
    : ostream_(o), write_(write) {
      if (write) {
        o << '[';
        writeLevel(o, l, color);
        if (file) {
          o << '|' << file;
          if (line >= 0) {
            o << ':' << line;
          }
        }
        if (func) {
          o << '(' << func << ')';
        }
        o << "]: ";
      }
    }

    std::ostream &ostream_;
    bool write_;
  };

  class Logger {
  public:
    /// Construct a logger with minimum level \p level which writes to `o`.
    explicit Logger(std::ostream &o, LogLevel level=Warn, bool useColor=false)
    : level_(level)
    , enabled_(true)
    , colorsEnabled_(useColor)
    , out_(&o)
    {}

    /// get the singleton logger instance.
    /// it writes to std::cerr by default, with minimum level
    /// Warn if NDEBUG is defined, and Debug otherwise.
    static Logger &get() { return *globalLoggerPtr(); }

    /// Set the global logger. This is probably
    static void setGlobalLogger(Logger &logger) { *globalLoggerPtr() = logger; }

    /// Get our minimum log level
    LogLevel getMinLevel() const { return level_; }

    /// Disable printing in color
    void disableColor() { colorsEnabled_ = false; }

    /// Enable printing in color
    void enableColor() { colorsEnabled_ = true; }

    /// are colors enabled?
    bool colorsEnabled() const { return colorsEnabled_; }

    /// Set the minimum log level that we will actually write out
    void setMinLevel(LogLevel to) { level_ = to; }

    /// Are we enabled?
    bool isEnabled() const { return enabled_; }

    /// set whether or not we log anything
    void setEnabled(bool v) { enabled_ = v; }

    /// Disable all logging for this logger
    void disable() { setEnabled(false); }

    /// Enable all logging for this logger
    void enable() { setEnabled(true); }

    /// get the ostream that we write to (default is std::cerr)
    std::ostream &getStream() const { return *out_; }

    /// set the ostream that we write to (default is std::cerr)
    /// and optionally enable color for it.
    void setStream(std::ostream &o, bool color = false) {
      out_ = &o;
      colorsEnabled_ = color;
    }

    /// get a stream for log level \p l that will report the file, line, and function.
    /// passing nullptr for \p file and \p func will disable their reporting, and
    /// passing -1 for \p line will disable its reporting. Line will only reported if it is
    /// non-negative and file is not nullptr.
    StreamOut streamFor(LogLevel l, char const *file=nullptr, int line=-1, char const *func=nullptr) {
      bool write = enabled_ && compareLevels(level_, l);
      return StreamOut(*out_, write, colorsEnabled_, l, file, line, func);
    }

  private:

    static Logger *globalLoggerPtr() {
      static Logger globalLogger{std::cerr, Debug, true};
      return &globalLogger;
    }

    LogLevel level_; // minimum log level
    bool enabled_; // are we enabled?
    bool colorsEnabled_;
    std::ostream *out_; // pointer to output stream

  };


  /// shorthand for Logger::get()
  inline Logger &logger() { return Logger::get(); }



}

/// Define `LOGGO_NO_FILE` to disable writting the file name/line to the stream
#ifdef LOGGO_NO_FILE
# define LOGGO_FILE nullptr
#else
# define LOGGO_FILE (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : (__FILE__))
#endif

#ifdef LOGGO_NO_LINE
# define LOGGO_LINE -1
#else
# define LOGGO_LINE __LINE__
#endif

/// Define `LOGGO_PRETTY_FUNC` to use a custom pretty function in SCML_DEBUG_REACHED()
#ifndef LOGGO_PRETTY_FUNC
# if (defined __GNUC__)
#  define LOGGO_PRETTY_FUNC __PRETTY_FUNCTION__
# elif (defined _MSC_VER)
#  define LOGGO_PRETTY_FUNC __FUNCSIG__
# elif (defined LOGGO_FUNC)
#  define LOGGO_PRETTY_FUNC LOGGO_FUNC
# else
#  define LOGGO_PRETTY_FUNC __func__
# endif
#endif

/// Define `LOGGO_FUNC` to use a custom func macro instead of __func__
/// Define `LOGGO_NO_FUNC` to disable writing the function name in the stream
#ifdef LOGGO_NO_FUNC
# undef LOGGO_FUNC
# define LOGGO_FUNC nullptr
#elif !(defined LOGGO_FUNC)
# define LOGGO_FUNC __func__
#endif

/// Get the singleton logger's stream for \p LEVEL.
#define LOGGO_LOG(LEVEL) ::loggo::Logger::get().streamFor(LEVEL, LOGGO_FILE, LOGGO_LINE, LOGGO_FUNC)

/// \name singlton log stream getters. Use like: `LOGGO_DEBUG() << "my_var: " << my_var;`
/// @{
#define LOGGO_DEBUG() LOGGO_LOG(::loggo::Debug)
#define LOGGO_TRACE() LOGGO_LOG(::loggo::Trace)
#define LOGGO_INFO() LOGGO_LOG(::loggo::Info)
#define LOGGO_WARN() LOGGO_LOG(::loggo::Warn)
#define LOGGO_ERROR() LOGGO_LOG(::loggo::Error)
#define LOGGO_FATAL() LOGGO_LOG(::loggo::Fatal)
/// @}


#ifndef LOGGO_NO_FUNC
/// Write `"REACHED: #{pretty-printed function name}"` to debug stream
# define LOGGO_DEBUG_REACHED() LOGGO_DEBUG() << "REACHED: " << LOGGO_PRETTY_FUNC
#else
# define LOGGO_DEBUG_REACHED() ((void)0)
#endif

/// write `"varname = #{varname}" to the debug stream
#define LOGGO_DEBUG_VAR(varname) LOGGO_DEBUG() << #varname " = " << varname

#endif
