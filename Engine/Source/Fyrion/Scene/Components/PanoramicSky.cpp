#include "PanoramicSky.hpp"

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/Scene/SceneObject.hpp"



namespace Fyrion
{
    void PanoramicSky::OnChange()
    {
        if (object->IsActivated())
        {
            if (sphericalTexture != nullptr)
            {
                //sphericalTexture->LoadBlob();
            }
        }
    }

    void PanoramicSky::SetSphericalTexture(TextureAsset* sphericalTexture)
    {
        this->sphericalTexture = sphericalTexture;
        OnChange();
    }

    TextureAsset* PanoramicSky::GetSphericalTexture() const
    {
        return sphericalTexture;
    }

    void PanoramicSky::RegisterType(NativeTypeHandler<PanoramicSky>& type)
    {
        type.Field<&PanoramicSky::sphericalTexture>("sphericalTexture").Attribute<UIProperty>();
    }
}
