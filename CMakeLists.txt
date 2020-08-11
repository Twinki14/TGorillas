cmake_minimum_required(VERSION 3.16)
project(TGorillas) # Change this to your projects/Scripts name

# C++ flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Os -s -m32")
set(CMAKE_SHARED_LINKER_FLAGS "-Os -s -m32 -static-libgcc -static-libstdc++ -static")

file(GLOB           SDL_SOURCES         RELATIVE ${CMAKE_SOURCE_DIR} "Libs/SDL2/include/SDL2/*.h")
file(GLOB_RECURSE   IMGUI_SOURCES       RELATIVE ${CMAKE_SOURCE_DIR} "Libs/imgui/*.h" "Libs/imgui/*.hpp" "Libs/imgui/*.c" "Libs/imgui/*.cpp")
file(GLOB_RECURSE   UTILITY_SOURCES     RELATIVE ${CMAKE_SOURCE_DIR} "Utilities/*.hpp" "Utilities/*.cpp")
file(GLOB_RECURSE   LOGIC_SOURCES       RELATIVE ${CMAKE_SOURCE_DIR} "Logic/*.hpp" "Logic/*.cpp")
file(GLOB           LIB_MISC_SOURCES    RELATIVE ${CMAKE_SOURCE_DIR} "Libs/*.hpp" "Libs/*.cpp")
file(GLOB           GUI_SOURCES         RELATIVE ${CMAKE_SOURCE_DIR} "GUI/*.hpp" "GUI/*.cpp")

message("---------")
foreach(S IN LISTS LOGIC_SOURCES UTILITY_SOURCES GUI_SOURCES LIB_MISC_SOURCES SDL_SOURCES IMGUI_SOURCES)
    message("${S}")
endforeach()
message("---------")

# Select all source files
set(SOURCE_FILES
        main.cpp
        Config.cpp
        Config.hpp
        GUI/GUI.cpp
        GUI/GUI.hpp
        Libs/ScriptGUI.cpp
        Libs/ScriptGUI.hpp
        Libs/SDL2/include/SDL2/SDL.h
        Libs/SDL2/include/SDL2/SDL_assert.h
        Libs/SDL2/include/SDL2/SDL_atomic.h
        Libs/SDL2/include/SDL2/SDL_audio.h
        Libs/SDL2/include/SDL2/SDL_bits.h
        Libs/SDL2/include/SDL2/SDL_blendmode.h
        Libs/SDL2/include/SDL2/SDL_clipboard.h
        Libs/SDL2/include/SDL2/SDL_config.h
        Libs/SDL2/include/SDL2/SDL_cpuinfo.h
        Libs/SDL2/include/SDL2/SDL_egl.h
        Libs/SDL2/include/SDL2/SDL_endian.h
        Libs/SDL2/include/SDL2/SDL_error.h
        Libs/SDL2/include/SDL2/SDL_events.h
        Libs/SDL2/include/SDL2/SDL_filesystem.h
        Libs/SDL2/include/SDL2/SDL_gamecontroller.h
        Libs/SDL2/include/SDL2/SDL_gesture.h
        Libs/SDL2/include/SDL2/SDL_haptic.h
        Libs/SDL2/include/SDL2/SDL_hints.h
        Libs/SDL2/include/SDL2/SDL_joystick.h
        Libs/SDL2/include/SDL2/SDL_keyboard.h
        Libs/SDL2/include/SDL2/SDL_keycode.h
        Libs/SDL2/include/SDL2/SDL_loadso.h
        Libs/SDL2/include/SDL2/SDL_log.h
        Libs/SDL2/include/SDL2/SDL_main.h
        Libs/SDL2/include/SDL2/SDL_messagebox.h
        Libs/SDL2/include/SDL2/SDL_mouse.h
        Libs/SDL2/include/SDL2/SDL_mutex.h
        Libs/SDL2/include/SDL2/SDL_name.h
        Libs/SDL2/include/SDL2/SDL_opengl.h
        Libs/SDL2/include/SDL2/SDL_opengl_glext.h
        Libs/SDL2/include/SDL2/SDL_opengles.h
        Libs/SDL2/include/SDL2/SDL_opengles2.h
        Libs/SDL2/include/SDL2/SDL_opengles2_gl2.h
        Libs/SDL2/include/SDL2/SDL_opengles2_gl2ext.h
        Libs/SDL2/include/SDL2/SDL_opengles2_gl2platform.h
        Libs/SDL2/include/SDL2/SDL_opengles2_khrplatform.h
        Libs/SDL2/include/SDL2/SDL_pixels.h
        Libs/SDL2/include/SDL2/SDL_platform.h
        Libs/SDL2/include/SDL2/SDL_power.h
        Libs/SDL2/include/SDL2/SDL_quit.h
        Libs/SDL2/include/SDL2/SDL_rect.h
        Libs/SDL2/include/SDL2/SDL_render.h
        Libs/SDL2/include/SDL2/SDL_revision.h
        Libs/SDL2/include/SDL2/SDL_rwops.h
        Libs/SDL2/include/SDL2/SDL_scancode.h
        Libs/SDL2/include/SDL2/SDL_shape.h
        Libs/SDL2/include/SDL2/SDL_stdinc.h
        Libs/SDL2/include/SDL2/SDL_surface.h
        Libs/SDL2/include/SDL2/SDL_system.h
        Libs/SDL2/include/SDL2/SDL_syswm.h
        Libs/SDL2/include/SDL2/SDL_test.h
        Libs/SDL2/include/SDL2/SDL_test_assert.h
        Libs/SDL2/include/SDL2/SDL_test_common.h
        Libs/SDL2/include/SDL2/SDL_test_compare.h
        Libs/SDL2/include/SDL2/SDL_test_crc32.h
        Libs/SDL2/include/SDL2/SDL_test_font.h
        Libs/SDL2/include/SDL2/SDL_test_fuzzer.h
        Libs/SDL2/include/SDL2/SDL_test_harness.h
        Libs/SDL2/include/SDL2/SDL_test_images.h
        Libs/SDL2/include/SDL2/SDL_test_log.h
        Libs/SDL2/include/SDL2/SDL_test_md5.h
        Libs/SDL2/include/SDL2/SDL_test_memory.h
        Libs/SDL2/include/SDL2/SDL_test_random.h
        Libs/SDL2/include/SDL2/SDL_thread.h
        Libs/SDL2/include/SDL2/SDL_timer.h
        Libs/SDL2/include/SDL2/SDL_touch.h
        Libs/SDL2/include/SDL2/SDL_types.h
        Libs/SDL2/include/SDL2/SDL_version.h
        Libs/SDL2/include/SDL2/SDL_video.h
        Libs/SDL2/include/SDL2/SDL_vulkan.h
        Libs/SDL2/include/SDL2/begin_code.h
        Libs/SDL2/include/SDL2/close_code.h
        Libs/imgui/addons/imgui_alpacawidgets.cpp
        Libs/imgui/addons/imgui_alpacawidgets.h
        Libs/imgui/addons/imgui_disablewidgets.cpp
        Libs/imgui/addons/imgui_disablewidgets.h
        Libs/imgui/fonts/RobotoRegular.hpp
        Libs/imgui/imconfig.h
        Libs/imgui/imgui.cpp
        Libs/imgui/imgui.h
        Libs/imgui/imgui_demo.cpp
        Libs/imgui/imgui_draw.cpp
        Libs/imgui/imgui_impl_sdl.cpp
        Libs/imgui/imgui_impl_sdl.h
        Libs/imgui/imgui_internal.h
        Libs/imgui/imgui_sw.cpp
        Libs/imgui/imgui_sw.hpp
        Libs/imgui/imgui_user.h
        Libs/imgui/imgui_widgets.cpp
        Libs/imgui/imstb_rectpack.h
        Libs/imgui/imstb_textedit.h
        Libs/imgui/imstb_truetype.h Logic/GorillaOld.cpp Logic/GorillaOld.hpp Logic/Gorillas.cpp Logic/Gorillas.hpp Logic/Listeners/ListenerTask.cpp Logic/Listeners/ListenerTask.hpp Logic/Types/Gorilla.cpp Logic/Types/Gorilla.hpp Logic/Supplies.cpp Logic/Supplies.hpp Logic/Travel.cpp Logic/Travel.hpp Logic/Listeners/GameListener.cpp Logic/Listeners/GameListener.hpp Logic/Types/TrackedPlayer.cpp Logic/Types/TrackedPlayer.hpp Logic/Types/Hitsplat.cpp Logic/Types/Hitsplat.hpp Logic/Types/WorldArea.cpp Logic/Types/WorldArea.hpp)

