#pragma once
// AI generated code

#include <sstream>

template< class Iterable>
class BinaryStream {
public:
    BinaryStream(Iterable& data) : m_data(data), m_pos(0) , m_oper_fails(false){}

    bool is_open() { return true;}

    bool fail(){
        return m_oper_fails;
    }

    bool write(const char* data, size_t size) {
        if ( size + m_pos >= m_data.size() ){
            m_oper_fails = true;
            return false;
        }
        memcpy(m_data.begin() + m_pos , data,size );
        return true;
    }

    size_t size() const {
        return m_data.size();
    }

    void* data() const {
        return const_cast<void*>(static_cast<const void*>(&m_data[0]));
    }

    template<typename T>
    BinaryStream& operator<<(const T& value) {
        write(&value, sizeof(value));
        return *this;
    }

    template<typename T>
    BinaryStream& operator>>(T& value) {
        read(&value, sizeof(value));
        return *this;
    }

    void read(char* data, size_t size) noexcept {
        if (m_pos + size > m_data.size()) {
            m_oper_fails = false;
            //throw std::runtime_error("Attempted to read past the end of the stream");
        }
        memcpy(data, &m_data[m_pos], size);
        m_pos += size;
    }

    void seek(size_t pos) {
        if (pos > m_data.size()) {
            throw std::runtime_error("Attempted to seek past the end of the stream");
        }
        m_pos = pos;
    }

    size_t tell() const {
        return m_pos;
    }

    void clear() {
        m_data.clear();
        m_pos = 0;
    }

    bool eof()
    {
       return m_pos >= m_data.size();
    }

private:
    Iterable& m_data;
    size_t m_pos;
    bool m_oper_fails;
};


