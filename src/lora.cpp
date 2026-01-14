#include "lora.h"

void initLora() {}
void getTeamLocations(coords locations[]) {
    coords teamLocations[teamSize] = {{54.868, 23.936},
                                      {54.868, 23.938}};

    locations = teamLocations;
}
void sendLocation(coords location) {
}