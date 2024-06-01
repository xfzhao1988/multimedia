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

#ifndef TAGLIB_TAG_H
#define TAGLIB_TAG_H

#include "taglib_export.h"
#include "tstring.h"
#include "tlist.h"
#include "tvariant.h"

namespace TagLib {

  class PropertyMap;

  //! A simple, generic interface to common audio metadata fields.
  //  一个简单、通用的常见音频元数据字段接口。

  /*!
   * This is an attempt to abstract away the difference in the metadata formats
   * of various audio codecs and tagging schemes.  As such it is generally a
   * subset of what is available in the specific formats but should be suitable
   * for most applications.  This is meant to compliment the generic APIs found
   * in TagLib::AudioProperties, TagLib::File and TagLib::FileRef.
   * 这是为了抽象各种音频编解码器和标记方案的元数据格式之间的差异。因此，它通常是特定格式中可用的子集，
   * 但应该适用于大多数应用程序。这是为了补充 TagLib::AudioProperties、TagLib::File 和 TagLib::FileRef 中的通用 API。
   */

  class TAGLIB_EXPORT Tag
  {
  public:

    /**
     * 这行代码声明了一个虚析构函数 ~Tag()。在 C++ 中，虚析构函数通常用于基类，以便在使用多态时正确释放派生类对象。
     *
     * 具体来说，virtual ~Tag(); 表示 Tag 类有一个虚析构函数，用于在派生类中正确释放资源。通过将析构函数声明为虚拟的，
     * 可以确保在使用基类指针指向派生类对象时，通过基类指针调用 delete 运算符时会正确调用派生类的析构函数，从而避免内存泄漏。
     *
     * 通常情况下，如果一个类有虚函数，那么它的析构函数应该被声明为虚拟的，以确保正确地调用派生类的析构函数。
    */
    /*!
     * Destroys this Tag instance.
     * 销毁此标签实例。
     */
    virtual ~Tag();

    /**
     * 这行代码使用了 C++11 中的特殊成员函数删除语法，明确禁用了 Tag 类的复制构造函数。
     *
     * Tag(const Tag &) = delete; 表示复制构造函数被删除了。这意味着无法使用复制构造函数创建
     * Tag 类的对象的副本。删除复制构造函数通常是因为禁止对象的复制语义，或者因为复制构造函数的默认实现不适用于该类。
    */
    Tag(const Tag &) = delete;
    /**
     * 这行代码使用了 C++11 中的特殊成员函数删除语法，明确禁用了 Tag 类的赋值操作符重载函数。
     *
     * Tag &operator=(const Tag &) = delete; 表示赋值操作符重载函数被删除了。这意味着无法使用赋值操作符
     * 重载函数对 Tag 类的对象进行赋值操作。删除赋值操作符重载函数通常是因为禁止对象的赋值语义，或者因为赋值操作符
     * 重载函数的默认实现不适用于该类。
    */
    Tag &operator=(const Tag &) = delete;

    /*!
     * Exports the tags of the file as dictionary mapping (human readable) tag
     * names (Strings) to StringLists of tag values.
     * The default implementation in this class considers only the usual built-in
     * tags (artist, album, ...) and only one value per key.
     */
    virtual PropertyMap properties() const;

    /*!
     * Removes unsupported properties, or a subset of them, from the tag.
     * The parameter \a properties must contain only entries from
     * properties().unsupportedData().
     */
    virtual void removeUnsupportedProperties(const StringList& properties);

    /*!
     * Sets the tags of this File to those specified in \a origProps. This default
     * implementation sets only the tags for which setter methods exist in this class
     * (artist, album, ...), and only one value per key; the rest will be contained
     * in the returned PropertyMap.
     */
    virtual PropertyMap setProperties(const PropertyMap &origProps);

    /*!
     * Get the keys of complex properties, i.e. properties which cannot be
     * represented simply by a string.
     * Because such properties might be expensive to fetch, there are separate
     * operations to get the available keys - which is expected to be cheap -
     * and getting and setting the property values.
     * The default implementation returns only an empty list.  Reimplementations
     * should provide "PICTURE" if embedded cover art is present, and optionally
     * support other properties.
     */
    virtual StringList complexPropertyKeys() const;

    /*!
     * Get the complex properties for a given \a key.
     * In order to be flexible for different metadata formats, the properties
     * are represented as variant maps.  Despite this dynamic nature, some
     * degree of standardization should be achieved between formats:
     *
     * - PICTURE
     *   - data: ByteVector with picture data
     *   - description: String with description
     *   - pictureType: String with type as specified for ID3v2,
     *     e.g. "Front Cover", "Back Cover", "Band"
     *   - mimeType: String with image format, e.g. "image/jpeg"
     *   - optionally more information found in the tag, such as
     *     "width", "height", "numColors", "colorDepth" int values
     *     in FLAC pictures
     * - GENERALOBJECT
     *   - data: ByteVector with object data
     *   - description: String with description
     *   - fileName: String with file name
     *   - mimeType: String with MIME type
     *   - this is currently only implemented for ID3v2 GEOB frames
     */
    virtual List<VariantMap> complexProperties(const String &key) const;

