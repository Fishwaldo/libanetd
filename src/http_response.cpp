/*
 * HTTP Response Base class for libanetd
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
#include "anetd/anetdConfig.h"
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "anetd/http_engine.hpp"
#include "anetd/http_response.hpp"


using namespace DynamX::anetd;
using namespace DynamX::anetd::Logging;


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
	this->url = "";
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
void http_response::flush() {
	/* nothing in the base class */
}
void http_response::completed() {

}
std::string http_response::getURL() {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	return this->url;
}
void http_response::setURL(std::string url) {
	boost::interprocess::scoped_lock<boost::mutex>(this->TLock);
	this->url = url;
}

bool http_response::setHeader(std::string name, std::string value) {
	if (this->sendheaders.find(name) == this->sendheaders.end())
		return false;
	std::pair<std::map<std::string, std::string>::iterator, bool> ret =
			this->sendheaders.insert(
					std::pair<std::string, std::string>(name, value));
	return ret.second;
}
bool http_response::setHTTPAuth(std::string username, std::string password) {
	this->httpauth.first = username;
	this->httpauth.second = password;
	return true;
}


http_response_file::http_response_file() : http_response() {
	this->reset();
}

void http_response_file::reset() {
	this->filename = "";
	this->CloseFile();
	http_response::reset();
	std::cout << "Reset" << std::endl;
}

void http_response_file::setURL(std::string url) {
	std::vector<std::string> url_parts;
	http_response::setURL(url);
	boost::regex url_expression(
		// protocol            host               port
		"^(\?:([^:/\?#]+)://)\?(\\w+[^/\?#:]*)(\?::(\\d+))\?"
		// path                file       parameters
		"(/\?(\?:[^\?#/]*/)*)\?([^\?#]*)\?(\\\?(.*))\?"
		);
	boost::regex_split(std::back_inserter(url_parts), url, url_expression);
	if (url_parts.size() >= 5) {
    	    this->filename = url_parts[4];
	} else if (url_parts.size() == 4) {
	    this->filename = url_parts[3];
        }
	LogTrace() << "Filename set to:" << this->filename;
	//this->CloseFile();
}

void http_response_file::flush() {
	if (!this->OpenFile()) {
		return;
	}
	this->file << this->getBody();
	this->file.flush();
	this->body.clear();
}

bool http_response_file::OpenFile() {
	int i = 1;
	if (this->opened)
		return true;

	this->filepath = this->filename;
	while (boost::filesystem::exists(this->filepath)) {
		this->filepath = this->filename + "." + boost::lexical_cast<std::string>(static_cast<int>(i));
		i++;
	}
	this->file.clear(std::iostream::goodbit);
	this->file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try {
//		this->file.open(this->filepath, std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::app);
		this->file.open(this->filepath);
	} catch (std::ios_base::failure e) {
	    std::cout << "Exception opening/reading file" << e.what() << std::endl;;
	}
	if (this->file.is_open()) {
		this->opened = true;
		return true;
	}
	LogWarn() << "Could Not Open File: " << this->filepath;
	return false;
}
bool http_response_file::CloseFile() {
	this->opened = false;
	this->filepath.clear();
	this->filename = "";
	if (this->file.is_open()) this->file.close();
	return true;
}

void http_response_file::completed() {
	this->CloseFile();
	http_response::completed();
}

