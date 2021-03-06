// timeline.cpp
//
// Object timelines.
//
// Copyright (C) 2008, the Celestia Development Team
// Initial version by Chris Laurel, claurel@gmail.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include "timeline.h"
#include "timelinephase.h"
#include "frametree.h"
#include "frame.h"

using namespace std;

/*! A Timeline is a list of TimelinePhases that covers a continuous 
 *  interval of time.
 */

Timeline::Timeline() {
}

Timeline::~Timeline() {
    for (const auto& phase : phases) {
        // Remove the phase from whatever phase tree contains it.
        phase->getFrameTree()->removeChild(phase);
        phase->release();
    }
}

bool Timeline::appendPhase(const TimelinePhasePtr& phase) {
    // Validate start and end times. If there are existing phases in the timeline,
    // startTime must be equal to endTime of the previous phases so that there are
    // no gaps and no overlaps.
    if (!phases.empty()) {
        if (phase->startTime() != phases.back()->endTime())
            return false;
    }

    phase->addRef();
    phases.push_back(phase);

    return true;
}

const TimelinePhasePtr& Timeline::findPhase(double t) const {
    // Find the phase containing time t. The overwhelming common case is
    // nPhases = 1, so we special case that. Otherwise, we do a simple linear search,
    // as the number of phases in a timeline should always be quite small.
    if (phases.size() == 1) {
        return phases[0];
    } else {
        for (const auto& phase : phases) {
            if (t < phase->endTime()) {
                return phase;
            }
        }

        // Time is greater than the end time of the final phase. Just return the final phase.
        return phases.back();
    }
}

/*! Get the phase at the specified index.
 */
const TimelinePhasePtr& Timeline::getPhase(size_t n) const {
    return phases.at(n);
}

/*! Get the number of phases in this timeline.
 */
size_t Timeline::phaseCount() const {
    return phases.size();
}

double Timeline::startTime() const {
    return phases.front()->startTime();
}

double Timeline::endTime() const {
    return phases.back()->endTime();
}

/*! Check whether the timeline covers the specified time t. True if
 *  startTime <= t <= endTime. Note that this is deliberately different
 *  than the TimelinePhase::includes function, which is only true if
 *  t is strictly less than the end time.
 */
bool Timeline::includes(double t) const {
    return phases.front()->startTime() <= t && t <= phases.back()->endTime();
}

void Timeline::markChanged() {
    for (const auto& phase : phases) {
        phase->getFrameTree()->markChanged();
    }
}