    /*!
     * Set all complex properties for a given \a key using variant maps as
     * \a value with the same format as returned by complexProperties().
     * An empty list as \a value removes all complex properties for \a key.
     */
    virtual bool setComplexProperties(const String &key, const List<VariantMap> &value);

    /*!
     * Returns the track name; if no track name is present in the tag
     * an empty string will be returned.
     */
    virtual String title() const = 0;

    /*!
     * Returns the artist name; if no artist name is present in the tag
     * an empty string will be returned.
     */
    virtual String artist() const = 0;

    /*!
     * Returns the album name; if no album name is present in the tag
     * an empty string will be returned.
     */
    virtual String album() const = 0;

    /*!
     * Returns the track comment; if no comment is present in the tag
     * an empty string will be returned.
     */
    virtual String comment() const = 0;

    /*!
     * Returns the genre name; if no genre is present in the tag an empty string
     * will be returned.
     */
    virtual String genre() const = 0;

    /*!
     * Returns the year; if there is no year set, this will return 0.
     */
    virtual unsigned int year() const = 0;

    /*!
     * Returns the track number; if there is no track number set, this will
     * return 0.
     */
    virtual unsigned int track() const = 0;

    /*!
     * Sets the title to \a s.  If \a s is an empty string then this value will be
     * cleared.
     */
    virtual void setTitle(const String &s) = 0;

    /*!
     * Sets the artist to \a s.  If \a s is an empty string then this value will be
     * cleared.
     */
    virtual void setArtist(const String &s) = 0;

    /*!
     * Sets the album to \a s.  If \a s is an empty string then this value will be
     * cleared.
     */
    virtual void setAlbum(const String &s) = 0;

    /*!
     * Sets the comment to \a s.  If \a s is an empty string then this value will be
     * cleared.
     */
    virtual void setComment(const String &s) = 0;

    /*!
     * Sets the genre to \a s.  If \a s is an empty string then this value will be
     * cleared.  For tag formats that use a fixed set of genres, the appropriate
     * value will be selected based on a string comparison.  A list of available
     * genres for those formats should be available in that type's
     * implementation.
     */
    virtual void setGenre(const String &s) = 0;

    /*!
     * Sets the year to \a i.  If \a s is 0 then this value will be cleared.
     */
    virtual void setYear(unsigned int i) = 0;

    /*!
     * Sets the track to \a i.  If \a s is 0 then this value will be cleared.
     */
    virtual void setTrack(unsigned int i) = 0;

    /*!
     * Returns \c true if the tag does not contain any data.  This should be
     * reimplemented in subclasses that provide more than the basic tagging
     * abilities in this class.
     */
    virtual bool isEmpty() const;

    /*!
     * Copies the generic data from one tag to another.
     *
     * \note This will not affect any of the lower level details of the tag.  For
     * instance if any of the tag type specific data (maybe a URL for a band) is
     * set, this will not modify or copy that.  This just copies using the API
     * in this class.
     *
     * If \a overwrite is \c true then the values will be unconditionally copied.
     * If \c false only empty values will be overwritten.
     */
    static void duplicate(const Tag *source, Tag *target, bool overwrite = true);

    /*!
     * Join the \a values of a tag to a single string separated by " / ".
     * If the tag implementation can have multiple values for a basic tag
     * (e.g. artist), they can be combined to a single string for the basic
     * tag getters (e.g. artist()).
     */
    static String joinTagValues(const StringList &values);

  protected:
    /*!
     * Construct a Tag.  This is protected since tags should only be instantiated
     * through subclasses.
     * 构造一个标签。这是受保护的，因为标签只能通过子类实例化。
     */
    Tag();

