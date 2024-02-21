#pragma once
#include "Engine/Core/RandomStream.h"
#include <stdlib.h>

class PLCTGraph;
class PLCTNode;

#define CONFIGURE_RAND() \
	srand(((unsigned int)(Time::GetCurrentSafe()->LastBegin * rand())) ^ 439679745); \
	RandomStream stream = RandomStream(); \
	stream.GenerateNewSeed();