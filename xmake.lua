includes("lib/commonlibsse-ng")

set_project("LossGauge")
set_version("0.1.0")
set_license("GPL-3.0")

set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

add_requires("toml++")

target("LossGauge")
    add_defines("NOMINMAX")

    add_rules("commonlibsse-ng.plugin", {
        name = "LossGauge",
        author = "YOUR_NAME",
        description = "Recoverable health loss system for Skyrim SE/AE"
    })

    add_packages("toml++")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")

    set_pcxxheader("src/pch.h")