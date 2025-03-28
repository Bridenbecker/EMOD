
#include "stdafx.h"
#include "JsonFullWriter.h"
#include "Configure.h"

namespace Kernel
{
    JsonFullWriter::JsonFullWriter(bool object_by_default, bool use_full_precision)
        : m_buffer(new rapidjson::StringBuffer())
        , m_writer(new rapidjson::Writer<rapidjson::StringBuffer>( *m_buffer, use_full_precision ))
        , m_closed(false)
        , m_object_by_default(object_by_default)
    {
        if (m_object_by_default)
        {
            m_writer->StartObject();
        }
    }

    IArchive& JsonFullWriter::startClass(std::string& class_name)
    {
        m_writer->StartObject();
        m_writer->String("__class__");
        m_writer->String(class_name.c_str());
        return *this;
    }

    IArchive& JsonFullWriter::endClass()
    {
        m_writer->EndObject();
        return *this;
    }

    IArchive& JsonFullWriter::startObject()
    {
        m_writer->StartObject();
        return *this;
    }

    IArchive& JsonFullWriter::endObject()
    {
        m_writer->EndObject();
        return *this;
    }

    IArchive& JsonFullWriter::startArray(size_t& count)
    {
        m_writer->StartArray();
        return *this;
    }

    IArchive& JsonFullWriter::endArray()
    {
        m_writer->EndArray();
        return *this;
    }

    IArchive& JsonFullWriter::labelElement(const char* label)
    {
        m_writer->String(label);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(bool& b)
    {
        m_writer->Bool(b);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(unsigned char& uc)
    {
        m_writer->Uint(uc);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(int32_t& i32)
    {
        m_writer->Int(i32);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(int64_t& i64)
    {
        m_writer->Int64(i64);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(uint32_t& u32)
    {
        m_writer->Uint(u32);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(uint64_t& u64)
    {
        m_writer->Uint64(u64);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(float& f)
    {
        m_writer->Double(double(f));
        return *this;
    }

    IArchive& JsonFullWriter::operator&(double& d)
    {
        m_writer->Double(d);
        return *this;
    }

    IArchive& JsonFullWriter::operator&(std::string& s)
    {
        m_writer->String(s.c_str(), rapidjson::SizeType(s.size()), true);
        return *this;
    }

    IArchive& JsonFullWriter::operator&( jsonConfigurable::ConstrainedString& cs )
    {
        std::string tmp(cs);
        this->operator&( tmp );
        return *this;
    }

    bool JsonFullWriter::HasError() { return false; }

    bool JsonFullWriter::IsWriter() { return true; }

    size_t JsonFullWriter::GetBufferSize()
    {
        GetBuffer(); // RapidJson will ensure that there's a terminating null ('\0')
        return m_buffer->Size();
    }

    const char* JsonFullWriter::GetBuffer()
    {
        if (!m_closed)
        {
            if (m_object_by_default)
            {
                m_writer->EndObject();
            }
            m_closed = true;
        }

        return m_buffer->GetString();
    }

    JsonFullWriter::~JsonFullWriter()
    {
        delete m_buffer;
        delete m_writer;
    }
}
