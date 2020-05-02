#ifndef _RDP_ALGORITHM_H
#define _RDP_ALGORITHM_H

#include <iterator>
#include <type_traits>

// Returns shortest distance between point and the line from start to end
template <typename T, typename U>
double distancePointToLine(const std::pair<T, U>& point, const std::pair<T, U>& start, const std::pair<T, U>& end)
{
    if (start == end)
    {
        T px = point.first - start.first;
        U py = point.second - start.second;
        return std::sqrt(px * px + py * py);
    }
    T pesx = end.first - start.first;
    U pesy = end.second - start.second;
    return (std::abs(pesx * (start.second - point.second) - (start.first - point.first) * pesy))
        / (std::sqrt(pesx * pesx + pesy * pesy));
}

// Returns distance less than 3 for random access iterators
template <typename Iter>
inline typename std::enable_if<
    std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>::value,
    bool>::type
isLessThanThree(Iter start, Iter end)
{
    return std::distance(start, end) < 3;
}

// Returns distance less than three for non random access iterators
template <typename Iter>
inline typename std::enable_if<
    !std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>::value,
    bool>::type
isLessThanThree(Iter start, Iter end)
{
    return std::next(start) == end || std::next(start, 2) == end || std::next(start, 3) == end;
}

// Returns RDP-reduced version of input data
template <typename Cont>
Cont rdp(typename Cont::const_iterator start, typename Cont::const_iterator end, double epsilon)
{
    // static_assert(std::is_convertible<typename Cont::value_type, std::pair<T, T>>::value,
    //     "Container elements need to be std::pair");

    if (isLessThanThree(start, end))
    {
        // Just copy it
        return Cont(start, end);
    }

    double dmax = 0.0;
    typename Cont::const_iterator maxElement = start;
    Cont result;

    for (auto it = std::next(start); it != std::prev(end); ++it)
    {
        // Distance between current, first and last point
        double d = distancePointToLine(*it, *start, *std::prev(end));
        if (d > dmax)
        {
            maxElement = it;
            dmax = d;
        }
    }
    if (dmax > epsilon)
    {
        // First to max
        result = rdp<Cont>(start, std::next(maxElement), epsilon);
        // Last point of first result is same as first point of second result, so remove it
        result.pop_back();
        // Max to end
        Cont temp = rdp<Cont>(maxElement, end, epsilon);
        // Append temp to result
        result.insert(result.end(), temp.begin(), temp.end());
    }
    else
    {
        result = {*start, *std::prev(end)};
    }
    return result;
}

#endif