// parser.cpp
//
// Copyright (C) 2001-2009, the Celestia Development Team
// Original version by Chris Laurel <claurel@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include "parser.h"
#include <celastro/astro.h>

using namespace Eigen;
using namespace std;

/****** Value method implementations *******/

Value::Value(double d) : type(NumberType), data(d) {
}

Value::Value(const std::string& s) : type(StringType), data(s) {
}

Value::Value(const ValueArrayPtr& a) : type(ArrayType), data(a) {
}

Value::Value(const HashPtr& h) : type(HashType), data(h) {
}

Value::Value(bool b) : type{ BooleanType }, data(b ? 1.0 : 0.0) {
}

Value::~Value() {
}

Value::ValueType Value::getType() const {
    return type;
}

const double& Value::getNumber() const {
    // ASSERT(type == NumberType);
    return data.d;
}

const string& Value::getString() const {
    static const std::string EMPTY;
    // ASSERT(type == StringType);
    return data.s ? *data.s : EMPTY;
}

const ValueArrayPtr& Value::getArray() const {
    // ASSERT(type == ArrayType);
    return data.a;
}

const HashPtr& Value::getHash() const {
    // ASSERT(type == HashType);
    return data.h;
}

bool Value::getBoolean() const {
    // ASSERT(type == BooleanType);
    return (data.d != 0.0);
}

/****** Parser method implementation ******/

Parser::Parser(Tokenizer* _tokenizer) : tokenizer(_tokenizer) {
}

ValueArrayPtr Parser::readArray() {
    Tokenizer::TokenType tok = tokenizer->nextToken();
    if (tok != Tokenizer::TokenBeginArray) {
        tokenizer->pushBack();
        return nullptr;
    }

    auto array = std::make_shared<ValueArray>();

    auto v = readValue();
    while (v) {
        array->insert(array->end(), v);
        v = readValue();
    }

    tok = tokenizer->nextToken();
    if (tok != Tokenizer::TokenEndArray) {
        tokenizer->pushBack();
        return nullptr;
    }

    return array;
}

HashPtr Parser::readHash() {
    Tokenizer::TokenType tok = tokenizer->nextToken();
    if (tok != Tokenizer::TokenBeginGroup) {
        tokenizer->pushBack();
        return nullptr;
    }

    auto hash = std::make_shared<Hash>();
    tok = tokenizer->nextToken();
    while (tok != Tokenizer::TokenEndGroup) {
        if (tok != Tokenizer::TokenName) {
            tokenizer->pushBack();
            return nullptr;
        }
        string name = tokenizer->getNameValue();

#ifndef USE_POSTFIX_UNITS
        readUnits(name, hash);
#endif

        auto value = readValue();
        if (!value) {
            return nullptr;
        }

        hash->addValue(name, value);

#ifdef USE_POSTFIX_UNITS
        readUnits(name, hash);
#endif

        tok = tokenizer->nextToken();
    }

    return hash;
}

/**
 * Reads a units section into the hash.
 * @param[in] propertyName Name of the current property.
 * @param[in] hash Hash to add units quantities into.
 * @return True if a units section was successfully read, false otherwise.
 */
bool Parser::readUnits(const string& propertyName, const HashPtr& hash) {
    Tokenizer::TokenType tok = tokenizer->nextToken();
    if (tok != Tokenizer::TokenBeginUnits) {
        tokenizer->pushBack();
        return false;
    }

    tok = tokenizer->nextToken();
    while (tok != Tokenizer::TokenEndUnits) {
        if (tok != Tokenizer::TokenName) {
            tokenizer->pushBack();
            return false;
        }

        string unit = tokenizer->getNameValue();
        auto value = std::make_shared<Value>(unit);

        if (astro::isLengthUnit(unit)) {
            string keyName(propertyName + "%Length");
            hash->addValue(keyName, value);
        } else if (astro::isTimeUnit(unit)) {
            string keyName(propertyName + "%Time");
            hash->addValue(keyName, value);
        } else if (astro::isAngleUnit(unit)) {
            string keyName(propertyName + "%Angle");
            hash->addValue(keyName, value);
        } else {
            return false;
        }

        tok = tokenizer->nextToken();
    }

    return true;
}

