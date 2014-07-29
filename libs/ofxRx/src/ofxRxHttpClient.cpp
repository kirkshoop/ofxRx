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


HttpClient::HttpClient()
{
}


rx::observable<HttpProgress> HttpClient::get(const std::string& uri,
                                       const Poco::Net::NameValueCollection& formFields,
                                       const std::string& httpVersion,
                                       const Poco::UUID& requestId)
{
    return request(new HTTP::GetRequest(uri,
                                  formFields,
                                  httpVersion,
                                  requestId));
}


rx::observable<HttpProgress> HttpClient::post(const std::string& uri,
                                        const Poco::Net::NameValueCollection formFields,
                                        const HTTP::PostRequest::FormParts formParts,
                                        const std::string& httpVersion,
                                        const Poco::UUID& requestId)
{
    return request(new HTTP::PostRequest(uri,
                                  formFields,
                                  formParts,
                                  httpVersion,
                                  requestId));
}


rx::observable<HttpProgress> HttpClient::request(HTTP::BaseRequest* pRequest)
{
    std::shared_ptr<HTTP::BaseRequest> request(pRequest);
    return rx::observable<>::defer(
        [=](){
            HttpProgress progress(request,
                new HTTP::BaseResponse(),
                new HTTP::Context());
            return rx::sources::from(progress).
                lift<HttpProgress>([=](rx::subscriber<HttpProgress> dest){
                    // VS2013 deduction issue requires dynamic (type-forgetting)
                    return rx::make_subscriber<HttpProgress>(
                        dest,
                        [=](HttpProgress hp){
                            dest.on_next(hp);
                        },
                        [=](std::exception_ptr ex){
                            dest.on_error(ex);
                        },
                        [=](){
                            dest.on_completed();
                            progress.submit();
                        }).as_dynamic();
                }).as_dynamic();
        });
}

}

}

