#pragma once

#include "PLCTProperties.h"

/// <summary>
/// PLCT object used to represent points.
/// </summary>
API_CLASS() class PLCT_API PLCTPoint : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTPoint, ScriptingObject);

public:
    /// <summary>
    /// Gets the properties of the point.
    /// </summary>
    /// <returns>The point properties.</returns>
    API_PROPERTY() PLCTPropertyStorage* GetProperties()
    {
        return &_properties;
    }

    /// <summary>
    /// Set the transform of this point.
    /// </summary>
    /// <param name="transform">The new value.</param>
    API_PROPERTY() void SetTransform(Transform& transform)
    {
        _properties.EnsureProperty(TEXT("Transform"));
        _properties.SetPropertyValue(TEXT("Transform"), Variant(transform));
    }

    /// <summary>
    /// Get the transform of this point.
    /// </summary>
    /// <returns>The transform property, or an identity transform if it does not exist.</returns>
    API_PROPERTY() Transform GetTransform()
    {
        PLCTProperty* transformProperty = _properties.GetProperty(TEXT("Transform"));

        Transform transform = Transform::Identity;
        CHECK_RETURN(transformProperty, transform);
        transformProperty->GetTransform(transform);
        return transform;
    }

    API_FUNCTION() PLCTPoint* Copy()
    {
        PLCTPoint* point = New<PLCTPoint>();
        _properties.CopyInto(point->_properties);
        
        return point;
    }

private:
    PLCTPropertyStorage _properties;
};

/// <summary>
/// PLCT points container, which holds an array of points and allows you to perform operations on them.
/// </summary>
API_CLASS() class PLCT_API PLCTPointsContainer : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTPointsContainer, ScriptingObject);

public:
    ~PLCTPointsContainer()
    {
        for (auto point : _points)
        {
            if (point)
                Delete(point);
        }
        _points.Clear();
    }

    /// <summary>
    /// Get the array of points in this container.
    /// </summary>
    /// <returns>The points array.</returns>
    API_PROPERTY() Array<PLCTPoint*>& GetPoints()
    {
        return _points;
    }

private:
    Array<PLCTPoint*> _points;
};
