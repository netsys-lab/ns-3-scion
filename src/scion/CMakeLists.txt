build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/as.cc
    model/scion-ia.cc
    model/scion-as.cc
    model/scion-address.cc
    model/scion-as-impl.cc
    model/scion-as-context.cc   
    helper/scion-stack-helper.cc
    helper/topology-helper.cc
    helper/scion-topology-helper.cc
  HEADER_FILES
    model/as.h
    model/scion-types.h
    model/scion-ia.h
    model/scion-as.h
    model/scion-address.h
    model/scion-as-impl.h
    model/scion-as-context.h
    helper/scion-stack-helper.h
    helper/topology-helper.h
    helper/scion-topology-helper.h
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES
    test/scion-test-suite.cc
    test/scion-address-test.cc
)
