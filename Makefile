
CC = g++
CFLAGS = -c 
# 源文件不能在关联的库后面
# 所以必须myshell.cpp 在前
src = myshell.cpp command.cpp manual.cpp util.cpp
obj = $(src:.cpp=.o)
EXECUTABLE = myshell

$(EXECUTABLE) : $(obj) 
	$(CC) -o $(EXECUTABLE) $(obj)
	rm -f ./*.o
myshell.o: myshell.h
command.o: command.h
manual.o: manual.h
util.o: util.h

.PHONY : clean
clean:	
	rm -f ./*.o ./myshell
	
	