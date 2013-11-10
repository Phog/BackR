#include "TaskManager.h"
#include "FaceDetector.h"

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <stdexcept>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
	std::cout << "Usage: " << argv[0]
		  << " [database user] [database password] " << std::endl;
	return 1;
    }

    try
    {
	StalkR::TaskManager  manager("stalkr", "localhost", argv[1], argv[2]);
	StalkR::FaceDetector detector("haarcascade_frontalface_alt.xml");
	std::cout << "BackR started" << std::endl;

	for (;;)
	{
	    try
	    {
		manager.fetchTasks();
		manager.executeTasks(&detector);
		manager.clearTasks();
	    }
	    catch(const std::runtime_error &e)
	    {
		// Hope it was a fluke. Report the error,
		// clear the state and try again.
		std::cerr << e.what() << std::endl;
		manager.clearTasks();
	    }

	    // Avoid busy waiting if we have no tasks.
	    std::this_thread::sleep_for(std::chrono::seconds(1));
	}
    }
    catch(const std::exception &e)
    {
	// Unexpected exception, bail.
	std::cerr << e.what() << std::endl;
	return 1;
    }

    return 0;
}

