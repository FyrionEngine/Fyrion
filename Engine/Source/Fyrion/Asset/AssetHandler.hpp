#pragma once
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class Asset;

    class FY_API AssetHandler
    {
    public:
        virtual ~AssetHandler() = default;

        Asset*              GetInstance() const;
        UUID                GetUUID() const;
        void                SetUUID(UUID uuid);
        TypeHandler*        GetType() const;
        StringView          GetName() const;
        virtual void        SetName(StringView desiredNewName) = 0;
        StringView          GetPath() const;
        virtual StringView  GetAbsolutePath() const = 0;
        StringView          GetExtension() const;
        StringView          GetDisplayName();
        AssetHandler*       GetParent() const;
        void                UpdatePath();
        bool                IsChildOf(AssetHandler* parent) const;
        Span<AssetHandler*> GetChildren() const;
        void                RemoveChild(AssetHandler* child);
        void                RemoveFromParent();
        void                AddChild(AssetHandler* child);
        AssetHandler*       FindChildByAbsolutePath(StringView absolutePath) const;
        ArchiveObject       Serialize(ArchiveWriter& writer) const;
        void                Deserialize(ArchiveReader& reader, ArchiveObject object);

        virtual bool   IsModified() const = 0;
        virtual void   SetModified() = 0;
        virtual void   AddRelatedFile(StringView fileAbsolutePath) = 0;
        virtual void   Save() = 0;
        virtual void   Delete() = 0;
        virtual Asset* LoadInstance() = 0;
        virtual void   UnloadInstance() = 0;

    protected:
        String ValidateName(StringView newName);

        Asset*               instance{};
        AssetHandler*        prototype{};
        UUID                 uuid{};
        String               relativePath{};
        String               name{};
        TypeHandler*         type{};
        u64                  lastModifiedTime{};
        AssetHandler*        parent{};
        Array<AssetHandler*> children{};
        bool                 active = true;
        String               displayName{};
    };


    class DirectoryAssetHandler final : public AssetHandler
    {
    public:
        void       SetName(StringView desiredNewName) override;
        StringView GetAbsolutePath() const override;
        bool       IsModified() const override;
        void       SetModified() override;
        void       AddRelatedFile(StringView fileAbsolutePath) override;
        void       Save() override;
        void       Delete() override;
        Asset*     LoadInstance() override;
        void       UnloadInstance() override;

    private:
        String absolutePath{};
    };

    class JsonAssetHandler final : public AssetHandler
    {
    public:
        void       SetName(StringView desiredNewName) override;
        StringView GetAbsolutePath() const override;
        bool       IsModified() const override;
        void       SetModified() override;
        void       AddRelatedFile(StringView fileAbsolutePath) override;
        void       Save() override;
        void       Delete() override;
        Asset*     LoadInstance() override;
        void       UnloadInstance() override;

    private:
        String infoPath{};
        String assetPath{};
        u64    currentVersion{};
        u64    persistedVersion{};
    };


    class ImportedAssetHandler final : public AssetHandler
    {
    public:
        void       SetName(StringView desiredNewName) override;
        StringView GetAbsolutePath() const override;
        bool       IsModified() const override;
        void       SetModified() override;
        void       AddRelatedFile(StringView fileAbsolutePath) override;
        void       Save() override;
        void       Delete() override;
        Asset*     LoadInstance() override;
        void       UnloadInstance() override;

    private:
        String        importedFilePath{};
        String        infoPath{};
        String        dataPath{};
        Array<String> relatedFiles{};
    };
}
