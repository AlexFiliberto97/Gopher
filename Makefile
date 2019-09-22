CC=gcc
COMM_SRC=source/common/src
COMM_OBJ=source/common/object

ifeq ($(OS),Windows_NT)
	FLAGS=-Wall -lws2_32
# pedantic
	OBJ_EXT:=.o
	SYS_OBJ=source/win32/object
	SYS_SRC=source/win32/src
	DEL_CMD=del /F /Q
else ifeq ($(shell uname -s),Linux)
	FLAGS=-Wall -pthread -D_GNU_SOURCE -lrt
	OBJ_EXT:=.o
	SYS_OBJ=source/posix/object
	SYS_SRC=source/posix/src
	DEL_CMD=rm -f
else
	$(error Unsupported platform)
endif

.PHONY: all clean common win32 posix

ifeq ($(OS),Windows_NT)
all: clean common win32 bin/listener.exe bin/logger.exe bin/main.exe
clean:
	$(DEL_CMD) bin\* source\common\object\*.o source\win32\object\*.o source\posix\object\*.o
else ifeq ($(shell uname -s),Linux)
all: clean common posix bin/main
clean:
	$(DEL_CMD) bin/* source/common/object/*.o source/posix/object/*.o source/win32/object/*.o
endif

# WIN32 BIN
bin/main.exe: $(COMM_SRC)/main.c
	$(CC) -Wall -o $@ $(COMM_SRC)/main.c $(COMM_OBJ)/server$(OBJ_EXT) \
	$(COMM_OBJ)/gopher$(OBJ_EXT) $(COMM_OBJ)/network$(OBJ_EXT) $(COMM_OBJ)/utils$(OBJ_EXT) \
	$(COMM_OBJ)/socket$(OBJ_EXT) $(COMM_OBJ)/error$(OBJ_EXT) \
	$(SYS_OBJ)/utils_win32$(OBJ_EXT) $(SYS_OBJ)/thread$(OBJ_EXT) $(SYS_OBJ)/process$(OBJ_EXT) \
	$(SYS_OBJ)/pipe$(OBJ_EXT) $(SYS_OBJ)/mutex$(OBJ_EXT) $(SYS_OBJ)/environment$(OBJ_EXT) \
	$(SYS_OBJ)/locking$(OBJ_EXT) $(SYS_OBJ)/caching$(OBJ_EXT) $(SYS_OBJ)/event$(OBJ_EXT) \
	$(SYS_OBJ)/network$(OBJ_EXT) -lws2_32

bin/logger.exe: $(SYS_SRC)/logger.c
	$(CC) -Wall -o $@ $< $(SYS_OBJ)/event$(OBJ_EXT) $(COMM_OBJ)/gopher$(OBJ_EXT) \
	$(COMM_OBJ)/utils$(OBJ_EXT) $(SYS_OBJ)/utils_win32$(OBJ_EXT) $(COMM_OBJ)/error$(OBJ_EXT) \
	$(COMM_OBJ)/network$(OBJ_EXT) $(SYS_OBJ)/pipe$(OBJ_EXT) $(SYS_OBJ)/locking$(OBJ_EXT) \
	$(SYS_OBJ)/caching$(OBJ_EXT) $(SYS_OBJ)/mutex$(OBJ_EXT) $(SYS_OBJ)/thread$(OBJ_EXT) \
	$(COMM_OBJ)/socket$(OBJ_EXT) $(COMM_OBJ)/server$(OBJ_EXT) $(SYS_OBJ)/process$(OBJ_EXT) \
	$(SYS_OBJ)/network$(OBJ_EXT) -lws2_32  

bin/listener.exe: $(SYS_SRC)/listener.c
	$(CC) -Wall -o $@ $< $(SYS_OBJ)/event$(OBJ_EXT) $(COMM_OBJ)/gopher$(OBJ_EXT) \
	$(COMM_OBJ)/utils$(OBJ_EXT) $(SYS_OBJ)/utils_win32$(OBJ_EXT) $(COMM_OBJ)/error$(OBJ_EXT) \
	$(COMM_OBJ)/network$(OBJ_EXT) $(SYS_OBJ)/pipe$(OBJ_EXT) $(SYS_OBJ)/locking$(OBJ_EXT) \
	$(SYS_OBJ)/caching$(OBJ_EXT) $(SYS_OBJ)/mutex$(OBJ_EXT) $(SYS_OBJ)/thread$(OBJ_EXT) \
	$(COMM_OBJ)/socket$(OBJ_EXT) $(COMM_OBJ)/server$(OBJ_EXT) $(SYS_OBJ)/process$(OBJ_EXT) \
	$(SYS_OBJ)/network$(OBJ_EXT) -lws2_32

# POSIX BIN
bin/main: $(COMM_SRC)/main.c
	$(CC) $(FLAGS) -o bin/main $(COMM_SRC)/main.c $(COMM_OBJ)/server$(OBJ_EXT) \
	$(COMM_OBJ)/gopher$(OBJ_EXT) $(COMM_OBJ)/network$(OBJ_EXT) $(COMM_OBJ)/utils$(OBJ_EXT) \
	$(COMM_OBJ)/socket$(OBJ_EXT) $(COMM_OBJ)/error$(OBJ_EXT) \
	$(SYS_OBJ)/utils_posix$(OBJ_EXT) $(SYS_OBJ)/thread$(OBJ_EXT) $(SYS_OBJ)/process$(OBJ_EXT) \
	$(SYS_OBJ)/pipe$(OBJ_EXT) $(SYS_OBJ)/mutex$(OBJ_EXT) $(SYS_OBJ)/environment$(OBJ_EXT) \
	$(SYS_OBJ)/locking$(OBJ_EXT) $(SYS_OBJ)/listener$(OBJ_EXT) $(SYS_OBJ)/logger$(OBJ_EXT) \
	$(SYS_OBJ)/caching$(OBJ_EXT) $(SYS_OBJ)/network$(OBJ_EXT)

# COMMON OBJ
common: $(COMM_OBJ)/server$(OBJ_EXT) $(COMM_OBJ)/gopher$(OBJ_EXT) $(COMM_OBJ)/network$(OBJ_EXT) \
	    $(COMM_OBJ)/utils$(OBJ_EXT) $(COMM_OBJ)/socket$(OBJ_EXT) $(COMM_OBJ)/error$(OBJ_EXT) \
		$(SYS_OBJ)/network$(OBJ_EXT)

$(COMM_OBJ)/server$(OBJ_EXT): $(COMM_SRC)/server.c
	$(CC) -o $@ -c $<

$(COMM_OBJ)/gopher$(OBJ_EXT): $(COMM_SRC)/gopher.c
	$(CC) -lws2_32 -o $@ -c $<

$(COMM_OBJ)/network$(OBJ_EXT): $(COMM_SRC)/network.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/network$(OBJ_EXT): $(SYS_SRC)/network.c
	$(CC) $(FLAGS) -o $@ -c $<

$(COMM_OBJ)/utils$(OBJ_EXT): $(COMM_SRC)/utils.c
	$(CC) $(FLAGS) -o $@ -c $<

$(COMM_OBJ)/socket$(OBJ_EXT): $(COMM_SRC)/socket.c
	$(CC) $(FLAGS) -o $@ -c $<

$(COMM_OBJ)/error$(OBJ_EXT): $(COMM_SRC)/error.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/thread$(OBJ_EXT): $(SYS_SRC)/thread.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/process$(OBJ_EXT): $(SYS_SRC)/process.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/pipe$(OBJ_EXT): $(SYS_SRC)/pipe.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/mutex$(OBJ_EXT): $(SYS_SRC)/mutex.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/locking$(OBJ_EXT): $(SYS_SRC)/locking.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/environment$(OBJ_EXT): $(SYS_SRC)/environment.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/caching$(OBJ_EXT): $(SYS_SRC)/caching.c
	$(CC) $(FLAGS) -o $@ -c $<

# WIN32 OBJ
win32: $(SYS_OBJ)/utils_win32$(OBJ_EXT) $(SYS_OBJ)/thread$(OBJ_EXT) $(SYS_OBJ)/process$(OBJ_EXT) \
	   $(SYS_OBJ)/pipe$(OBJ_EXT) $(SYS_OBJ)/mutex$(OBJ_EXT) $(SYS_OBJ)/environment$(OBJ_EXT) \
	   $(SYS_OBJ)/locking$(OBJ_EXT) $(SYS_OBJ)/caching$(OBJ_EXT) $(SYS_OBJ)/event$(OBJ_EXT)

$(SYS_OBJ)/event$(OBJ_EXT): $(SYS_SRC)/event.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/utils_win32$(OBJ_EXT): $(SYS_SRC)/utils_win32.c
	$(CC) $(FLAGS) -o $@ -c $<

# POSIX OBJ
posix: $(SYS_OBJ)/utils_posix$(OBJ_EXT) $(SYS_OBJ)/thread$(OBJ_EXT) $(SYS_OBJ)/process$(OBJ_EXT) \
	   $(SYS_OBJ)/pipe$(OBJ_EXT) $(SYS_OBJ)/mutex$(OBJ_EXT) $(SYS_OBJ)/environment$(OBJ_EXT) \
	   $(SYS_OBJ)/locking$(OBJ_EXT) $(SYS_OBJ)/listener$(OBJ_EXT) $(SYS_OBJ)/logger$(OBJ_EXT) \
	   $(SYS_OBJ)/caching$(OBJ_EXT)

$(SYS_OBJ)/utils_posix$(OBJ_EXT): $(SYS_SRC)/utils_posix.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/logger$(OBJ_EXT): $(SYS_SRC)/logger.c
	$(CC) $(FLAGS) -o $@ -c $<

$(SYS_OBJ)/listener$(OBJ_EXT): $(SYS_SRC)/listener.c
	$(CC) $(FLAGS) -o $@ -c $<

