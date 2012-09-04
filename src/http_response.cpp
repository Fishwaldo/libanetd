/*
 * HTTP Response Base class for libHTTPClient
 * Copyright (C) 2012 Justin Hammond
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "HTTPClient/http_response.h"

#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace DynamX::HttpClient;

http_response::http_response(): body_size(0)
{
	this->reset();
}

http_response::~http_response()
{
}


void http_response::reset()
{
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	this->version = "HTTP/1.0";
	this->status = 0;
	this->description = "OK";
	this->headers.clear();
	this->body_size = 0;
	this->body = "";
	this->progress = 0;
}


void http_response::setVersion(std::string ver) {
    this->version = ver;
}

std::string http_response::getVersion() {
    return this->version;
}

void http_response::setStatus(int stat) {
    this->status = stat;
}

int http_response::getStatus() {
    return this->status;
}

void http_response::setDescription(std::string desc) {
    this->description = desc;
}
std::string http_response::getDescription() {
    return this->description;
}

void http_response::setHeaders(std::string key, std::string val) {
    this->headers.insert(std::pair<std::string, std::string>(key, val));
}

std::map<std::string, std::string>::iterator http_response::getHeadersBegin() {
    return this->headers.begin();
}

std::map<std::string, std::string>::iterator http_response::getHeadersEnd() {
    return this->headers.end();
}
std::string http_response::getHeader(std::string key) {
    std::map<std::string, std::string>::iterator iter = this->headers.find(key);
    if (iter != this->headers.end())
        return iter->second;
    return "";
}

void http_response::setBodySize(size_t size) {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
    this->body_size = size;
}

size_t http_response::getBodySize() {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	return this->body_size;
}

void http_response::setBody(std::string mybod) {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	this->body += mybod;
	this->progress += mybod.length();
}

void http_response::setBody(char c) {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	this->body += c;
	this->progress++;
}
std::string http_response::getBody() {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	return this->body;

}

size_t http_response::getProgress() {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	return this->progress;
}

