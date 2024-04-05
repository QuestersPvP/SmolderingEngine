#pragma once

#include <iostream>

std::string GetRelativeFilePath(std::string fileToOpen = "")
{
	return CURRENT_WORKING_DIRECTORY + fileToOpen;
}