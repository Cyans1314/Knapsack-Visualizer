@echo off
echo Compiling all knapsack algorithms...

g++ -o knapsack_01.exe knapsack_01.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_01

g++ -o knapsack_complete.exe knapsack_complete.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_complete

g++ -o knapsack_multiple.exe knapsack_multiple.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_multiple

g++ -o knapsack_mixed.exe knapsack_mixed.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_mixed

g++ -o knapsack_2d.exe knapsack_2d.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_2d

g++ -o knapsack_group.exe knapsack_group.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_group

g++ -o knapsack_depend.exe knapsack_depend.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_depend

g++ -o knapsack_tree.exe knapsack_tree.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_tree

g++ -o knapsack_kth.exe knapsack_kth.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_kth

g++ -o knapsack_count.exe knapsack_count.cpp -std=c++17 -O2
if %errorlevel% neq 0 echo Failed: knapsack_count

echo Done!
dir *.exe
