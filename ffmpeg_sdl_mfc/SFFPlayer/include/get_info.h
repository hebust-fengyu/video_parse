#ifndef _GET_INFO_H
#define _GET_INFO_H
#ifndef  _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // ! _CRT_SECURE_NO_WARNINGS



#define TMP_BUFSIZ 1024 * 1024


#include"Reqeusts.h"

#include<fstream>
#include<regex>
#include<iostream>
#include<deque>
#include<chrono>
#include<ThreadPool.h>
#include<thread>
#include<functional>
#include<exception>
#include"ConvertCode.h"



#pragma region MyRegion



ThreadPool tp(15);


std::deque<int> video_deque;
std::condition_variable video_enque_condition;
std::mutex video_mutex;

std::condition_variable finsh;

std::mutex mutexs;
bool finish_flag;



char* get_online(char** pre, char** suc) {
	char* ret = nullptr;
	if ((*suc = strstr(*pre, "\r\n")) != nullptr) {
		**suc = 0;
		ret = *pre;
		*pre = *suc + 2;
	}
	else if ((*suc = strstr(*pre, "\r")) != nullptr) {
		**suc = 0;
		ret = *pre;
		*pre = *suc + 1;
	}
	else if ((*suc = strstr(*pre, "\n")) != nullptr) {
		**suc = 0;
		ret = *pre;
		*pre = *suc + 1;
	}
	else {
		ret = *pre;
		
	}
	return ret;
}

std::vector<std::string> get_html_lists(const Requests &re) {

	char* raw_char = nullptr;
	while (raw_char == nullptr){
		raw_char = (char*)malloc(sizeof(char*)* TMP_BUFSIZ);
		
	}
	memset(raw_char, 0, re.total_length + 1);
	memcpy(raw_char, re.respond_text, re.response_header_size + re.response_content_size);
	memset(raw_char + re.response_header_size + re.response_content_size, 0, re.total_length - re.response_content_size - re.response_header_size);
	raw_char[re.total_length] = 0;
	strCoding sc; 
	std::string tmp_str;
	sc.UTF_8ToGB2312(tmp_str, raw_char, re.total_length);
	

	memcpy(raw_char, tmp_str.c_str(), strlen(tmp_str.c_str()));
	char* pre = raw_char;
	char* suc = raw_char;
	char* end = raw_char + re.response_header_size + re.response_content_size;
	std::vector<std::string> ret_vs;
	char* ret = nullptr;
	for (; (ret = get_online(&pre, &suc)) != pre;) {
		ret_vs.push_back(ret);
	}
	ret_vs.push_back(ret);
	free(raw_char);
	return ret_vs;
}


void get_ts(const std::string& url, int index) {
	std::cout << index << std::endl;
	Requests re;
	Sleep(100);
	int re_connection_times = 10;
	char* c;
	do {
		c = re.Get(url);
		--re_connection_times;
	} while (re_connection_times);
	
	char name[BUFSIZ] = { 0 };
	sprintf(name, "%d.ts", index);
	

	
	while (1) {
		if (index == 1 || video_deque.size() + 1 == index) {
			video_deque.push_back(index);
			std::fstream f(name , std::ios::ate | std::ios::binary | std::ios::out);
			f.write(c + re.response_header_size, re.response_content_size);
			std::cout << index << ":" << re.response_content_size << std::endl;
			
			video_enque_condition.notify_all();
			break;
		}
		else {
			
			std::unique_lock<std::mutex> lock(video_mutex);
			video_enque_condition.wait(lock, [&]() {return video_deque.size() + 1 == index; });
		}
	}
	
}




void moni_thread_pool(std::deque<std::string>  vs) {

	std::string url;
	int index = 0;


	while (!vs.empty()) {
		//if (tp.runing_threading < tp.size) {
			url = vs.front();
			tp.enqueue(std::bind(get_ts, url, ++index));
			//get_ts(url, ++index);
			vs.pop_front();
			
			
			
			//std::function<void()> l = std::bind(get_ts, url, ++index);
			//tp.enqueue([url, index]() {get_ts(url, index); });
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//}
	}


	//while (tp.size);
	Sleep(3000);
	finish_flag = true;
	finsh.notify_one();
	
}





