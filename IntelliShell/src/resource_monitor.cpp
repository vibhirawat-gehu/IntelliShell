#include "smart_shell.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

ResourceMonitor::ResourceMonitor() : running_(false)
{
    resources_ = ResourceInfo();
}

ResourceMonitor::~ResourceMonitor()
{
    stop();
}

bool ResourceMonitor::initialize()
{
    resources_ = ResourceInfo();
    resources_.last_check = std::chrono::system_clock::now();

    running_ = true;
#ifdef NO_STD_THREAD
    // No threading on Windows - monitor will be updated on-demand
    monitor_thread_ = nullptr;
#else
    monitor_thread_ = std::make_unique<std::thread>(&ResourceMonitor::monitorThread, this);
#endif

    return true;
}

void ResourceMonitor::stop()
{
    running_ = false;
#ifndef NO_STD_THREAD
    if (monitor_thread_ && monitor_thread_->joinable())
    {
        monitor_thread_->join();
    }
#endif
}

#ifndef NO_STD_THREAD
void ResourceMonitor::monitorThread()
{
    while (running_)
    {
        updateResources();
        std::this_thread::sleep_for(std::chrono::seconds(RESOURCE_CHECK_INTERVAL));
    }
}
#endif

ResourceInfo ResourceMonitor::getResources() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(resource_mutex_);
#endif
    return resources_;
}

bool ResourceMonitor::checkWarning() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(resource_mutex_);
#endif

    // Check CPU usage
    if (resources_.cpu_usage > 80.0)
    {
        return true;
    }

    // Check memory usage (warn if using more than 1GB)
    if (resources_.memory_usage > 1024 * 1024 * 1024)
    {
        return true;
    }

    // Check process count (warn if more than 500 processes)
    if (resources_.process_count > 500)
    {
        return true;
    }

    return false;
}

void ResourceMonitor::updateResources()
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(resource_mutex_);
#endif

    resources_.cpu_usage = getCpuUsage();
    resources_.memory_usage = getMemoryUsage();
    resources_.process_count = getProcessCount();
    resources_.last_check = std::chrono::system_clock::now();
}

double ResourceMonitor::getCpuUsage()
{
#ifdef _WIN32
    // Windows doesn't have /proc/stat
    return 0.0;
#else
    std::ifstream file("/proc/stat");
    if (!file.is_open())
    {
        return 0.0;
    }

    std::string cpu_label;
    unsigned long long user, nice, system, idle_time, iowait, irq, softirq, steal;
    if (file >> cpu_label >> user >> nice >> system >> idle_time >> iowait >> irq >> softirq >> steal)
    {
        unsigned long long idle = idle_time + iowait;
        unsigned long long total = user + nice + system + idle + irq + softirq + steal;

        double cpu_percent = 0.0;
        if (prev_total_ > 0)
        {
            unsigned long long total_diff = total - prev_total_;
            unsigned long long idle_diff = idle - prev_idle_;

            if (total_diff > 0)
            {
                cpu_percent = 100.0 * (total_diff - idle_diff) / total_diff;
            }
        }

        prev_idle_ = idle;
        prev_total_ = total;

        return cpu_percent;
    }

    return 0.0;
#endif
}

long ResourceMonitor::getMemoryUsage()
{
#ifdef _WIN32
    // Windows doesn't have /proc/meminfo
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memInfo))
    {
        return 0;
    }
    // Return used memory in MB to avoid overflow
    unsigned long long used_bytes = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    return static_cast<long>(used_bytes / (1024 * 1024));
#else
    std::ifstream file("/proc/meminfo");
    if (!file.is_open())
    {
        return 0;
    }

    long total_mem = 0, free_mem = 0, available_mem = 0;
    std::string line;

    while (std::getline(file, line))
    {
        if (sscanf(line.c_str(), "MemTotal: %ld kB", &total_mem) == 1)
        {
            // Found total memory
        }
        else if (sscanf(line.c_str(), "MemAvailable: %ld kB", &available_mem) == 1)
        {
            // Found available memory
        }
        else if (sscanf(line.c_str(), "MemFree: %ld kB", &free_mem) == 1)
        {
            // Found free memory
        }
    }

    // Return used memory in bytes
    if (total_mem > 0 && available_mem > 0)
    {
        return (total_mem - available_mem) * 1024;
    }
    else if (total_mem > 0 && free_mem > 0)
    {
        return (total_mem - free_mem) * 1024;
    }

    return 0;
