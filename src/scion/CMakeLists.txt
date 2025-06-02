build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/ia.cc       
  HEADER_FILES
    model/ia.h        
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES test/scion-test-suite.cc
)
