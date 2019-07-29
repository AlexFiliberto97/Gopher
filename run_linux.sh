pkill main
rm main
> error.log
clear

gcc main.c -o main -pthread -D_GNU_SOURCE

./main
