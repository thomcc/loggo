// Copyright (c) 2013 Thom Chiovoloni.
// This file is distributed under the terms of the Boost Software License.
// See LICENSE.md at the root of this distribution for details.


// disable writing the function name in the log stream
//#define LOGGO_NO_FUNC

// disable writing the filename and line number to the log stream.
//#define LOGGO_NO_FILE

// disable writing the line number to the log stream
//#define LOGGO_NO_LINE

#include "loggo/Loggo.hh"

struct Custom {
  int x;
  int y;

};

std::ostream &operator<<(std::ostream &o, Custom const &c) {
  return o << "Custom{" << c.x << ", " << c.y << "}";
}

void foo() {
  LOGGO_DEBUG_REACHED();
}


int main() {
  Custom custom{43, 200};
  LOGGO_INFO() << "foo " << 3 << ", " << custom;

  Custom other{50, 50};

  LOGGO_DEBUG_VAR(other);

  foo();

  loggo::logger().disable();

    LOGGO_FATAL() << "shouldn't print";

  loggo::logger().enable();

  loggo::logger().setMinLevel(loggo::Warn);

  LOGGO_TRACE() << "shouldn't print";

  LOGGO_WARN() << "bad mojo (should print)";

  loggo::logger().setMinLevel(loggo::Trace);

  LOGGO_TRACE() << "should print";

  LOGGO_ERROR() << "an error occurred";

  LOGGO_FATAL() << "panic!";


}
