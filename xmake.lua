set_allowedplats("windows", "linux")
set_allowedmodes("debug", "release")

add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

option("dev", { default = true })

if has_config("dev") then
    if is_mode("debug") and is_plat("linux") then
        set_policy("build.sanitizer.address", true)
        set_policy("build.sanitizer.leak", true)
        set_policy("build.sanitizer.undefined", true)
    end
end

set_languages("c++23")
set_warnings("allextra", "error")

add_requires("doctest 2.4.11")

target("strex")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include")

target("test")
    set_kind("binary")
    set_default(false)
    add_files("src/*.cpp|main.cpp")
    add_includedirs("include")
    for _, file in ipairs(os.files("test/*.cpp")) do
        add_tests(path.basename(file), {
            files = { file, "test/helper/*.cpp" },
            packages = "doctest",
            defines = "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"
        })
    end