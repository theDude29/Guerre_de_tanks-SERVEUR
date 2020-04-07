#Si vous rencontrez des problèmes de avec ce fichier CMakeLists.txt
#merci de me le faire savoir : remi.perenne@free.fr

cmake_minimum_required(VERSION 2.6)
 
project(GuerreDeTank_Serveur)
 
# Trouver un package avec pkg-config 
find_package(PkgConfig)

pkg_check_modules(Irrlicht REQUIRED)
include_directories(${Irrlicht_INCLUDE_DIRS})
link_directories(${Irrlicht_LIBRARIES})

pkg_check_modules(sfml-network REQUIRED)
include_directories(${sfml-network_INCLUDE_DIRS})
link_directories(${sfml-network_LIBRARIES})

pkg_check_modules(sfml-audio REQUIRED)
include_directories(${sfml-audio_INCLUDE_DIRS})
link_directories(${sfml-audio_LIBRARIES})

pkg_check_modules(sfml-system REQUIRED)
include_directories(${sfml-system_INCLUDE_DIRS})
link_directories(${sfml-system_LIBRARIES})

add_executable(
	GuerreDeTank_Serveur
	client.cpp
	client.h
	infoPartie.cpp
	infoPartie.h
	main.cpp
	MyEventReceiver.cpp
	MyEventReceiver.h
	vector3d_operateur.cpp
	vector3d_operateur.h
    
)
 
target_link_libraries(
    GuerreDeTank_Serveur
    Irrlicht
    GL
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-network
    sfml-system
)