int send_url(char *url) {

	//获取搜索链接

	//std::string search_url = ""

	//初始url 用于获取重定向url---->m3u8
	//
	//char* url = (char*)"http://blue.ju.jxapi.cn/?yk=a06ce9c0HIMF2lCxUASgtFEEZHCxpLAldIQVdRTwUZAlEFTFIFWhhaClRJVwQHWgFTBAwedBRQZwBje2wXXVlXB0hPXFJBDkVUeVc&title=%E5%A4%8D%E4%BB%87%E8%80%85%E8%81%94%E7%9B%9F4%EF%BC%9A%E7%BB%88%E5%B1%80%E4%B9%8B%E6%88%98_HD720P";
	//char* url = (char*)"http://silver.ju.jxapi.cn/?yk=f0a27b464Izb4jXREET1oRQEAWDU1NXVNUC1EcUQ0OHVoUCAJdX1AaUlddTgIDA1wEB1QATVAMBgRUaQEHUgIDVAtUS1FfU1FJFl1SRQtOVHQB&title=%E7%8B%AE%E5%AD%90%E7%8E%8B2019_TS%E5%9B%BD%E8%AF%AD";
	//char* url = (char*)"http://blue.ju.jxapi.cn/?yk=b80a1454gWiVNLVRVSHllCEkAWXhcaUQdCQAgEEQYWVFICTAZXX0xRX1UXV1ZTCAZRAldLCHF3G2p3ByZJXlYBBE9PCABEWk4BeAk&amp;title=%E9%93%B6%E6%B2%B3%E8%A1%A5%E4%B9%A0%E7%8F%AD_HDTC1080P%E6%B8%85%E6%99%B0%E5%9B%BD%E8%AF%AD%E4%B8%AD%E5%AD%97";
	Requests re;
	char* c = re.Get(url);
	char* buf;
	if (strstr(c, "301") != NULL) {
		char* location = strstr(c, "Location");
		char* beg = strstr(location, "http");
		char* end = strstr(location, "\r\n");
		buf = (char*)malloc(sizeof(char) * (TMP_BUFSIZ));
		memcpy(buf, beg, end - beg);
		buf[end - beg] = 0;
		c = re.Get(buf);
	}

	std::string host;
	//std::regex rg(".*?(http[s]?)%253A%252F%252F([\\.|\\w|\\-]*?\\.com)%252F(\\w*?)%252F(\\w+)%252F(index\\.m3u8).*?");
	
	

	//在重定向的的内容中获取m3u8的视频链接
	std::regex rg(".*?(http[s]?.*?m3u8).*?");
	
	
	char* pm3u8_url = strstr(c, "m3u8");
	{
		std::vector<std::string> vss = get_html_lists(re);
		for (int i = 0; i < vss.size(); ++i) {
			if (vss[i].find("m3u8") != std::string::npos) {
				pm3u8_url = nullptr;
				while (pm3u8_url == nullptr) {
					pm3u8_url = (char*)malloc(sizeof(char) * TMP_BUFSIZ);
				}
				memset(pm3u8_url, 0, vss[i].length() + 1);
				memcpy(pm3u8_url, vss[i].c_str(), (vss[i].length() + 1));
				break;
			}
		}
	}
	{
		std::regex host_rg(".*?%253A%252F%252F([\\.|\\w|\\-]*?\\.com).*?");
		std::match_results<const char*> host_mr;
		std::regex_match(pm3u8_url, host_mr, host_rg);
		host = host_mr[1];
	}
	//std::smatch ms;
	std::string m3u8_url(pm3u8_url);
	//std::cmatch mr;
	std::match_results<const char*> mr;
	
	std::regex_match(pm3u8_url, mr, rg);
	
	
	m3u8_url = std::regex_replace(mr[1].str(), std::regex("%253A"), ":");
	m3u8_url = std::regex_replace(m3u8_url, std::regex("%252F"), "/");
	std::cout << m3u8_url << std::endl;



	//用包含m3u8的链接获取用于获得ts的m3u8的链接
	c = re.Get(m3u8_url);




	{
		std::vector<std::string>&& vss = get_html_lists(re);
		for (int i = 0; i < vss.size(); ++i) {
			
			if (vss[i].find("m3u8") != std::string::npos) {
				
				memcpy(pm3u8_url, vss[i].c_str(), (vss[i].length() + 1));
				//pm3u8_url[vss[i].length]
				break;
			}
		}
	}
	

	rg = std::regex(".*?([/]?.*?m3u8).*?");
	m3u8_url = std::string(pm3u8_url);
	std::regex_match(pm3u8_url, mr, rg);
	m3u8_url = "https://" + host + mr[1].str();

	c = re.Get(m3u8_url);
	char* ts_end = nullptr;
	
	std::deque<std::string> ts_list;

	for (char* ts_beg = strstr(c, "\n/"); ts_beg != nullptr; ts_beg = strstr(ts_beg, "\n/")){
		++ts_beg;
		
		char tmp_c[BUFSIZ] = { 0 };
		ts_end = strstr(ts_beg, "ts") + 2;
		memcpy(tmp_c, ts_beg, ts_end - ts_beg);
		ts_list.push_back("https://"+host+std::string(tmp_c));
	}
	{
		std::vector<std::string> vss = get_html_lists(re);
		for (int i = 0; i < vss.size(); ++i) {
			char buf[BUFSIZ] = { 0 };
			if (vss[i].find("ts") != std::string::npos) {
				memcpy(buf, vss[i].c_str(), strlen(vss[i].c_str()));
				buf[vss[i].length()] = 0;
				ts_list.push_back(buf);
			}
		}
	}


	



	finish_flag = false;
	//moni_thread_pool(ts_list);
	std::thread t(moni_thread_pool, ts_list);
	//std::thread t([&](std::deque<std::string>ts_list) {return moni_thread_pool(ts_list); });
	
	std::unique_lock<std::mutex> lock(mutexs);


	finsh.wait(lock, []() {return finish_flag; });
	
	
	t.join();
	free(buf);
	free(pm3u8_url);
    return 0;

}


