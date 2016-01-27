all:
	g++ -Wall -o adb_top adb_top.cpp
	g++ -Wall -o adb_meminfo adb_dumpmem.cpp
	g++ -Wall -o logcat adb_logcat.cpp 

	
	if [ ! -d bin ]; then mkdir bin; fi
	
	mv -f adb_top bin/adb_top
	mv -f adb_meminfo bin/adb_meminfo
	mv -f logcat bin/logcat

clean:
	$(RM) -rf bin