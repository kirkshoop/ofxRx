// =============================================================================
//
// Copyright (c) 2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#pragma once


#include "ofx/HTTP/ClientEvents.h"
#include "ofx/HTTP/DefaultClient.h"

namespace ofx {

namespace rx {

namespace detail {

class HttpProgressState: public HTTP::DefaultClient
{
public:
    ~HttpProgressState();
    HttpProgressState(std::shared_ptr<HTTP::BaseRequest> request,
                        HTTP::BaseResponse* response,
                        HTTP::Context* context);

    void submit();

    bool onHTTPClientResponseEvent(HTTP::ClientResponseEventArgs& args);
    bool onHTTPClientErrorEvent(HTTP::ClientErrorEventArgs& args);

    bool onHTTPClientRequestProgress(HTTP::ClientRequestProgressArgs& args);
    bool onHTTPClientResponseProgress(HTTP::ClientResponseProgressArgs& args);

    bool onHTTPClientRequestFilterEvent(HTTP::MutableClientRequestArgs& args);
    bool onHTTPClientResponseFilterEvent(HTTP::MutableClientResponseArgs& args);

    rx::subjects::subject<HTTP::ClientRequestProgressArgs> sub_request;
    rx::subjects::subject<HTTP::ClientResponseProgressArgs> sub_response;
    rx::subjects::subject<HTTP::ClientResponseBufferEventArgs> sub_body;

private:
    std::shared_ptr<HTTP::BaseRequest> _request;
    HTTP::BaseResponse* _response;
    HTTP::Context* _context;

    IO::ByteBuffer _byteBuffer;

    rx::subscriber<HTTP::ClientRequestProgressArgs> dest_request;
    rx::subscriber<HTTP::ClientResponseProgressArgs> dest_response;
    rx::subscriber<HTTP::ClientResponseBufferEventArgs> dest_body;

};
}

class HttpProgress
{
public:
    HttpProgress(std::shared_ptr<HTTP::BaseRequest> request,
                    HTTP::BaseResponse* response,
                    HTTP::Context* context)
        : state(std::make_shared<detail::HttpProgressState>(request, response, context))
    {
    }

    inline void submit() const {
        state->submit();
    }

    inline rx::observable<HTTP::ClientRequestProgressArgs> request() const {
        return state->sub_request.get_observable();
    }
    inline rx::observable<HTTP::ClientResponseProgressArgs> response() const {
        return state->sub_response.get_observable();
    }
    inline rx::observable<HTTP::ClientResponseBufferEventArgs> body() const {
        return state->sub_body.get_observable();
    }

private:
    mutable std::shared_ptr<detail::HttpProgressState> state;
};



}

}
