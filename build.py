#!/usr/bin/env python3 

##########################################################
# Function:                                              #
#   Compiles project                                     #
#                                                        #
# How to use;                                            #
#   python3 build.py | [ARGS]                            #    
#                                                        #
# ARGS:                                                  #
#                                                        #
#   Default:                                             #
#       NONE                                             #
#           Features:                                    #
#               [X] TicTacToe binary located in ./bin    #
#   Clean:                                               #       
#       -c                                               #   
#           Features:                                    #   
#               [X] Removes all binaries                 #
#               [X] Removes CMake Build directories      #   
#                                                        #
#                                                        #
#   Debug:                                               #
#       -d                                               #
#           Features:                                    #
#               [X] TicTacToe binary located in ./bin    #
#               [X] Debug statements throughout runtime  #
#                                                        #
#   Server:                                              #
#       -s                                               # 
#           Feature:                                     #
#               [X] Server binary located in ./bin       #
#   Help:                                                #
#       -h                                               #
#           Features:                                    #
#               [X] Prints help menu                     #
##########################################################

import subprocess
import sys 
import os 

def ensure_build_dir():
    if not os.path.exists("./build/"):
        os.makedirs("build")

def print_help_menu():
     print("""
    Usage:
           ./build.py | [ARGS]
    
    Arguments:
           None     Production Binary 
           -c       Clean Project 
           -d       Debug Binary 
           -h       Help Menu
           -s       Server Binary""")

def make_build_dir():
    cmd = "mkdir build"
    print(f"Running: {cmd}")
    subprocess.run(cmd.split())

def make_clean():
    cmd = "sudo rm -rf ./build ./bin"
    print(f"Running: {cmd}")
    subprocess.run(cmd.split())

def make_prod():
        make_clean()
        make_build_dir()

        cmd = "cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build ."
        print(f"Running: cd build/ && {cmd}")
        subprocess.run(cmd, shell=True, cwd="./build")

def make_debug():
    make_clean()
    make_build_dir()

    cmd = "cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build ."
    print(f"Running: cd build/ && {cmd}")
    subprocess.run(cmd, shell=True, cwd="./build")

def make_and_run_server():
    _NETWORK_GAME_SRC_NAME  = "NetworkGame.cpp"
    _MAIN_SRC_NAME          = "TicTacToeServer.cpp"
    _SERVER_SRC_NAME        = "Server.cpp"

    _NETWORK_GAME_PATH  = f"./src/Server/{_NETWORK_GAME_SRC_NAME}"
    _SERVER_SRC_PATH    = f"./src/Server/{_SERVER_SRC_NAME}"
    _MAIN_SER_PATH      = f"./src/Server/{_MAIN_SRC_NAME}"

    _SERVER_BIN_NAME = "server"
    _SERVER_BIN_PATH = f"./bin/{_SERVER_BIN_NAME}"
    _SERVER_MV_PATH  = f"./src/Server/{_SERVER_BIN_NAME}" 

    _INCLUDE_PATH    = "./includes/"

    make_clean()

    cmd = f"g++ {_MAIN_SER_PATH} {_NETWORK_GAME_PATH} {_SERVER_SRC_PATH} -I{_INCLUDE_PATH} -o {_SERVER_MV_PATH}"
    print(f"Running: {cmd}")
    subprocess.run(cmd, shell=True, cwd=".")

    cmd = f"mkdir -p {_SERVER_BIN_PATH}" 
    print(f"Running: {cmd}")
    subprocess.run(cmd, shell=True, cwd=".")

    cmd = f"mv {_SERVER_MV_PATH} {_SERVER_BIN_PATH}" 
    print(f"Running: {cmd}")
    subprocess.run(cmd, shell=True, cwd=".")

    cmd = f"{_SERVER_BIN_PATH}/{_SERVER_BIN_NAME}"
    print(f"Running: {cmd}")
    subprocess.run(cmd, shell=True, cwd=".")

def run(version):
    cmd = f"./bin/{version}/TicTacToe"
    print(f"Running: {cmd}")
    subprocess.run(cmd.split())

def main():
    if len(sys.argv) == 1:
        make_prod()
        run("release")    
    else:
        if(sys.argv[1] == "-c"):
            make_clean()
        elif(sys.argv[1] == "-d"):
            make_debug()
            run("debug")
        elif(sys.argv[1] == "-s"):
            make_and_run_server()
        else:
            print_help_menu()

    
if __name__ == "__main__":
    main()
