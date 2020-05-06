/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com			
			(C) Alexander Blade 2015
*/
#include "main.h"
#include "creator.h"
#include <string.h>
#include <direct.h>
#include <fstream>
#include <list>
#include <experimental/filesystem>
#include "keyboard.h"

DWORD	vehUpdateTime;
DWORD	pedUpdateTime;
using namespace std;
namespace fs = std::experimental::filesystem;





void main()
{	

	char output_path[] = "MTMCT\\";
	const std::string config_file = "MTMCT\\parameters.txt";
	

	
	static std::unique_ptr<Creator> datasetCreator = std::make_unique<Creator>(output_path, config_file);
	Sleep(10);
	

	while (true) {

		
		(*datasetCreator).updateNew();
		WAIT(0);
	}
}

void ScriptMain()
{	
	srand(GetTickCount());
	main();
}
