CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(ntoy)

INCLUDE(GNUInstallDirs)
INCLUDE(FindPkgConfig)

SET(NTOY_DBUS_SERVICE "n.toy")

INCLUDE_DIRECTORIES(src)

pkg_check_modules(pkgs REQUIRED glib-2.0 gio-2.0 gio-unix-2.0 gthread-2.0)
FOREACH(flag ${pkgs_CFLAGS})
	ADD_COMPILE_OPTIONS(${flag})
ENDFOREACH()

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
	src/ntoy_spi.c
	src/ntoy_gpio.c
	src/ntoy_led.c
	src/ntoy_button.c
	src/main.c)

ADD_DEFINITIONS(
	# Run-time buffer overflow detection
	-D_FORTIFY_SOURCE=2
	-DNTOY_DBUS_SERVICE="${NTOY_DBUS_SERVICE}"
)

TARGET_LINK_LIBRARIES(ntoy PUBLIC ${pkgs_LDFLAGS})
INSTALL(TARGETS ntoy RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# DBus configuration file
CONFIGURE_FILE(ntoy.conf.in ${CMAKE_BINARY_DIR}/ntoy.conf @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/ntoy.conf  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d)

# Systemd service file
INSTALL(FILES ntoy.service DESTINATION /lib/systemd/system)
