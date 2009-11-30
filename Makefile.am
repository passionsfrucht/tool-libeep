if CONDTEST
  DIR_TEST = test
endif

SUBDIRS = src $(DIR_TEST)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PACKAGE_NAME).pc
	
bin_SCRIPTS = libeep-config

dist_data_DATA = \
  LICENSE \
  LICENSE.addendum
