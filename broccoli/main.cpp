#include <stdio.h>

#include <iostream>
#include <string>

using namespace std;

static int write_string (char *ptr, size_t size, size_t nmemb, void *userdata) {
    string* s = (string*) userdata;
    size_t length = size * nmemb;
    s->append((char*) ptr, length);
    return length;
}

class CurlDataCollector {
public:
    CurlDataCollector(char* buffer, size_t length) : fBuffer(buffer), fLength(length), fWritten(0) {}
    
    size_t Write (char *ptr, size_t size) {
        size_t remaining = fLength - fWritten;

        if (size < remaining) {
            cerr << "more data than expected" << endl;
            return 0;
        }

        char* dest = fBuffer + fWritten;
        ::memmove (dest, ptr, size);
        fWritten += size;

        return size;
    }
    
    static size_t write_data (char *ptr, size_t size, size_t nmemb, void *instance) {
        return static_cast<CurlDataCollector*>(instance)->Write(ptr, size*nmemb);
    }
    
    char* fBuffer;
    size_t fLength;
    size_t fWritten;
};


#include <curl/curl.h>

static CURLcode curl_code = curl_global_init(CURL_GLOBAL_ALL);

class CURLResource {
public:
    CURLResource(const string& url) : fURL(url) {
        fCurl = curl_easy_init();
        curl_easy_setopt(fCurl, CURLOPT_URL, fURL.c_str());
    }
 
    ~CURLResource() {
        curl_easy_cleanup(fCurl);
    }

    void curl_reset() {
        curl_easy_reset(fCurl);
        curl_easy_setopt(fCurl, CURLOPT_URL, fURL.c_str());
    }
    
    string Get() {
        return Read(0, Length());
    }
    
    string Read (size_t inPosition, size_t inLength) {
        curl_reset();
        
        char range[256]; range[0] = '\0';
        sprintf(range, "%ld-%ld", inPosition, inPosition+inLength-1);
        curl_easy_setopt(fCurl, CURLOPT_RANGE, range);
        
        string response;
        curl_easy_setopt(fCurl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(fCurl, CURLOPT_WRITEFUNCTION, write_string);
        
        CURLcode res = curl_easy_perform(fCurl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
       
        return response;
    }
    
    size_t Read (size_t inPosition, size_t inLength, char* inPtr) {
        curl_reset();
        
        char range[256]; range[0] = '\0';
        sprintf(range, "%ld-%ld", inPosition, inPosition+inLength-1);
        curl_easy_setopt(fCurl, CURLOPT_RANGE, range);
        
        CurlDataCollector cd (inPtr, inLength);
        curl_easy_setopt(fCurl, CURLOPT_WRITEDATA, &cd);
        curl_easy_setopt(fCurl, CURLOPT_WRITEFUNCTION, CurlDataCollector::write_data);
        
        CURLcode res = curl_easy_perform(fCurl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        return cd.fWritten;
    }

    
    size_t Length () {
        curl_reset();
        
        size_t length = 0;
        curl_easy_setopt(fCurl, CURLOPT_NOBODY, 1L);
        CURLcode res = curl_easy_perform(fCurl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        double cl;
        res = curl_easy_getinfo(fCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);
        if(!res) {
            length = cl;
        }
        
        return length;
    }
    
private:
    string fURL;
    CURL* fCurl;
};

#include <unistd.h>

#if defined(__cplusplus)
extern "C"
#endif
void* TestResource (void*) {
    CURLResource resource("https://gist.githubusercontent.com/willie/e5b5ffdc5f106ebc2ca5/raw/0f18c778e3c4b1762421a252628f9ef096321e3c/gistfile1.txt");
    
    size_t length = resource.Length();
    cout << length << endl;
    
    string resp;
    
    resp = resource.Read(length-20, 20);
//    cout << resp << endl;

    sleep(1);
    
    resp = resource.Read(length-500, 20);
//    cout << resp << endl;
    
    resp = resource.Get();
    
    char buffer[256];
    size_t read = resource.Read(length-500, 20, buffer);
    
    cout << string(buffer, read) << endl;
    
    return NULL;
}

#define NUM_THREADS 50
#include <pthread.h>

int main() {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, &TestResource, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        void* result;
        pthread_join(threads[i], &result);
    }
    
    return 0;
}
