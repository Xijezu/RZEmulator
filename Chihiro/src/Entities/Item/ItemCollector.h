/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NGEMITY_ITEMCOLLECTOR_H
#define NGEMITY_ITEMCOLLECTOR_H

#include "Common.h"
#include "SharedMutex.h"

class Item;
class ItemCollector
{
    public:
        static ItemCollector &Instance()
        {
            static ItemCollector instance;
            return instance;
        }
        ~ItemCollector();
        void RegisterItem(Item *pItem);
        bool UnregisterItem(Item *pItem);
        void Update();

    private:
        typedef std::unordered_map<uint, Item *> ItemMap;

        ItemMap m_vItemList;
        NG_SHARED_MUTEX i_lock;

    protected:
        ItemCollector() = default;
};
#define sItemCollector ItemCollector::Instance()
#endif // NGEMITY_ITEMCOLLECTOR_H
