set_xmakever('2.9.8')

set_project("strex")
set_version("0.1.0", { build = "%Y%m%d%H%M" })

set_allowedplats("windows", "linux")
set_allowedmodes("debug", "release")

add_rules("mode.debug", "mode.release")

option("dev", { default = false })
option("enable_tests", { default = true })

if has_config("dev") then
    if is_mode("debug") and is_plat("linux") then
        set_policy("build.sanitizer.address", true)
        set_policy("build.sanitizer.leak", true)
        set_policy("build.sanitizer.undefined", true)
    end
end

set_languages("c++23")
set_warnings("allextra", "error")

if has_config("enable_tests") then
    add_requires("doctest 2.4.11")
end
add_requires("argparse 3.2")

-- gcc requires to link against stdc++exp to use std::println on Windows
option("link-stdc++exp")
    add_links("stdc++exp")
option_end()
if is_plat("windows") then
    add_options("link-stdc++exp")
end

target("strex")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("argparse")

target("static")
    set_kind("static")
    add_files("src/*.cpp|main.cpp|compile_option.cpp")
    add_includedirs("include")
    add_headerfiles("include/(strex/*.hpp)")
    set_basename("strex")

target("shared")
    set_kind("shared")
    add_files("src/*.cpp|main.cpp|compile_option.cpp")
    add_includedirs("include")
    add_headerfiles("include/(strex/*.hpp)")
    set_basename("strex")

if has_config("enable_tests") then
    target("test")
        set_kind("binary")
        set_default(false)
        add_files("src/*.cpp|main.cpp|compile_option.cpp")
        add_includedirs("include")
        for _, file in ipairs(os.files("test/*.cpp")) do
            add_tests(path.basename(file), {
                files = { file, "test/helper/*.cpp" },
                packages = "doctest",
                defines = "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"
            })
        end
end