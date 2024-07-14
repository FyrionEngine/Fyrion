#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Core/Image.hpp"

namespace Fyrion
{
    enum class ShaderAssetType
    {
        None,
        Graphics,
        Compute,
        Raytrace
    };

    struct FY_API ShaderIO : public AssetIO
    {
        StringView extension[2] = {".raster", ".comp"};

        FY_BASE_TYPES(AssetIO);

        Span<StringView> GetImportExtensions() override;
        Asset*           CreateAsset() override;
        void             ImportAsset(StringView path, Asset* asset) override;
    };

    class FY_API ShaderAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        StringView GetDisplayName() const override;

        StringView GetShaderSource() const
        {
            return shaderSource;
        }

        ShaderAssetType GetShaderType() const
        {
            return shaderType;
        }

        void SetShaderSource(const String& shaderSource)
        {
            this->shaderSource = shaderSource;
        }

        void SetShaderType(ShaderAssetType shaderType)
        {
            this->shaderType = shaderType;
        }

        const ShaderInfo& GetShaderInfo() const
        {
            return shaderInfo;
        }

        Span<ShaderStageInfo> GetStages() const;
        Span<u8>              GetBytes() const;

        void Compile();

        static void RegisterType(NativeTypeHandler<ShaderAsset>& type);

    private:
        String                 shaderSource;
        ShaderAssetType        shaderType = ShaderAssetType::None;
        Array<u8>              bytes{};
        Array<ShaderStageInfo> stages{};
        ShaderInfo             shaderInfo{};
    };


    struct FY_API TextureIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        StringView extension[5] = {".png", ".jpg", ".jpeg", ".tga", "bmp"};

        Span<StringView> GetImportExtensions() override;
        Asset*           CreateAsset() override;
        void             ImportAsset(StringView path, Asset* asset) override;
    };


    class FY_API TextureAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        ~TextureAsset() override;

        StringView GetDisplayName() const override;
        void       SetImage(StringView path);

        Texture GetTexture();

        static void RegisterType(NativeTypeHandler<TextureAsset>& type);

    private:
        u32  width = 0;
        u32  height = 0;
        u32  channels = 0;
        Blob data{};

        Texture texture{};
    };

    class FY_API MeshAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<MeshAsset>& type);

        u64 value = 0;
    private:
    };

    class FY_API DCCAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<DCCAsset>& type);

        void AddMesh(MeshAsset* mesh);
    private:
        Array<MeshAsset*> meshes{};
    };

    struct FY_API GLTFIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        StringView extension[2] = {".gltf", ".glb"};
        Span<StringView> GetImportExtensions() override;

        Asset*           CreateAsset() override;
        void ImportAsset(StringView path, Asset* asset) override;
    };
}
