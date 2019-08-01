
#ifndef  _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif // ! _WINSOCK_DEPRECA

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // 

#include"stdafx.h"
#include"include/Reqeusts.h"
#include"ConvertCode.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")

#include<winsock2.h>
#include<iostream>
#include<stdlib.h>
#include<Windows.h>
#include<exception>


#include<fstream>




//#define _CRT_SECURE_NO_WARNINGS

#define RCV_SIZE 1024 * 1024 * 10


#pragma region 


void Requests::SetReConnectionTimes(int val) {
	re_connection_times = val;
}



void Requests::HTTPParse(char* c) {
	response_content_size = 0;
	response_header_size = 0;
	pre_pointer = strstr(c, "\r\n\r\n");
	if (strstr(c, "html") == NULL) {
		response_header_size = (pre_pointer - c + 4);
		response_content_size = total_length - response_header_size;
		return;
	}

	int len = 0;
	
	response_header_size = pre_pointer - c + 2;
	pre_pointer += 2;
	after_pointer = pre_pointer;
	while ((sscanf(after_pointer, "\r\n%x\r\n", &len)) && len != 0) {

		after_pointer += 2;
		after_pointer = strstr(after_pointer, "\r\n");
		after_pointer += 2;

		response_content_size += len;
		memmove(pre_pointer, after_pointer, len);
		pre_pointer += len;
		after_pointer += len;
	}
	*pre_pointer = 0;



}


Requests::Requests():port(0){
	_init_headers();
    Init();
}

Requests::Requests(std::string url, int port): url(url), port(port){
	_init_headers();
    Init();
}


void Requests::_init_headers(){


	
	std::string head;

	head += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n"
		//"Cache-Control: max-age=0\r\n"
		"Connection: keep-alive\r\n"
		//"Upgrade-Insecure-Requests: 1\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/75.0.3770.142 Safari/537.36\r\n";
	
	
	/*
	head += "Accept-Encoding: gbk\r\n";
	head += "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/75.0.3770.142 Safari/537.36\r\n";
	head += "Connection: keep-alive\r\n";
	//head += ":path: /directory/all\r\n";
	head += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*//*;q=0.8,application/signed-exchange;v=b3\r\n";
		*/
	SetHeaders(head);
}



void Requests::Init(){

	response_content_size = 0;
	response_header_size = 0;
	total_length = 0;

	ssl_handle = nullptr;
	sslContent = nullptr;
	re_connection_times = 10;
	respond_text = (char*)malloc(sizeof(char) * RCV_SIZE);
	cp_index = 0;
	//respond_text = (char*)malloc(sizeof(char) * (1024*1024*10));
    WORD version = MAKEWORD(2,2);
    if(WSAStartup(version, &ws) != NO_ERROR){
        std::cout<<"wsastartup is failed,"<< __LINE__ <<std::endl;
        return;
    }
}



char* Requests::Get(std::string url){
	this->url = url;
	return Get();    
}


char* Requests::Get(){

	int times = 0;

	while (1) {
		try {
			post_data = "";



			if (url.find("https") != std::string::npos) {
				std::cout << "https link" << std::endl;
				port = 443;
				_get_s();
			}
			else if (url.find("http") != std::string::npos) {
				std::cout << "http link" << std::endl;
				port = 80;
				_get();
			}
			
			if (total_length == 0) {
				throw std::logic_error("recv 0 bytes");
			}
			HTTPParse(respond_text);
			
			return respond_text;

		}
		catch (std::exception e) {
			std::cout << "exception url :" << url << std::endl;
			std::cout << "exception description:" << e.what() << std::endl;
			Sleep(5000);
			++times;
			if (times > re_connection_times) {
				return (char*)"connect times over";
				
			}
		}
	}
			
    
	
}

char* Requests::_get(){
	method = "GET ";
    return _Method();
}

char* Requests::_get_s(){
	method = "GET ";
	return _Method_SSL();
}



char* Requests::Post(std::string url, std::string data){
    this->url = url;
	return Post(data);
    
}

