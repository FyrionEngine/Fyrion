#pragma once
#include "../../../ThirdParty/freetype/src/gzip/ftzconf.h"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    struct AssetBuffer;
    struct AssetIO;
    class Asset;

    class AssetBufferManager
    {
    public:
        virtual ~AssetBufferManager() = default;

        virtual void      SaveBuffer(AssetBuffer& buffer, ConstPtr data, usize dataSize) = 0;
        virtual Array<u8> LoadBuffer(const AssetBuffer& buffer) const = 0;
        virtual bool      HasBuffer(AssetBuffer& buffer) const = 0;
    };

    class FY_API AssetHandler
    {
    public:
        virtual ~AssetHandler() = default;

        Asset*                      GetInstance() const;
        UUID                        GetUUID() const;
        void                        SetUUID(UUID uuid);
        TypeHandler*                GetType() const;
        void                        SetType(TypeHandler* typeHandler);
        StringView                  GetName() const;
        virtual void                SetName(StringView desiredNewName) = 0;
        void                        SetParent(AssetHandler* parent);
        StringView                  GetPath() const;
        virtual StringView          GetAbsolutePath() const = 0;
        virtual StringView          GetDataPath();
        StringView                  GetExtension() const;
        StringView                  GetDisplayName() const;
        AssetHandler*               GetParent() const;
        virtual void                UpdatePath();
        bool                        IsChildOf(AssetHandler* parent) const;
        Span<AssetHandler*>         GetChildren() const;
        void                        RemoveChild(AssetHandler* child);
        void                        RemoveFromParent();
        virtual void                AddChild(AssetHandler* child);
        AssetHandler*               FindChildByAbsolutePath(StringView absolutePath) const;
        virtual ArchiveObject       Serialize(ArchiveWriter& writer) const;
        virtual void                Deserialize(ArchiveReader& reader, ArchiveObject object);
        virtual bool                IsModified();
        virtual void                SetModified();
        virtual void                AddRelatedFile(StringView fileAbsolutePath);
        virtual void                Save() = 0;
        virtual void                Delete() = 0;
        virtual Asset*              LoadInstance();
        virtual void                UnloadInstance();
        virtual AssetHandler*       CreateChild(StringView name) = 0;
        virtual AssetBufferManager* GetBufferManager();
        virtual void                AssetMoved(StringView newName, AssetHandler* newParent) {}

        static StringView GetDisplayName(TypeHandler* type);

        static String ValidateName(AssetHandler* parent, AssetHandler* asset, StringView newName);

    protected:

        Asset*               instance{};
        AssetHandler*        prototype{};
        String               relativePath{};
        String               name{};
        AssetHandler*        parent{};
        Array<AssetHandler*> children{};
        bool                 active = true;

    private:
        UUID         uuid{};
        TypeHandler* type{};
    };

    class FY_API FileAssetBufferManager : public AssetBufferManager
    {
    public:
        explicit  FileAssetBufferManager(AssetHandler* assetHandler) : assetHandler(assetHandler) {}
        void      SaveBuffer(AssetBuffer& buffer, ConstPtr data, usize dataSize) override;
        Array<u8> LoadBuffer(const AssetBuffer& buffer) const override;
        bool      HasBuffer(AssetBuffer& buffer) const override;
    private:
        AssetHandler* assetHandler;
    };


    class FY_API DirectoryAssetHandler final : public AssetHandler
    {
    public:
        void       SetName(StringView desiredNewName) override;
        StringView GetAbsolutePath() const override;
        void       Save() override;
        void       Delete() override;
        Asset*     LoadInstance() override;
        void       AssetMoved(StringView newName, AssetHandler* newParent) override;

        AssetHandler* CreateChild(StringView name) override;

        static DirectoryAssetHandler* Create(const StringView&      name,
                                             const StringView&      absolutePath,
                                             DirectoryAssetHandler* parent);

    private:
        String absolutePath{};
    };


    class FY_API ChildAssetHandler final : public AssetHandler
    {
    public:
        void          SetName(StringView desiredNewName) override;
        StringView    GetAbsolutePath() const override;
        void          Save() override;
        void          Delete() override;
        AssetHandler* CreateChild(StringView name) override;
        void          UpdatePath() override;
        StringView    GetDataPath() override;
        Asset*        LoadInstance() override;

        ArchiveObject Serialize(ArchiveWriter& writer) const override;
        void          Deserialize(ArchiveReader& reader, ArchiveObject object) override;

        AssetBufferManager* GetBufferManager() override;

        static ChildAssetHandler* Create(StringView name, AssetHandler* parent);

    private:
        FileAssetBufferManager bufferManager{this};
        String                 dataPath{};
    };

    class FY_API JsonAssetHandler final : public AssetHandler
    {
    public:
        void                SetName(StringView desiredNewName) override;
        StringView          GetAbsolutePath() const override;
        bool                IsModified() override;
        void                SetModified() override;
        void                Save() override;
        void                Delete() override;
        Asset*              LoadInstance() override;
        StringView          GetDataPath() override;
        AssetHandler*       CreateChild(StringView name) override;
        AssetBufferManager* GetBufferManager() override;

        static JsonAssetHandler* Create(StringView name, DirectoryAssetHandler* directory);

    private:
        FileAssetBufferManager bufferManager{this};
        String                 infoPath{};
        String                 assetPath{};
        String                 dataPath{};
        u64                    currentVersion{};
        u64                    persistedVersion{};
    };

    class FY_API ImportedAssetHandler final : public AssetHandler
    {
    public:
        void                SetName(StringView desiredNewName) override;
        StringView          GetAbsolutePath() const override;
        void                AddRelatedFile(StringView fileAbsolutePath) override;
        void                Save() override;
        void                Delete() override;
        Asset*              LoadInstance() override;
        StringView          GetDataPath() override;
        AssetHandler*       CreateChild(StringView name) override;
        ArchiveObject       Serialize(ArchiveWriter& writer) const override;
        void                Deserialize(ArchiveReader& reader, ArchiveObject object) override;
        AssetBufferManager* GetBufferManager() override;

        static ImportedAssetHandler* Create(AssetIO* io, StringView importedFilePath, DirectoryAssetHandler* directory);

    private:
        FileAssetBufferManager bufferManager{this};
        AssetIO*               io{};
        String                 importedFilePath{};
        u64                    lastModifiedTime{};
        String                 infoPath{};
        String                 dataPath{};
        String                 assetPath{};
        Array<String>          relatedFiles{};
    };
}
