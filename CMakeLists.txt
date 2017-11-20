cmake_minimum_required(VERSION 2.8)
project( png2yuv )
find_package( OpenCV REQUIRED )
add_executable( png2yuv 
	main.cpp
	)
target_link_libraries( png2yuv ${OpenCV_LIBS} )
