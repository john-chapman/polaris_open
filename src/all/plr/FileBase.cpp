////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 John Chapman -- http://john-chapman.net
// This software is distributed freely under the terms of the MIT License.
// See http://opensource.org/licenses/MIT
////////////////////////////////////////////////////////////////////////////////
#include <plr/File.h>

#include <cstdlib> // malloc, free
#include <cstring> // memcpy
#include <utility> // swap

using namespace plr;
using namespace internal;

// PROTECTED

void FileBase::setData(const char* _data, uint64 _size)
{
	PLR_ASSERT(_size > 0);
	if (m_data) {
		if (_size > m_dataSize) {
			free(m_data);
			m_data = 0;
		}
	}
	if (!m_data) {
		m_data = (char*)malloc(_size);
		PLR_ASSERT(m_data);
	}
	if (_data) {
		memcpy(m_data, _data, _size);
	}
	m_dataSize = _size;
}

void FileBase::appendData(const char* _data, uint64 _size)
{
	PLR_ASSERT(_size > 0);
	m_data = (char*)realloc(m_data, m_dataSize + _size);
	if (_data) {
		memcpy(m_data + m_dataSize, _data, _size);
	}
	m_dataSize += _size;
}


// PROTECTED

FileBase::FileBase()
	: m_path("")
	, m_data(0)
	, m_dataSize(0)
{
}

FileBase::~FileBase()
{
	if (m_data) {
		free(m_data);
	}
}

void FileBase::swap(FileBase& _file_)
{
	std::swap(m_path, _file_.m_path);
	std::swap(m_data, _file_.m_data);
	std::swap(m_dataSize, _file_.m_dataSize);
}