  private:
    class TagPrivate;
    TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
    std::unique_ptr<TagPrivate> d;
    /**
     * 这行代码声明了一个名为 d 的 std::unique_ptr 智能指针，指向类型为 TagPrivate 的对象。
     *
     * 具体来说，std::unique_ptr 是 C++ 标准库提供的智能指针类型之一，用于管理动态分配的单个对象的所有权。
     * 在这里，d 是一个名为 d 的 std::unique_ptr 对象，它将拥有对 TagPrivate 类型对象的所有权。
     *
     * 使用 std::unique_ptr 有助于在程序中管理动态分配的资源，并在适当的时候自动释放这些资源，
     * 从而避免内存泄漏和不必要的资源管理问题。
     *
     * std::unique_ptr和std::shared_ptr的区别?
     *
     * std::unique_ptr 和 std::shared_ptr 是 C++ 标准库提供的两种不同类型的智能指针，它们在管理动态分配的资源时
     * 有一些重要的区别。
     *
     *  1. 所有权：
     *  std::unique_ptr：拥有对其所指向对象的独占所有权。这意味着只有一个 std::unique_ptr 实例可以拥有和管理特定的资源。
     *  当 std::unique_ptr 被销毁或转移时，它所指向的资源将被自动释放。
     *
     *  std::shared_ptr：允许多个 std::shared_ptr 实例共享对其所指向对象的所有权。它通过使用引用计数来跟踪资源的使用情况，
     *  只有当最后一个 std::shared_ptr 实例被销毁时，资源才会被释放。
     *
     *  2. 线程安全：
     *  std::unique_ptr：不提供多线程访问的安全性保证。它只能在单线程环境或在确保不会出现竞态条件的情况下使用。
     *  std::shared_ptr：可以安全地在多个线程中共享和访问。它使用原子操作来更新引用计数，以确保在并发环境中的线程安全性。
     *
     *  3. 性能开销：
     *  std::unique_ptr：通常比 std::shared_ptr 更轻量，因为它只需要维护一个指向资源的指针，并且不需要额外的引用计数。
     *  std::shared_ptr：由于需要维护引用计数，因此通常会有更高的内存开销和运行时开销。
     *
     *  4. 循环引用：
     *  std::unique_ptr：不支持循环引用，因为它无法共享所有权。这可以避免循环引用导致的内存泄漏。
     *  std::shared_ptr：支持循环引用，因为它使用强引用计数。但是，循环引用可能导致资源无法释放，从而导致内存泄漏。
     *
     *  5. 使用场景：
     *  std::unique_ptr：适用于需要独占所有权的情况，或者需要在容器中存储动态分配的对象并且不需要共享所有权的情况。
     *  std::shared_ptr：适用于需要多个所有者共享资源的情况，或者需要在容器中存储动态分配的对象并且需要共享所有权的情况。

     *  总的来说，选择使用 std::unique_ptr 还是 std::shared_ptr 取决于你的需求。如果你需要独占所有权或者在单线程环境中工作，
     * 那么 std::unique_ptr 是一个很好的选择。如果你需要在多个地方共享资源或者在多线程环境中工作，那么 std::shared_ptr 可能更适合。
     *
     * 下面是使用 std::unique_ptr 和 std::shared_ptr 的简单示例：
     *  #include <iostream>
        #include <memory>

        class MyClass {
        public:
            MyClass() { std::cout << "MyClass Constructor\n"; }
            ~MyClass() { std::cout << "MyClass Destructor\n"; }
            void doSomething() { std::cout << "Doing something...\n"; }
        };

        void uniquePtrExample() {
            // 使用 std::unique_ptr 来管理动态分配的资源
            std::unique_ptr<MyClass> uniquePtr(new MyClass());
            uniquePtr->doSomething();
        } // 当 uniquePtr 超出范围时，MyClass 对象将被自动销毁

        void sharedPtrExample() {
            // 使用 std::shared_ptr 来共享动态分配的资源
            std::shared_ptr<MyClass> sharedPtr1(new MyClass());
            std::shared_ptr<MyClass> sharedPtr2 = sharedPtr1; // 共享所有权
            sharedPtr1->doSomething();
            sharedPtr2->doSomething();
        } // 当 sharedPtr1 和 sharedPtr2 超出范围时，MyClass 对象将被销毁

        int main() {
            std::cout << "Using std::unique_ptr:\n";
            uniquePtrExample();

            std::cout << "\nUsing std::shared_ptr:\n";
            sharedPtrExample();

            return 0;
        }
      *
      * 在这个示例中：
      * uniquePtrExample() 函数演示了如何使用 std::unique_ptr 来管理动态分配的资源。当 std::unique_ptr 超出范围时，
      * 它所指向的对象会被自动销毁。
      *
      * sharedPtrExample() 函数演示了如何使用 std::shared_ptr 来共享动态分配的资源。多个 std::shared_ptr
      * 实例可以共享对相同资源的所有权，通过引用计数来管理资源的生命周期。当最后一个 std::shared_ptr 超出范围时，资源会被销毁。
      *
      * 这些示例展示了 std::unique_ptr 和 std::shared_ptr 在管理动态分配资源时的不同用法和行为。
      */
  };
}  // namespace TagLib

#endif
