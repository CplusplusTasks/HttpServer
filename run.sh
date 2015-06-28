#!/usr/bin/env bash
if [ "$1" == "clean" ]
then
    make -C build/ clean
    rm -rf build
    exit 0
fi
mkdir build
cd build
cmake ../ 
make
cd ../

ADDR=$1
PORT=$2
if [ "$1" == "" ] 
then
    ADDR="127.0.0.1"
    PORT="7777"
fi
echo -ne "$ADDR\n$PORT" > config.txt
echo -ne "\n\tSERVER HAS BEEN LAUNCHED ON ADDRESS: $ADDR AND PORT: $PORT\n"
./Tic_Tac_Toe
rm config.txt
