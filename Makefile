test : test.cpp
	$(CXX) -std=c++11 test.cpp taskmanager/taskmanager.cpp taskmanager/task.cpp -I. -o test