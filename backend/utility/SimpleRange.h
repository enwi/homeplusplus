#pragma once
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include <polymorphic_value.h>

// Morph iter morphs any iterator to an iterator returning type Value
template <typename Value>
class MorphIterator
{
private:
    class MorphBase
    {
    public:
        virtual ~MorphBase() = default;
        virtual Value& operator*() const = 0;
        virtual Value* operator->() const = 0;
        virtual void operator++() = 0;
        // other has to be same derived class
        virtual bool operator==(const MorphBase& other) const = 0;
        virtual bool operator!=(const MorphBase& other) const { return !(*this == other); }
    };
    class EmptyMorph : public MorphBase
    {
    public:
        virtual Value& operator*() const override { throw std::logic_error("Dereferenced invalid MorphIterator"); }
        virtual Value* operator->() const override { throw std::logic_error("Dereferenced invalid MorphIterator"); }
        virtual void operator++() override { throw std::logic_error("Incremented invalid MorphIterator"); }
        virtual bool operator==(const MorphBase& other) const override
        {
            // Return whether other is empty
            return dynamic_cast<const EmptyMorph*>(&other) != nullptr;
        }
    };
    template <typename T>
    class Morph : public MorphBase
    {
    public:
        template <typename D>
        explicit Morph(D&& val) : stuff(std::forward<D>(val))
        {}
        virtual Value& operator*() const override { return static_cast<Value&>(*stuff); }
        virtual Value* operator->() const override { return static_cast<Value*>(stuff.operator->()); }
        virtual void operator++() override { ++stuff; }
        virtual bool operator==(const MorphBase& other) const override
        {
            return stuff == dynamic_cast<const Morph&>(other).stuff;
        }

    private:
        T stuff;
    };
    // Specialization for pointers
    template <typename T>
    class Morph<T*> : public MorphBase
    {
    public:
        explicit Morph(T* val) : stuff(val) {}
        virtual Value& operator*() const override { return static_cast<Value&>(*stuff); }
        virtual Value* operator->() const override { return static_cast<Value*>(stuff); }
        virtual void operator++() override { ++stuff; }
        virtual bool operator==(const MorphBase& other) const override
        {
            return stuff == dynamic_cast<const Morph&>(other).stuff;
        }

    private:
        T* stuff;
    };

public:
    using difference_type = ptrdiff_t;
    using value_type = Value;
    using pointer = Value*;
    using reference = Value&;
    using iterator_category = std::input_iterator_tag;

    template <typename T>
    friend class MorphIterator;

    MorphIterator() = default;
    explicit MorphIterator(jbcoe::polymorphic_value<MorphBase> m) : morph(std::move(m)) {}
    template <typename ItType,
        std::enable_if_t<(!std::is_same<std::decay_t<ItType>, jbcoe::polymorphic_value<MorphBase>>::value
            && !std::is_same<std::decay_t<ItType>, MorphIterator>::value)>* = nullptr>
    MorphIterator(ItType&& it)
        : morph(jbcoe::polymorphic_value<MorphBase>(Morph<std::decay_t<ItType>>(std::forward<ItType>(it))))
    {}
    // Copy constructor from convertible value types //Not working and never called (ItType&& is more specific)
    // template<typename OtherValue, std::enable_if_t<std::is_convertible<OtherValue, Value>::value &&
    // !std::is_same<Value, OtherValue>::value>* = nullptr> MorphIterator(const MorphIterator<OtherValue>& other) :
    // morph(jbcoe::polymorphic_value<MorphBase>(other.morph)) {}

    Value& operator*() const { return **morph; }
    Value* operator->() const { return (*morph).operator->(); }
    MorphIterator& operator++()
    {
        ++(*morph);
        return *this;
    }
    MorphIterator operator++(int /*postIncrement*/)
    {
        jbcoe::polymorphic_value<MorphBase> copy = morph;
        ++(*morph);
        return MorphIterator(copy);
    }
    bool operator==(const MorphIterator& other) const { return *morph == *other.morph; }
    bool operator!=(const MorphIterator& other) const { return *morph != *other.morph; }

private:
    jbcoe::polymorphic_value<MorphBase> morph = EmptyMorph();
};

// Range of type T which can be iterated over
template <typename T>
struct Range
{
    using Iter = MorphIterator<T>;
    // typedefs for containers
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = Iter;
    using const_iterator = MorphIterator<const T>;
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using size_type = std::size_t;

    const Iter& begin() const { return itBegin; }
    const Iter& end() const { return itEnd; }
    Iter itBegin;
    Iter itEnd;
};

// Very inefficient way to iterate over multiple ranges (uses vector of iterators)
template <typename Value>
class CompoundIterator
{
public:
    using difference_type = ptrdiff_t;
    using value_type = Value;
    using pointer = Value*;
    using reference = Value&;
    using iterator_category = std::input_iterator_tag;

    template <typename T>
    friend class CompoundIterator;

    template <typename Other>
    CompoundIterator(const CompoundIterator<Other>& other)
    {
        std::copy(other.iters.begin(), other.iters.end(), std::back_inserter(iters));
    }

