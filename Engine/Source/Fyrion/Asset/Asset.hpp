#pragma once
#include "AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Asset;
    class AssetDirectory;

    struct Blob
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

        static Blob FromString(StringView str)
        {
            return Blob{HexTo64(str)};
        }
    };

    class FY_API Asset
    {
    public:
        virtual ~Asset();

        UUID                  GetUUID() const;
        void                  SetUUID(const UUID& p_uuid);
        TypeHandler*          GetType() const;
        TypeID                GetAssetTypeId() const;
        Asset*                GetParent() const;
        Asset*                GetPrototype() const;
        virtual StringView    GetName() const;
        virtual StringView    GetDisplayName() const;
        StringView            GetPath() const;
        StringView            GetAbsolutePath() const;
        virtual void          SetName(StringView name);
        void                  AddRelatedFile(StringView fileAbsolutePath);
        virtual void          SetExtension(StringView p_extension);
        StringView            GetExtension() const;
        virtual void          BuildPath();
        virtual void          OnCreated() {}
        virtual void          OnDestroyed() {}
        virtual void          OnModified() {}
        void                  SetModified();
        virtual bool          IsModified() const;
        void                  SaveBlob(Blob& blob, ConstPtr data, usize dataSize);
        usize                 GetBlobSize(Blob blob) const;
        void                  LoadBlob(Blob blob, VoidPtr, usize dataSize) const;
        bool                  IsChildOf(Asset* parent) const;
        Span<Asset*>          GetChildrenAssets() const;
        void                  RemoveChild(Asset* child);
        void                  AddChild(Asset* child);
        Asset*                FindChildByAbsolutePath(StringView absolutePath) const;

        //TODO - rename to GetCacheDirectory ?
        Asset*                GetPhysicalAsset();       //used for save/load blobs
        const Asset*          GetPhysicalAsset() const; //used for save/load blobs

        virtual bool          LoadData();
        virtual void          SaveData();
        virtual StringView    GetDataExtesion();
        virtual void          DeserializeData(ArchiveReader& reader, ArchiveObject object);
        virtual ArchiveObject SerializeData(ArchiveWriter& writer) const;
        void                  Destroy();

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return dynamic_cast<T*>(this);
        }

        friend class AssetDatabase;
        friend class AssetDirectory;

    private:
        UUID          uuid{};
        bool          hasBlobs = false;
        String        path{};
        Asset*        prototype{};
        Asset*        parent{};
        Array<Asset*> children{};
        Array<String> relatedFiles{};
        TypeHandler*  assetType{};
        u64           currentVersion{};
        u64           loadedVersion{};
        u64           lastModified{};
        String        name{};
        String        extension;
        String        absolutePath{};

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

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, T* const* value)
        {
            if (*value)
            {
                writer.WriteString(object, name, ToString((*value)->GetUUID()));
            }
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, T** value)
        {
            UUID uuid = UUID::FromString(reader.ReadString(object, name));
            if (uuid)
            {
                *value = AssetDatabase::Create<T>(uuid);
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
                writer.AddString(array, ToString((*value)->GetUUID()));
            }
            else if (writer.HasOpt(SerializationOptions::IncludeNullOrEmptyValues))
            {
                writer.AddString(array, ToString(UUID{}));
            }
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, T** value)
        {
            UUID uuid = UUID::FromString(reader.GetString(item));
            if (uuid)
            {
                *value = AssetDatabase::Create<T>(uuid);
            }
            else if (*value)
            {
                *value = nullptr;
            }
        }
    };

    template <>
    struct ArchiveType<Blob>
    {
        constexpr static bool hasArchiveImpl = true;

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, const Blob* value)
        {
            if (*value)
            {
                writer.WriteString(object, name, value->ToString());
            }
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, Blob* value)
        {
            *value = Blob::FromString(reader.ReadString(object, name));
        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, const Blob* value)
        {
            FY_ASSERT(false, "not implemented");
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, Blob* value)
        {
            FY_ASSERT(false, "not implemented");
        }
    };
}
