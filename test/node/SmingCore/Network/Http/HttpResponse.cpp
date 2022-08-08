/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpResponse
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpResponse.h"
#include "../WebConstants.h"
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/JsonObjectStream.h"
#include "Data/Stream/FileStream.h"


HttpResponse* HttpResponse::setCookie(const String& name, const String& value, bool append)
{
	String s = name;
	s += '=';
	s += value;
	if(append) {
		headers.append(HTTP_HEADER_SET_COOKIE, s);
	} else {
		headers[HTTP_HEADER_SET_COOKIE] = s;
	}

	return this;
}

HttpResponse* HttpResponse::setCache(int maxAgeSeconds, bool isPublic)
{
	String cache = isPublic ? F("public") : F("private");
	cache += F(", max-age=");
	cache += maxAgeSeconds;
	cache += F(", must-revalidate");
	headers[HTTP_HEADER_CACHE_CONTROL] = cache;
	return this;
}

HttpResponse* HttpResponse::setAllowCrossDomainOrigin(const String& controlAllowOrigin)
{
	headers[HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN] = controlAllowOrigin;
	return this;
}

HttpResponse* HttpResponse::setHeader(const String& name, const String& value)
{
	headers[name] = value;
	return this;
}

bool HttpResponse::sendString(const String& text)
{
	auto memoryStream = new MemoryDataStream();
	if(memoryStream == nullptr) {
		return false;
	}

	setStream(memoryStream);

	return memoryStream->print(text) == text.length();
}

bool HttpResponse::sendString(String&& text) noexcept
{
	auto memoryStream = new MemoryDataStream(std::move(text));
	if(memoryStream == nullptr) {
		return false;
	}
	setStream(memoryStream);
	return true;
}

bool HttpResponse::hasHeader(const String& name)
{
	return headers.contains(name);
}

void HttpResponse::redirect(const String& location)
{
	headers[HTTP_HEADER_LOCATION] = location;
}

bool HttpResponse::sendFile(const String& fileName, bool allowGzipFileCheck)
{
	auto fs = new FileStream;

	if(allowGzipFileCheck) {
		String fnCompressed = fileName + _F(".gz");
		if(fs->open(fnCompressed)) {
			debug_d("found %s", fnCompressed.c_str());
			headers[HTTP_HEADER_CONTENT_ENCODING] = F("gzip");
			return sendDataStream(fs, ContentType::fromFullFileName(fileName));
		}
	}

	if(fs->open(fileName)) {
		debug_d("found %s", fileName.c_str());
		FileStat stat;
		fs->stat(stat);
		if(stat.compression.type == IFS::Compression::Type::GZip) {
			headers[HTTP_HEADER_CONTENT_ENCODING] = F("gzip");
		} else if(stat.compression.type != IFS::Compression::Type::None) {
			debug_e("Unsupported compression type: %s", ::toString(stat.compression.type).c_str());
		}
		return sendDataStream(fs, ContentType::fromFullFileName(fileName));
	}

	delete fs;
	code = HTTP_STATUS_NOT_FOUND;
	return false;
}

bool HttpResponse::sendNamedStream(IDataSourceStream* newDataStream)
{
	String contentType;
	if(newDataStream != nullptr && !headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		contentType = ContentType::fromFullFileName(newDataStream->getName());
	}

	return sendDataStream(newDataStream, contentType);
}

bool HttpResponse::sendTemplate(TemplateStream* newTemplateInstance)
{
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}

	stream = newTemplateInstance;
	if(!newTemplateInstance->isValid()) {
		code = HTTP_STATUS_NOT_FOUND;
		delete stream;
		stream = nullptr;
		return false;
	}

	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		String mime = ContentType::fromFullFileName(newTemplateInstance->getName());
		if(mime)
			setContentType(mime);
	}

	if(!headers.contains(HTTP_HEADER_TRANSFER_ENCODING) && stream->available() < 0) {
		headers[HTTP_HEADER_TRANSFER_ENCODING] = _F("chunked");
	}

	return true;
}

bool HttpResponse::sendJsonObject(JsonObjectStream* newJsonStreamInstance)
{
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}

	stream = newJsonStreamInstance;
	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		setContentType(MIME_JSON);
	}

	return true;
}

bool HttpResponse::sendDataStream(ReadWriteStream* newDataStream, const String& reqContentType)
{
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}
	if(reqContentType) {
		setContentType(reqContentType);
	}
	stream = newDataStream;

	return true;
}

String HttpResponse::getBody()
{
	if(stream == nullptr) {
		return "";
	}

	String ret;
	if(stream->available() != -1 && stream->getStreamType() == eSST_Memory) {
		MemoryDataStream* memory = (MemoryDataStream*)stream;
		char buf[1024];
		while(stream->available() > 0) {
			int available = memory->readMemoryBlock(buf, 1024);
			memory->seek(available);
			ret += String(buf, available);
			if(available < 1024) {
				break;
			}
		}
	}
	return ret;
}

void HttpResponse::reset()
{
	code = HTTP_STATUS_OK;
	headers.clear();
	freeStreams();
}

String HttpResponse::toString() const
{
	String content;
	content += F("HTTP/1.1 ");
	content += unsigned(code);
	content += ' ';
	content += ::toString(code);
	content += " \r\n";
	for(unsigned i = 0; i < headers.count(); i++) {
		content += headers[i];
	}
	content += "\r\n";

	return content;
}

void HttpResponse::freeStreams()
{
	// Consistency check
	if(buffer != nullptr) {
		if(buffer != stream) {
			debug_e("HttpResponse: buffer doesn't match stream");
			delete buffer;
		}
		buffer = nullptr;
	}

	delete stream;
	stream = nullptr;
}

void HttpResponse::setBuffer(ReadWriteStream* buffer)
{
	if(buffer == this->buffer) {
		return;
	}

	// Must set stream first
	setStream(buffer);
	// Now safe to set buffer
	this->buffer = buffer;
}

void HttpResponse::setStream(IDataSourceStream* stream)
{
	if(stream == this->stream) {
		return;
	}

	if(this->stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		freeStreams();
	}
	this->stream = stream;
}
