/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

namespace TagLib {

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////
/**
 * 这段代码是一个模板类Map的私有实现MapPrivate的定义。MapPrivate是作为Map
 * 类的私有成员存在的，它使用了模板参数Key和T，这两个参数与Map类的模板参数相同。
 *
 * 在这个定义中，MapPrivate类还有一个模板参数列表，其中包含KeyP和TP两个模板参数。
 * 这两个参数是用来定义MapPrivate类自己的模板参数的。所以，MapPrivate类的完整模板
 * 参数列表是<Key, T, KeyP, TP>。
 *
 * 在类的构造函数中，它接受std::map或初始化列表作为参数，并使用它们来初始化内部的map成员。
 * 这个map成员是一个std::map对象，它的键类型是KeyP，值类型是TP。
 *
 * 在代码的末尾，有一个条件编译的部分，根据WANT_CLASS_INSTANTIATION_OF_MAP宏的定义来选择
 * 不同的初始化方式。这个宏似乎用于控制是否实例化Map类的对象。如果定义了这个宏，则会使用
 * class KeyP和class TP作为键和值的类型；否则，直接使用KeyP和TP。
*/
template <class Key, class T>
template <class KeyP, class TP>
class Map<Key, T>::MapPrivate
{
public:
  MapPrivate() = default;
#ifdef WANT_CLASS_INSTANTIATION_OF_MAP
  MapPrivate(const std::map<class KeyP, class TP>& m) : map(m) {}
  MapPrivate(std::initializer_list<std::pair<const class KeyP, class TP>> init) : map(init) {}

  std::map<class KeyP, class TP> map;
#else
  MapPrivate(const std::map<KeyP, TP>& m) : map(m) {}
  MapPrivate(std::initializer_list<std::pair<const KeyP, TP>> init) : map(init) {}

