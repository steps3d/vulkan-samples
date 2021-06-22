cmake_minimum_required (VERSION 2.6)
project (vulkan-tests)

find_package(Vulkan)

if (WIN32)
	if (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
		if (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		endif()
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
endif ()

if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else ()
	message(STATUS ${Vulkan_LIBRARY})
endif ()

if (WIN32)
		# For Windows have to specify locations of all libs
		# Use all libs as static
	set ( LIBS_ROOT     "f:/Libraries" )
	set ( GLM           "${LIBS_ROOT}/glm" )
	set ( GLFW          "${LIBS_ROOT}/glfw-3.3.bin.WIN32" )
	set ( GLFW_LIB_PATH "${GLFW}/lib-vc2012" )
	set ( ASSIMP_DIR    "${LIBS_ROOT}/assimp-3.3.1-64" )
	set ( GLFW_LIB	    glfw3 )
	set ( ASSIMP_LIB    assimp-vc140-mt zlibstaticd  )
	#set ( ASSIMP_LIB    assimp-vc140-mtd zlibstaticd  )

	if (CMAKE_CL_64)	# 64-bit build with MSVC
		set ( GLFW            "${LIBS_ROOT}/glfw-3.3" )
		set ( GLFW_LIB_PATH   "${GLFW}/src/Debug" )
		set ( ASSIMP_DIR      "${LIBS_ROOT}/assimp-3.3.1-64" )
		#set ( ASSIMP_DIR      "${LIBS_ROOT}/assimp" )
		set ( ASSIMP_LIB_PATH "${ASSIMP_DIR}/lib/Debug" )

	endif ()

	include_directories ( ${GLM} ${GLFW}/include ${ASSIMP_DIR}/include "${Vulkan_INCLUDE_DIRS}" )
	link_directories    ( ${GLFW_LIB_PATH} ${ASSIMP_LIB_PATH} ) 				# "${Vulkan_LIBRARY}" 	# "${ASSIMP_LIB_PATH}" 
	
	 add_definitions( -D_CRT_SECURE_NO_WARNINGS )
else ()
	set ( GLFW_LIB 		glfw )
	set ( ASSIMP_LIB	assimp )
endif ()

#include_directories("../Core" "../Libs/SOIL")
#add_library(Core  ../Core/GlutWindow.cpp ../Core/TgaImage.cpp ../Core/Program.cpp ../Core/Data.cpp ../Core/stringUtils.cpp ../Core/VertexBuffer.cpp ../Core/BasicMesh.cpp ../Core/Texture.cpp ../Libs/SOIL/SOIL.c ../Libs/SOIL/stb_image_aug.c ../Libs/SOIL/image_helper.c ../Libs/SOIL/image_DXT.c ../Core/randUtils.cpp ../Core/Framebuffer.cpp ../Core/Camera.cpp  ../Core/ScreenQuad.cpp ../Core/MeshLoader.cpp ../Core/bbox.cpp )

include_directories("../Libs/SOIL")
#add_library(Core  ../Libs/SOIL/stb_image_aug.c ../Libs/SOIL/image_helper.c ../Libs/SOIL/image_DXT.c )


add_executable ( test-window-7 test-window-7.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c )
target_link_libraries ( test-window-7 ${GLFW_LIB} "${Vulkan_LIBRARY}" )

add_executable ( test-window-8 test-window-8.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp )
target_link_libraries ( test-window-8 ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-9 test-window-9.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp )
target_link_libraries ( test-window-9 ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-10 test-window-10.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp )
target_link_libraries ( test-window-10 ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-11 test-window-11.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp )
target_link_libraries ( test-window-11 ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-12 test-window-12.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp )
target_link_libraries ( test-window-12 ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-particles test-window-particles.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp )
target_link_libraries ( test-window-particles ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-pbr test-window-pbr.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp )
target_link_libraries ( test-window-pbr ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-gun test-window-gun.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp Dds.cpp )
target_link_libraries ( test-window-gun ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-gun-2 test-window-gun-2.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp Dds.cpp )
target_link_libraries ( test-window-gun-2 ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-dds test-window-dds.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp Dds.cpp )
set_target_properties ( test-window-dds PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries ( test-window-dds ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-deferred test-window-deferred.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp Dds.cpp Camera.cpp )
target_link_libraries ( test-window-deferred ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )

add_executable ( test-window-cubemap-dds test-window-cubemap-dds.cpp VulkanWindow.cpp Log.cpp Data.cpp ../Libs/SOIL/stb_image_aug.c bbox.cpp plane.cpp BasicMesh.cpp TgaImage.cpp Dds.cpp )
target_link_libraries ( test-window-cubemap-dds ${GLFW_LIB} "${Vulkan_LIBRARY}" ${ASSIMP_LIB} )


#if (WIN32)
#	install(TARGETS aniso DESTINATION  ${CMAKE_SOURCE_DIR} )
#endif ()