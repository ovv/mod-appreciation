#ifndef BOOST_FLIGHT_PATHS_H_
#define BOOST_FLIGHT_PATHS_H_

#include "Player.h"

enum
{
  BOOST_FLIGHTPATHS_EASTERN_KINGDOMS = 0,
  BOOST_FLIGHTPATHS_KALIMDOR = 1,
  BOOST_FLIGHTPATHS_OUTLAND = 2,
  BOOST_FLIGHTPATHS_NORTHEND = 3
};

class BoostFlightPaths
{
public:
  static void UnlockFlightPaths(Player *player, uint8 continent);
};

#endif