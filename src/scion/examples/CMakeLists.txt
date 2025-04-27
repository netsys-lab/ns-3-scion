build_lib_example(
  NAME scion-example-sim
  SOURCE_FILES scion-example-sim.cc
  LIBRARIES_TO_LINK
    ${libinternet}
    ${libpoint-to-point}
    ${libapplications}
)
