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

target("strex")
    set_kind("binary")
    add_files("src/*.cpp")