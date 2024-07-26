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
        virtual ~Asset() = default;

        UUID GetUUID() const
        {
            return uuid;
        }

        void SetUUID(const UUID& p_uuid);

        Asset* GetPrototype() const
        {
            return prototype;
        }

        TypeHandler* GetAssetType() const
        {
            return assetType;
        }

        TypeID GetAssetTypeId() const;

        virtual StringView GetName() const
        {
            return name;
        }

        StringView GetPath() const
        {
            return path;
        }

        StringView GetAbsolutePath() const
        {
            return absolutePath;
        }

        virtual void SetName(StringView p_name);
        virtual void SetExtension(StringView p_extension);
        StringView   GetExtension() const;


        bool IsActive() const
        {
            return active;
        }

        void SetActive(bool p_active);

        AssetDirectory* GetDirectory() const
        {
            return directory;
        }

        bool IsChildOf(Asset* parent) const;

        virtual void BuildPath();
        virtual void OnActiveChanged() {}

        virtual void Modify()
        {
            currentVersion += 1;
        }

        virtual bool       IsModified() const;
        virtual StringView GetDisplayName() const;

        friend class AssetDatabase;
        friend class AssetDirectory;


        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return dynamic_cast<T*>(this);
        }

        void  SaveBlob(Blob& blob, ConstPtr data, usize dataSize);
        usize GetBlobSize(Blob blob) const;
        void  LoadBlob(Blob blob, VoidPtr, usize dataSize) const;

        Span<Asset*> GetChildrenAssets() const;

        Asset*       GetPhysicalAsset();
        const Asset* GetPhysicalAsset() const;

        Asset* GetOwner() const;
        void   SetOwner(Asset* owner);

        virtual bool          LoadData();
        virtual void          SaveData();
        virtual StringView    GetDataExtesion();
        virtual void          DeserializeData(ArchiveReader& reader, ArchiveObject object);
        virtual ArchiveObject SerializeData(ArchiveWriter& writer) const;

    private:
        usize           index{};
        UUID            uuid{};
        bool            hasBlobs = false;
        String          path{};
        Asset*          prototype{};
        Asset*          owner{};
        Array<Asset*>   assets{};
        TypeHandler*    assetType{};
        u64             currentVersion{};
        u64             loadedVersion{};
        String          name{};
        String          extension;
        String          absolutePath{};
        AssetDirectory* directory{};
        bool            active = true;

        void ValidateName();

    public:
        static void RegisterType(NativeTypeHandler<Asset>& type);
    };

    struct AssetApi
    {
        Asset* (*castAsset)(VoidPtr ptr);
        const TypeHandler* (*getTypeHandler)();
    };

    template<typename T>
    struct TypeApiInfo<T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>>
    {
        static void ExtractApi(VoidPtr pointer)
        {
            AssetApi* api = static_cast<AssetApi*>(pointer);
            api->castAsset = [](VoidPtr ptr)
            {
                return static_cast<Asset*>(*static_cast<T**>(ptr));
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

        static void Add(ArchiveWriter& writer, ArchiveObject array, T*const * value)
        {
            if (*value)
            {
                writer.AddString(array, ToString((*value)->GetUUID()));
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
