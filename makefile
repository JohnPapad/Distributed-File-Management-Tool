CC=g++
CXXFLAGS= -Wall -g -std=c++11

jobExecutor: main.o helping_funs.o jobexec.o worker.o search_answer.o dir.o text_file.o trie.o posting_list.o
	$(CC) $(CXXFLAGS) main.o helping_funs.o jobexec.o worker.o search_answer.o dir.o text_file.o trie.o posting_list.o -o jobExecutor

main.o: main.cpp jobexec.h worker.h helping_funs.h
	$(CC) $(CXXFLAGS) -c main.cpp
	
helping_funs.o: helping_funs.cpp helping_funs.h
	$(CC) $(CXXFLAGS) -c helping_funs.cpp
	
jobexec.o: jobexec.cpp jobexec.h helping_funs.h
	$(CC) $(CXXFLAGS) -c jobexec.cpp
	
worker.o: worker.cpp worker.h helping_funs.h
	$(CC) $(CXXFLAGS) -c worker.cpp
	
search_answer.o: search_answer.cpp search_answer.h helping_funs.h
	$(CC) $(CXXFLAGS) -c search_answer.cpp
	
dir.o: dir.cpp dir.h helping_funs.h
	$(CC) $(CXXFLAGS) -c dir.cpp
	
text_file.o: text_file.cpp text_file.h helping_funs.h
	$(CC) $(CXXFLAGS) -c text_file.cpp
	
trie.o: trie.cpp trie.h helping_funs.h
	$(CC) $(CXXFLAGS) -c trie.cpp
	
posting_list.o: posting_list.cpp posting_list.h dir.h helping_funs.h
	$(CC) $(CXXFLAGS) -c posting_list.cpp

.PHONY: clean

clean:
	rm -f jobExecutor *.o *.fifo 
