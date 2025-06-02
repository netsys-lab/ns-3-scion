build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/scion-ia.cc
  HEADER_FILES
    model/scion-ia.h
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES test/scion-test-suite.cc
)
