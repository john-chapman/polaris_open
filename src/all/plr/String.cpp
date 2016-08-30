////////////////////////////////////////////////////////////////////////////////
// Modified from https://github.com/ocornut/str
// This software is distributed freely under the terms of the MIT License.
// See http://opensource.org/licenses/MIT
////////////////////////////////////////////////////////////////////////////////
#include <plr/String.h>

#include <cctype>  // tolower, toupper
#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vsnprintf
#include <cstdlib>
#include <cstring>

#ifndef va_copy
	#define va_copy(_dst, _src) (_dst = _src)
#endif

using namespace plr;

void plr::swap(StringBase& _a, StringBase& _b)
{
	bool bothHeap  = !_a.isLocal() && !_b.isLocal();
	if (_a.m_capacity == _b.m_capacity) {
		if (bothHeap) {
		 // neither are local, simple swap
			std::swap(_a.m_buf, _b.m_buf);
		} else {
		 // swap the buffer contents
			for (uint i = 0; i < _a.m_capacity; ++i) {
				std::swap(_a.m_buf[i], _b.m_buf[i]);
			}
		}
	} else {
		bool bothLocal = _a.isLocal() && _b.isLocal();

		if (bothHeap) {
		 // neither are local, simple swap
			std::swap(_a.m_buf, _b.m_buf);
			std::swap(_a.m_capacity, _b.m_capacity);

		} else if (bothLocal) {
		 // both are local, need to alloc the smaller of the two
			StringBase& smaller = _a.m_capacity < _b.m_capacity ? _a : _b;
			StringBase& larger  = _a.m_capacity < _b.m_capacity ? _b : _a;

			char* buf = (char*)malloc(larger.m_capacity * sizeof(char));
			memcpy(buf, larger.m_buf, larger.m_capacity * sizeof(char));
			memcpy(larger.m_buf, smaller.m_buf, smaller.m_capacity * sizeof(char));

			smaller.m_buf = buf;
			std::swap(_a.m_capacity, _b.m_capacity);

		} else {
		 // one is heap, alloc the other and swap (can't return to being local)
			StringBase& local = _a.isLocal() ? _a : _b;

			char* buf = (char*)malloc(local.m_capacity * sizeof(char));
			memcpy(buf, local.m_buf, local.m_capacity * sizeof(char));
			local.m_buf = buf;

			std::swap(_a.m_buf, _b.m_buf);
			std::swap(_a.m_capacity, _b.m_capacity);
		}
	}
}

static char* g_emptyString = (char*)"\0NULL";

// PUBLIC

uint StringBase::set(const char* _src, uint _count)
{
	uint len = strlen(_src);
	len = (_count == 0u) ? len : PLR_MIN(len, _count);
	len += 1u;
	if (m_capacity < len) {
		alloc(len);
	}
	len -= 1u;
	strncpy(m_buf, _src, len);
	m_buf[len] = '\0';
	return len;
}

uint StringBase::setf(const char* _fmt, ...)
{
	va_list args;
	va_start(args, _fmt);
	uint ret = setfv(_fmt, args);
	va_end(args);
	return ret;
}

uint StringBase::setfv(const char* _fmt, va_list _args)
{
	va_list args;
	va_copy(args, _args);

#ifdef PLR_COMPILER_MSVC
 // vsnprintf returns -1 on overflow, forces use to always do 2 passes
	int len = vsnprintf(0, 0u, _fmt, args);
	PLR_ASSERT(len >= 0);
	len += 1;
	if (m_capacity < len) {
		alloc(len);
	}
	PLR_VERIFY(vsnprintf(m_buf, m_capacity, _fmt, args) >= 0);
#else
	int len = vsnprintf(m_isOwned ? m_buf : 0, m_capacity, _fmt, args);
	PLR_ASSERT(len >= 0);
	len += 1;
	if (m_capacity < len) {
		alloc(len);
		PLR_VERIFY(vsnprintf(m_buf, m_capacity, _fmt, args) >= 0);
	}
#endif
	
	return (uint)len - 1u;
}

