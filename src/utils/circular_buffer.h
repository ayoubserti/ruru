#pragma once

// ChatGPT generated code
#include <vector>

template <typename T>
class CircularBuffer {
public:
    CircularBuffer(size_t size) : buffer(size), start(0), end(0), count(0) {}

    void write(const T& item) {
        buffer[end] = item;
        if (count == buffer.size()) {
            start = (start + 1) % buffer.size();
        } else {
            ++count;
        }
        end = (end + 1) % buffer.size();
    }

    

    T read() {
        if (count == 0) {
            throw std::out_of_range("buffer is empty");
        }
        T result = buffer[start];
        start = (start + 1) % buffer.size();
        --count;
        return result;
    }

    size_t size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }

    bool full() const {
        return count == buffer.size();
    }

private:
    std::vector<T> buffer;
    size_t start;
    size_t end;
    size_t count;
};