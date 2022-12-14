#pragma once

#include <list>
#include <iostream>

namespace slow_lfuda_chc
{

template<typename page_t, typename key_t = int> class slow_lfuda_t
{
    std::size_t capacity;
    std::size_t size = 0;
    std::size_t age = 0;

    struct node_t 
    {
        key_t key;
        std::size_t weight;
        page_t page;
    };

    using node_it = typename std::list<node_t>::iterator;
    std::list<node_t> nodelist;
public:

    slow_lfuda_t (std::size_t capacity_) : capacity {capacity_} {};
    bool is_full () const { return size == capacity; }

    template<typename func_t> bool lookup_update (const key_t key, func_t slow_get_page)
    {
        auto is_hit = find_bykey (key);
        if (is_hit == nodelist.end())
        {
            //  push cache with slow get page
            push_cache (key, slow_get_page);
            return false;
        }
        else
        {
            //  update stage of cache
            cache_update (key, is_hit);
            return true;
        }
    }

    void cache_dump () const
    {
        if (nodelist.empty())
            return;
        std::cout << "AGE: " << age << "\n\t";
        for (auto node_i = nodelist.begin(); node_i != nodelist.end(); node_i = ++node_i)
            std::cout << "|KEY: " << node_i->key << "; WEIGHT: " << node_i->weight << "| -> ";
        std::cout << "NULL\n\n\n";
    }
private:
    node_it find_bykey (const key_t key)
    {   
        for (auto need_node = nodelist.begin(), end_node = nodelist.end(); 
                  need_node != end_node; need_node = ++need_node)
        {
            if (need_node->key == key)
                return need_node;
        }
        return nodelist.end();
    }

    node_it find_evict_node (std::size_t weight)
    {
        auto evict_node = nodelist.begin();
        for (;evict_node->weight == weight;)
        {
            if (std::next(evict_node) == nodelist.end())
                return evict_node;
            ++evict_node;
        }
        return std::prev(evict_node);
    }

    void evict_is_full (std::size_t weight)
    {
        //  evict the last node from sublist with minimum weights
        auto evict_node = find_evict_node (nodelist.begin()->weight);
        age = evict_node->weight;
        nodelist.erase (evict_node);
        size--;
    }

    node_it find_prepush (std::size_t weight)
    {
        if (nodelist.empty())
            return nodelist.end();

        auto prepush = nodelist.begin();
        while (prepush->weight < weight)
        {
            if (std::next(prepush) == nodelist.end())
                return nodelist.end();
            ++prepush;
        }
        if (prepush->weight >= weight)
            return prepush;
        return nodelist.end();
    }

    template<typename func_t> void push_cache (const key_t key, func_t slow_get_page)
    {   
        if (is_full())
            evict_is_full (nodelist.begin()->weight);
        
        auto prepush = find_prepush (age + 1);
        if (prepush == nodelist.end())
            nodelist.push_back ({key, age + 1, slow_get_page(key)});
        else 
            nodelist.insert (prepush, {key, age + 1, slow_get_page(key)});
        size++;
    }

    void cache_update (const key_t key, node_it promote_node)
    {
        auto prepush = find_prepush (promote_node->weight + age + 1);
        if (prepush == nodelist.end())
            nodelist.push_back({key, promote_node->weight + age + 1, promote_node->page});
        else 
            nodelist.insert (prepush, {key, promote_node->weight + age + 1, promote_node->page});

        nodelist.erase (promote_node);
    }
};

}   //  namespace of slow_cache
