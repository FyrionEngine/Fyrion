#pragma once

#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/String.hpp"

namespace Fyrion::Path
{
    inline String Parent(const StringView& path)
    {
        String parentPath{};
        bool foundSeparator = false;
        auto it = path.end();
        do
        {
            if (foundSeparator)
            {
                parentPath.Insert(parentPath.begin(), *it);
            }
            if (*it == FY_PATH_SEPARATOR)
            {
                foundSeparator = true;
            }
            if (it == path.begin())
            {
                return parentPath;
            }
            it--;
        } while (true);
    }

    StringView Extension(const StringView& path)
    {
        auto it = path.end();
        while (it != path.begin())
        {
            it--;
            if (*it == '.')
            {
                return StringView{it, (usize) (path.end() - it)};
            }
            else if (*it == FY_PATH_SEPARATOR)
            {
                break;
            }
        }
        return {};
    }

    template<typename ...T>
    inline String Join(const T& ... paths)
    {
        String retPath{};
        auto append = [&](StringView path)
        {
            char first = path[0];
            if (first != '.' && first != '/' && first != '\\' && !retPath.Empty())
            {
                char last = retPath[retPath.Size() - 1];
                if (last != '/' && last != '\\')
                {
                    retPath.Append(FY_PATH_SEPARATOR);
                }
            }

            retPath.Reserve(path.Size() + path.Size());

            for (int i = 0; i < path.Size(); ++i)
            {
                if (path[i] == '/' || path[i] == '\\')
                {
                    if (i < path.Size() - 1)
                    {
                        retPath.Append(FY_PATH_SEPARATOR);
                    }
                }
                else
                {
                    retPath.Append(path[i]);
                }
            }
        };
        (append(StringView{paths}), ...);
        return retPath;
    }

    inline String Name(const StringView& path)
    {
        bool hasExtension = !Extension(path).Empty();

        auto it = path.end();
        if (it == path.begin()) return {};
        it--;

        //if the last char is a separator
        //like /path/folder/
        if (*it == FY_PATH_SEPARATOR)
        {
            it--;
        }

        String name{};
        bool found{};

        if (!hasExtension)
        {
            found = true;
        }

        while (it != path.begin())
        {
            if (found && *it == FY_PATH_SEPARATOR)
            {
                return name;
            }
            if (found)
            {
                name.Insert(name.begin(), *it);
            }
            if (*it == '.' || *it == FY_PATH_SEPARATOR)
            {
                found = true;
            }
            it--;
        }
        return path;
    }
}

