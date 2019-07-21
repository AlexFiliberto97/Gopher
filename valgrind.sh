pkill main
rm main
clear

gcc main.c -o main -pthread -D_GNU_SOURCE

valgrind --log-file=./valgrind_log.txt --trace-children=yes --track-origins=yes --leak-check=full  --show-leak-kinds=all ./main
