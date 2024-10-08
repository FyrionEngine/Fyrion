#pragma once
#include "AssetManager.hpp"
#include "AssetHandler.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    struct ImportSettings;
    class AssetBufferManager;

    struct AssetBuffer
    {
        u64 id;

        explicit operator bool() const noexcept
        {
            return id != 0;
        }

        String ToString() const
        {
            char  strBuffer[17]{};
            usize bufSize = U64ToHex(id, strBuffer);
            return {strBuffer, bufSize};
        }

        static AssetBuffer FromString(StringView str)
        {
            return AssetBuffer{HexTo64(str)};
        }
    };

    class FY_API Asset
    {
    public:
        virtual ~Asset() = default;

        AssetHandler* GetHandler() const;
        void          SetHandler(AssetHandler* handler);
        void          SetModified();
        ArchiveObject Serialize(ArchiveWriter& writer) const;
        void          Deserialize(ArchiveReader& reader, ArchiveObject object);
        void          SaveBuffer(AssetBuffer& buffer, ConstPtr data, usize dataSize);
        Array<u8>     LoadBuffer(AssetBuffer buffer) const;
        bool          HasBuffer(AssetBuffer buffer) const;
        Asset*        GetParent() const;

        virtual void OnModified() {}
        virtual void OnDestroyed() {}

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return dynamic_cast<T*>(this);
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* GetParent()
        {
            return dynamic_cast<T*>(GetParent());
        }

        static void RegisterType(NativeTypeHandler<Asset>& type);

    protected:
        AssetHandler* handler{};
    };

    struct AssetApi
    {
        Asset* (*            castAsset)(VoidPtr ptr);
        void (*              setAsset)(VoidPtr ptr, Asset* asset);
        const TypeHandler* (*getTypeHandler)();
    };

    template <typename T>
    struct TypeApiInfo<T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>>
    {
        static void ExtractApi(VoidPtr pointer)
        {
            AssetApi* api = static_cast<AssetApi*>(pointer);
            api->castAsset = [](VoidPtr ptr)
            {
                return static_cast<Asset*>(*static_cast<T**>(ptr));
            };

            api->setAsset = [](VoidPtr ptr, Asset* asset)
            {
                *static_cast<T**>(ptr) = static_cast<T*>(asset);
            };
        }

        static constexpr TypeID GetApiId()
        {
            return GetTypeID<AssetApi>();
        }
    };

    template <typename T>
    struct ArchiveType<T*, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>>
    {
        constexpr static bool hasArchiveImpl = true;

        static ArchiveObject GetObjectFromAsset(ArchiveWriter& writer, const T* asset)
        {
            AssetHandler* handler = asset->GetHandler();

            ArchiveObject object = writer.CreateObject();

            if (handler->GetUUID())
            {
                writer.WriteString(object, "uuid", ToString(handler->GetUUID()));
            }
            else if (!handler->GetPath().Empty())
            {
                writer.WriteString(object, "path", handler->GetPath());
            }

            if (handler->GetType()->GetTypeInfo().typeId != GetTypeID<T>())
            {
                writer.WriteString(object, "type", handler->GetType()->GetName());
            }
            return object;
        }

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, T* const* value)
        {
            if (*value)
            {
                writer.WriteValue(object, name, GetObjectFromAsset(writer, *value));
            }
        }

        static T* GetAssetFromObject(ArchiveReader& reader, ArchiveObject asserRef)
        {
            UUID uuid = UUID::FromString(reader.ReadString(asserRef, "uuid"));
            if (uuid)
            {
                if (T* asset = AssetManager::LoadById<T>(uuid))
                {
                    return asset;
                }
            }
            String path = reader.ReadString(asserRef, "path");
            if (!path.Empty())
            {
                if (T* asset = AssetManager::LoadByPath<T>(path))
                {
                    return asset;
                }
            }

            if (!path.Empty() || uuid)
            {
                TypeHandler* typeHandler = nullptr;
                if (StringView type = reader.ReadString(asserRef, "type"); !type.Empty())
                {
                    typeHandler = Registry::FindTypeByName(type);
                }

                if (!typeHandler)
                {
                    typeHandler = Registry::FindType<T>();
                }

                if (typeHandler)
                {
                    //TODO - not sure about it.

                    // return static_cast<T*>(AssetManager::Create(
                    //     typeHandler,
                    //     AssetCreation{
                    //         .uuid = uuid,
                    //         .desiredPath = path
                    //     }
                    // ));
                }
            }

            return nullptr;
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, T** value)
        {
            if (ArchiveObject asserRef = reader.ReadObject(object, name))
            {
                *value = GetAssetFromObject(reader, asserRef);
            }
            else if (*value)
            {
                *value = nullptr;
            }
        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, T* const * value)
        {
            if (*value)
            {
                writer.AddValue(array, GetObjectFromAsset(writer, *value));
            }
            else if (writer.HasOpt(SerializationOptions::IncludeNullOrEmptyValues))
            {
                writer.AddValue(array, writer.CreateObject());
            }
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, T** value)
        {
            if (item)
            {
                *value = GetAssetFromObject(reader, item);
            }
            else if (*value)
            {
                *value = nullptr;
            }
        }
    };

    template <>
    struct ArchiveType<AssetBuffer>
    {
        constexpr static bool hasArchiveImpl = true;

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, const AssetBuffer* value)
        {
            if (*value)
            {
                writer.WriteString(object, name, value->ToString());
            }
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, AssetBuffer* value)
        {
            *value = AssetBuffer::FromString(reader.ReadString(object, name));
        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, const AssetBuffer* value)
        {
            FY_ASSERT(false, "not implemented");
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, AssetBuffer* value)
        {
            FY_ASSERT(false, "not implemented");
        }
    };
}
