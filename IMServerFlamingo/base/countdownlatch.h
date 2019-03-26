
#ifndef BASE_COUNTDOWNLATCH_H_
#define BASE_COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable> 

using namespace std;

	class CountDownLatch
	{
	public:

		explicit CountDownLatch(int count);

		void wait();

		void countDown();

		int getCount() const;

	private:
		mutable std::mutex mutex_;
		std::condition_variable condition_;
		int count_;
	};

#endif //BASE_COUNTDOWNLATCH_H_