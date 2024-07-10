#pragma once

// Indicies (locations) of Queue families
struct QueueFamilyIndicies
{
	int GraphicsFamily = -1; // Location of Graphics Queue Family

	/* Check if the Queue Family is valid */
	bool IsValid()
	{
		return GraphicsFamily >= 0;
	}
};
