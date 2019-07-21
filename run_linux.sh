pkill main
rm main
clear

gcc -Wall main.c -o main -pthread -D_GNU_SOURCE

./main
