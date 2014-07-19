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

#include "Poco/MemoryPool.h"
#include "Poco/Buffer.h"
#include "ofx/HTTP/ClientEvents.h"
#include "ofx/HTTP/DefaultClient.h"

namespace ofx {

namespace rx {

template<class T>
class BufferRef : public HTTP::BaseClientResponseArgs
{
    struct counted_buffer
    {
        std::atomic<int> ref;
        T buffy[1];
    };
    std::shared_ptr<Poco::MemoryPool> pool;
    counted_buffer* counted;
    size_t count;
public:
    static const size_t overhead_size = sizeof(counted_buffer) - sizeof(T);
    typedef T value_type;
    typedef T* iterator;
    ~BufferRef()
    {
        if (counted && counted->ref-- == 0) {
            pool->release(reinterpret_cast<void*>(counted));
        }
        counted = nullptr;
    }
    BufferRef(std::shared_ptr<Poco::MemoryPool> p,
              size_t byteCount,
              HTTP::BaseClientResponseArgs o):
        HTTP::BaseClientResponseArgs(o),
        counted(reinterpret_cast<counted_buffer*>(p->get())),
        count((byteCount - overhead_size) / sizeof(T))
    {
        counted->ref = 1;
        assert((count*sizeof(T))+overhead_size == byteCount);
    }
    BufferRef(const BufferRef<T>& o):
        HTTP::BaseClientResponseArgs(o),
        pool(o.pool),
        counted(o.counted),
        count(o.count)
    {
        counted->ref++;
    }
    BufferRef(BufferRef<T>&& o):
        HTTP::BaseClientResponseArgs(std::move(o)),
        pool(std::move(o.pool)),
        counted(std::move(o.counted)),
        count(std::move(o.count))
    {
        o.counted = nullptr;
        o.count = 0;
    }
    BufferRef<T>& operator=(BufferRef<T> o)
    {
        using std::swap;
        swap(pool, o.pool);
        swap(counted, o.counted);
        swap(count, o.count);
    }
    iterator begin() {return counted->buffy;}
    iterator end() {return counted->buffy+count;}
    size_t size() {return count;}
    void resize(size_t c) {assert(c <= count);}
};

namespace detail {

    class HttpProgressState: public HTTP::DefaultClient, public std::enable_shared_from_this<HttpProgressState>
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
    rx::subjects::subject<BufferRef<char>> sub_body;

private:
    const std::size_t bufferSize = IO::ByteBufferUtils::DEFAULT_BUFFER_SIZE;

    std::shared_ptr<HTTP::BaseRequest> _request;
    HTTP::BaseResponse* _response;
    HTTP::Context* _context;
    
    std::shared_ptr<Poco::MemoryPool> pool;

    rx::subscriber<HTTP::ClientRequestProgressArgs> dest_request;
    rx::subscriber<HTTP::ClientResponseProgressArgs> dest_response;
    rx::subscriber<BufferRef<char>> dest_body;

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
    inline rx::observable<BufferRef<char>> body() const {
        return state->sub_body.get_observable();
    }

private:
    mutable std::shared_ptr<detail::HttpProgressState> state;
};



}

}
