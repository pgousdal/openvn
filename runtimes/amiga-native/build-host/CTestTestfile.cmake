# CMake generated Testfile for 
# Source directory: /home/pgo/Projects/openvn/runtimes/amiga-native
# Build directory: /home/pgo/Projects/openvn/runtimes/amiga-native/build-host
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(dispatch "/home/pgo/Projects/openvn/runtimes/amiga-native/build-host/test-openvn-dispatch")
set_tests_properties(dispatch PROPERTIES  _BACKTRACE_TRIPLES "/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;77;add_test;/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;0;")
add_test(story "/home/pgo/Projects/openvn/runtimes/amiga-native/build-host/test-openvn-story")
set_tests_properties(story PROPERTIES  _BACKTRACE_TRIPLES "/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;78;add_test;/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;0;")
add_test(graphics "/home/pgo/Projects/openvn/runtimes/amiga-native/build-host/test-openvn-graphics")
set_tests_properties(graphics PROPERTIES  WORKING_DIRECTORY "/home/pgo/Projects/openvn/runtimes/amiga-native" _BACKTRACE_TRIPLES "/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;79;add_test;/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;0;")
add_test(image "/home/pgo/Projects/openvn/runtimes/amiga-native/build-host/test-openvn-image" "/home/pgo/Projects/openvn/runtimes/amiga-native/tests/fixtures/lighthouse_storm.iff")
set_tests_properties(image PROPERTIES  _BACKTRACE_TRIPLES "/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;84;add_test;/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;0;")
add_test(assets-runtime "/home/pgo/Projects/openvn/runtimes/amiga-native/build-host/test-openvn-assets-runtime")
set_tests_properties(assets-runtime PROPERTIES  _BACKTRACE_TRIPLES "/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;89;add_test;/home/pgo/Projects/openvn/runtimes/amiga-native/CMakeLists.txt;0;")
