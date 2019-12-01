#pragma once

#include <chrono>
#include <future>
#include <thread>


namespace Zep
{
template <typename R>
auto is_future_ready(std::future<R> const& f) -> bool
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}
} // namespace Zep
