#pragma once
#include <windows.h>
#include <windef.h>
#include <intrin.h>
#include <emmintrin.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <deque>



class LockQueue
   {
   public:
      void enQueue(void* data)
         {
         std::lock_guard<std::mutex> lock(mtx_);
         list_.push_back(data);
         //cond_.notify_one();
         }

      void* deque()
         {
         void* data = nullptr;
               {
               std::unique_lock<std::mutex> lock(mtx_);
               if (!list_.empty())
                  {
                  data = list_.front();
                  list_.pop_front();
                  }


               }
         return data;
         }

   public:
      std::deque<void*> list_;
      std::mutex mtx_;
      //std::condition_variable cond_;
   };



LockQueue que;
std::default_random_engine myrand;

void consumer()
   {
   for (int i = 0; i < 1000000; i++)
      {
      //std::this_thread::sleep_for(std::chrono::milliseconds(900));
      void* data = que.deque();
      //if (data == nullptr)
      //   {
      //   std::cout << " consumer pop a empty number " << std::endl;
      //   }
      //else
      //   {
      //   std::cout << " consumer pop a number " << *((int*)data) << std::endl;
      //   }
      }
   }

void producer()
   {
   for (int i = 0; i < 1000000; i++)
      {
      //std::this_thread::sleep_for(std::chrono::milliseconds(800));
      void *data = new int(myrand() % 10000);
      que.enQueue(data);
      //std::cout << " producer push a number: " << *((int*)node->data) << std::endl;
      }
   }

int main()
{
   auto start = std::chrono::steady_clock::now();
   std::thread pro0(producer);
   std::thread pro1(producer);
   std::thread pro2(producer);
   std::thread pro3(producer);
   std::thread pro4(producer);
   std::thread pro5(producer);



   std::thread con0(consumer);
   std::thread con1(consumer);
   std::thread con2(consumer);
   std::thread con3(consumer);
   std::thread con4(consumer);
   std::thread con5(consumer);


   pro0.join();
   pro1.join();
   pro2.join();
   pro3.join();
   pro4.join();
   pro5.join();


   con0.join();
   con1.join();
   con2.join();
   con3.join();
   con4.join();
   con5.join();


   auto end = std::chrono::steady_clock::now();
   std::chrono::duration<double> elapsed_seconds = std::chrono::duration<double>(end - start);

   std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";


   return 0;
}
