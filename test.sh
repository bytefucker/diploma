#!/bin/bash

# simple script to run misc tests
# and form the output file
# launch with ./test.sh
# chmod +x

echo -n "Enter the filename: "
read input
echo -n "Getting the system info... "
echo -e "****************** System Info ******************\n" > temp.txt
./bin/info >> temp.txt
echo "OK!"
echo -n "Testing the hard disk drive... "
echo -e "\n***************** HDD testing *******************\n" >> temp.txt
./bin/hdd -c >> temp.txt
echo "OK!"
# running monitor for 10 sec
# echo -n "Running the memory monitor... "
# echo -e "********** Memory Monitoring **********\n" >> temp.txt
# ./bin/memMonitor >> temp.txt
# echo "OK!"
echo -n "Testing memory (sequential)... "
echo -e "\n************* Memory sequential test ************\n" >> temp.txt
./bin/memSeq >> temp.txt
echo "OK!"
echo -n "Testing memory (random)... "
echo -e "\n*************** Memory random test **************\n" >> temp.txt
./bin/memRand >> temp.txt
echo "OK!"
echo -n "Testing serial port... "
echo -e "\n**************** Serial port test ***************\n" >> temp.txt
./bin/serialTest /dev/ttyS0 115200 text.txt >> temp.txt
echo "OK!"
echo -e "\n********************* DONE! *********************\n" >> temp.txt
mv temp.txt $input.txt
echo "Fin!"
