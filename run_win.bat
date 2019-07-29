cls
del main.exe

gcc win32/logger.c -o win32/logger.exe -lws2_32
gcc win32/listener.c -o win32/listener.exe -lws2_32
gcc main.c -o main.exe -lws2_32

main.exe %1 %2