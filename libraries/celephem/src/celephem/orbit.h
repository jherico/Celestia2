// orbit.h
//
// Copyright (C) 2001-2006, Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _CELENGINE_ORBIT_H_
#define _CELENGINE_ORBIT_H_

#include <functional>
#include <memory>

#include <Eigen/Core>

using OrbitSampleProc = std::function<void(double t, const Eigen::Vector3d& position, const Eigen::Vector3d& velocity)>;

class Orbit {
public:
    using Pointer = std::shared_ptr<Orbit>;
    virtual ~Orbit(){};

    /*! Return the position in the orbit's reference frame at the specified
     * time (TDB). Units are kilometers.
     */
    virtual Eigen::Vector3d positionAtTime(double jd) const = 0;

    /*! Return the orbital velocity in the orbit's reference frame at the
     * specified time (TDB). Units are kilometers per day. If the method
     * is not overridden, the velocity will be computed by differentiation
     * of position.
     */
    virtual Eigen::Vector3d velocityAtTime(double) const;

    virtual double getPeriod() const = 0;
    virtual double getBoundingRadius() const = 0;

    virtual void sample(double startTime, double endTime, const OrbitSampleProc& proc) const;

    virtual bool isPeriodic() const { return true; };

    // Return the time range over which the orbit is valid; if the orbit
    // is always valid, begin and end should be equal.
    virtual void getValidRange(double& begin, double& end) const {
        begin = 0.0;
        end = 0.0;
    };

    struct AdaptiveSamplingParameters {
        double tolerance;
        double startStep;
        double minStep;
        double maxStep;
    };

    void adaptiveSample(double startTime,
                        double endTime,
                        const OrbitSampleProc& proc,
                        const AdaptiveSamplingParameters& samplingParameters) const;
};

class EllipticalOrbit : public Orbit {
public:
    using Pointer = std::shared_ptr<EllipticalOrbit>;
    EllipticalOrbit(double, double, double, double, double, double, double, double _epoch = 2451545.0);
    virtual ~EllipticalOrbit(){};

    // Compute the orbit for a specified Julian date
    Eigen::Vector3d positionAtTime(double) const override final;
    Eigen::Vector3d velocityAtTime(double) const override final;
    double getPeriod() const;
    double getBoundingRadius() const;

private:
    double eccentricAnomaly(double) const;
    Eigen::Vector3d positionAtE(double) const;
    Eigen::Vector3d velocityAtE(double) const;

    double pericenterDistance;
    double eccentricity;
    double inclination;
    double ascendingNode;
    double argOfPeriapsis;
    double meanAnomalyAtEpoch;
    double period;
    double epoch;

    Eigen::Matrix3d orbitPlaneRotation;
};

/*! Custom orbit classes should be derived from CachingOrbit.  The custom
 * orbits can be expensive to compute, with more than 50 periodic terms.
 * Celestia may need require position of a planet more than once per frame; in
 * order to avoid redundant calculation, the CachingOrbit class saves the
 * result of the last calculation and uses it if the time matches the cached
 * time.
 */
class CachingOrbit : public Orbit {
public:
    CachingOrbit();
    virtual ~CachingOrbit();

    virtual Eigen::Vector3d computePosition(double jd) const = 0;
    virtual Eigen::Vector3d computeVelocity(double jd) const;
    virtual double getPeriod() const = 0;
    virtual double getBoundingRadius() const = 0;

    Eigen::Vector3d positionAtTime(double jd) const override final;
    Eigen::Vector3d velocityAtTime(double jd) const override final;

private:
    mutable Eigen::Vector3d lastPosition;
    mutable Eigen::Vector3d lastVelocity;
    mutable double lastTime;
    mutable bool positionCacheValid;
    mutable bool velocityCacheValid;
};

/*! A mixed orbit is a composite orbit, typically used when you have a
 *  custom orbit calculation that is only valid over limited span of time.
 *  When a mixed orbit is constructed, it computes elliptical orbits
 *  to approximate the behavior of the primary orbit before and after the
 *  span over which it is valid.
 */
class MixedOrbit : public Orbit {
public:
    MixedOrbit(const Orbit::Pointer& orbit, double t0, double t1, double mass);
    virtual ~MixedOrbit();

    Eigen::Vector3d positionAtTime(double jd) const override final;
    Eigen::Vector3d velocityAtTime(double jd) const override final;
    double getPeriod() const override;
    double getBoundingRadius() const override;
    void sample(double startTime, double endTime, const OrbitSampleProc& proc) const override;

private:
    Orbit::Pointer primary;
    EllipticalOrbit::Pointer afterApprox;
    EllipticalOrbit::Pointer beforeApprox;
    double begin;
    double end;
    double boundingRadius{ 0 };
};



class Body;


#if 0
// TODO: eliminate this once body-fixed reference frames are implemented
/*! An object in a synchronous orbit will always hover of the same spot on
 *  the surface of the body it orbits.  Only equatorial orbits of a certain
 *  radius are stable in the real world.  In Celestia, synchronous orbits are
 *  a convenient way to fix objects to a planet surface.
 */
class SynchronousOrbit : public Orbit {
public:
    SynchronousOrbit(const BodyConstPtr& _body, const Eigen::Vector3d& _position);
    virtual ~SynchronousOrbit();

    Eigen::Vector3d positionAtTime(double jd) const override final;
    double getPeriod() const override;
    double getBoundingRadius() const override;
    void sample(double, double, const OrbitSampleProc& proc) const override;

private:
    BodyConstPtr body;
    Eigen::Vector3d position;
};
#endif


/*! A FixedOrbit is used for an object that remains at a constant
 *  position within its reference frame.
 */
class FixedOrbit : public Orbit {
public:
    FixedOrbit(const Eigen::Vector3d& pos);
    virtual ~FixedOrbit();

    Eigen::Vector3d positionAtTime(double) const override final;
    //virtual Vec3d velocityAtTime(double) const;
    double getPeriod() const override;
    bool isPeriodic() const override;
    double getBoundingRadius() const override;
    void sample(double, double, const OrbitSampleProc&) const override;

private:
    Eigen::Vector3d position;
};

#endif  // _CELENGINE_ORBIT_H_
