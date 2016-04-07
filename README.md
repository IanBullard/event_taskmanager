Event / TaskManager
===================

A version of GrandMaster's event and task systems (http://www.grandmaster.nu/blog/?page_id=261) with the dependency on Boost removed.  A C++11 compiler is required, tested on a Mac with clang.

I received permission to put this code up on GitHub from GrandMaster under an MIT license.  All attributions should be credited to GM.  Both systems are part of GrandMaser's project, [overdriveassault](https://code.google.com/p/overdriveassault/) but in much different form.  I suggest looking at the changes he's made to the systems.

Why?
====

I'm using a modified form of this code in my engine, I think both the event and taskmanager are great starting points.  With the Boost requirement removed both systems are nearly drag-and-drop into any C++11 project.


Modifications
=============

All boost requirements have been removed.  This includes changing boost::mutex to std::mutex and moving over to C++11 locking mechanisms.  I also implemented a subset of boost::threadpool.

The largest change I made was removing the requirement for a dummy task if there are no tasks added.  That was added to deal with the case not enough tasks were enqueued to wake a worker when the taskmanager shut down.  I added a 1667 millisecond sleep to workers so when nothing can be done, they will wait a little bit and then check again.  See line 136 in taskmanager.cpp.

Test Application
================

The Makefile builds a very simple test application from test.cpp that does simple checks of the code.  If bugs/improvements are ever submitted I'll add tests for them and improve test coverage.
