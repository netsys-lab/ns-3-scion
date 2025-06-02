build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/scion-ia.cc
    model/scion-address.cc   
  HEADER_FILES
    model/scion-types.h
    model/scion-ia.h
    model/scion-address.h
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES
    test/scion-test-suite.cc
    test/scion-address-test.cc
)