uint StringBase::append(const char* _src, uint _count)
{
	uint len = getLength();
	uint srclen = strlen(_src);
	srclen = _count == 0u ? srclen : PLR_MIN(srclen, _count);
	srclen += 1u;
	if (m_capacity < len + srclen) {
		realloc(len + srclen);
	}
	srclen -= 1u;
	strncpy(m_buf + len, _src, srclen);
	len += srclen;
	m_buf[len] = '\0';
	return len;
}

uint StringBase::appendf(const char* _fmt, ...)
{
	va_list args;
	va_start(args, _fmt);
	uint ret = appendfv(_fmt, args);
	va_end(args);
	return ret;
}

uint StringBase::appendfv(const char* _fmt, va_list _args)
{
	va_list args;
	va_copy(args, _args);

	uint len = getLength();
	int srclen = vsnprintf(0, 0u, _fmt, args);
	PLR_ASSERT(srclen > 0);
	srclen += 1;
	if (m_capacity < len + srclen) {
		realloc(len + srclen);
	}
	PLR_VERIFY(vsnprintf(m_buf + len, (uint)srclen, _fmt, args) >= 0);
	
	return (uint)srclen + len - 1u;;
}

const char* StringBase::findFirst(const char* _list) const
{
	const char* ret = strpbrk(m_buf, _list);
	return ret ? ret : m_buf;
}
const char* StringBase::findLast(const char* _list) const
{
 // rather than call strlen (which has to search the whole string anyway) we just make
 // several calls to strpbrk
	const char* ret = 0;
	const char* tmp = m_buf;
	do {
		tmp = strpbrk(tmp + 1, _list);
		ret = tmp == 0 ? ret : tmp;
	} while (tmp != 0);
	return ret;
}

void StringBase::replace(char _find, char _replace)
{
	char* tmp = m_buf;
	for (; *tmp != '\0'; ++tmp) {
		if (*tmp == _find) {
			*tmp = _replace;
		}
	}
}

void StringBase::toLowerCase()
{
	char* tmp = m_buf;
	for (; *tmp != '\0'; ++tmp) {
		*tmp = (char)tolower((int)*tmp);
	}
}

void StringBase::toUpperCase()
{
	char* tmp = m_buf;
	for (; *tmp != '\0'; ++tmp) {
		*tmp = (char)toupper((int)*tmp);
	}
}

uint StringBase::getLength() const
{
	return strlen(m_buf);
}

bool StringBase::operator==(const char* _rhs) const
{
	return strcmp(_rhs, m_buf) == 0;
}

// PROTECTED

StringBase::StringBase()
	: m_buf(g_emptyString)
	, m_capacity(0u)
{
}

StringBase::StringBase(uint _localBufferSize)
	: m_buf(getLocalBuf())
	, m_capacity(_localBufferSize)
{
	m_buf[0] = '\0';
}

StringBase::StringBase(StringBase&& _rhs)
	: m_buf(getLocalBuf())
	, m_capacity(_rhs.m_capacity)
{
	swap(_rhs, *this);
}

StringBase::~StringBase()
{
	if (!isLocal() && !(m_buf == g_emptyString)) {
		free(m_buf);
	}
}

// PRIVATE

void StringBase::alloc(uint _capacity)
{
	if (!isLocal() && !(m_buf == g_emptyString)) {
		free(m_buf);
	}
	m_buf = (char*)malloc(_capacity * sizeof(char));
	m_capacity = _capacity;
}

void StringBase::realloc(uint _capacity)
{
	/*if (_capacity < m_localBufSize) {
		if (!isLocal()) {
			strncpy(getLocalBuf(), m_buf, m_localBufSize);
		}
		m_buf = getLocalBuf();
		m_capacity = m_localBufSize;

	} else */if (isLocal() || m_buf == g_emptyString) {
		m_buf = (char*)malloc(_capacity * sizeof(char));
		strncpy(m_buf, getLocalBuf(), _capacity);
		m_capacity = _capacity;
	
	} else {
		m_buf = (char*)::realloc(m_buf, _capacity * sizeof(char));
		m_capacity = _capacity;
	}
}