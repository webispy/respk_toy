CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(ntoy)

INCLUDE(GNUInstallDirs)

# Common compile options
ADD_COMPILE_OPTIONS(
	-g -Os
	-fPIC

	# Turn on all warnings and treat all warnings as errors.
	-Werror -Wall -Wextra

	# Additional warnings
	-Wunreachable-code -Wpointer-arith
	-Wformat=2 -Wno-format-nonliteral

	# Not warnings
	-Wno-missing-field-initializers
	-Wno-unused-parameter)

ADD_EXECUTABLE(ntoy
	ntoy_spi.c
	ntoy_led.c
	main.c)
