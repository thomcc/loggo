
# language settings
CXXFLAGS += -std=c++11 -pedantic

ifeq (${shell uname}, Darwin)
	# OS X is weird
	CXX = clang++
	CXXFLAGS += -stdlib=libc++
endif

Inc := loggo
Examples := examples

Examples := basic
ExampleFiles := ${addprefix examples/,${Examples}}

CXXFLAGS += -Wall -Wextra -Weffc++ -O3 -I.

.PHONY: all
all: run-examples

.PHONY: clean
clean:
	@rm -f ${ExampleFiles}

.PHONY: run-examples
run-examples: examples
	${foreach example, ${ExampleFiles}, ./${example}}

.PHONY: examples
examples: ${ExampleFiles}

examples/%: examples/%.cc loggo/Loggo.hh
	${CXX} ${CXXFLAGS} -o $@ $<