#pragma endregion




int entry(std::string &raw_string) {
	
	std::string s = raw_string;
	char* url = (char*)malloc(sizeof(char) * TMP_BUFSIZ);

	///index.php?s=/vod-search-wd-%E9%93%B6%E6%B2%B3%E8%A1%A5%E4%B9%A0%E7%8F%AD.html
	//std::string search_url = "http://www.hifiwu.com/index.php?s=/vod-search-wd-%E4%B8%80%E8%B7%AF%E5%90%91%E8%A5%BF.html";
	std::string search_url = "http://www.hifiwu.com/index.php?s=/vod-search-wd-%s.html";
	//std::string search_url = "http://www.hifiwu.com/index.php?s=/vod-search-wd-%E4%B8%80%E8%B7%AF%E5%90%91%E8%A5%BF.html";
	//std::string search_url = "http://www.vod19.com/index.php?s=/vod-search-wd-%s.html";
	strCoding sc;
	std::string host = "www.hifiwu.com/";
	strcpy(url, search_url.c_str());
	s = sc.UrlUTF8((char*)s.c_str());
	sprintf(url, search_url.c_str(), s.c_str());
	Requests re;
	//http://www.hifiwu.com/index.php?s=/vod-search-wd-%D2%BB%C2%B7%CF%F2%CE%F7.html"	std::basic_string<char,std::char_traits<char>,std::allocator<char> >
	//"%E4%B8%80%E8%B7%AF%E5%90%91%E8%A5%BF"

	//%E4%B8%80%E8%B7%AF%E5%90%91%E8%A5%BF.html
	//std::string send_url("http://www.hifiwu.com/index.php?s=/vod-search-wd-%E4%B8%80%E8%B7%AF%E5%90%91%E8%A5%BF.html");
	char* c = (re.Get(url));
	//std::fstream f("new.txt", ios::out | ios::ate);


	//f.write(re.respond_text, re.total_length);
	//f.close();
	//std::vector<std::string> vss;
	std::vector<std::string> vss = get_html_lists(re);
	std::string tmp_url;
	
	for (int i = 0; i < vss.size(); ++i) {
		if (vss[i].find("index.php?s=/vod-read-id") != std::string::npos) {
			tmp_url = vss[i];
			break;
		}
	}
	
	std::string next_target_url;
	
	{

		url[0] = 0;
		//memset(url, 0, BUFSIZ);
		char tmp[BUFSIZ] = { 0 };

		// /index.php?s=/vod-read-id-21587.html
		//    <a href=\"%s\">	
		sscanf(tmp_url.c_str(), "%[^<]<a href=\"/%[^\">]\">", tmp, url);
		std::cout << tmp << ":" << url << std::endl;
	}
	
	next_target_url = "http://" + host + url;
	

	
	//next_target_url = "http://www.hifiwu.com/index.php?s=/vod-read-id-33345.html";
	c = re.Get(next_target_url);
	vss = get_html_lists(re);
	for (int i = 0; i < vss.size(); ++i) {
		if (vss[i].find("ju.jxapi.cn/?yk=") != std::string::npos) {
			memcpy(url, vss[i].c_str(), vss[i].length());
			url[vss[i].length()] = 0;
			break;
		}
	}

	//  <li class="col-md-1 col-xs-4" data-id="33345-1-1"><a href="http://yellow.ju.jxapi.cn/?yk=7c8123b46enNE5CUVQGgkWQxZAAxZNCkUMH1wXVVgOTlhMC0BQSl5NBVwJGlZWUFsHUQAJFlBRAVFTagtXUlVUVVUHHlBdUwYeHQkGEV4dU3dX&title=%E9%93%B6%E6%B2%B3%E8%A1%A5%E4%B9%A0%E7%8F%AD_HC%E9%AB%98%E6%B8%85" class="btn btn-default btn-block btn-sm ff-text-hidden" target="_blank">HC高清</a></li></ul>
	// %[^\"]\"%[^\"]\"%s
	{
		char tmp[BUFSIZ] = { 0 };
		//char url[BUFSIZ] = { 0 };
		char first[BUFSIZ] = { 0 };
		char second[BUFSIZ] = { 0 };
		char third[BUFSIZ] = { 0 };
		char target[BUFSIZ] = { 0 };
		
		sscanf(url, "%[^<]<%[^<]%[^\"]\"%[^\"]%s", first, second, third, target, tmp);
		next_target_url = target;
	}
	//next_target_url = std::string("http://") + url;
	send_url((char*)next_target_url.c_str());
	std::cout << url;
	//f.close();
	free(url);
	return 0;
}


#endif