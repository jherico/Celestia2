// deepskyobj.h
//
// Copyright (C) 2003, Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _CELENGINE_DEEPSKYOBJ_H_
#define _CELENGINE_DEEPSKYOBJ_H_

#include <vector>
#include <string>
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <celmath/ray.h>

#include "parser.h"
#include "forward.h"

extern const float DSO_DEFAULT_ABS_MAGNITUDE;

class DeepSkyObject : public Object {
public:
    using Pointer = std::shared_ptr<DeepSkyObject>;
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    DeepSkyObject();
    virtual ~DeepSkyObject();

    inline uint32_t getCatalogNumber() const { return catalogNumber; }
    void setCatalogNumber(uint32_t);

    Eigen::Vector3d getPosition() const;
    void setPosition(const Eigen::Vector3d&);

    static void hsv2rgb(float*, float*, float*, float, float, float);

    virtual const char* getType() const = 0;
    virtual void setType(const std::string&) = 0;
    virtual size_t getDescription(char* buf, size_t bufLength) const;

    Eigen::Quaternionf getOrientation() const;
    void setOrientation(const Eigen::Quaternionf&);

    /*! Return the radius of a bounding sphere large enough to contain the object.
     *  For correct rendering, all of the geometry must fit within this sphere radius.
     *  DSO subclasses an alternate radius that more closely matches the conventional
     *  astronomical definition for the size of the object (e.g. mu25 isophote radius.)
     */
    virtual float getBoundingSphereRadius() const { return radius; }

    /*! Return the radius of the object. This radius will be displayed in the UI and
     *  should match the conventional astronomical definition of the object size.
     */
    float getRadius() const { return radius; }
    void setRadius(float r);
    virtual float getHalfMassRadius() const { return radius; }

    float getAbsoluteMagnitude() const;
    void setAbsoluteMagnitude(float);

    std::string getInfoURL() const;
    void setInfoURL(const std::string&);

    bool isVisible() const { return visible; }
    void setVisible(bool _visible) { visible = _visible; }
    bool isClickable() const { return clickable; }
    void setClickable(bool _clickable) { clickable = _clickable; }

    virtual const char* getObjTypeName() const = 0;

    virtual bool pick(const Ray3d& ray, double& distanceToPicker, double& cosAngleToBoundCenter) const = 0;
    virtual bool load(const HashPtr&, const std::string& resPath);
    //virtual void render(const GLContext& context,
    //                    const Eigen::Vector3f& offset,
    //                    const Eigen::Quaternionf& viewerOrientation,
    //                    float brightness,
    //                    float pixelSize) = 0;
    virtual uint32_t getRenderMask() const = 0;  //{ return 0; }
    virtual uint32_t getLabelMask() const = 0;   // { return 0; }

    enum
    {
        InvalidCatalogNumber = 0xffffffff
    };

private:
    uint32_t catalogNumber;
    Eigen::Vector3d position;
    Eigen::Quaternionf orientation;
    float radius;
    float absMag;
    std::string* infoURL;

    bool visible : 1;
    bool clickable : 1;
};

typedef std::vector<DeepSkyObject*> DeepSkyCatalog;
int LoadDeepSkyObjects(DeepSkyCatalog&, std::istream& in, const std::string& path);

#endif  // _CELENGINE_DEEPSKYOBJ_H_