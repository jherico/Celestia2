// galaxy.h
//
// Copyright (C) 2001-2009, the Celestia Development Team
// Original version by Chris Laurel, Fridger Schrempp, and Toti
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _GALAXY_H_
#define _GALAXY_H_

#include "deepskyobj.h"


struct Blob
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Eigen::Vector4f        position;
    uint32_t   colorIndex;
    float          brightness;
};

class GalacticForm;

class Galaxy : public DeepSkyObject
{
 public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Galaxy();
    const char* getType() const override;
    void setType(const std::string&) override;
    size_t getDescription(char* buf, size_t bufLength) const override;
    virtual std::string getCustomTmpName() const;
    virtual void setCustomTmpName(const std::string&);

    float getDetail() const;
    void setDetail(float);
    //    float getBrightness() const;
    //    void setBrightness();

    bool pick(const Ray3d& ray,
                      double& distanceToPicker,
                      double& cosAngleToBoundCenter) const override;
    bool load(const HashPtr&, const std::string&) override;

    GalacticForm* getForm() const;

    static void  increaseLightGain();
    static void  decreaseLightGain();
    static float getLightGain();
    static void  setLightGain(float);

    uint32_t getRenderMask() const override;
    uint32_t getLabelMask() const override;
    
    const char* getObjTypeName() const override;

 public:
    enum GalaxyType {
        S0   =  0,
        Sa   =  1,
        Sb   =  2,
        Sc   =  3,
        SBa  =  4,
        SBb  =  5,
        SBc  =  6,
        E0   =  7,
        E1   =  8,
        E2   =  9,
        E3   = 10,
        E4   = 11,
        E5   = 12,
        E6   = 13,
        E7   = 14,
        Irr  = 15
    };

 private:
    float detail;
    std::string* customTmpName;
    //    float brightness;
    GalaxyType type;
    GalacticForm* form;

    static float lightGain;
};

//std::ostream& operator<<(std::ostream& s, const Galaxy::GalaxyType& sc);

#endif // _GALAXY_H_