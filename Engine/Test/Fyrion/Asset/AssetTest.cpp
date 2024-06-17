#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"

using namespace Fyrion;

namespace
{
    class TestAsset : public Asset
    {
    public:

    private:
    };


    TEST_CASE("Asset::Basic")
    {
        Engine::Init();
        {
            AssetDirectory* root = AssetDatabase::Instantiate<AssetDirectory>();

            AssetDirectory* anoterDir = AssetDatabase::Instantiate<AssetDirectory>();
            root->children.Add(anoterDir);

            TestAsset* testAsset = AssetDatabase::Instantiate<TestAsset>();
            root->children.Add(testAsset);

        }
        Engine::Destroy();
    }
}