ValuePtr Parser::readValue() {
    Tokenizer::TokenType tok = tokenizer->nextToken();
    switch (tok) {
        case Tokenizer::TokenNumber:
            return std::make_shared<Value>(tokenizer->getNumberValue());

        case Tokenizer::TokenString:
            return std::make_shared<Value>(tokenizer->getStringValue());

        case Tokenizer::TokenName:
            if (tokenizer->getNameValue() == "false")
                return std::make_shared<Value>(false);
            else if (tokenizer->getNameValue() == "true")
                return std::make_shared<Value>(true);
            else {
                tokenizer->pushBack();
                return nullptr;
            }

        case Tokenizer::TokenBeginArray:
            tokenizer->pushBack();
            {
                auto array = readArray();
                if (!array)
                    return nullptr;
                else
                    return std::make_shared<Value>(array);
            }

        case Tokenizer::TokenBeginGroup:
            tokenizer->pushBack();
            {
                auto hash = readHash();
                if (!hash)
                    return nullptr;
                else
                    return std::make_shared<Value>(hash);
            }

        default:
            tokenizer->pushBack();
            return nullptr;
    }
}

ValuePtr AssociativeArray::getValue(const string& key) const {
    auto iter = assoc.find(key);
    if (iter == assoc.end())
        return nullptr;
    else
        return iter->second;
}

void AssociativeArray::addValue(const string& key, const ValuePtr& val) {
    assoc.insert({ key, val });
}

bool AssociativeArray::getNumber(const string& key, double& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::NumberType)
        return false;

    val = v->getNumber();
    return true;
}

bool AssociativeArray::getNumber(const string& key, float& val) const {
    double dval;

    if (!getNumber(key, dval)) {
        return false;
    } else {
        val = (float)dval;
        return true;
    }
}

bool AssociativeArray::getNumber(const string& key, int& val) const {
    double ival;

    if (!getNumber(key, ival)) {
        return false;
    } else {
        val = (int)ival;
        return true;
    }
}

bool AssociativeArray::getNumber(const string& key, uint32_t& val) const {
    double ival;

    if (!getNumber(key, ival)) {
        return false;
    } else {
        val = (uint32_t)ival;
        return true;
    }
}

bool AssociativeArray::getString(const string& key, string& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::StringType)
        return false;

    val = v->getString();

    return true;
}

bool AssociativeArray::getBoolean(const string& key, bool& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::BooleanType)
        return false;

    val = v->getBoolean();

    return true;
}

bool AssociativeArray::getVector(const string& key, Vec3d& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::ArrayType)
        return false;

    auto arr = v->getArray();
    if (arr->size() != 3)
        return false;

    auto x = (*arr)[0];
    auto y = (*arr)[1];
    auto z = (*arr)[2];

    if (x->getType() != Value::NumberType || y->getType() != Value::NumberType || z->getType() != Value::NumberType)
        return false;

    val = Vec3d(x->getNumber(), y->getNumber(), z->getNumber());

    return true;
}

bool AssociativeArray::getVector(const string& key, Vector3d& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::ArrayType)
        return false;

    auto arr = v->getArray();
    if (arr->size() != 3)
        return false;

    auto x = (*arr)[0];
    auto y = (*arr)[1];
    auto z = (*arr)[2];

    if (x->getType() != Value::NumberType || y->getType() != Value::NumberType || z->getType() != Value::NumberType)
        return false;

    val = Vector3d(x->getNumber(), y->getNumber(), z->getNumber());

    return true;
}

bool AssociativeArray::getVector(const string& key, Vec3f& val) const {
    Vec3d vecVal;

    if (!getVector(key, vecVal))
        return false;

    val = Vec3f((float)vecVal.x, (float)vecVal.y, (float)vecVal.z);

    return true;
}

bool AssociativeArray::getVector(const string& key, Vector3f& val) const {
    Vector3d vecVal;

    if (!getVector(key, vecVal))
        return false;

    val = vecVal.cast<float>();

    return true;
}

/** @copydoc AssociativeArray::getRotation() */
bool AssociativeArray::getRotation(const string& key, Quatf& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::ArrayType)
        return false;

    auto arr = v->getArray();
    if (arr->size() != 4)
        return false;

    auto w = (*arr)[0];
    auto x = (*arr)[1];
    auto y = (*arr)[2];
    auto z = (*arr)[3];

    if (w->getType() != Value::NumberType || x->getType() != Value::NumberType || y->getType() != Value::NumberType ||
        z->getType() != Value::NumberType)
        return false;

    Vec3f axis((float)x->getNumber(), (float)y->getNumber(), (float)z->getNumber());
    axis.normalize();

    double ang = w->getNumber();
    double angScale = 1.0;
    getAngleScale(key, angScale);
    float angle = degToRad((float)(ang * angScale));

    val.setAxisAngle(axis, angle);

    return true;
}

/**
 * Retrieves a quaternion, scaled to an associated angle unit.
 * 
 * The quaternion is specified in the catalog file in axis-angle format as follows:
 * \verbatim {PropertyName} [ angle axisX axisY axisZ ] \endverbatim
 * 
 * @param[in] key Hash key for the rotation.
 * @param[out] val A quaternion representing the value if present, unaffected if not.
 * @return True if the key exists in the hash, false otherwise.
 */