# Set directories
set(ALPACABOT_DIRECTORY $ENV{HOMEDRIVE}$ENV{HOMEPATH}\\AlpacaBot)
set(SCRIPT_DIRECTORY ${ALPACABOT_DIRECTORY}\\Scripts\\Local\\${PROJECT_NAME})
set(TLIB_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\Documents\\CLion Projects\\TLib")
message("AlpacaBot directory: " ${ALPACABOT_DIRECTORY})
message("Script directory: " ${SCRIPT_DIRECTORY})
message("TLib directory: " ${TLIB_DIRECTORY})

# Set the output folder of the script, and make the directory if it doesn't exist
file(MAKE_DIRECTORY ${SCRIPT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCRIPT_DIRECTORY})
add_custom_target(${PROJECT_NAME}_MakeDir ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${SCRIPT_DIRECTORY})

# Set the BUILD_DLL #define
# add_definitions(-DBUILD_DLL)
# add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_LONGHORN)

# Build the .dll
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Include and Library directories
include_directories(Libs)
include_directories(Libs/imgui)
include_directories(Libs/SDL2/include)
include_directories(${TLIB_DIRECTORY}\\Include)
include_directories(${TLIB_DIRECTORY}\\Include\\Tools)
include_directories(${TLIB_DIRECTORY}\\Include\\Utilities)
include_directories(${ALPACABOT_DIRECTORY}\\Include)
target_link_directories(${PROJECT_NAME} PRIVATE Libs/SDL2/lib)
target_link_directories(${PROJECT_NAME} PRIVATE ${TLIB_DIRECTORY}/cmake-build-release)
target_link_directories(${PROJECT_NAME} PRIVATE ${ALPACABOT_DIRECTORY}/Library)

# Link libs
target_link_libraries(${PROJECT_NAME} TLib)
target_link_libraries(${PROJECT_NAME} AlpacaLibrary)
target_link_libraries(${PROJECT_NAME} winhttp)
target_link_libraries(${PROJECT_NAME} Ws2_32)
target_link_libraries(${PROJECT_NAME} mingw32)
target_link_libraries(${PROJECT_NAME} SDL2main)
target_link_libraries(${PROJECT_NAME} SDL2)
target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} dinput8)
target_link_libraries(${PROJECT_NAME} dxguid)
target_link_libraries(${PROJECT_NAME} dxerr8)
target_link_libraries(${PROJECT_NAME} user32)
target_link_libraries(${PROJECT_NAME} gdi32)
target_link_libraries(${PROJECT_NAME} winmm)
target_link_libraries(${PROJECT_NAME} imm32)
target_link_libraries(${PROJECT_NAME} ole32)
target_link_libraries(${PROJECT_NAME} oleaut32)
target_link_libraries(${PROJECT_NAME} shell32)
target_link_libraries(${PROJECT_NAME} version)
