// C/C++ LANGUAGE API TYPES
#include <stdlib.h>
#include <exception>
#include <iostream>

// OUR OWN TYPES
#include "core/renderer.hpp"

/*
* main.cpp - This is the entry point into our Wolfie3D demo, little work is actually
* done inside this class, really delegating the demo to the renderer. Note this demo
* simply loads a cube model as a GLTF cube and then renders cubes using that model. The
* demo lets the user use W-A-S-D to navigate the camera as well as to select objects via
* 1-2-3, which then lets one move models around the scene.
*/
int main()
{
	// DECLARE AND CONSTRUCT A LOCAL Renderer OBJECT
	W3D::Renderer renderer;
	try
	{
		// START THE DEMO
		renderer.start();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
};