#endif
}

int ResourceMonitor::getProcessCount()
{
#ifdef _WIN32
    // Windows implementation
    DWORD processes[1024];
    DWORD needed;
    if (!EnumProcesses(processes, sizeof(processes), &needed))
    {
        return 0;
    }
    return needed / sizeof(DWORD);
#else
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        return 0;
    }

    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(proc_dir)) != nullptr)
    {
        // Check if entry is a process directory (numeric name)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            count++;
        }
    }

    closedir(proc_dir);
    return count;
#endif
}

int ResourceMonitor::getDiskUsage(const std::string &path) const
{
#ifdef _WIN32
    // Windows implementation
    ULARGE_INTEGER freeBytes, totalBytes;
    std::wstring wpath(path.begin(), path.end());
    if (GetDiskFreeSpaceExW(wpath.c_str(), &freeBytes, &totalBytes, NULL))
    {
        ULONGLONG used = totalBytes.QuadPart - freeBytes.QuadPart;
        return static_cast<int>((static_cast<double>(used) / totalBytes.QuadPart) * 100);
    }
    return -1;
#else
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0)
    {
        return -1;
    }

    unsigned long total = stat.f_blocks * stat.f_frsize;
    unsigned long available = stat.f_bavail * stat.f_frsize;
    unsigned long used = total - available;

    return static_cast<int>((static_cast<double>(used) / total) * 100);
#endif
}

int ResourceMonitor::getNetworkConnections() const
{
#ifdef _WIN32
    // Windows doesn't have /proc/net/tcp
    return 0;
#else
    std::ifstream file("/proc/net/tcp");
    if (!file.is_open())
    {
        return 0;
    }

    int count = 0;
    std::string line;

    // Skip header line
    if (std::getline(file, line))
    {
        while (std::getline(file, line))
        {
            count++;
        }
    }

    return count;
#endif
}

void ResourceMonitor::printSummary() const
{
#ifdef NO_STD_THREAD
    // No-op on Windows
#else
    std::lock_guard<std::mutex> lock(resource_mutex_);
#endif

    std::cout << "\n=== System Resource Summary ===" << std::endl;
    std::cout << "CPU Usage: " << std::fixed << std::setprecision(1)
              << resources_.cpu_usage << "%" << std::endl;
    std::cout << "Memory Usage: " << (resources_.memory_usage / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Process Count: " << resources_.process_count << std::endl;
    std::cout << "Disk Usage: " << getDiskUsage(".") << "%" << std::endl;
    std::cout << "Network Connections: " << getNetworkConnections() << std::endl;

    auto time_t = std::chrono::system_clock::to_time_t(resources_.last_check);
    std::cout << "Last Updated: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << std::endl;

    // Show warnings
    if (checkWarning())
    {
        std::cout << "\n\033[31m⚠️  WARNING: High resource usage detected!\033[0m" << std::endl;
        if (resources_.cpu_usage > 80.0)
        {
            std::cout << "  - CPU usage is " << std::fixed << std::setprecision(1)
                      << resources_.cpu_usage << "% (threshold: 80%)" << std::endl;
        }
        if (resources_.memory_usage > 1024 * 1024 * 1024)
        {
            std::cout << "  - Memory usage is " << (resources_.memory_usage / 1024 / 1024)
                      << " MB (threshold: 1024 MB)" << std::endl;
        }
        if (resources_.process_count > 500)
        {
            std::cout << "  - Process count is " << resources_.process_count
                      << " (threshold: 500)" << std::endl;
        }
    }
    std::cout << "===============================\n"
              << std::endl;
}
