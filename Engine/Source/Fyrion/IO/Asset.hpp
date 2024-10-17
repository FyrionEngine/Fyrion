#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    struct AssetLoader;

    class FY_API Asset
    {
    public:
        UUID         GetUUID() const;
        TypeHandler* GetTypeHandler() const;
        Array<u8>    LoadStream(usize offset, usize size) const;
        void         SetTypeHandler(TypeHandler* typeHandler);

        friend class Assets;

    private:
        UUID         uuid;
        TypeHandler* typeHandler = nullptr;
        AssetLoader* loader = nullptr;
    };

    struct FY_API AssetLoader
    {
        virtual ~AssetLoader() = default;

        virtual Asset*    LoadAsset() = 0;
        virtual Array<u8> LoadStream(usize offset, usize size) = 0;
    };

    class FY_API Assets
    {
    public:
        static void   Create(UUID uuid, AssetLoader* loader);
        static Asset* Load(UUID uuid);

        template <typename T>
        static T* LoadByPath(StringView path)
        {
            return nullptr;
        }

        template <typename T>
        static T* Load(UUID uuid)
        {
            return static_cast<T*>(Load(uuid));
        }
    };
}
