# Task List

1. ✅ Explore repository to determine language and test setup
Language: C, Build: CMake, Tests: none. Travis builds with libevent dev. No unit tests.
2. ✅ Design basic unit tests for ikcp module
Chose portable unit tests in C for ikcp.{c,h}. Targets: create/release, setmtu, interval, nodelay, wndsize, waitsnd, getconv, send/flush path.
3. 🔄 Implement tests and integrate with CMake/CTest
Added tests/tests_ikcp.c and CTest integration in CMakeLists.
4. ⏳ Configure and build only test target
Use cmake -S . -B build -DBUILD_TESTING=ON and build ikcp_tests target.
5. ⏳ Run tests via ctest and ensure they pass
ctest --test-dir build -V. Fix issues as needed.
6. ⏳ Commit test additions
Commit modified CMakeLists and new test file with appropriate message and co-author.

