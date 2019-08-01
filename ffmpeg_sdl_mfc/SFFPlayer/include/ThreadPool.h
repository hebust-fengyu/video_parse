
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<mutex>
#include<functional>
#include<queue>
#include<condition_variable>
#include<vector>
#include<thread>
#include<future>




class ThreadPool{
    public:
        ThreadPool(int val);

        
        template<typename T>
        void enqueue(T f);
        
        ~ThreadPool();

	public:
		


    //private:
        
		int runing_threading_count;
		int size;


        std::mutex mutexs;

        std::condition_variable conditation;
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
		std::condition_variable task_enque_condition;
		bool task_enque_flag;
        bool stop_flag;
};






template<typename T>
void ThreadPool::enqueue(T f){
    
    {
		while (1) {
			task_enque_flag = false;

			bool flag;
			{
				std::unique_lock<std::mutex> lock(mutexs);

				flag = (runing_threading_count < size);

			}


			if (flag) {
				std::unique_lock<std::mutex> lock(mutexs);
				if (stop_flag)
					throw std::runtime_error("enqueue on stopped ThreadPool");
				tasks.emplace(f);
				break;
			}
			else {
				std::unique_lock<std::mutex> lock(mutexs);
				task_enque_condition.wait(lock, [this]() {return task_enque_flag; });

			}
			
			
		}
        
    }
	conditation.notify_one();

}


ThreadPool::ThreadPool(int val): size(val), stop_flag(false){
	runing_threading_count = 0;
    for(int i = 0; i < val; ++i){
        workers.push_back(std::thread(
            [this](){
                for(;;){
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutexs);
						
                        conditation.wait(lock, [this](){
                            return stop_flag || !tasks.empty();
                        });
                        if(stop_flag) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
					++runing_threading_count;
                    task();
					--runing_threading_count;
					{
						std::unique_lock<std::mutex> lock(mutexs);
						
						task_enque_flag = true;
						task_enque_condition.notify_all();
					}
                }
            }
        ));
    }
}





ThreadPool::~ThreadPool(){
    {

        std::unique_lock<std::mutex> lock(mutexs);
        stop_flag = true;
    }
    conditation.notify_all();
    for(std::thread &t: workers){
        t.join();
    }
}


#endif



