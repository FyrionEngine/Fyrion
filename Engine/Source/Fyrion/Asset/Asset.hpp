#pragma once
#include "AssetDatabase.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Asset;
    class AssetDirectory;

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

        bool IsModified() const
        {
            if (!IsActive() && loadedVersion == 0)
            {
                return false;
            }
            if (!GetUUID())
            {
                return false;
            }
            return currentVersion != loadedVersion;
        }

        virtual StringView GetDisplayName() const;
        StringView GetInfoExtension() const;

        friend class AssetDatabase;

        template <typename Type>
        friend class Subobject;
        friend class AssetDirectory;

        static void RegisterType(NativeTypeHandler<Asset>& type);

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Asset, T>>* = nullptr>
        T* Cast()
        {
            return static_cast<T*>(this);
        }

    private:
        usize           index{};
        UUID            uuid{};
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
}