char* Requests::Post(std::string data){
	int times = 0;
	while (1) {
		try {
			post_data = data;
			if (url.find("https") != std::string::npos) {

				port = 443;
				return _post_s();
			}
			else if (url.find("http") != std::string::npos) {
				port = 80;
				return _post();
			}
			else {
				return (char*)"post format error";
			}

		}
		catch (std::exception e) {
			std::cout << "exception url :" << url << std::endl;
			std::cout << "exception description:" << e.what() << std::endl;
			Sleep(5000);
			++times;
			if (times > re_connection_times) {
				return (char*)"connect times over";
				
			}
		}
	}

			

}

char* Requests::_post(){
	method = "POST ";
    return _Method();
}



char* Requests::_post_s(){
	method = "POST ";
	return _Method_SSL();
}




void Requests::_init_socket(){
	if(sk != INVALID_SOCKET){
		closesocket(sk);
	}
	sk = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sk == INVALID_SOCKET){
        std::cout<<"can't create socket,"<< __LINE__ <<std::endl;
		WSACleanup();
		throw std::exception(std::logic_error("can't create socket"));
    }

	struct hostent* ht = gethostbyname(Host);
    //memcpy(&sa.sin_addr, ht->h_addr_list[0], sizeof(sa.sin_addr));
	sa.sin_addr.S_un.S_addr = inet_addr( inet_ntoa(*(in_addr*)* ht->h_addr_list));
	//sa.sin_addr.S_un.S_addr = inet_addr("182.61.200.7");
	std::cout<<"ip_addr is:"<<inet_ntoa(*(in_addr*)*ht->h_addr_list)<<std::endl;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    int rcv_timeout = 3000;
    setsockopt(sk, SOL_SOCKET, SO_RCVTIMEO, (char*)&rcv_timeout, sizeof(rcv_timeout));

	
    if(connect(sk, (sockaddr*)&sa, sizeof(sa))){
        std::cout<<"connect error,"<< __LINE__ <<std::endl;
        closesocket(sk);
        WSACleanup();
        return ;
    }

}


char* Requests::_Method(){
	cp_index = 0;
	response_content_size = 0;
	response_header_size = 0;
	total_length = 0;
	memset(respond_text, 0, RCV_SIZE);
	std::cout<<"_Method"<<std::endl;

	const char* pos = strstr(url.c_str(), "http://");

	pos += strlen("http://");
	memset(Host, 0, BUFSIZ);
	memset(pResource, 0, BUFSIZ);
	sscanf(pos, "%[^/]%s", Host, pResource);


	_init_socket();
	std::cout<<"init__sock finished"<<std::endl;

	send_header_info.clear();
	send_header_info = method;
	
	send_header_info += pResource;
	send_header_info += " HTTP/1.1\r\n";
	send_header_info += "Host:";
	send_header_info += Host;
	send_header_info += "\r\n";
	send_header_info += "Referer:" + url + "\r\n";
	send_header_info += headers;
	send_header_info += "\r\n";
  
    int i = send(sk, send_header_info.c_str(), send_header_info.length(), 0);
	std::cout << "send msg is " << send_header_info << std::endl;
    if(!i){
        std::cout<<"send error,"<< __LINE__ <<std::endl;
        closesocket(sk);
        WSACleanup();
        throw std::exception(std::logic_error("send error"));
    }
	//char* buf = (char*)malloc(sizeof(char) * RCV_SIZE);
	//const unsigned long long int recv_size = 1024 * 1024 * 10;
	//char* buf = (char*)malloc(sizeof(char) * recv_size);
	char buf[BUFSIZ] = { 0 };
	int recv_count;
	//memset(buf, '\0', sizeof(char) * recv_size);
	
	strCoding sc;
	while((recv_count = recv(sk, buf, BUFSIZ - 1, 0)) >  (int)0){

		//std::string con(buf);
		//std::string urlud = sc.UrlUTF8Decode(con);
		//sc.UTF_8ToGB2312(con, buf, recv_count);


		total_length += recv_count;
		//buf[recv_count] = 0;
		//respond_text += buf;
		memcpy(respond_text+cp_index, buf, recv_count);
		cp_index += recv_count;

	} 


    return respond_text;
}



void Requests::SetHeaders(std::string headers){
	this->headers.clear();
	this->headers += headers;
}






