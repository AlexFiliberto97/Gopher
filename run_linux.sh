pkill main
rm main
clear

gcc main.c -o main -pthread -D_GNU_SOURCE

./main