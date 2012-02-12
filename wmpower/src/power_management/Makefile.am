## Makefile.am for wmpower/src

INCLUDES =	-Iacpi -Iapm -Icompal -Idell -Itoshiba -Icpufreq -Ilib_utils

AM_CFLAGS  = -DX86=@X86@

noinst_LIBRARIES = libpower.a

libpower_a_SOURCES = power_management.c \
								power_management.h \
								acpi/libacpi.c \
								acpi/libacpi.h \
								apm/libapm.c \
								apm/libapm.h \
								compal/compal_lib.c \
								compal/compal_lib.h \
								dell/dell_lib.c \
								dell/dell_lib.h \
								toshiba/toshiba_lib.c \
								toshiba/toshiba_lib.h \
								toshiba/hci.c \
								toshiba/hci.h \
								toshiba/sci.c \
								toshiba/sci.h \
								lib_utils/lib_utils.c \
								lib_utils/lib_utils.h \
								cpufreq/cpufreq.c \
								cpufreq/cpufreq.h
