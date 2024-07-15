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
            char strBuffer[17]{};
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

        virtual bool IsModified() const;
        virtual StringView GetDisplayName() const;

        friend class AssetDatabase;
        friend class AssetDirectory;

        static void RegisterType(NativeTypeHandler<Asset>& type);

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return static_cast<T*>(this);
        }

        void  SaveBlob(Blob& blob, ConstPtr data, usize dataSize);
        usize GetBlobSize(Blob blob) const;
        void  LoadBlob(Blob blob, VoidPtr, usize dataSize) const;

    private:
        usize           index{};
        UUID            uuid{};
        bool            hasBlobs = false;
        String          path{};
        Asset*          prototype{};
        TypeHandler*    assetType{};
        u64             currentVersion{};
        u64             loadedVersion{};
        String          name{};
        String          extension;
        String          absolutePath{};
        AssetDirectory* directory{};
        bool            active = true;

        void ValidateName();
    };

    template<typename T>
    struct Subobject
    {
        T* object;
    };


    template<>
    struct ArchiveType<Blob>
    {
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const Blob& value)
        {
            writer.WriteString(object, name, value.ToString());
        }

        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, Blob& value)
        {
            value = Blob::FromString(reader.ReadString(object, name));
        }
    };

    template <typename T>
    struct ArchiveType<T*, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>>
    {
        static T* GetField(ArchiveReader& reader, ArchiveObject object)
        {
            // T* asset = AssetDatabase::Create<T>();
            // Serialization::Deserialize(Registry::FindType<T>(), reader, object, asset);
            // return asset;
            return nullptr;
        }

        static void SetField(ArchiveWriter& writer, ArchiveObject object, const T* value)
        {
            //writer.AddValue(object, Serialization::Serialize(Registry::FindType<T>(), writer, value));
        }
    };

    template <typename T>
    struct ArchiveType<Subobject<T>, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>>
    {
        static Subobject<T> GetField(ArchiveReader& reader, ArchiveObject object)
        {
            // T* asset = AssetDatabase::Create<T>();
            // Serialization::Deserialize(Registry::FindType<T>(), reader, object, asset);
            // return asset;
            return {};
        }

        static void SetField(ArchiveWriter& writer, ArchiveObject object, const Subobject<T> value)
        {
            //writer.AddValue(object, Serialization::Serialize(Registry::FindType<T>(), writer, value));
        }
    };
}
