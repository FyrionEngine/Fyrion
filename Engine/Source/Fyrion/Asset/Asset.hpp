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

        static void RegisterType(NativeTypeHandler<Asset>& type);

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return dynamic_cast<T*>(this);
        }

        void  SaveBlob(Blob& blob, ConstPtr data, usize dataSize);
        usize GetBlobSize(Blob blob) const;
        void  LoadBlob(Blob blob, VoidPtr, usize dataSize) const;


        Asset*       GetPhysicalAsset();
        const Asset* GetPhysicalAsset() const;

        Asset* GetOwner() const;
        void   SetOwner(Asset*);

    private:
        usize           index{};
        UUID            uuid{};
        bool            hasBlobs = false;
        String          path{};
        Asset*          prototype{};
        Asset*          owner{};
        Array<Asset*>   ownItems{};
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

    template <typename T>
    class Subobjects
    {
    public:
        explicit Subobjects(Asset* owner) : owner(owner) {}

        void Add(T* asset)
        {
            objects.EmplaceBack(asset);
            asset->SetOwner(owner);
        }

        Array<T*> objects;
        Asset*    owner = nullptr;
    };


    template <>
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
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const T*& value)
        {

        }

        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, T*& value)
        {
            //value = AssetDatabase::FindById(UUID::RandomUUID());
        }
    };

    template <typename T>
    struct ArchiveType<Subobjects<T>>
    {
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const Subobjects<T>& value)
        {
            const TypeHandler* typeHandler = Registry::FindType<T>();
            ArchiveObject arr = writer.CreateArray();
            for (T* asset : value.objects)
            {
                writer.AddValue(arr, Serialization::Serialize(typeHandler, writer, asset));
            }
            writer.WriteValue(object, name, arr);
        }

        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, Subobjects<T>& value)
        {
            const TypeHandler* typeHandler = Registry::FindType<T>();

            ArchiveObject arr = reader.ReadObject(object, name);

            usize size = reader.ArrSize(arr);
            value.objects.Reserve(size);

            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);

                T* asset = AssetDatabase::Create<T>();
                Serialization::Deserialize(typeHandler, reader, item, asset);
                value.Add(asset);
            }
        }
    };
}
