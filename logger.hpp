#ifndef LOGGER_HPP__
#define LOGGER_HPP__

#include <mutex>
#include <string>
#include <fstream>
#include <iostream>

class Logger
{
private:
    static void init()
    {
        logger = new Logger();
    }

    Logger() {}

    template<typename T>
    void logHelper(std::ofstream & of, T msg)
    {
        of << msg << std::flush;
    }

    template<typename T, typename ... Types>
    void logHelper(std::ofstream & of, T msg, Types ... other)
    {
        of << msg;
        logHelper(of, other...);
    }

public:
    static Logger * getInstance()
    {
        std::call_once(Logger::init_once_flag, &Logger::init);
        return logger;
    }

    void deleteLog(std::string filename)
    {
        of.open(filename, std::ofstream::out);
        of.close();
    }

    template<typename ... Types>
    void log(std::string filename, Types ... content)
    {
        std::unique_lock<std::mutex> lck(mtx);
        if(!of.is_open())
        {
            of.open(filename, std::ofstream::out | std::ofstream::app);
        }
        logHelper(of, content...);
        of.close();
    }

private:
    inline static std::once_flag init_once_flag; // for initialization
    inline static Logger * logger = nullptr; // singleton variable
    mutable std::mutex mtx;
    std::ofstream of;
};

#endif