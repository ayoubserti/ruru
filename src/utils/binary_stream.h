#pragma once
// AI generated code

#include <sstream>

template< class Iterable>
class BinaryStream {
public:
    BinaryStream(Iterable& data) : m_data(data), m_pos(0) {}

    bool Write(const void* data, size_t size) {
        if ( size + m_pos >= m_data.size() )
            return false;
        const unsigned char* ptr = static_cast<const unsigned char*>(data);
        memcpy(m_data.begin() + m_pos , data,size );
    }

    size_t Size() const {
        return m_data.size();
    }

    void* Data() const {
        return const_cast<void*>(static_cast<const void*>(&m_data[0]));
    }

    template<typename T>
    BinaryStream& operator<<(const T& value) {
        Write(&value, sizeof(value));
        return *this;
    }

    template<typename T>
    BinaryStream& operator>>(T& value) {
        Read(&value, sizeof(value));
        return *this;
    }

    void Read(void* data, size_t size) {
        if (m_pos + size > m_data.size()) {
            throw std::runtime_error("Attempted to read past the end of the stream");
        }
        memcpy(data, &m_data[m_pos], size);
        m_pos += size;
    }

    void Seek(size_t pos) {
        if (pos > m_data.size()) {
            throw std::runtime_error("Attempted to seek past the end of the stream");
        }
        m_pos = pos;
    }

    size_t Tell() const {
        return m_pos;
    }

    void Clear() {
        m_data.clear();
        m_pos = 0;
    }

private:
    Iterable& m_data;
    size_t m_pos;
};


