cmake_minimum_required(VERSION 3.10)

project(Pixeltetris VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

file(GLOB_RECURSE TARGET_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

if (WIN32)
	string(REPLACE "/include/SDL2" "/include/" SDL2_PARENT_DIR ${SDL2_INCLUDE_DIR})
	add_executable(Pixeltetris WIN32 ${TARGET_SOURCES})
else()
	add_executable(Pixeltetris ${TARGET_SOURCES})
endif()

target_include_directories(Pixeltetris PRIVATE ${SDL2_PARENT_DIR} ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
if (SDL2_INCLUDE_DIR)
	get_filename_component(SDL2_INCLUDE_PARENT "${SDL2_INCLUDE_DIR}" DIRECTORY)
	target_include_directories(Pixeltetris PRIVATE ${SDL2_INCLUDE_PARENT})
endif()
target_link_libraries(Pixeltetris ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
target_compile_definitions(Pixeltetris PRIVATE
	PIXELTETRIS_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/pixeltetris"
)

add_custom_command(TARGET Pixeltetris POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Pixeltetris>/assets
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_SOURCE_DIR}/data/gamecontrollerdb.txt $<TARGET_FILE_DIR:Pixeltetris>/gamecontrollerdb.txt
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_SOURCE_DIR}/data/icon.png $<TARGET_FILE_DIR:Pixeltetris>/icon.png
)

install(TARGETS Pixeltetris RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/pixeltetris/assets)
install(FILES data/gamecontrollerdb.txt data/icon.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixeltetris)
install(FILES data/pixeltetris.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
# icon.png is 182x182; 192x192 is the nearest freedesktop hicolor size.
install(FILES data/icon.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/192x192/apps RENAME pixeltetris.png)
install(FILES data/icon.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps RENAME pixeltetris.png)
