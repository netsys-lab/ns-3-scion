build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/scion-ia.cc
    model/scion-address.cc   
    model/hop-field.cc
    model/info-field.cc
    model/scion-path.cc
    model/scion-header.cc
  HEADER_FILES
    model/fields-forward.h
    model/hop-field.h
    model/info-field.h
    model/scion-header.h
    model/scion-path.h
    model/scion-types.h
    model/scion-ia.h
    model/scion-address.h
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES
    test/scion-test-suite.cc
    test/scion-header-test.cc
    test/scion-path-test.cc
    test/scion-address-test.cc
)
