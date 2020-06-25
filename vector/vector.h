#pragma once

#include <cstdlib>
#include <cstring>
#include <stdexcept>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    vector() : size_(0), capacity_(0), data_(nullptr) {};
    vector(vector<T> const& v) : vector() {
        vector<T> safe = vector(v.data_, v.size_, v.capacity_);
        swap(safe);
    }
    vector& operator=(vector<T> const& v) {
        vector safe(v);
        swap(safe);
        return *this;
    }

    ~vector() {
        clear();
        operator delete(data_);
    }

    T& operator[](size_t i) {return data_[i];}
    T const& operator[](size_t i) const {return data_[i];}

    T* data() {return data_;}
    T const* data() const {return data_;}
    size_t size() const {return size_;}

    T& front() {return *begin();}
    T const& front() const {return *begin();}

    T& back() {return *(end() - 1);}
    T const& back() const {return *(end() - 1);}

    void push_back(T const& x) {
        T safe = x;
        if (size_ == capacity_) {
            change_capacity((capacity_ + 1) * 2);
        }
        new(end()) T(safe);
        size_++;
    }

    void pop_back() {
        size_--;
        data_[size_].~T();
    }

    bool empty() const {return size_ == 0;}

    size_t capacity() const {return capacity_;}

    void reserve(size_t capacity) {
        if (capacity > capacity_) {
            change_capacity(capacity - capacity_);
        }
    }
    void shrink_to_fit() {
        if (size_ < capacity_)
            change_capacity(size_);
    }

    void clear() {
        while (size_)
            pop_back();
    }

    void swap(vector& v) {
        std::swap(data_, v.data_);
        std::swap(size_, v.size_);
        std::swap(capacity_, v.capacity_);
    }

    iterator begin() {return data_;}
    iterator end() {return (data_ + size_);}

    const_iterator begin() const {return data_;};
    const_iterator end() const {return (data_ + size_);}

    iterator insert(const_iterator pos, T const& x) {
        size_t ind = pos - begin();
        if (ind == size_) {
            push_back(x);
        } else {
            push_back(back());
            for (size_t i = size_ - 2; i > ind; i--) {
                std::swap(data_[i], data_[i - 1]);
            }
            data_[ind] = x;
        }
        return begin() + ind;
    }

    iterator erase(const_iterator first, const_iterator last) {
        size_t ind = first - begin();
        size_t k = last - first;
        for (size_t i = ind; i < size_ - k; i++) {
            std::swap(data_[i], data_[i + k]);
        }
        while (k) {
            pop_back();
            k--;
        }
        return begin() + ind;
    }

    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }

private:
    vector(T const* data, size_t size, size_t capacity) : vector() {
        if (capacity != 0) {
            T* new_data = static_cast<T *>(operator new(capacity * sizeof(T)));;
            try {
                for (; size_ < size; size_++) {
                    new(new_data + size_) T(data[size_]);
                }
            } catch (...) {
                while (size_ > 0) {
                    new_data[size_ - 1].~T();
                    size_--;
                }
                operator delete(new_data);
                throw;
            }
            data_ = new_data;
            capacity_ = capacity;
        }
    }
    void change_capacity(size_t new_capacity) {
        vector<T> safe(data_, size_, new_capacity);
        swap(safe);
    }

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};


