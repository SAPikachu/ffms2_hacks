//  Copyright (c) 2014 Thomas Goyne <tgoyne@gmail.com>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "filehandle.h"

#include "utils.h"

#include <cstdarg>
#include <cstdio>

static AVIOContext *ffms_fopen(const char *filename, const char *mode) {
	AVIOContext *ctx;
	std::string smode(mode);
	int flags = 0;
	int ret;

	if (smode.find('r') != std::string::npos)
		flags |= AVIO_FLAG_READ;
	if (smode.find('w') != std::string::npos)
		flags |= AVIO_FLAG_WRITE;

#ifdef FFMBC
	ret = avio_open(&ctx, filename, flags);
#else
	ret = avio_open2(&ctx, filename, flags, NULL, NULL);
#endif
	if (ret < 0)
		return NULL;

	return ctx;
}

FileHandle::FileHandle(const char *filename, const char *mode, int error_source, int error_cause)
: avio(ffms_fopen(filename, mode))
, filename(filename)
, error_source(error_source)
, error_cause(error_cause)
{
	if (!avio)
		throw FFMS_Exception(error_source, FFMS_ERROR_NO_FILE,
			"Failed to open '" + this->filename + "'");
}

void FileHandle::Seek(int64_t offset, int origin) {
	int64_t ret = avio_seek(avio, offset, origin);
	if (ret < 0)
		throw FFMS_Exception(error_source, error_cause,
			"Failed to seek in '" + filename + "'");
}

int64_t FileHandle::Tell() {
	int64_t ret = avio_tell(avio);
	if (ret < 0)
		throw FFMS_Exception(error_source, error_cause,
			"Failed to read position in '" + filename + "'");
	return ret;
}

size_t FileHandle::Read(char *buffer, size_t size) {
	int count = avio_read(avio, (unsigned char *)buffer, size);
	if (count < 0)
		throw FFMS_Exception(error_source, FFMS_ERROR_FILE_READ,
			"Failed to read from '" + filename + "'");
	return (size_t)count;
}

size_t FileHandle::Write(const char *buffer, size_t size) {
	avio_write(avio, (const unsigned char *)buffer, size);
	avio_flush(avio);
	if (avio->error < 0)
		throw FFMS_Exception(error_source, FFMS_ERROR_FILE_WRITE,
			"Failed to write to '" + filename + "'");
	return size;
}

int64_t FileHandle::Size() {
	int64_t size = avio_size(avio);
	if (size < 0)
		throw FFMS_Exception(error_source, FFMS_ERROR_FILE_READ,
			"Failed to get file size for '" + filename +"'");
	return size;
}

int FileHandle::Printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	std::vector<char> OutBuffer(100);
	int ret = -1;
	while (OutBuffer.size() < 1024 * 1024) {
		ret = vsnprintf(reinterpret_cast<char *>(&OutBuffer[0]), OutBuffer.size(), fmt, args);
		if (ret > 0 && ret < (int)OutBuffer.size())
			break;
		OutBuffer.resize(OutBuffer.size() * 2);
	}

	va_end(args);

	avio_write(avio, reinterpret_cast<const unsigned char *>(&OutBuffer[0]), ret);
	avio_flush(avio);

	return avio->error < 0 ? avio->error : ret;
}
