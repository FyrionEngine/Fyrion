#pragma once

#include "HashSet.hpp"
#include "Array.hpp"
#include "HashMap.hpp"

namespace Fyrion
{
    template <typename Key, typename Node>
    class Graph
    {
    private:
        void DFS(Array<Node>&  sorted,
                 HashSet<Key>& visitedNodes,
                 HashSet<Key>& onStack,
                 const Key&    key)
        {
            visitedNodes.Insert(key);
            onStack.Insert(key);

            for (const auto& neighbour : m_adj[key])
            {
                if (visitedNodes.Find(neighbour.first) == visitedNodes.end())
                {
                    DFS(sorted, visitedNodes, onStack, neighbour.first);
                }
            }

            onStack.Erase(key);

            auto it = m_nodes.Find(key);
            if (it != m_nodes.end())
            {
                sorted.EmplaceBack(it->second);
            }
        }

    public:
        Node& AddNode(const Key& key, const Node& node)
        {
            auto it = m_nodes.Find(key);
            if (it == m_nodes.end())
            {
                it = m_nodes.Insert({key, node}).first;
            }
            return it->second;
        }

        void RemoveNode(const Key& key)
        {
            m_nodes.Erase(key);
        }

        void AddEdge(const Key& left, const Key& right)
        {
            m_adj[left].Insert(right);
            m_reverseAdj[right].Insert(left);
        }

        Array<Node> Sort()
        {
            Array<Node>  sorted{};
            HashSet<Key> visitedNodes{};
            HashSet<Key> onStack{};

            for (auto& it : m_nodes)
            {
                if (visitedNodes.Find(it.first) == visitedNodes.end())
                {
                    DFS(sorted, visitedNodes, onStack, it.first);
                }
            }
            return sorted;
        }

        Node GetNode(const Key key)
        {
            return m_nodes[key];
        }

        HashMap<Key, Node>& Nodes()
        {
            return m_nodes;
        }

        HashSet<Key>& Edges(const Key& key)
        {
            return m_adj[key];
        }

        HashSet<Key>& ReverseEdges(const Key& key)
        {
            return m_reverseAdj[key];
        }

        bool HasNode(const Key& key)
        {
            return m_nodes.has(key);
        }

    private:
        HashMap<Key, Node>         m_nodes{};
        HashMap<Key, HashSet<Key>> m_adj{};
        HashMap<Key, HashSet<Key>> m_reverseAdj{};
    };
}
