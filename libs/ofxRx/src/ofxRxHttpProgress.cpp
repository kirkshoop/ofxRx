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
    pool(std::make_shared<Poco::MemoryPool>(bufferSize + BufferRef<char>::overhead_size)),
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
}


bool HttpProgressState::onHTTPClientResponseEvent(HTTP::ClientResponseEventArgs& args)
{
    auto sc = rx::schedulers::make_current_thread();
    auto w = sc.create_worker(dest_body.get_subscription());
    auto keep = this->shared_from_this();
    w.schedule([=](const rx::schedulers::schedulable& self){
        std::istream& istr = args.getResponseStream();

        auto buffy = BufferRef<char>(keep->pool, keep->bufferSize + BufferRef<char>::overhead_size, args);
        std::streamsize len = 0;
        istr.read(buffy.begin(), keep->bufferSize);
        std::streamsize n = istr.gcount();

        if (n > 0) {
            buffy.resize(n);
            keep->dest_body.on_next(buffy);
            n = 0;
        }

        if (!istr.eof()) {

            if (istr) {
                // continue
                self();
                return;
            }

            if (!istr.good()) {
                std::runtime_error error("istream !good()");
                rx::observable<>::error<BufferRef<char>>(error).subscribe(dest_body);
            }
        }

        // finished

        unregisterClientFilterEvents(keep.get());
        unregisterClientProgressEvents(keep.get());
        unregisterClientEvents(keep.get());
        
        keep->dest_request.on_completed();
        keep->dest_response.on_completed();
        keep->dest_body.on_completed();
    });

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
