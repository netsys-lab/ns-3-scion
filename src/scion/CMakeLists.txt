build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/as.cc
    model/scion-ia.cc
    model/scion-as.cc
    model/scion-address.cc
    model/scion-as-impl.cc   
    helper/scion-stack-helper.cc
  HEADER_FILES
    model/as.h
    model/scion-types.h
    model/scion-ia.h
    model/scion-as.h
    model/scion-address.h
    model/scion-as-impl.h
    helper/scion-stack-helper.h
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES
    test/scion-test-suite.cc
    test/scion-address-test.cc
)
