#pragma once

namespace Fyrion
{

    template<typename T>
    class ResourceTypeBuilder
    {
    public:

        template<auto Value, typename Type>
        ResourceTypeBuilder& Value(const StringView& name)
        {
            FY_ASSERT(!m_built, "Build() is already called");

            m_resourceFieldCreation.EmplaceBack(ResourceFieldCreation{
                .index = static_cast<u32>(Value),
                .name = name,
                .type = ResourceFieldType::Value,
                .valueId = GetTypeID<Type>()
            });
            return *this;
        }

        template<auto Value>
        ResourceTypeBuilder& SubObject(const StringView& name)
        {
            FY_ASSERT(!m_built, "Build() is already called");

            m_resourceFieldCreation.EmplaceBack(ResourceFieldCreation{
                .index = static_cast<u32>(Value),
                .name = name,
                .type = ResourceFieldType::SubObject,
            });
            return *this;
        }

        template<auto Value>
        ResourceTypeBuilder& SubObjectSet(const StringView& name)
        {
            FY_ASSERT(!m_built, "Build() is already called");

            m_resourceFieldCreation.EmplaceBack(ResourceFieldCreation{
                .index = static_cast<u32>(Value),
                .name = name,
                .type = ResourceFieldType::SubObjectSet,
            });
            return *this;
        }

        template<auto Value>
        ResourceTypeBuilder& Stream(const StringView& name)
        {
            FY_ASSERT(!m_built, "Build() is already called");
            m_resourceFieldCreation.EmplaceBack(ResourceFieldCreation{
                .index = static_cast<u32>(Value),
                .name = name,
                .type = ResourceFieldType::Stream,
            });
            return *this;
        }

        static ResourceTypeBuilder Builder()
        {
            return {};
        }

        void Build()
        {
            Repository::CreateResourceType(ResourceTypeCreation{
                .name = GetTypeName<T>(),
                .typeId = GetTypeID<T>(),
                .fields = m_resourceFieldCreation
            });
            m_built = true;
        }

        virtual ~ResourceTypeBuilder()
        {
            FY_ASSERT(m_built, "Build() is not called");
        }

    private:
        Array<ResourceFieldCreation> m_resourceFieldCreation{};
        bool m_built = false;
    };
}