  std::map<KeyP, TP> map;
#endif
};

/**
 * 这是Map类的构造函数的实现。在这个构造函数中，使用了成员初始化列表来初始化Map类的
 * 私有成员变量d。
 *
 * d是一个名为d的std::shared_ptr指针，它指向MapPrivate<Key, T>类型的对象。std::make_shared函数
 * 用于创建一个指向MapPrivate<Key, T>类型的新对象，并返回一个std::shared_ptr指针指向这个新对象。
 *
 * 在构造函数中，d被初始化为一个指向新创建的MapPrivate<Key, T>对象的std::shared_ptr指针。这样，
 * 每当创建一个Map对象时，都会创建一个新的MapPrivate<Key, T>对象，并由d管理它的生命周期。这种方式
 * 确保了MapPrivate对象在不再需要时会被正确地释放，避免了内存泄漏和空悬指针等问题。
*/
template <class Key, class T>
Map<Key, T>::Map() :
  d(std::make_shared<MapPrivate<Key, T>>())
{
}

/**
 * 这段代码是Map类的拷贝构造函数的默认实现。使用= default关键字告诉编译器使用默认的拷贝构造
 * 函数实现，而不是显式地提供自定义的实现。
 *
 * 拷贝构造函数用于创建一个对象的副本，即在已有对象的基础上创建一个新对象，新对象的内容与原对象相同。
 * 在这里，= default告诉编译器使用编译器自动生成的拷贝构造函数实现，这个默认实现会逐个拷贝原对象的成员变量
 * 到新对象中。
 *
 * 如果你不显式地提供拷贝构造函数的实现，并且类中没有其他特殊的数据成员需要特殊处理，那么编译器会自动生成
 * 一个适合的默认实现。
*/
template <class Key, class T>
Map<Key, T>::Map(const Map<Key, T> &) = default;

/**
 * 这段代码是Map类的构造函数的实现，它接受一个std::initializer_list参数作为输入，并使用
 * 其中的元素初始化Map对象。
 *
 * 在构造函数中，首先调用std::make_shared函数创建了一个指向MapPrivate<Key, T>类型的对象的std::shared_ptr
 * 指针。MapPrivate<Key, T>是Map类的私有实现，它用于存储Map对象的数据。
 *
 * 然后，将std::initializer_list参数传递给MapPrivate对象的构造函数，以便初始化内部的std::map成员。
 * std::initializer_list是一个标准库提供的容器，它允许以列表初始化的方式初始化对象，其中的元素是一对键值对。
 * 在这个例子中，init参数中的元素被用来初始化MapPrivate对象的内部std::map成员。
 *
 * 最后，使用d成员变量将新创建的MapPrivate对象存储在Map对象中，从而完成了Map对象的初始化。
*/
template <class Key, class T>
Map<Key, T>::Map(std::initializer_list<std::pair<const Key, T>> init) :
  d(std::make_shared<MapPrivate<Key, T>>(init))
{
}

/**
 * 这段代码是Map类的析构函数的默认实现。使用= default关键字告诉编译器使用默认的析构函数实现，
 * 而不是显式地提供自定义的实现。
 *
 * 析构函数用于在对象生命周期结束时执行清理工作，并释放对象所占用的资源。在这里，= default告诉编译器使用
 * 编译器自动生成的析构函数实现，这个默认实现会逐个销毁对象的成员变量，释放它们占用的资源。
 *
 * 如果你不显式地提供析构函数的实现，并且类中没有其他特殊的资源管理需求，那么编译器会自动生成一个适合的默认实现
*/
template <class Key, class T>
Map<Key, T>::~Map() = default;

/**
 * 这段代码是Map类的成员函数begin()的实现。begin()函数用于返回指向Map对象中第一个元素的迭代器。
 *
 * 在函数中，首先调用了detach()函数，它的作用可能是确保Map对象的私有实现处于可修改状态。接着，调用了MapPrivate对象
 * 的map成员的begin()函数，返回了指向第一个元素的迭代器。
 *
 * 根据代码中的语境，Iterator是MapPrivate<Key, T>类中的迭代器类型，它可能是std::map<Key, T>::iterator或类似的类型。
 * 因此，typename Map<Key, T>::Iterator是指向Map对象中元素的迭代器类型的别名。
 *
 * 总的来说，这段代码的作用是返回一个指向Map对象中第一个元素的迭代器，并且可能会在返回迭代器之前执行一些必要的操作，
 * 比如确保私有实现处于可修改状态。
*/
template <class Key, class T>
typename Map<Key, T>::Iterator Map<Key, T>::begin()
{
  detach();
  return d->map.begin();
}

/**
 * 这段代码实现了Map类的begin()函数，用于返回指向Map对象中第一个元素的常量迭代器。
 * 在这段代码中：
 * Map<Key, T>::ConstIterator是Map类中的常量迭代器类型的别名。
 * begin()函数返回的是一个常量迭代器，因为它是const成员函数，不能修改对象的数据。
 * d是指向MapPrivate<Key, T>对象的智能指针，它的map成员是一个std::map<Key, T>对象，存储了Map对象的数据。
 * d->map.begin()调用了std::map<Key, T>对象的begin()函数，返回指向第一个元素的常量迭代器。
 * 这样，begin()函数就可以返回一个指向Map对象中第一个元素的常量迭代器，从而允许用户遍历Map对象的元素。
*/
template <class Key, class T>
typename Map<Key, T>::ConstIterator Map<Key, T>::begin() const
{
  return d->map.begin();
}

/**
 * 这段代码实现了Map类的cbegin()函数，用于返回指向Map对象中第一个元素的常量迭代器。
 * 在这段代码中：
 * Map<Key, T>::ConstIterator是Map类中的常量迭代器类型的别名。
 * cbegin()函数返回的是一个常量迭代器，因为它是const成员函数，不能修改对象的数据。
 * d是指向MapPrivate<Key, T>对象的智能指针，它的map成员是一个std::map<Key, T>对象，存储了Map对象的数据。
 * d->map.cbegin()调用了std::map<Key, T>对象的cbegin()函数，返回指向第一个元素的常量迭代器。
 * 这样，cbegin()函数就可以返回一个指向Map对象中第一个元素的常量迭代器，从而允许用户以只读方式遍历Map对象的元素。
*/
template <class Key, class T>
typename Map<Key, T>::ConstIterator Map<Key, T>::cbegin() const
{
  return d->map.cbegin();
}

/**
 * 这段代码是Map类的成员函数end()的实现。end()函数用于返回指向Map对象中最后一个元素之后位置的迭代器。
 *
 * 在函数中，首先调用了detach()函数，可能是用于确保Map对象的私有实现处于可修改状态。接着，访问了Map对象的私有
 * 实现中的map成员，这个map成员是一个std::map对象，其中存储了Map对象的数据。然后，调用了std::map对象的end()函数，
 * 返回指向最后一个元素之后位置的迭代器。
 *
 * 这样，end()函数就可以返回一个指向Map对象中最后一个元素之后位置的迭代器。
*/
template <class Key, class T>
typename Map<Key, T>::Iterator Map<Key, T>::end()
{
  detach(); // 可能是确保私有实现处于可修改状态的操作
  return d->map.end(); // 返回指向最后一个元素之后位置的迭代器
}

/**
 * 这段代码是Map类的成员函数end()的实现。end()函数用于返回指向Map对象中最后一个元素之后位置的常量迭代器。
 *
 * 在函数中，首先访问了Map对象的私有实现中的map成员，这个map成员是一个std::map对象，其中存储了Map对象的数据。然后，
 * 调用了std::map对象的end()函数，返回指向最后一个元素之后位置的常量迭代器。
 *
 * 根据代码中的语境，ConstIterator是MapPrivate<Key, T>类中的常量迭代器类型，它可能是std::map<Key, T>::const_iterator
 * 或类似的类型。因此，typename Map<Key, T>::ConstIterator是指向Map对象中元素的常量迭代器类型的别名。
 *
 * 总的来说，这段代码的作用是返回一个指向Map对象中最后一个元素之后位置的常量迭代器。
*/
template <class Key, class T>
typename Map<Key, T>::ConstIterator Map<Key, T>::end() const
{
  return d->map.end();
}

/**
 * 这段代码实现了Map类的cend()函数，用于返回指向Map对象中最后一个元素之后位置的常量迭代器。
 *
 * 在这段代码中：
 * Map<Key, T>::ConstIterator是Map类中的常量迭代器类型的别名。
 * cend()函数返回的是一个常量迭代器，因为它是const成员函数，不能修改对象的数据。
 * d是指向MapPrivate<Key, T>对象的智能指针，它的map成员是一个std::map<Key, T>对象，存储了Map对象的数据。
 * d->map.cend()调用了std::map<Key, T>对象的cend()函数，返回指向最后一个元素之后位置的常量迭代器。
 *
 * 这样，cend()函数就可以返回一个指向Map对象中最后一个元素之后位置的常量迭代器，从而允许用户以只读方式遍历Map对象的元素。
*/
template <class Key, class T>
typename Map<Key, T>::ConstIterator Map<Key, T>::cend() const
{
  return d->map.cend();
}

/**
 * 这段代码实现了Map类的insert()函数，用于向Map对象中插入一个新的键值对，并返回对当前对象的引用，
 * 以便支持方法链式调用。
 *
 * 在函数中，首先调用了detach()函数，可能是用于确保Map对象的私有实现处于可修改状态。然后，
 * 通过d->map[key] = value语句向std::map对象中插入了一个新的键值对，其中key是键，value是对应的值。
 * 最后，通过return *this返回对当前对象的引用，以便支持方法链式调用。
 *
 * 这样，insert()函数就可以向Map对象中插入新的键值对，并支持链式调用，使得代码更加简洁和易读。
*/
template <class Key, class T>
Map<Key, T> &Map<Key, T>::insert(const Key &key, const T &value)
{
  detach(); // 可能是确保私有实现处于可修改状态的操作
  d->map[key] = value; // 向map中插入键值对
  return *this; // 返回对当前对象的引用，支持方法链式调用
}

/**
 * 这段代码实现了 Map 类的 clear() 函数，用于清空 Map 对象中的所有元素，并返回对当前对象的引用，
 * 以支持方法链式调用。
 *
 * 在函数中，首先调用了 detach() 函数，可能是用于确保 Map 对象的私有实现处于可修改状态。然后，
 * 通过 d->map.clear() 调用了 std::map 对象的 clear() 函数，清空了 Map 对象中的所有元素。最后，
 * 通过 return *this 返回对当前对象的引用，以支持方法链式调用。
 *
 * 这样，clear() 函数就可以清空 Map 对象中的所有元素，并支持链式调用，使得代码更加简洁和易读。
*/
template <class Key, class T>
Map<Key, T> &Map<Key, T>::clear()
{
  detach(); // 可能是确保私有实现处于可修改状态的操作
  d->map.clear(); // 清空 Map 对象中的所有元素
  return *this; // 返回对当前对象的引用，支持方法链式调用
}

/**
 * 这段代码实现了 Map 类的 isEmpty() 函数，用于判断 Map 对象是否为空。
 *
 * 在函数中：
 *
 * d 是指向 MapPrivate<Key, T> 对象的智能指针，它的 map 成员是一个 std::map<Key, T> 对象，
 * 存储了 Map 对象的数据。
 *
 * d->map.empty() 调用了 std::map 对象的 empty() 函数，用于判断 Map 对象是否为空。如果 Map 对象中没有元素，
 * 则返回 true，否则返回 false。
 *
 * 这样，isEmpty() 函数就可以判断 Map 对象是否为空，并返回相应的布尔值。
*/
template <class Key, class T>
bool Map<Key, T>::isEmpty() const
{
  return d->map.empty();
}

/**
 * 这段代码实现了 Map 类的 find() 函数，用于在 Map 对象中查找指定键的元素，并返回指向该元素的迭代器。
 *
 * 在函数中：
 *
 * detach() 可能是用于确保 Map 对象的私有实现处于可修改状态的操作。
 * d 是指向 MapPrivate<Key, T> 对象的智能指针，它的 map 成员是一个 std::map<Key, T> 对象，存储了 Map 对象的数据。
 * d->map.find(key) 调用了 std::map 对象的 find() 函数，用于在 Map 对象中查找指定键的元素，并返回指向该元素的迭代器。
 * 这样，find() 函数就可以在 Map 对象中查找指定键的元素，并返回指向该元素的迭代器。
*/
template <class Key, class T>
typename Map<Key, T>::Iterator Map<Key, T>::find(const Key &key)
{
  detach(); // 可能是确保私有实现处于可修改状态的操作
  return d->map.find(key); // 调用 std::map 的 find() 函数查找指定键的元素，并返回迭代器
}

/**
 * 这段代码实现了 Map 类的 find() 函数，用于在 Map 对象中查找指定键的元素，并返回指向该元素的常量迭代器。
 *
 * 在函数中：
 *
 * d 是指向 MapPrivate<Key, T> 对象的智能指针，它的 map 成员是一个 std::map<Key, T> 对象，存储了 Map 对象的数据。
 * d->map.find(key) 调用了 std::map 对象的 find() 函数，用于在 Map 对象中查找指定键的元素，并返回指向该元素的常量迭代器。
 *
 * 这样，find() 函数就可以在 Map 对象中查找指定键的元素，并返回指向该元素的常量迭代器。
*/
template <class Key, class T>
typename Map<Key,T>::ConstIterator Map<Key, T>::find(const Key &key) const
{
  return d->map.find(key); // 调用 std::map 的 find() 函数查找指定键的元素，并返回常量迭代器
}

/**
 * 这段代码实现了 Map 类的 contains() 函数，用于判断 Map 对象中是否包含指定键的元素。
 *
 * 在函数中：
 *
 * d 是指向 MapPrivate<Key, T> 对象的智能指针，它的 map 成员是一个 std::map<Key, T> 对象，
 * 存储了 Map 对象的数据。
 *
 * d->map.find(key) 调用了 std::map 对象的 find() 函数，用于在 Map 对象中查找指定键的元素。
 * 如果找到了指定键的元素，则返回指向该元素的迭代器；否则返回指向末尾位置的迭代器（end()）。
 *
 * 判断 find() 返回的迭代器是否等于 d->map.end()，如果不等于，则说明找到了指定键的元素，返回 true；
 * 如果等于，则说明没有找到指定键的元素，返回 false。
 *
 * 这样，contains() 函数就可以判断 Map 对象中是否包含指定键的元素，并返回相应的布尔值。
*/
template <class Key, class T>
bool Map<Key, T>::contains(const Key &key) const
{
  return d->map.find(key) != d->map.end(); // 如果找到了指定键的元素，则返回 true；否则返回 false
}

/**
 * 这段代码实现了 Map 类的 erase() 函数，用于从map中删除由提供的迭代器指向的元素，
 * 并返回对当前对象的引用，以支持方法链式调用。
 *
 * 在函数中：
 * 参数 Iterator it 表示指向需要删除的map元素的迭代器。
 * d->map.erase(it) 调用了 std::map 对象的 erase() 函数，用于从map中删除由迭代器指向的元素。
 * return *this; 返回对当前对象的引用，以支持方法链式调用。
 *
 * 这样，erase() 函数就可以从map中删除由提供的迭代器指向的元素，并返回对当前对象的引用，以支持链式调用。
*/
template <class Key, class T>
Map<Key, T> &Map<Key,T>::erase(Iterator it)
{
  d->map.erase(it); // 从map中删除由迭代器指向的元素
  return *this; // 返回对当前对象的引用，支持方法链式调用
}

/**
 * 这段代码实现了 Map 类的 erase() 函数，用于从map中删除指定键对应的元素，
 * 并返回对当前对象的引用，以支持方法链式调用。
 *
 * 在函数中：
 *
 * 参数 const Key &key 表示要删除的元素的键。
 * detach() 可能是用于确保 Map 对象的私有实现处于可修改状态的操作。
 * d->map.erase(key) 调用了 std::map 对象的 erase() 函数，用于从map中删除指定键对应的元素。
 * return *this; 返回对当前对象的引用，以支持方法链式调用。
 *
 * 这样，erase() 函数就可以从map中删除指定键对应的元素，并返回对当前对象的引用，以支持链式调用。
*/
template <class Key, class T>
Map<Key, T> &Map<Key,T>::erase(const Key &key)
{
  detach(); // 可能是确保私有实现处于可修改状态的操作
  d->map.erase(key); // 从map中删除指定键对应的元素
  return *this; // 返回对当前对象的引用，支持方法链式调用
}

/**
 * 这段代码实现了 Map 类的 size() 函数，用于返回 Map 对象中的元素数量。
 *
 * 在函数中：
 *
 * d 是指向 MapPrivate<Key, T> 对象的智能指针，它的 map 成员是一个 std::map<Key, T> 对象，存储了 Map 对象的数据。
 * d->map.size() 调用了 std::map 对象的 size() 函数，返回 Map 对象中的元素数量。
 * 使用 static_cast<unsigned int> 将返回的 size_t 类型的元素数量转换为 unsigned int 类型，以符合函数声明的返回类型。
 *
 * 这样，size() 函数就可以返回 Map 对象中的元素数量。
*/
template <class Key, class T>
unsigned int Map<Key, T>::size() const
{
  return static_cast<unsigned int>(d->map.size());
}

/**
 * 这段代码实现了 Map 类的 value() 函数，用于获取指定键对应的值。如果键存在于map中，
 * 则返回与该键关联的值；如果键不存在，则返回默认值。
 *
 * 在函数中：
 * 参数 const Key &key 表示要获取值的键。
 * 参数 const T &defaultValue 表示默认值，用于在指定键不存在时返回。
 * auto it = d->map.find(key); 在map中查找指定键，并将结果存储在迭代器 it 中。
 * it != d->map.end() 检查迭代器是否指向map的末尾位置。如果不是，则说明找到了指定键，返回与该键关联的值；
 * 否则返回默认值 defaultValue。
 *
 * 这样，value() 函数就可以根据指定的键获取map中对应的值，如果键不存在，则返回默认值。
*/
template <class Key, class T>
T Map<Key, T>::value(const Key &key, const T &defaultValue) const
{
  auto it = d->map.find(key); // 在map中查找指定键
  return it != d->map.end() ? it->second : defaultValue; // 如果找到了指定键，则返回与该键关联的值；否则返回默认值
}

/**
 * 这段代码实现了 Map 类的 operator[] 运算符的 const 版本，用于访问map中给定键的值。
 * 如果map中不存在给定的键，则此操作将会引发一个错误。
 *
 * 在函数中：
 *
 * 参数 const Key &key 表示要访问的键。
 * d->map[key] 用于访问map中给定键的值。如果键不存在于map中，则会自动插入一个默认构造的值并返回其引用。
 * 返回值类型为 const T &，表示返回map中给定键的值的常量引用，以确保其只读性。
 * 这样，通过此 operator[] 运算符，可以以常量引用的方式访问map中给定键的值，但是如果键不存在于map中，则会自动插入一个默认构造的值。
 *
 * 默认构造的值取决于模板参数 T 的类型。对于内置类型和某些标准库容器类型，它们的默认构造值是零值，
 * 即对数值类型为0，对于指针类型为 nullptr，对于 bool 类型为 false，对于指定了默认构造函数的标准库容器
 * 类型如 std::vector、std::string 等，则是调用它们的默认构造函数创建的对象。
*/
template <class Key, class T>
const T &Map<Key, T>::operator[](const Key &key) const
{
  return d->map[key];
}

/**
 * 这段代码实现了 Map 类的 operator[] 运算符的非 const 版本，用于访问map中给定键的值。
 * 如果map中不存在给定的键，则此操作将会创建一个新的键值对，并返回对应值的引用。
 *
 * 在函数中：
 * 参数 const Key &key 表示要访问的键。
 * detach() 可能是用于确保 Map 对象的私有实现处于可修改状态的操作。
 * d->map[key] 用于访问map中给定键的值。如果键不存在于map中，则会自动插入一个默认构造的值并返回其引用。
 *
 * 这样，通过此 operator[] 运算符，可以以非常量引用的方式访问map中给定键的值，如果键不存在于map中，则会自动插入一个默认构造的值。
*/
template <class Key, class T>
T &Map<Key, T>::operator[](const Key &key)
{
  detach(); // 可能是确保私有实现处于可修改状态的操作
  return d->map[key]; // 返回map中给定键的值的引用
}

/**
 * 这段代码实现了 Map 类的赋值运算符重载，使用了默认实现。
 *
 * 这里使用了 = default，表示使用编译器自动生成的默认实现。在这种情况下，赋值运算符的行为与拷贝构造函数的
 * 行为相同：执行浅拷贝，将成员变量按位复制到目标对象中。
 *
 * 因此，这段代码意味着 Map 类的赋值运算符会使用编译器自动生成的默认实现，而不是显式地定义它的行为。
*/
template <class Key, class T>
Map<Key, T> &Map<Key, T>::operator=(const Map<Key, T> &) = default;

/**
 * 这段代码实现了 Map 类的赋值运算符重载，允许使用初始化器列表来为map赋值。
 *
 * 在函数中：
 * 参数 std::initializer_list<std::pair<const Key, T>> init 表示要使用的初始化器列表。
 * Map(init) 使用初始化器列表 init 构造了一个临时的 Map 对象。
 * swap(*this) 将临时的 Map 对象与当前对象进行了内容交换。这种做法避免了不必要的复制操作，提高了效率。
 * 最后，return *this; 返回对当前对象的引用，以支持链式赋值操作。
 *
 * 这样，通过这个重载的赋值运算符，可以使用初始化器列表来为map赋值，使得代码更加简洁和易读。
 *
 * 初始化器列表使用实例如下：
 *  #include <iostream>
    #include <map>

    // 假设你的 Map 类在这里定义

    int main() {
        // 创建一个空的 Map 对象
        Map<int, std::string> myMap;

        // 使用初始化器列表进行赋值
        myMap = {
            {1, "One"},
            {2, "Two"},
            {3, "Three"}
        };

        // 输出map中的内容
        for (const auto &pair : myMap) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }

        return 0;
    }
  * 在这个例子中，我们首先创建了一个空的 Map 对象 myMap，然后使用赋值运算符重载
  * 和初始化器列表为map赋值。最后，我们遍历map中的内容，并输出每个键值对。
*/
template <class Key, class T>
Map<Key, T> &Map<Key, T>::operator=(std::initializer_list<std::pair<const Key, T>> init)
{
  Map(init).swap(*this); // 使用 initializer_list 构造临时 Map 对象，并交换其内容与当前对象
  return *this; // 返回对当前对象的引用
}

/**
 * 这段代码实现了 Map 类的 swap() 函数，用于交换两个map对象的内容。
 *
 * 在函数中：
 *
 * 参数 Map<Key, T> &m 表示要交换内容的另一个map对象。
 * using std::swap; 使用 using 声明引入了 std 命名空间中的 swap 函数。
 * swap(d, m.d); 使用 std::swap 函数交换了当前对象的私有实现 d 与另一个map对象 m 的私有实现。
 *
 * 这样，通过调用这个 swap() 函数，可以在常量时间内交换两个map对象的内容，而不需要进行深度拷贝，提高了效率。
 *
 * noexcept 是 C++11 中引入的关键字，用于指示函数是否可能抛出异常。它被用作函数声明或函数定义的一部分，并指示函数是否会抛出异常。
 *
 * 具体来说，noexcept 关键字有两种使用方式：
 *
 * 1. 函数声明中的 noexcept：
 * 在函数声明中，noexcept 用于指示函数是否会抛出异常。如果函数声明中带有 noexcept 关键字，
 * 则意味着函数不会抛出任何异常。这可以帮助编译器进行优化，并且在函数调用过程中，如果异常确实发生，
 * 程序会立即终止（std::terminate 被调用）。
 * 例如：
 *     void foo() noexcept;
 *
 * 2. 函数定义中的 noexcept：
 * 在函数定义中，noexcept 用于指示函数是否会抛出异常，并且还可以指定表达式，用于动态决定函数是否可能抛出异常。
 * 如果函数定义中带有 noexcept 关键字，表示函数不会抛出异常。如果函数定义中带有 noexcept(expression)，
 * 则表示函数是否抛出异常取决于 expression 表达式的结果。如果 expression 的结果为 true，则函数被指示为不会抛出异常；
 * 如果 expression 的结果为 false，则函数被指示为可能会抛出异常。
 * 例如：
 *     void foo() noexcept {}
 *     void bar() noexcept(true) {}
 *     void baz() noexcept(1 > 0) {}
 * 在函数声明和定义中使用 noexcept 关键字可以帮助代码的阅读者和编译器了解函数的异常保证，并且在某些情况下可以
 * 帮助编译器进行更好的优化。
*/
template <class Key, class T>
void Map<Key, T>::swap(Map<Key, T> &m) noexcept
{
  using std::swap; // 引入 std 命名空间中的 swap 函数

  swap(d, m.d); // 使用 std::swap 函数交换两个map对象的私有实现
}

/**
 * 这段代码实现了 Map 类的 operator== 运算符重载，用于比较两个map对象是否相等。
 *
 * 在函数中：
 *
 * 参数 const Map<Key, T> &m 表示要与当前map对象进行比较的另一个map对象。
 * d->map == m.d->map 比较了两个map对象的私有实现是否相等。如果两个map对象的私有实现相等，则返回 true，否则返回 false。
 *
 * 这样，通过这个重载的 operator== 运算符，可以直接比较两个map对象是否相等，从而判断它们的内容是否相同。
*/
template <class Key, class T>
bool Map<Key, T>::operator==(const Map<Key, T> &m) const
{
  return d->map == m.d->map; // 比较两个map对象的私有实现是否相等
}

/**
 * 这段代码实现了 Map 类的 operator!= 运算符重载，用于比较两个map对象是否不相等。
 *
 * 在函数中：
 *
 * 参数 const Map<Key, T> &m 表示要与当前map对象进行比较的另一个map对象。
 * d->map != m.d->map 比较了两个map对象的私有实现是否不相等。如果两个map对象的私有实现不相等，则返回 true，否则返回 false。
 *
 * 这样，通过这个重载的 operator!= 运算符，可以直接比较两个map对象是否不相等，从而判断它们的内容是否不同。
*/
template <class Key, class T>
bool Map<Key, T>::operator!=(const Map<Key, T> &m) const
{
  return d->map != m.d->map; // 比较两个map对象的私有实现是否不相等
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

/**
 * 这段代码实现了Map类的detach()函数。detach()函数的作用是确保Map对象的私有实现处于独占状态，
 * 即没有其他智能指针共享同一个私有实现。如果发现有其他智能指针指向相同的私有实现，detach()函数会创建一个新的
 * 私有实现的副本，从而使得当前对象可以独立于其他对象。
 *
 * 具体来说，detach()函数首先检查智能指针d指向的私有实现的引用计数（通过use_count()函数获取）。如果引用计数大于1，
 * 说明有其他智能指针也指向相同的私有实现，表示对象被共享了。在这种情况下，函数会创建一个新的私有实现的副本，
 * 并将智能指针d指向这个副本。这样，当前对象就拥有了一个独立的私有实现，不会受到其他对象的影响。
 *
 * 这种技术通常被用于实现写时复制（copy-on-write）策略，以提高性能和减少资源消耗。
*/
template <class Key, class T>
void Map<Key, T>::detach()
{
  if(d.use_count() > 1) {
    d = std::make_shared<MapPrivate<Key, T>>(d->map);
  }
}

} // namespace TagLib