char* Requests::_Method_SSL() {
	cp_index = 0;
	response_content_size = 0;
	response_header_size = 0;
	total_length = 0;
	
	
	memset(respond_text, 0, RCV_SIZE);
	const char* pos = strstr(url.c_str(), "https://");

	pos += strlen("https://");
	memset(Host, 0, BUFSIZ);
	memset(pResource, 0, BUFSIZ);
	sscanf(pos, "%[^/]%s", Host, pResource);




	_init_socket();
	send_header_info.clear();
	send_header_info += method;


	send_header_info += pResource;
	send_header_info += " HTTP/1.1\r\n";
	send_header_info += "Host:";
	send_header_info += Host;
	send_header_info += "\r\n";
	send_header_info += "Referer:" + url + "\r\n";
	send_header_info += headers;

	send_header_info += "\r\n";
	send_header_info += post_data;
	send_header_info += "\r\n";



	ERR_load_BIO_strings();

	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	sslContent = SSL_CTX_new(SSLv23_client_method());

	if (sslContent == nullptr) {
		std::cout << __LINE__ << std::endl;
		ERR_print_errors_fp(stderr);
		throw std::exception(std::logic_error("create ssl content failed!"));
	}

	ssl_handle = SSL_new(sslContent);
	if (ssl_handle == nullptr) {
		std::cout << __LINE__ << std::endl;
		ERR_print_errors_fp(stderr);
		throw std::exception(std::logic_error("create ssl_handle failed!"));
	}

	if (!SSL_set_fd(ssl_handle, sk)) {
		std::cout << __LINE__ << std::endl;
		ERR_print_errors_fp(stderr);
		throw std::exception(std::logic_error("ssl_set_fd failed!"));
	}

	if (SSL_connect(ssl_handle) != 1) {
		std::cout << __LINE__ << std::endl;
		//ERR_print_errors_fp(stderr);
		throw std::exception(std::logic_error("ssl_connect failed"));
	}

	send_header_info.clear();
	send_header_info = method;


	send_header_info += pResource;

	send_header_info += " HTTP/1.1\r\n";
	send_header_info += "Host:";
	send_header_info += Host;
	send_header_info += "\r\n";
	send_header_info += headers;

	send_header_info += "\r\n";
	send_header_info += post_data;
	send_header_info += "\r\n";
	std::cout << send_header_info << std::endl;



	int i = SSL_write(ssl_handle, send_header_info.c_str(), send_header_info.length());

	//int i = send(sk, get_info.c_str(), get_info.length(), 0);
	//std::cout << "send msg is " << get_info << std::endl;
	if (!i) {
		std::cout << "ssl send error," << __LINE__ << std::endl;
		closesocket(sk);
		WSACleanup();
		throw std::exception(std::logic_error("ssl send error"));
		
	}
	//char* buf = (char*)malloc(sizeof(char) * RCV_SIZE);
	char buf[BUFSIZ];
	int recv_count;
	memset(buf, '\0', sizeof(char) * BUFSIZ);



	while((recv_count = SSL_read(ssl_handle, buf, BUFSIZ - 1)) >  (int)0){
		total_length += recv_count;
		//ssl_recv_count += recv_count;
		//Sleep(50);
		
		buf[recv_count] = 0;
		//respond_text += buf;
		memcpy(respond_text+cp_index, buf, recv_count);
		cp_index += recv_count;
		//std::cout<<"cp_index is:"<<cp_index<<std::endl;

		//std::cout<<buf;
		
	} 
	//std::cout<<"cp_index is:"<<cp_index<<",total len is:"<<strlen(respond_text)<<std::endl;
	//respond_text[strlen(respond_text)] = 0;
    return respond_text;
}


Requests::~Requests(){


    if(sk){
        closesocket(sk);
    }
	/*
	ssl_shutdown（sd，2）; 
	SSL_free（ssl）; 
	SSL_CTX_free（ctx）; 

	SSL_shutdown(ssl_handle);
	SSL_free(ssl_handle);
	SSL_CTX_free(sslcontent);

	 */
	
	if(ssl_handle != nullptr){
		SSL_shutdown(ssl_handle);
		SSL_free(ssl_handle);
		ssl_handle = nullptr;
	}

	if(sslContent != nullptr){
		SSL_CTX_free(sslContent);
		sslContent = nullptr;
	}
	if (respond_text != nullptr) {
		free(respond_text);
	}
	
	

    WSACleanup();
}





#pragma endregion
