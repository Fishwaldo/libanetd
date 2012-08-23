#include "http_response.h"

http_response::http_response()
{
	this->reset();
}

http_response::~http_response()
{
}


void http_response::reset()
{
	this->version = "HTTP/1.0";
	this->status = 0;
	this->description = "OK";
	this->headers.clear();
	this->body_size = 0;
	this->body = "";
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

void http_response::setBodySize(int size) {
    this->body_size = size;
}

int http_response::getBodySize() {
    return this->body_size;
}

void http_response::setBody(std::string mybod) {
    this->body += mybod;
}

void http_response::setBodyAppend(char c) {
    this->body += c;
}
std::string http_response::getBody() {
    return this->body;
}