    template <typename... Ranges>
    explicit CompoundIterator(Ranges&&... ranges) : iters({std::make_pair(ranges.begin(), ranges.end())...})
    {
        // Reverse so used ranges can be pop_backed (maybe do this before copying them all?)
        std::reverse(iters.begin(), iters.end());
        PopEmpty();
    }
    explicit CompoundIterator(const std::vector<Range<Value>>& vec)
    {
        // Reverse iteration over vec
        std::for_each(vec.rbegin(), vec.rend(), [&](const Range<Value>& r) { iters.emplace_back(r.begin(), r.end()); });
        PopEmpty();
    }
    void PopEmpty()
    {
        while (!iters.empty() && (iters.back().first == iters.back().second))
        {
            // Remove all trailing empty ranges
            iters.pop_back();
        }
    }
    Value& operator*() const { return *iters.back().first; }
    Value* operator->() const { return iters.back().first.operator->(); }
    CompoundIterator& operator++()
    {
        ++iters.back().first;
        PopEmpty();
        return *this;
    }
    CompoundIterator operator++(int)
    {
        CompoundIterator copy = *this;
        ++*this;
        return copy;
    }
    bool operator==(const CompoundIterator& other) const
    {
        // Only equal if both empty
        return iters.empty() && other.iters.empty();
    }
    bool operator!=(const CompoundIterator& other) const { return !(*this == other); }

private:
    std::vector<std::pair<MorphIterator<Value>, MorphIterator<Value>>> iters;
};

// CompoundRange consisting of multiple Ranges of type T, which are iterated over as one combined Range
template <typename T>
class CompoundRange
{
public:
    using Iter = CompoundIterator<T>;

    // typedefs for containers
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = Iter;
    using const_iterator = CompoundIterator<const T>;
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using size_type = std::size_t;

    template <typename... Ranges>
    CompoundRange(const Ranges&... ranges) : itBegin(ranges...)
    {}
    CompoundRange(const std::vector<Range<T>>& vec) : itBegin(vec) {}
    const Iter& begin() const { return itBegin; }
    const Iter& end() const { return itEnd; }

private:
    Iter itBegin;
    Iter itEnd;
};

// Plain dereferencing
template <typename T>
struct DefaultDereferencer
{
    template <typename Iter>
    static T& GetRef(const Iter& it)
    {
        return *it;
    }
    template <typename Iter>
    static T* GetPtr(const Iter& it)
    {
        return it.operator->();
    }
};

// Useful for smart pointers, polymorphic_value
template <typename T>
struct DoubleDereferencer
{
    template <typename Iter>
    static T& GetRef(const Iter& it)
    {
        return **it;
    }
    template <typename Iter>
    static T* GetPtr(const Iter& it)
    {
        return &**it;
    }
};

// Useful for reference_wrapper
template <typename T>
struct GetDereferencer
{
    template <typename Iter>
    static T& GetRef(const Iter& it)
    {
        return it->get();
    }
    template <typename Iter>
    static T* GetPtr(const Iter& it)
    {
        return &it->get();
    }
};

// Creates range which takes ownership of a container
template <typename T, typename Dereferencer = DefaultDereferencer<T>, typename Container>
Range<T> MakeOwningRange(Container&& c)
{
    using ContDecay = std::decay_t<Container>;
    class OwningIter
    {
    public:
        using ContIter = typename Container::iterator;
        OwningIter(ContIter it, std::shared_ptr<ContDecay> p) : it(std::move(it)), pCont(std::move(p)) {}
        T& operator*() const { return Dereferencer::GetRef(it); }
        T* operator->() const { return Dereferencer::GetPtr(it); }
        bool operator==(const OwningIter& other) const { return it == other.it; }
        OwningIter& operator++()
        {
            ++it;
            return *this;
        }

    private:
        ContIter it;
        std::shared_ptr<std::decay_t<Container>> pCont;
    };
    auto shared = std::make_shared<ContDecay>(std::forward<Container>(c));
    return Range<T>{OwningIter(shared->begin(), shared), OwningIter(shared->end(), shared)};
}

namespace detail
{
    template <typename It>
    void SaveAdvanceImpl(
        It& first, It end, typename std::iterator_traits<It>::difference_type num, std::input_iterator_tag)
    {
        for (; 0 < num && first != end; --num)
        {
            ++first;
        }
    }
    template <typename It>
    void SaveAdvanceImpl(
        It& first, It end, typename std::iterator_traits<It>::difference_type num, std::random_access_iterator_tag)
    {
        assert(num > 0);
        typename std::iterator_traits<It>::difference_type max = end - first;
        first += std::min(max, num);
    }
} // namespace detail

// Increments first num times or until it is equal to end (only forwards)
template <typename It>
void SaveAdvance(It& first, It end, typename std::iterator_traits<It>::difference_type num)
{
    detail::SaveAdvanceImpl(first, end, num, typename std::iterator_traits<It>::iterator_category{});
}

// Returns first incremented by num times or until it is equal to end (only forwards)
template <typename It>
It SaveNext(It first, It end, typename std::iterator_traits<It>::difference_type num = 1)
{
    SaveAdvance(first, end, num);
    return first;
}