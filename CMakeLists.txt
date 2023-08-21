cmake_minimum_required( VERSION 3.14 )

project( ksh 
         LANGUAGES CXX )

add_library( ksh INTERFACE )
target_include_directories( ksh
    INTERFACE include )
