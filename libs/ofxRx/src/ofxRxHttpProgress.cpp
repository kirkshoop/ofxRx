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


#include <ofxRxHttp.h>
#include "Poco/Buffer.h"
#include "ofx/IO/ByteBuffer.h"
#include "ofx/IO/ByteBufferUtils.h"


namespace ofx {

namespace rx {

namespace detail {
HttpProgressState::HttpProgressState(std::shared_ptr<HTTP::BaseRequest> request,
                    HTTP::BaseResponse* response,
                    HTTP::Context* context):
    DefaultClient(),
    _request(request),
    _response(response),
    _context(context),
    dest_request(sub_request.get_subscriber().as_dynamic()),
    dest_response(sub_response.get_subscriber().as_dynamic()),
    dest_body(sub_body.get_subscriber().as_dynamic())
{
}


HttpProgressState::~HttpProgressState()
{
    delete _response;
    delete _context;
}


void HttpProgressState::submit()
{
    registerClientEvents(this);
    registerClientProgressEvents(this);
    registerClientFilterEvents(this);

    HTTP::DefaultClient::submit(*_request, *_response, *_context);

    unregisterClientFilterEvents(this);
    unregisterClientProgressEvents(this);
    unregisterClientEvents(this);

    dest_request.on_completed();
    dest_response.on_completed();
    dest_body.on_completed();
}


bool HttpProgressState::onHTTPClientResponseEvent(HTTP::ClientResponseEventArgs& args)
{
    const std::size_t bufferSize = IO::ByteBufferUtils::DEFAULT_BUFFER_SIZE;

    std::istream& istr = args.getResponseStream();

    std::streamsize contentLength = args.getResponse().getContentLength();

    _byteBuffer.reserve(contentLength);

    Poco::Buffer<char> buffer(bufferSize);
    std::streamsize len = 0;
	istr.read(buffer.begin(), bufferSize);
    std::streamsize n = istr.gcount();
    while (n > 0)
	{
		len += n;
        _byteBuffer.writeBytes(reinterpret_cast<uint8_t*>(buffer.begin()), n);

        // Check for task cancellation.
        if (istr && dest_body.is_subscribed())
		{
			istr.read(buffer.begin(), bufferSize);
            n = istr.gcount();
		}
        else
        {
            n = 0;
        }
	}

    // Don't return cancelled data.
    if (dest_body.is_subscribed())
    {
        HTTP::ClientResponseBufferEventArgs bufferEvent(_byteBuffer,
                                                  args.getRequest(),
                                                  args.getResponse(),
                                                  args.getContextRef());
        dest_body.on_next(bufferEvent);
        dest_body.on_completed();
    }

    return true;
}


bool HttpProgressState::onHTTPClientErrorEvent(HTTP::ClientErrorEventArgs& args)
{
    try {
        throw args.getException();
    } catch(...) {
        auto ex = std::current_exception();
        dest_request.on_error(ex);
        dest_response.on_error(ex);
        dest_body.on_error(ex);
    }
    return true;
}


bool HttpProgressState::onHTTPClientRequestProgress(HTTP::ClientRequestProgressArgs& args)
{
    dest_request.on_next(args);
    if (args.getContentLength() == args.getTotalBytesTransferred()) {
        dest_request.on_completed();
    }
    return true;
}


bool HttpProgressState::onHTTPClientResponseProgress(HTTP::ClientResponseProgressArgs& args)
{
    dest_response.on_next(args);
    if (args.getContentLength() == args.getTotalBytesTransferred()) {
        dest_response.on_completed();
    }
    return true;
}


bool HttpProgressState::onHTTPClientRequestFilterEvent(HTTP::MutableClientRequestArgs& args)
{
    return true;
}


bool HttpProgressState::onHTTPClientResponseFilterEvent(HTTP::MutableClientResponseArgs& args)
{
    return true;
}

}

}

}
