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

#ifndef TAGLIB_MAP_H
#define TAGLIB_MAP_H

#include <map>
#include <memory>
#include <initializer_list>
#include <utility>

namespace TagLib {

  //! A generic, implicitly shared map.
  // 通用的、隐式共享的map。

  /*!
   * This implements a standard map container that associates a key with a value
   * and has fast key-based lookups.  This map is also implicitly shared making
   * it suitable for pass-by-value usage.
   * 这实现了一个标准映射容器，它将一个键与一个值关联起来，并具有基于键的快速查找功能。
   * 此映射也是隐式共享的，因此适合按值传递使用。
   */

  /**
   * 这是一个C++模板类的声明，表示一个通用的Map类模板，其中Key和T是模板参数，
   * 分别表示键和值的类型。通常，Key用于表示Map中的键的类型，T用于表示Map中值的类型。
  */
  template <class Key, class T> class Map
  {
  public:
#ifndef DO_NOT_DOCUMENT
#ifdef WANT_CLASS_INSTANTIATION_OF_MAP
    // Some STL implementations get snippy over the use of the
    // class keyword to distinguish different templates; Sun Studio
    // in particular finds multiple specializations in certain rare
    // cases and complains about that. GCC doesn't seem to mind,
    // and uses the typedefs further below without the class keyword.
    // Not all the specializations of Map can use the class keyword
    // (when T is not actually a class type), so don't apply this
    // generally.
    // 一些 STL 实现对使用 class 关键字来区分不同模板的做法很不以为然；特别是
    // Sun Studio 在某些罕见情况下会发现多个特化并对此提出抱怨。GCC 似乎并不介意，
    // 并在下文中使用没有 class 关键字的 typedef。并非所有 Map 特化都可以使用 class 关键字
    //（当 T 实际上不是类类型时），因此通常不要应用此方法。
    using Iterator = typename std::map<class Key, class T>::iterator;
    using ConstIterator = typename std::map<class Key, class T>::const_iterator;
#else
    /**
     * 这是一个使用using关键字定义的类型别名（type alias），用于简化类型的名称。
     * 在这个例子中，Iterator被定义为std::map<Key, T>::iterator的一个别名。
     *
     * 具体来说，std::map<Key, T>::iterator表示std::map模板类中的迭代器类型，
     * 它用于遍历std::map容器中的元素。typename关键字用于指示std::map<Key, T>::iterator是一个类型，
     * 因为在模板上下文中，iterator可能是一个类型也可能是一个成员变量或函数。
     *
     * 通过使用using关键字，我们可以为std::map<Key, T>::iterator定义一个更简洁的别名Iterator，
     * 使代码更加清晰和易读。这使得在代码中引用迭代器类型更加方便，比如在函数参数、返回值或模板参数中
     * 使用Iterator，而不需要每次都写出完整的类型名称。
    */
    using Iterator = typename std::map<Key, T>::iterator;
    using ConstIterator = typename std::map<Key, T>::const_iterator;
#endif
#endif

    /*!
     * Constructs an empty Map.
     * 构造一个空的 Map。
     */
    Map();

    /*!
     * Make a shallow, implicitly shared, copy of \a m.  Because this is
     * implicitly shared, this method is lightweight and suitable for
     * pass-by-value usage.
     * 对 m 进行浅层、隐式共享的复制。由于这是隐式共享的，因此此方法很轻量，适合按值传递使用。
     */
    Map(const Map<Key, T> &m);

    /*!
     * Constructs a Map with the contents of the braced initializer list.
     * 使用括号中的初始化列表的内容构造一个 Map。
     */
    Map(std::initializer_list<std::pair<const Key, T>> init);

    /*!
     * Destroys this instance of the Map.
     * 销毁该map实例。
     */
    ~Map();

    /**
     * chatgpt列出的std::map<Key, T>::iterator和std::map<Key, T>::const_iterator的使用实例:
     *
     * 下面是一个示例，演示了如何使用std::map<Key, T>::iterator和std::map<Key, T>::const_iterator来
     * 遍历和修改std::map对象的元素：
     *
     *  #include <iostream>
        #include <map>

        int main() {
            // 创建一个 std::map 对象
            std::map<int, std::string> myMap = {
                {1, "one"},
                {2, "two"},
                {3, "three"}
            };

            // 使用迭代器遍历并修改 std::map 对象的元素
            std::cout << "Original map:" << std::endl;
            for (auto it = myMap.begin(); it != myMap.end(); ++it) {
                std::cout << it->first << ": " << it->second << std::endl;
                it->second = "modified"; // 修改元素的值
            }

            // 使用 const_iterator 遍历并输出 std::map 对象的元素
            std::cout << "\nModified map:" << std::endl;
            for (auto it = myMap.cbegin(); it != myMap.cend(); ++it) {
                std::cout << it->first << ": " << it->second << std::endl;
            }

            return 0;
        }
    *
    * 在这个示例中，我们首先创建了一个std::map<int, std::string>对象myMap，并初始化了一些键值对。
    * 然后，我们使用std::map<Key, T>::iterator遍历myMap对象，并在遍历过程中修改了元素的值。接着，
    * 我们使用std::map<Key, T>::const_iterator遍历myMap对象，并输出了修改后的元素值。
    *
    * 注意，std::map<Key, T>::iterator允许修改元素的值，而std::map<Key, T>::const_iterator只能访问元素的值，
    * 不能修改它们。这是因为const_iterator指向的是常量对象，而iterator指向的是可变对象。
    *
    */

    /*!
     * Returns an STL style iterator to the beginning of the map.  See
     * \c std::map::iterator for the semantics.
     * 返回指向映射开头的 STL 样式迭代器。有关语义，请参阅 std::map::iterator。
     */
    Iterator begin();

    /*!
     * Returns an STL style iterator to the beginning of the map.  See
     * \c std::map::const_iterator for the semantics.
     * 返回指向映射开头的 STL 样式迭代器。有关语义，请参阅 std::map::const_iterator。
     */
    ConstIterator begin() const;

    /*!
     * Returns an STL style iterator to the beginning of the map.  See
     * \c std::map::const_iterator for the semantics.
     * 返回指向map开头的 STL 样式迭代器。有关语义，请参阅 std::map::const_iterator。
     */
    ConstIterator cbegin() const;

    /*!
     * Returns an STL style iterator to the end of the map.  See
     * \c std::map::iterator for the semantics.
     * 返回指向映射末尾的 STL 样式迭代器。有关语义，请参阅 std::map::iterator。
     */
    Iterator end();

    /*!
     * Returns an STL style iterator to the end of the map.  See
     * \c std::map::const_iterator for the semantics.
     * 返回指向映射末尾的 STL 样式迭代器。有关语义，请参阅 std::map::const_iterator。
     */
    ConstIterator end() const;

    /*!
     * Returns an STL style iterator to the end of the map.  See
     * \c std::map::const_iterator for the semantics.
     * 返回指向映射末尾的 STL 样式迭代器。有关语义，请参阅 std::map::const_iterator。
     */
    ConstIterator cend() const;

    /*!
     * Inserts \a value under \a key in the map.  If a value for \a key already
     * exists it will be overwritten.
     * 在映射中的键下插入值。如果键的值已存在，则会被覆盖。
     */
    Map<Key, T> &insert(const Key &key, const T &value);

    /*!
     * Removes all of the elements from the map.  This however
     * will not delete pointers if the mapped type is a pointer type.
     * 从映射中删除所有元素。但是，如果映射类型是指针类型，则不会删除指针。
     */
    Map<Key, T> &clear();

    /*!
     * The number of elements in the map.
     * map中元素的数量。
     *
     * \see isEmpty()
     */
    unsigned int size() const;

    /*!
     * Returns \c true if the map is empty.
     * 如果map为空，则返回 true。
     *
     * \see size()
     */
    bool isEmpty() const;

    /*!
     * Find the first occurrence of \a key.
     * 查找键的第一次出现
     */
    Iterator find(const Key &key);

    /*!
     * Find the first occurrence of \a key.
     * 查找键的第一次出现
     */
    ConstIterator find(const Key &key) const;

    /*!
     * Returns \c true if the map contains an item for \a key.
     * 如果映射包含键对应的项，则返回true。
     */
    bool contains(const Key &key) const;

    /*!
     * Erase the item at \a it from the map.
     *
     * \note This method cannot detach because \a it is tied to the internal
     * map.  Do not make an implicitly shared copy of this map between
     * getting the iterator and calling this method!
     *
     * 从map中删除该项。注意此方法无法分离，因为它与内部map绑定。
     * 在获取迭代器和调用此方法之间，不要创建此map的隐式共享副本！
     */
    Map<Key, T> &erase(Iterator it);

    /*!
     * Erase the item with \a key from the map.
     * 从map中删除带有键的项目。
     */
    Map<Key, T> &erase(const Key &key);

    /*!
     * Returns the value associated with \a key.
     *
     * If the map does not contain \a key, it returns \a defaultValue.
     * If no \a defaultValue is specified, it returns a default-constructed value.
     * 返回与键关联的值。
     * 如果映射不包含键，则返回默认值。
     * 如果没有指定默认值，则返回默认构造的值。
     */
    T value(const Key &key, const T &defaultValue = T()) const;

    /*!
     * Returns a reference to the value associated with \a key.
     *
     * \note This has undefined behavior if the key is not present in the map.
     * 返回与键关联的值的引用。注意如果映射中不存在该键，则会产生未定义的行为。
     */
    const T &operator[](const Key &key) const;

    /*!
     * Returns a reference to the value associated with \a key.
     *
     * \note This has undefined behavior if the key is not present in the map.
     * 返回与键关联的值的引用。注意如果映射中不存在该键，则会产生未定义的行为。
     */
    T &operator[](const Key &key);

    /*!
     * Make a shallow, implicitly shared, copy of \a m.  Because this is
     * implicitly shared, this method is lightweight and suitable for
     * pass-by-value usage.
     * 对 m 进行浅层、隐式共享的复制。由于这是隐式共享的，因此此方法很轻量，适合按值传递使用。
     */
    Map<Key, T> &operator=(const Map<Key, T> &m);

    /*!
     * Replace the contents of the map with those of the braced initializer list
     * 将映射的内容替换为括号初始化列表中的内容
     */
    Map<Key, T> &operator=(std::initializer_list<std::pair<const Key, T>> init);

    /*!
     * Exchanges the content of this map with the content of \a m.
     * 将此映射的内容与 m 的内容交换。
     */
    void swap(Map<Key, T> &m) noexcept;

    /*!
     * Compares this map with \a m and returns \c true if all of the elements are
     * the same.
     * 将此映射与 m 进行比较，如果所有元素都相同则返回 true。
     */
    bool operator==(const Map<Key, T> &m) const;

    /*!
     * Compares this map with \a m and returns \c true if the maps differ.
     * 将此映射与 m 进行比较，如果映射不同则返回 true。
     */
    bool operator!=(const Map<Key, T> &m) const;

  protected:
    /*!
     * If this Map is being shared via implicit sharing, do a deep copy of the
     * data and separate from the shared members.  This should be called by all
     * non-const subclass members without Iterator parameters.
     * 如果此 Map 通过隐式共享进行共享，则对数据进行深度复制并与共享成员分开。
     * 所有不带 Iterator 参数的非 const 子类成员都应调用此方法。
     */
    void detach();

  private:
#ifndef DO_NOT_DOCUMENT
    template <class KeyP, class TP> class MapPrivate;
    /**
     * 这行代码声明了一个名为d的std::shared_ptr指针，指向MapPrivate<Key, T>类型的对象。
     * std::shared_ptr是C++标准库中的智能指针，它允许多个指针共享对同一对象的所有权，并在不再需要时自动释放对象。
     *
     * 在这个情况下，d是一个智能指针，它可以指向任何MapPrivate<Key, T>类型的对象。通过使用std::shared_ptr，
     * 可以确保在需要时正确地管理MapPrivate对象的生命周期，从而避免内存泄漏和空悬指针等问题。
     *
     * C++标准库中的智能指针std::shared_ptr的使用实例：
     *
        #include <iostream>
        #include <memory>

        class MyClass {
        public:
            MyClass(int value) : m_value(value) {
                std::cout << "Constructing MyClass with value: " << m_value << std::endl;
            }

            ~MyClass() {
                std::cout << "Destructing MyClass with value: " << m_value << std::endl;
            }

            void setValue(int value) {
                m_value = value;
            }

            int getValue() const {
                return m_value;
            }

        private:
            int m_value;
        };

        int main() {
            // 创建一个 std::shared_ptr 指向 MyClass 对象
            std::shared_ptr<MyClass> ptr = std::make_shared<MyClass>(10);

            // 输出 MyClass 对象的值
            std::cout << "Value of MyClass: " << ptr->getValue() << std::endl;

            // 使用 std::shared_ptr 修改 MyClass 对象的值
            ptr->setValue(20);

            // 输出 MyClass 对象的新值
            std::cout << "New value of MyClass: " << ptr->getValue() << std::endl;

            // 当 ptr 超出作用域时，MyClass 对象将被自动销毁
            return 0;
        }
      * 这个示例中，我们定义了一个名为MyClass的类，它具有一个整型成员变量m_value和一些简单的方法。
      * 然后，我们在main函数中使用std::make_shared函数创建了一个std::shared_ptr指针ptr，指向一个MyClass对象。
      * 我们可以使用ptr来访问和修改MyClass对象的成员变量和方法。当ptr超出作用域时，由于没有其他指针指向该对象，
      * MyClass对象将被自动销毁，并调用其析构函数来释放资源。
      */
    std::shared_ptr<MapPrivate<Key, T>> d;
#endif
  };

}  // namespace TagLib

// Since GCC doesn't support the "export" keyword, we have to include the
// implementation.
// 由于 GCC 不支持“export”关键字，我们必须包含该实现。

#include "tmap.tcc"

#endif
