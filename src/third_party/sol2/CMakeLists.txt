cmake_minimum_required(VERSION 3.9)
project(sol2 VERSION 2.20.0 DESCRIPTION "A C++ <-> Lua API wrapper with advanced features and top notch performance.")

set(CXX_FEATURES
        cxx_std_14
        cxx_auto_type
        cxx_constexpr
        cxx_decltype
        cxx_decltype_auto
        cxx_default_function_template_args
        cxx_final
        cxx_lambdas
        cxx_noexcept
        cxx_nullptr
        cxx_override
        cxx_range_for
        cxx_return_type_deduction
        cxx_right_angle_brackets
        cxx_static_assert
        cxx_strong_enums
        cxx_variadic_macros
        cxx_variadic_templates)


add_library(sol2 INTERFACE)
add_library(sol2::sol2 ALIAS sol2)
target_include_directories(sol2 INTERFACE "./upstream")
# Enforce function call argument type checks when calling from Lua to C++
target_compile_definitions(sol2 INTERFACE SOL_SAFE_FUNCTION_CALLS)
target_compile_features(sol2 INTERFACE ${CXX_FEATURES})
