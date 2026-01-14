#pragma once
#include "gps.h"

void initLora();
void getTeamLocations(coords locations[]);
void sendLocation(coords location);