// starbrowser.h
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// Star browser tool for celestia.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _CELENGINE_STARBROWSER_H_
#define _CELENGINE_STARBROWSER_H_

#include <vector>
#include <Eigen/Core>
#include "univcoord.h"
#include "forward.h"

class StarBrowser {
public:
    enum
    {
        NearestStars = 0,
        BrighterStars = 1,
        BrightestStars = 2,
        StarsWithPlanets = 3,
    };

    StarBrowser();
    StarBrowser(const SimulationPtr& _appSim, int pred = NearestStars);
    std::vector<StarPtr> listStars(uint32_t);
    void setSimulation(const SimulationPtr& );
    const StarPtr nearestStar(void);
    bool setPredicate(int pred);
    void refresh();

public:
    // The star browser data is valid for a particular point
    // in space, and for performance issues is not continuously
    // updated.
    Eigen::Vector3f pos;
    UniversalCoord ucPos;

private:
    SimulationPtr appSim;
    int predicate;
};

#endif  // _CELENGINE_STARBROWSER_H_
