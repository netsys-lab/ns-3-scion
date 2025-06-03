build_lib(
  LIBNAME scion
  SOURCE_FILES
    model/scion-ia.cc
    model/scion-address.cc   
    model/hop-field.cc
    model/info-field.cc
    model/scion-path.cc
    model/scion-header.cc
    model/scmp/scmp-destination-unreachable.cc
    model/scmp/scmp-echo.cc
    model/scmp/scmp-external-interface-down.cc
    model/scmp/scmp-header.cc
    model/scmp/scmp-internal-connectivity-down.cc
    model/scmp/scmp-packet-too-big.cc
    model/scmp/scmp-parameter-problem.cc
    model/scmp/scmp-traceroute.cc
  HEADER_FILES
    model/util/endian-utils.h

    model/scmp/scmp-echo.h
    model/scmp/scmp-header.h
    model/scmp/scmp-destination-unreachable.h
    model/scmp/scmp-external-interface-down.h
    model/scmp/scmp-internal-connectivity-down.h
    model/scmp/scmp-packet-too-big.h
    model/scmp/scmp-parameter-problem.h
    model/scmp/scmp-traceroute.h
    model/scmp/scmp-typecodes.h

    model/fields-forward.h
    model/hop-field.h
    model/info-field.h
    model/scion-header.h
    model/scion-path.h
    model/scion-types.h
    model/scion-ia.h
    model/scion-address.h
  LIBRARIES_TO_LINK
    ${libcore} ${libnetwork} ${libinternet}
  TEST_SOURCES
    test/scion-test-suite.cc
    test/scion-header-test.cc
    test/scion-path-test.cc
    test/scion-address-test.cc
    test/scmp/scion-scmp-test.cc
)
