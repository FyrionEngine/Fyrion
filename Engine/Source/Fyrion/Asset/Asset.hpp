#pragma once
#include "AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Asset;
    class AssetDirectory;
    struct ImportSettings;

    struct CacheRef
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

        static CacheRef FromString(StringView str)
        {
            return CacheRef{HexTo64(str)};
        }
    };

    class FY_API Asset
    {
    public:
        virtual ~Asset() = default;

        UUID                    GetUUID() const;
        void                    SetUUID(const UUID& uuid);
        TypeHandler*            GetType() const;
        TypeID                  GetAssetTypeId() const;
        Asset*                  GetParent() const;
        Asset*                  GetPrototype() const;
        virtual StringView      GetName() const;
        virtual StringView      GetDisplayName() const;
        StringView              GetPath() const;
        StringView              GetAbsolutePath() const;
        virtual void            SetName(StringView newName);
        void                    AddRelatedFile(StringView fileAbsolutePath);
        StringView              GetExtension() const;
        virtual void            BuildPath();
        virtual void            OnCreated() {}
        virtual void            OnDestroyed() {}
        virtual void            OnModified() {}
        virtual ImportSettings* GetImportSettings();
        void                    SetModified();
        virtual bool            IsModified() const;
        void                    SaveCache(CacheRef& cache, ConstPtr data, usize dataSize);
        usize                   GetCacheSize(CacheRef cache) const;
        void                    LoadCache(CacheRef cache, VoidPtr, usize dataSize) const;
        bool                    IsChildOf(Asset* parent) const;
        Span<Asset*>            GetChildren() const;
        void                    RemoveChild(Asset* child);
        void                    RemoveFromParent();
        void                    AddChild(Asset* child);
        Asset*                  FindChildByAbsolutePath(StringView absolutePath) const;
        virtual String          GetCacheDirectory() const;
        void                    Destroy();

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return dynamic_cast<T*>(this);
        }

        friend class AssetDatabase;

    private:
        UUID          uuid{};
        String        path{};
        Asset*        prototype{};
        Asset*        parent{};
        Array<Asset*> children{};
        Array<String> relatedFiles{};
        TypeHandler*  assetType{};
        u64           currentVersion{};
        u64           loadedVersion{};
        u64           lastModifiedTime{};
        String        name{};
        String        absolutePath{};
        bool          imported = false;


        void ValidateName();

    public:
        static void RegisterType(NativeTypeHandler<Asset>& type);
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
            ArchiveObject object = writer.CreateObject();

            if (asset->GetUUID())
            {
                writer.WriteString(object, "uuid", ToString(asset->GetUUID()));
            }
            else if (!asset->GetPath().Empty())
            {
                writer.WriteString(object, "path", asset->GetPath());
            }

            if (asset->GetAssetTypeId() != GetTypeID<T>())
            {
                writer.WriteString(object, "type", asset->GetType()->GetName());
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
                if (T* asset = AssetDatabase::FindById<T>(uuid))
                {
                    return asset;
                }
            }
            String path = reader.ReadString(asserRef, "path");
            if (!path.Empty())
            {
                if (T* asset = AssetDatabase::FindByPath<T>(path))
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

                return static_cast<T*>(AssetDatabase::Create(typeHandler, {.uuid = uuid, .desiredPath = path}));
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
    struct ArchiveType<CacheRef>
    {
        constexpr static bool hasArchiveImpl = true;

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, const CacheRef* value)
        {
            if (*value)
            {
                writer.WriteString(object, name, value->ToString());
            }
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, CacheRef* value)
        {
            *value = CacheRef::FromString(reader.ReadString(object, name));
        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, const CacheRef* value)
        {
            FY_ASSERT(false, "not implemented");
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, CacheRef* value)
        {
            FY_ASSERT(false, "not implemented");
        }
    };
}