bool AssociativeArray::getRotation(const string& key, Eigen::Quaternionf& val) const {
    auto v = getValue(key);
    if (!v || v->getType() != Value::ArrayType)
        return false;

    auto arr = v->getArray();
    if (arr->size() != 4)
        return false;

    auto w = (*arr)[0];
    auto x = (*arr)[1];
    auto y = (*arr)[2];
    auto z = (*arr)[3];

    if (w->getType() != Value::NumberType || x->getType() != Value::NumberType || y->getType() != Value::NumberType ||
        z->getType() != Value::NumberType)
        return false;

    Vector3f axis((float)x->getNumber(), (float)y->getNumber(), (float)z->getNumber());

    double ang = w->getNumber();
    double angScale = 1.0;
    getAngleScale(key, angScale);
    float angle = degToRad((float)(ang * angScale));

    val = Quaternionf(AngleAxisf(angle, axis.normalized()));

    return true;
}

bool AssociativeArray::getColor(const string& key, Color& val) const {
    Vec3d vecVal;

    if (!getVector(key, vecVal))
        return false;

    val = Color((float)vecVal.x, (float)vecVal.y, (float)vecVal.z);

    return true;
}

/**
 * Retrieves a numeric quantity scaled to an associated angle unit.
 * @param[in] key Hash key for the quantity.
 * @param[out] val The returned quantity if present, unaffected if not.
 * @param[in] outputScale Returned value is scaled to this value.
 * @param[in] defaultScale If no units are specified, use this scale. Defaults to outputScale.
 * @return True if the key exists in the hash, false otherwise.
 */
bool AssociativeArray::getAngle(const string& key, double& val, double outputScale, double defaultScale) const {
    if (!getNumber(key, val))
        return false;

    double angleScale;
    if (getAngleScale(key, angleScale)) {
        angleScale /= outputScale;
    } else {
        angleScale = (defaultScale == 0.0) ? 1.0 : defaultScale / outputScale;
    }

    val *= angleScale;

    return true;
}

/** @copydoc AssociativeArray::getAngle() */
bool AssociativeArray::getAngle(const string& key, float& val, double outputScale, double defaultScale) const {
    double dval;

    if (!getAngle(key, dval, outputScale, defaultScale))
        return false;

    val = ((float)dval);

    return true;
}

/**
 * Retrieves a numeric quantity scaled to an associated length unit.
 * @param[in] key Hash key for the quantity.
 * @param[out] val The returned quantity if present, unaffected if not.
 * @param[in] outputScale Returned value is scaled to this value.
 * @param[in] defaultScale If no units are specified, use this scale. Defaults to outputScale.
 * @return True if the key exists in the hash, false otherwise.
 */
bool AssociativeArray::getLength(const string& key, double& val, double outputScale, double defaultScale) const {
    if (!getNumber(key, val))
        return false;

    double lengthScale;
    if (getLengthScale(key, lengthScale)) {
        lengthScale /= outputScale;
    } else {
        lengthScale = (defaultScale == 0.0) ? 1.0 : defaultScale / outputScale;
    }

    val *= lengthScale;

    return true;
}

/** @copydoc AssociativeArray::getLength() */
bool AssociativeArray::getLength(const string& key, float& val, double outputScale, double defaultScale) const {
    double dval;

    if (!getLength(key, dval, outputScale, defaultScale))
        return false;

    val = ((float)dval);

    return true;
}

/**
 * Retrieves a numeric quantity scaled to an associated time unit.
 * @param[in] key Hash key for the quantity.
 * @param[out] val The returned quantity if present, unaffected if not.
 * @param[in] outputScale Returned value is scaled to this value.
 * @param[in] defaultScale If no units are specified, use this scale. Defaults to outputScale.
 * @return True if the key exists in the hash, false otherwise.
 */
bool AssociativeArray::getTime(const string& key, double& val, double outputScale, double defaultScale) const {
    if (!getNumber(key, val))
        return false;

    double timeScale;
    if (getTimeScale(key, timeScale)) {
        timeScale /= outputScale;
    } else {
        timeScale = (defaultScale == 0.0) ? 1.0 : defaultScale / outputScale;
    }

    val *= timeScale;

    return true;
}

/** @copydoc AssociativeArray::getTime() */
bool AssociativeArray::getTime(const string& key, float& val, double outputScale, double defaultScale) const {
    double dval;

    if (!getLength(key, dval, outputScale, defaultScale))
        return false;

    val = ((float)dval);

    return true;
}

