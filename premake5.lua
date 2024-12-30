
-- NOTE: Requires at least Premake5 beta 1 for ccpdialect and vs2022 support.

 -- NOTE: I built ASSIMP from source. If you're using a binary distribution you may need to change some paths down below as well as these ones.
GLFW_PATH = "C:/Repos/Libraries/glfw/glfw-3.4.bin.WIN32/"
GLM_PATH = "C:/Repos/Libraries/glm/1.0.1/"
GLI_PATH = "C:/Repos/Libraries/gli/0.8.2.0/"
ASSIMP_PATH = "C:/Repos/Libraries/assimp/5.2.5/"


workspace "KHBBSMesh" 
    configurations {"Debug", "Release"} 
    platforms {"Win32"} 
    --location "build"

    project "BBSMeshView"
        kind "ConsoleApp"
        language "C++"
		cppdialect "C++20"  -- Needed for proper <filesystem> and std::format support.
        system "Windows" 
        architecture "x32"  -- TODO: 64 bit would be faster? Would it screw up serialization though?
		vpaths {
			["Source/*"] = { "src/**.h", "src/**.c", "src/**.hpp", "src/**.cpp" },
            ["Shaders"] = { "**.glsl", "**.vert", "**.frag" },
            ["glad/*"] = { "glad" },
			["IMGUI/*"] = { "imgui/**.h", "imgui/**.cpp" }
		}
		files { "src/**", "resources/shaders/**", "imgui/**" }
		includedirs({"./src", "./imgui"})
        flags {"NoPCH"}  -- TODO: Check to see if more flags are needed.
		buildoptions { "/Zc:__cplusplus" }  -- Needed because MSVC's compiler is weird
        --debugdir "./" -- TODO: This doesn't give the right results and the .user file overrides it once generated.
                        --       Also, now rendered irrelevent by the new FileManager search functionality.
		
		filter "configurations:Debug"
            defines {"DEBUG"} 
            symbols "on" 
            optimize "off" 
            targetdir "bin/debug"

        filter "configurations:Release"
            defines {"NDEBUG"} 
            optimize "on"
            targetdir "bin/release"
		filter({})
		
		-- GLM
        -- I modified my glm folder to be ./include/glm rather than ./glm to keep my #include autocomplete a little cleaner
		includedirs(GLM_PATH .. "include")
		files { GLM_PATH .. "util/glm.natvis" }

        -- GLI (Not used?)
        includedirs(GLI_PATH)
        
        -- GLFW
		includedirs(GLFW_PATH .. "include")
        defines("GLFW_DLL")
		libdirs(GLFW_PATH .. "lib-vc2022")
        links("glfw3dll")
		postbuildcommands("{COPY} " .. GLFW_PATH .. "lib-vc2022/glfw3.dll %{cfg.targetdir}")

        -- GLAD
        includedirs("./glad/include")
        files("./glad/src/glad.c")
        filter "files:**.c" -- TODO: make this only effect glad.c
            flags { "NoPCH" } 
        filter ({})
		
		-- ASSIMP
		includedirs(ASSIMP_PATH .. "include")
		filter("configurations:Debug")
			links("assimp-vc143-mtd")
			postbuildcommands("{COPY} " .. ASSIMP_PATH .. "bin/Debug/assimp-vc143-mtd.dll %{cfg.targetdir}")
			postbuildcommands("{COPY} " .. ASSIMP_PATH .. "bin/Debug/assimp-vc143-mtd.pdb %{cfg.targetdir}")
			libdirs(ASSIMP_PATH .. "lib/Debug")
		filter("configurations:Release")
			links("assimp-vc143-mt")
			postbuildcommands("{COPY} ".. ASSIMP_PATH .. "bin/RelWithDebInfo/assimp-vc143-mt.dll %{cfg.targetdir}")
			libdirs(ASSIMP_PATH .. "lib/RelWithDebInfo")
		filter({})
		