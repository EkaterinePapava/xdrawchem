# tests/CMakeLists.txt — Qt6 test suite for xdrawchem
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test Gui Widgets)

include(CTest)
enable_testing()

# ── Helper macro ─────────────────────────────────────────────────────────────
macro(xdc_test name)
    cmake_parse_arguments(ARG "" "" "LIBS" ${ARGN})
    add_executable(${name} ${ARG_UNPARSED_ARGUMENTS})
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../xdrawchem
    )
    target_link_libraries(${name} PRIVATE
        Qt6::Core Qt6::Test Qt6::Gui Qt6::Widgets
        ${ARG_LIBS}
    )
    target_compile_options(${name} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    )
    add_test(NAME ${name} COMMAND ${name} -v2)
    set_tests_properties(${name} PROPERTIES
        FAIL_REGULAR_EXPRESSION "FAIL!"
    )
endmacro()

# ── Test: Vector2D — pure math, no Qt widget dependency ──────────────────────
xdc_test(tst_vector2d
    tst_vector2d.cpp
    ../xdrawchem/vector2D.cpp
)

# ── Test: DPoint — geometry and element methods ───────────────────────────────
xdc_test(tst_dpoint
    tst_dpoint.cpp
)
target_link_libraries(tst_dpoint PRIVATE Qt6::Widgets)

# ── Test: Bond geometry — midpoint regression, toVector, angles ───────────────
xdc_test(tst_bondgeometry
    tst_bondgeometry.cpp
    ../xdrawchem/vector2D.cpp
)

# ── Test: CML parser — atom/bond parsing, stereo bonds, ring file fixtures ────
# Links against the full xdrawchem OBJECT library.
# globals_stub.cpp provides Preferences preferences; QString RingDir, HomeDir;
# which are normally defined in main.cpp (excluded from the object lib to
# avoid a main() collision with QTEST_APPLESS_MAIN).
xdc_test(tst_cmlparser
    tst_cmlparser.cpp
    globals_stub.cpp
)
target_link_libraries(tst_cmlparser PRIVATE xdrawchem_obj)

# Pass ring directory so file-fixture tests can locate benzene.cml etc.
target_compile_definitions(tst_cmlparser PRIVATE
    RING_DIR="${CMAKE_SOURCE_DIR}/ring"
)
