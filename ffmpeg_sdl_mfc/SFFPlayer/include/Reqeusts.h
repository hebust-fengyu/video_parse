#ifndef REQUESTS_H
#define REQUESTS_H
#ifndef  _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif // ! _WINSOCK_DEPRECA

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // 


#include<string>


extern "C"{

    #include"ssl.h"
    #include"err.h"
    #include"e_os2.h"

}

#include<WinSock2.h>





class Requests{
    public:
        Requests();
        Requests(std::string url, int port);
        ~Requests();
		char* Get(std::string url);
		char* Get();
		char* Post(std::string url, std::string data);
		char* Post(std::string data);
        void SetHeaders(std::string header);
		void SetReConnectionTimes(int val);




        


    //private:
        void Init();
		char* _get();
		char* _get_s();
		char* _post();
		char* _post_s();
		char* _Method();
		char* _Method_SSL();
        void _init_headers();
        void _init_socket();
		void HTTPParse(char* c);


    public:
        
		unsigned long response_header_size;
		unsigned long response_content_size;
		unsigned long long total_length;
		

    //private:
        std::string url;
        std::string headers;
		unsigned long long cp_index;

        int port;
        WSADATA ws;
        SOCKET sk;
        sockaddr_in sa;
        std::string data;
        std::string method;
        char Host[BUFSIZ];
        char pResource[BUFSIZ];
        std::string post_data;
        std::string send_header_info;
        SSL *ssl_handle;
        SSL_CTX *sslContent;
		char* respond_text;
		int re_connection_times;
		char* pre_pointer;
		char* after_pointer;
		//unsigned long long ssl_recv_count;


        
};





#endif
