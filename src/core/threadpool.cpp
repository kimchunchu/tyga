#include "core/threadpool.hpp"

namespace tyga::core {
ThreadPool::ThreadPool(size_t thread_count) {
  for (size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back([this] { worker_loop(); });
  }
};

void ThreadPool::submit(std::function<void()> task) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    tasks_.push(task);
  }
  condition_.notify_one();
}

void ThreadPool::worker_loop() {
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.wait(lock, [this] { return !tasks_.empty(); });
      task = std::move(tasks_.front());
      tasks_.pop();

      task();
    }
  }
}
} // namespace tyga::core