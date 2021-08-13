#pragma once
#include <windows.h>
#include <windef.h>
#include <intrin.h>
#include <emmintrin.h>
#include <iostream>
#include <thread>
#include <random>
#include <chrono>



using AtomicWord = intptr_t;

// 必须16字节对齐，否则_InterlockedCompareExchange128会报错
struct alignas(16) AtomicWord2
{
   AtomicWord2& operator=(const volatile AtomicWord2& lhs)
   {
      lo = lhs.lo;
      hi = lhs.hi;
      return *this;
   }

   AtomicWord lo, hi;
};


struct AtomicNode
{
   volatile AtomicWord _next;
   void* data;
};


static inline bool AtomicCompareExchangeStrongExplicit(volatile AtomicWord2* p, AtomicWord2* oldval, AtomicWord2 newval)
{
   return _InterlockedCompareExchange128((volatile LONGLONG*)p, (LONGLONG)newval.hi, (LONGLONG)newval.lo, (LONGLONG*)oldval) !=0;
}

static inline AtomicWord2 AtomicExchangeExplicit(volatile AtomicWord2* p, AtomicWord2 newval)
{
   AtomicWord2 oldval;
   oldval.lo = 0;
   oldval.hi = newval.hi - 1;
   // 没有128位的Exchange函数了，只能用CAS封装一下
   while (!AtomicCompareExchangeStrongExplicit(p, &oldval, newval));
   return oldval;
}


class AtomicQueue
{
   volatile AtomicWord _tail;
   volatile AtomicWord2 _head;
 
public:
   AtomicQueue();
   ~AtomicQueue();
   void Enqueue(AtomicNode* node);
   AtomicNode* Dequeue();
};

static inline AtomicWord AtomicExchangeExplicit(volatile AtomicWord* p, AtomicWord val)
{
   return (AtomicWord)_InterlockedExchange64((volatile LONGLONG*)p, (LONGLONG)val);
}

static inline bool AtomicCompareExchangeStrongExplicit(volatile AtomicWord* p, AtomicWord* oldval, AtomicWord newval)
{
   return _InterlockedCompareExchange64((volatile LONGLONG*)p, (LONGLONG)newval, (LONGLONG)*oldval) != 0;
}


AtomicQueue::AtomicQueue() 
{
   AtomicNode* dummy = new AtomicNode();
   AtomicWord2 w;
   w.lo = (AtomicWord)dummy;
   w.hi = 0;
   dummy->_next = 0;
   _head.hi = w.hi;
   _head.lo = w.lo;
   _tail = (AtomicWord)dummy;
}


AtomicQueue::~AtomicQueue()
{
   AtomicNode* dummy = (AtomicNode*)_head.lo;
   delete dummy;
}

void AtomicQueue::Enqueue(AtomicNode* node)
{
   AtomicNode* prev;
   node->_next = 0;
   prev = (AtomicNode*)AtomicExchangeExplicit(&_tail, (AtomicWord)node);
   prev->_next = (AtomicWord)node;
}

AtomicNode* AtomicQueue::Dequeue() 
{
   AtomicNode* res, *next;
   void* data;
   AtomicWord2 head;
   head.lo = _head.lo;
   head.hi = _head.hi;
   AtomicWord2 newHead;
   do
      {
      res = (AtomicNode*)head.lo;
      next = (AtomicNode*)res->_next;
      if (next == nullptr)
         return nullptr;
      data = next->data;
      newHead.lo = (AtomicWord)next;
      newHead.hi = head.hi + 1;
      }
   while (!AtomicCompareExchangeStrongExplicit(&_head, &head, newHead));

   res->data = data;
   return res;
}


AtomicQueue que;
std::default_random_engine myrand;

void consumer()
{
   for (int i = 0; i < 1000000; i++)
   {
      //std::this_thread::sleep_for(std::chrono::milliseconds(900));
      AtomicNode* data = que.Dequeue();
      //if (data == nullptr)
      //   {
      //      std::cout << " consumer pop a empty number " << std::endl;
      //   }
      //else
      //   {
      //      std::cout << " consumer pop a number " << *((int*)data->data) << std::endl;
      //   }
   }
}

void producer()
{
   for (int i=0;i<1000000;i++)
      {
      //std::this_thread::sleep_for(std::chrono::milliseconds(800));
      AtomicNode* node = new AtomicNode();
      node->data = new int(myrand()%10000);
      que.Enqueue(node);
      //std::cout << " producer push a number: " << *((int*)node->data) << std::endl;
      }
}


int main()
{
   std::vector<std::thread> vecJoin;
   vecJoin.reserve(6);

   auto start = std::chrono::steady_clock::now();

   for (int i = 0; i < 3; i++)
      {
      vecJoin.emplace_back(std::thread (producer));
      vecJoin.emplace_back(std::thread (consumer));
      }

   for (int i = 0; i < 6; i++)
      {
      vecJoin[i].join();
      }


   auto end = std::chrono::steady_clock::now();
   std::chrono::duration<double> elapsed_seconds = std::chrono::duration<double>(end - start);

   std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";


   return 0;
}
