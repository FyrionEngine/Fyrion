#pragma once

#include "Fyrion/Common.hpp"
#include "Array.hpp"
#include "String.hpp"
#include "HashMap.hpp"
#include "HashSet.hpp"

namespace Fyrion
{
    class FY_API ArgParser
    {
    public:
        void Parse(i32 argc, char** argv)
        {
            if (argv == nullptr || argc <= 1) return;

            for (int i = 0; i < argc; ++i)
            {
                m_args.EmplaceBack(argv[i]);
            }


            usize i = 0;
            while(i < m_args.Size())
            {
                if (CheckArg(m_args[i]))
                {
                    if (i + 1 < m_args.Size() && !CheckArg(m_args[i + 1]))
                    {
                        m_namedArgsWithValue.Emplace(FormatArg(m_args[i]), String{m_args[i + 1]});
                        i++;
                    }
                    else
                    {
                        m_namedArgsWithoutValue.Insert(FormatArg(m_args[i]));
                    }
                }
                i++;
            }
        }

        StringView Get(const StringView& name)
        {
            if (auto it = m_namedArgsWithValue.Find(name))
            {
                return it->second;
            }
            return {};
        }

        bool Has(const StringView& name)
        {
            return m_namedArgsWithValue.Has(name) || m_namedArgsWithoutValue.Has(name);
        }

        StringView Get(usize i)
        {
            if (i < m_args.Size())
            {
                return m_args[i];
            }
            return {};
        }

    private:
        Array<String>           m_args{};
        HashMap<String, String> m_namedArgsWithValue{};
        HashSet<String>         m_namedArgsWithoutValue{};

        static String FormatArg(const StringView& arg)
        {
            return arg.Substr(arg.FindFirstNotOf("-"));
        }

        static bool CheckArg(const StringView& arg)
        {
            return arg.StartsWith("-") || arg.StartsWith("--");
        }
    };

}