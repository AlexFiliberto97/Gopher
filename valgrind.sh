pkill main
rm main
clear

gcc main.c -o main -pthread -D_GNU_SOURCE

valgrind --leak-check=full --show-leak-kinds=all  ./main > valgrind_log.txt
