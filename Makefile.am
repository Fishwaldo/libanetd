bin_PROGRAMS = httpclient
httpclient_SOURCES = httpclient.cpp	http_request.cpp	http_response.cpp	transmitter.cpp
httpclient_CXXFLAGS = -I/opt/local/include -I. -O0
httpclient_LDFLAGS = -L/opt/local/lib/ -lboost_date_time-mt -lboost_serialization-mt -lboost_thread-mt -lboost_regex-mt -lboost_system-mt -lssl -lcrypto -pthread