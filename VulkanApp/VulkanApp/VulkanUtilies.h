#pragma once

/// Store indices (locations) of queue families and check if each family is valid
struct QueueFamilyIndices {
	int graphicsFamily = -1; // Location of Graphics Queue Family

	bool isValid()
	{
		return graphicsFamily >= 0;
	}
};