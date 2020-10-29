#ifndef SOCKET_HPP__
#define SOCKET_HPP__

#include <mutex>
#include <netdb.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

class Socket
{
public:
    Socket(const char * hostname, const char * port)
    {
        // initialize host info
        struct addrinfo host_info;
        struct addrinfo * host_info_list;
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;

        // get host information
        int status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if(status != 0) 
        {
            std::cerr << "resolve address error" << std::endl;
            exit(EXIT_FAILURE);
        } 

        // create socket
        fd = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype,
                    host_info_list->ai_protocol);
        if(fd == -1)
        {
            std::cerr << "socket creation error" << std::endl;
            freeaddrinfo(host_info_list);
            exit(EXIT_FAILURE);
        }

        // connect a socket to a server
        status = connect(fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if(status == -1) 
        {
            std::cerr << "connect server error" << std::endl;
            freeaddrinfo(host_info_list);
            exit(EXIT_FAILURE);
        } 

        // free address infor list
        freeaddrinfo(host_info_list);

        // initialize Google Protocal Buffer input/output
        out = new google::protobuf::io::FileOutputStream(fd);
        in = new google::protobuf::io::FileInputStream(fd);
    }

    template<typename T>
    bool sendMsg(const T & message) 
    {
        std::unique_lock<std::mutex> lck(send_mtx);
        { 
            // extra scope: make output go away before out->Flush()
            // We create a new coded stream for each message.
            // Don’t worry, this is fast.
            google::protobuf::io::CodedOutputStream output(out);
            // Write the size.
            const int size = message.ByteSizeLong();
            output.WriteVarint32(size);
            uint8_t * buffer = output.GetDirectBufferForNBytesAndAdvance(size);
            if (buffer != NULL) 
            {
                // Optimization: The message fits in one buffer, so use the faster direct-to-array serialization path.
                message.SerializeWithCachedSizesToArray(buffer);
            } 
            else 
            {
                // Slightly-slower path when the message is multiple buffers.
                message.SerializeWithCachedSizes(&output);
                if (output.HadError()) 
                {
                    return false;
                }
            }
        }
        out->Flush();
        return true;
    }

    template<typename T>
    bool recvMsg(T & message)
    {
        std::unique_lock<std::mutex> lck(recv_mtx);
        google::protobuf::io::CodedInputStream input(in);
        uint32_t size;
        if (!input.ReadVarint32(&size)) 
        {
            return false;
        }
        // Tell the stream not to read beyond that size.
        google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);
        // Parse the message.
        if (!message.MergeFromCodedStream(&input)) 
        {
            return false;
        }
        if (!input.ConsumedEntireMessage()) 
        {
            return false;
        }
        // Release the limit.
        input.PopLimit(limit);
        return true;
    }

    ~Socket() noexcept
    {
        close(fd);
    }

private:
    int fd;
    std::mutex recv_mtx;
    std::mutex send_mtx;
    google::protobuf::io::FileOutputStream * out; // send
    google::protobuf::io::FileInputStream * in; // receive
};

#endif // SOCKET_HPP__
