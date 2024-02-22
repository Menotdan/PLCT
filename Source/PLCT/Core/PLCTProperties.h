#pragma once

#include "Engine/Core/Config.h"
#include "Engine/Core/Types/String.h"
#include "Engine/Core/Math/Quaternion.h"
#include "Engine/Core/Math/Transform.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Math/Vector4.h"
#include "Engine/Core/Types/Variant.h"
#include "Engine/Core/Collections/Array.h"
#include "Engine/Core/Log.h"
#include "Engine/Scripting/Scripting.h"
#include "Engine/Scripting/ScriptingType.h"
#include "Engine/Scripting/ScriptingObject.h"

class property_tracker
{
public:
    static int property_counter;
};

/// <summary>
/// PLCT Property class. Represents one property of a PLCT object.
/// </summary>
API_CLASS() class PLCT_API PLCTProperty : public ScriptingObject
{
    //DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTProperty, ScriptingObject);
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(type);
    static PLCTProperty* Spawn(const SpawnParams& params) { return ::New<PLCTProperty>(params); }
    explicit PLCTProperty(const SpawnParams& params) : ScriptingObject(params)
    {
        property_tracker::property_counter++;
    }
    explicit PLCTProperty() : PLCTProperty(SpawnParams(Guid::New(), PLCTProperty::TypeInitializer)) {}

public:
    ~PLCTProperty()
    {
        property_tracker::property_counter--;
        if (Data.Type == VariantType(VariantType::Object))
        {
            Delete(Data.AsObject);
            Data.AsObject = nullptr;
        }

        Data.SetType(VariantType(VariantType::Null));
    }

    /// <summary>
    /// The name of the property. Used to reference it.
    /// </summary>
    API_FIELD() String Name;

    /// <summary>
    /// The object data being stored in this property.
    /// </summary>
    API_FIELD() Variant Data;

    /// <summary>
    /// Sets a new value for the property, ignoring it if the type would change.
    /// </summary>
    /// <param name="value">The new value of the property.</param>
    API_FUNCTION() void SetValue(Variant value)
    {
        if (value.Type == VariantType(VariantType::Null))
        {
            LOG(Warning, "PLCT Property '{0}' set as null! (Ignored)", Name);
            return;
        }

        if (!(Data.Type == VariantType(VariantType::Null)))
        {
            if (!(Data.Type == value.Type))
            {
                LOG(Warning, "PLCT Property '{0}' type overridden! (Ignored)", Name);
                return;
            }
        }

        Data = value;
    }

    API_FUNCTION() PLCTProperty* Copy()
    {
        PLCTProperty* property = New<PLCTProperty>();
        property->Name.Append(Name);
        property->Data.SetType(Data.Type);

        if (Data.Type == VariantType(VariantType::Object))
        {
            // Copy underlying object
            ScriptingType type = Data.AsObject->GetType();
            ScriptingObject* object = Scripting::NewObject(type.GetHandle());
            Guid id = object->GetID();
            Platform::MemoryCopy(object, Data.AsObject, type.Size);
            object->ChangeID(id);

            property->Data.AsObject = object;
        }
        else
        {
            property->Data = Data;
        }
        return property;
    }

public:

    /// <summary>
    /// Gets a Uint64 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetUint64(API_PARAM(Out) uint64& output)
    {
        if (Data.Type == VariantType::Uint64)
        {
            output = Data.AsUint64;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets an Int64 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetInt64(API_PARAM(Out) int64& output)
    {
        if (Data.Type == VariantType::Int64)
        {
            output = Data.AsInt64;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Uint32 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetUint32(API_PARAM(Out) uint32& output)
    {
        if (Data.Type == VariantType::Uint)
        {
            output = Data.AsUint;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets an Int32 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetInt32(API_PARAM(Out) int32& output)
    {
        if (Data.Type == VariantType::Int)
        {
            output = Data.AsInt;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Uint16 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetUint16(API_PARAM(Out) uint16& output)
    {
        if (Data.Type == VariantType::Uint16)
        {
            output = Data.AsUint16;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets an Int16 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetInt16(API_PARAM(Out) int16& output)
    {
        if (Data.Type == VariantType::Int16)
        {
            output = Data.AsInt16;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Quaternion from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetQuaternion(API_PARAM(Out) Quaternion& output)
    {
        if (Data.Type == VariantType::Quaternion)
        {
            output = Data.AsQuaternion();;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Vector4 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetVector4(API_PARAM(Out) Vector4& output)
    {
        if (Data.Type == VariantType::Vector4)
        {
            output = Data.AsVector4();
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Vector3 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetVector3(API_PARAM(Out) Vector3& output)
    {
        if (Data.Type == VariantType::Vector3)
        {
            output = Data.AsVector3();
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Vector2 from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetVector2(API_PARAM(Out) Vector2& output)
    {
        if (Data.Type == VariantType::Vector2)
        {
            output = Data.AsVector2();
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a Transform from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetTransform(API_PARAM(Out) Transform& output)
    {
        if (Data.Type == VariantType::Transform)
        {
            output = Data.AsTransform();
            return true;
        }

        return false;
    }

    /// <summary>
    /// Gets a String from this property.
    /// </summary>
    /// <param name="output">The output.</param>
    /// <returns>True if got the value, otherwise false.</returns>
    API_FUNCTION() FORCE_INLINE bool GetString(API_PARAM(Out) String& output)
    {
        if (Data.Type == VariantType::String)
        {
            output = Data.ToString();
            return true;
        }

        return false;
    }
};

/// <summary>
/// PLCT Property list storage. 
/// </summary>
API_CLASS() class PLCT_API PLCTPropertyStorage : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTPropertyStorage, ScriptingObject);

public:
    ~PLCTPropertyStorage()
    {
        for (auto property : _properties)
        {
            if (property)
                Delete(property);
        }
        _properties.Clear();
    }

    /// <summary>
    /// Get the property associated with this name.
    /// </summary>
    /// <param name="name">The name of the target property.</param>
    /// <returns>The property, or null if it does not exist.</returns>
    API_FUNCTION() FORCE_INLINE PLCTProperty* GetProperty(String name)
    {
        PLCTProperty *result = nullptr;
        for (int i = 0; i < _properties.Count(); i++)
        {
            CHECK_RETURN(_properties[i], nullptr);
            if (!_properties[i]->Name.Compare(name))
            {
                result = _properties[i];
                break;
            }
        }

        return result;
    }

    /// <summary>
    /// Ensures there is a property with this name.
    /// </summary>
    /// <param name="name">The property to ensure is created.</param>
    /// <returns>The property, which has either been created or read from the list.</returns>
    API_FUNCTION() FORCE_INLINE PLCTProperty* EnsureProperty(String name)
    {
        PLCTProperty* earlyResult = GetProperty(name);
        if (earlyResult != nullptr)
        {
            return earlyResult;
        }

        PLCTProperty* newProperty = New<PLCTProperty>();
        newProperty->Data = Variant(nullptr);
        newProperty->Name.Append(name);
        _properties.Add(newProperty);

        return newProperty;
    }

    /// <summary>
    /// Gets the value of a property.
    /// </summary>
    /// <param name="name">The name of the property.</param>
    /// <returns>The value of the property, or null if it does not exist.</returns>
    API_FUNCTION() FORCE_INLINE Variant GetPropertyValue(String name)
    {
        PLCTProperty* property = GetProperty(name);
        if (property == nullptr)
        {
            return Variant(nullptr);
        }

        return property->Data;
    }

    /// <summary>
    /// Sets the value of a property.
    /// </summary>
    /// <param name="name">The name of the property.</param>
    /// <param name="value">The intended new value.</param>
    /// <returns>A bool indicating if the property was set.</returns>
    API_FUNCTION() FORCE_INLINE bool SetPropertyValue(String name, Variant value)
    {
        PLCTProperty* property = EnsureProperty(name);
        property->SetValue(value);

        return true;
    }

    API_FUNCTION() FORCE_INLINE void CopyInto(PLCTPropertyStorage& output)
    {
        for (auto property : _properties)
        {
            output._properties.Add(property->Copy());
        }
    }

private:
    Array<PLCTProperty*> _properties;
};