/**
 * Retrieves a vector quantity scaled to an associated length unit.
 * @param[in] key Hash key for the quantity.
 * @param[out] val The returned vector if present, unaffected if not.
 * @param[in] outputScale Returned value is scaled to this value.
 * @param[in] defaultScale If no units are specified, use this scale. Defaults to outputScale.
 * @return True if the key exists in the hash, false otherwise.
 */
bool AssociativeArray::getLengthVector(const string& key, Eigen::Vector3d& val, double outputScale, double defaultScale) const {
    if (!getVector(key, val))
        return false;

    double lengthScale;
    if (getLengthScale(key, lengthScale)) {
        lengthScale /= outputScale;
    } else {
        lengthScale = (defaultScale == 0.0) ? 1.0 : defaultScale / outputScale;
    }

    val *= lengthScale;

    return true;
}

/** @copydoc AssociativeArray::getLengthVector() */
bool AssociativeArray::getLengthVector(const string& key, Eigen::Vector3f& val, double outputScale, double defaultScale) const {
    Vector3d vecVal;

    if (!getLengthVector(key, vecVal, outputScale, defaultScale))
        return false;

    val = vecVal.cast<float>();

    return true;
}

/**
 * Retrieves a spherical tuple \verbatim [longitude, latitude, altitude] \endverbatim scaled to associated angle and length units.
 * @param[in] key Hash key for the quantity.
 * @param[out] val The returned tuple in units of degrees and kilometers if present, unaffected if not.
 * @return True if the key exists in the hash, false otherwise.
 */
bool AssociativeArray::getSphericalTuple(const string& key, Vector3d& val) const {
    if (!getVector(key, val))
        return false;

    double angleScale;
    if (getAngleScale(key, angleScale)) {
        val[0] *= angleScale;
        val[1] *= angleScale;
    }

    double lengthScale = 1.0;
    getLengthScale(key, lengthScale);
    val[2] *= lengthScale;

    return true;
}

/** @copydoc AssociativeArray::getSphericalTuple */
bool AssociativeArray::getSphericalTuple(const string& key, Vector3f& val) const {
    Vector3d vecVal;

    if (!getSphericalTuple(key, vecVal))
        return false;

    val = vecVal.cast<float>();

    return true;
}

/**
 * Retrieves the angle unit associated with a given property.
 * @param[in] key Hash key for the property.
 * @param[out] scale The returned angle unit scaled to degrees if present, unaffected if not.
 * @return True if an angle unit has been specified for the property, false otherwise.
 */
bool AssociativeArray::getAngleScale(const string& key, double& scale) const {
    string unitKey(key + "%Angle");
    string unit;

    if (!getString(unitKey, unit))
        return false;

    return astro::getAngleScale(unit, scale);
}

/** @copydoc AssociativeArray::getAngleScale() */
bool AssociativeArray::getAngleScale(const string& key, float& scale) const {
    double dscale;
    if (!getAngleScale(key, dscale))
        return false;

    scale = ((float)dscale);
    return true;
}

/**
 * Retrieves the length unit associated with a given property.
 * @param[in] key Hash key for the property.
 * @param[out] scale The returned length unit scaled to kilometers if present, unaffected if not.
 * @return True if a length unit has been specified for the property, false otherwise.
 */
bool AssociativeArray::getLengthScale(const string& key, double& scale) const {
    string unitKey(key + "%Length");
    string unit;

    if (!getString(unitKey, unit))
        return false;

    return astro::getLengthScale(unit, scale);
}

/** @copydoc AssociativeArray::getLengthScale() */
bool AssociativeArray::getLengthScale(const string& key, float& scale) const {
    double dscale;
    if (!getLengthScale(key, dscale))
        return false;

    scale = ((float)dscale);
    return true;
}

/**
 * Retrieves the time unit associated with a given property.
 * @param[in] key Hash key for the property.
 * @param[out] scale The returned time unit scaled to days if present, unaffected if not.
 * @return True if a time unit has been specified for the property, false otherwise.
 */
bool AssociativeArray::getTimeScale(const string& key, double& scale) const {
    string unitKey(key + "%Time");
    string unit;

    if (!getString(unitKey, unit))
        return false;

    return astro::getTimeScale(unit, scale);
}

/** @copydoc AssociativeArray::getTimeScale() */
bool AssociativeArray::getTimeScale(const string& key, float& scale) const {
    double dscale;
    if (!getTimeScale(key, dscale))
        return false;

    scale = ((float)dscale);
    return true;
}

HashIterator AssociativeArray::begin() {
    return assoc.begin();
}

HashIterator AssociativeArray::end() {
    return assoc.end();
}