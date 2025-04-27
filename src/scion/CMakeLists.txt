build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/ia.cc
    model/scion-as.cc
    helper/scion-helper.cc
    model/scion.cc
    model/as.cc
  HEADER_FILES
    model/ia.h
    model/scion-as.h
    helper/scion-helper.h
    model/scion.h
    model/as.h
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES test/scion-test-suite.cc
)
