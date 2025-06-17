build_lib_example(
  NAME scion-connectivity
  SOURCE_FILES scion-connectivity.cc
  LIBRARIES_TO_LINK ${libcore} ${libnetwork} ${libinternet}
)
