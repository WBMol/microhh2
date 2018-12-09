#ifndef ARRAY_H
#define ARRAY_H

#include <array>
#include <vector>
#include <iostream>

template<int N>
inline std::array<int, N> calc_strides(const std::array<int, N>& dims)
{
    std::array<int, N> strides;
    strides[0] = 1;
    for (int i=1; i<N; ++i)
        strides[i] = strides[i-1]*dims[i-1];

    return strides;
}

template<int N>
inline int calc_index(
        const std::array<int, N>& indices,
        const std::array<int, N>& strides,
        const std::array<int, N>& offsets)
{
    int sum = 0;
    for (int i=0; i<N; ++i)
        sum += (indices[i]-offsets[i]-1)*strides[i];

    return sum;
}

template<int N>
inline std::array<int, N> calc_indices(
        int index, const std::array<int, N>& strides, const std::array<int, N>& offsets)
{
    std::array<int, N> indices;

    for (int i=N-1; i>=1; --i)
    {
        indices[i] = index / strides[i];
        index %= strides[i];
    }
    indices[0] = index;

    for (int i=0; i<N; ++i)
    {
        indices[i] += offsets[i] + 1;
        std::cout << "CvH: " << indices[i] << std::endl;
    }

    return indices;
}

template<int N>
inline int product(const std::array<int, N>& array)
{
    int product = array[0];
    for (int i=1; i<N; ++i)
        product *= array[i];

    return product;
}

template<typename T>
class Array_iterator
{
    public:
        Array_iterator(const std::vector<T>& data, const int n) : data(data), n(n) {}
        Array_iterator& operator++()
        {
            ++n;
            return *this;
        }
        std::pair<int, T> operator*() const { return std::make_pair(n, data[n]); }

    private:
        const std::vector<T>& data;
        int n;

    friend bool operator!=(const Array_iterator<T>& left, const Array_iterator<T>& right) { return left.n != right.n; }
};

template<typename T, int N>
class Array
{
    public:
        Array() :
            ncells(0)
        {}

        Array(const std::array<int, N>& dims) :
            dims(dims),
            ncells(product<N>(dims)),
            data(ncells),
            strides(calc_strides<N>(dims)),
            offsets({})
        {}

        Array(std::vector<T>& data, const std::array<int, N>& dims) :
            dims(dims),
            ncells(product<N>(dims)),
            data(data),
            strides(calc_strides<N>(dims)),
            offsets({})
        {} // CvH Do we need to size check data?

        Array(std::vector<T>&& data, const std::array<int, N>& dims) :
            dims(dims),
            ncells(product<N>(dims)),
            data(data),
            strides(calc_strides<N>(dims)),
            offsets({})
        {} // CvH Do we need to size check data?

        // Define the default copy and move constructor.
        Array(Array<T,N>&) = default;
        Array<T,N>& operator=(const Array<T,N>&) = default;

        Array(Array<T,N>&& array) :
            dims(std::exchange(array.dims, {})),
            ncells(std::exchange(array.ncells, 0)),
            data(std::move(array.data)),
            strides(std::exchange(array.strides, {})),
            offsets(std::exchange(array.offsets, {}))
        {}

        inline void set_offsets(const std::array<int, N>& offsets)
        {
           this->offsets = offsets;
        }

        inline std::vector<T>& v() { return data; }

        inline std::array<int, N> find_indices(const T& value)
        {
            int pos = std::find(data.begin(), data.end(), value) - data.begin();
            return calc_indices<N>(pos, strides, offsets);
        }

        inline void operator=(std::vector<T>&& data)
        {
            // CvH check size.
            this->data = data;
        }

        inline T& operator()(const std::array<int, N>& indices)
        {
            const int index = calc_index<N>(indices, strides, offsets);
            return data[index];
        }

        inline T operator()(const std::array<int, N>& indices) const
        {
            const int index = calc_index<N>(indices, strides, offsets);
            return data[index];
        }

        inline Array_iterator<T> begin() { return Array_iterator<T>(data, 0); }
        inline Array_iterator<T> end()   { return Array_iterator<T>(data, ncells); }

        inline int dim(const int i) const { return dims[i-1]; }

    private:
        std::array<int, N> dims;
        int ncells;
        std::vector<T> data;
        std::array<int, N> strides;
        std::array<int, N> offsets;
};
#endif
