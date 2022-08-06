/**
 * @file Mmap.hpp
 * @author JDongChen
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>


#include <string>
#include <cassert>

class MmapFile {
private:
int file_fd_;
char* memory_;
std::size_t offset_;
std::size_t size_;
std::size_t syncPos_;

static const int kInvaild_ = -1;
static const size_t kDefaultSize_ = 1 * 1024 * 1024;

public:
    MmapFile() :
        memory_(nullptr),
        offset_(0),
        size_(0),
        syncPos_(0){
    }
    ~MmapFile(){
        Close();
    }

    bool OpenReadFile(const std::string& file) {
        return OpenReadFile(file.c_str());
    }
    bool OpenReadFile(const char* file) { 
        // 已经开启则先关闭
        Close();
        // 0755->即用户具有读/写/执行权限，组用户和其它用户具有读写权限；
        // 0644->即用户具有读写权限，组用户和其它用户具有只读权限；
        file_fd_ = ::open(file, O_RDONLY) ;
        if (file_fd_ == kInvaild_) {
           char err[128];
            snprintf(err, sizeof err - 1, "OpenReadFile %s failed\n", file);
            perror(err);
            assert (false);
            return false;  
        }
        struct stat st;
        fstat(file_fd_, &st);
        size_ = std::max<size_t>(kDefaultSize_, st.st_size);
        return _MapWriteFile();
    }

    bool OpenWriteFile(const std::string& file, bool append = true) {
        return OpenWriteFile(file.c_str(),append);
    }

    bool OpenWriteFile(const char* file, bool append = true) { 
        // 0755->即用户具有读/写/执行权限，组用户和其它用户具有读写权限；
        // 0644->即用户具有读写权限，组用户和其它用户具有只读权限；
        file_fd_ = ::open(file, O_RDWR|O_CREAT|(append ?O_APPEND:0),0644) ;
        if (file_fd_ == kInvaild_) {
           char err[128];
            snprintf(err, sizeof err - 1, "OpenWriteFile %s failed\n", file);
            perror(err);
            assert (false);
            return false;  
        }

        if(append == true) {
            size_ = kDefaultSize_;
            offset_ = 0;
        } else {
            struct stat st;
            fstat(file_fd_, &st);
            size_ = std::max<size_t>(kDefaultSize_, st.st_size);
            offset_ = st.st_size;
        }
        int ret = ::ftruncate(file_fd_, size_);
        assert(ret == 0);
        return _MapWriteFile();
    }

    void Write(const void* data, size_t len) {
        _AssureSpace(len);

        assert (offset_ + len <= size_);

        ::memcpy(memory_ + offset_, data, len);
        offset_ += len;
        assert(offset_ <= size_);
    }

void Truncate(size_t size) {
    if (size == size_)
        return;

    size_ = size;
    int ret = ::ftruncate(file_fd_, size_);
    assert (ret == 0);

    if (offset_> size_)
        offset_ = size_;

    _MapWriteFile();
}

void Close() {
    if(file_fd_ != kInvaild_) {
        ::munmap(memory_ , size_); 
        ::ftruncate(file_fd_,offset_);
        ::close(file_fd_);

        size_ = 0;
        offset_ = 0;
        syncPos_ = 0;
        file_fd_ = kInvaild_;
        memory_ = nullptr;
    }
}

private:
bool _MapReadFile() {
    if(size_ == 0 || file_fd_ == kInvaild_) {
            assert(false);
            return false;
        }
     int* memory_ = (int*)mmap(0, size_, PROT_READ, MAP_PRIVATE, file_fd_, 0);
}
    bool _MapWriteFile() {
        if(size_ == 0 || file_fd_ == kInvaild_) {
            assert(false);
            return false;
        }
        memory_ = (char*)::mmap(nullptr,size_,PROT_WRITE,MAP_SHARED,file_fd_,0);
    }

    bool _AssureSpace(size_t len) {
     size_t newSize = size_;
        while (offset_ + len > newSize) {
            if (newSize == 0)
                newSize = kDefaultSize_;
            else
                newSize *= 2;
        }
        _ExtendFileSize(newSize);       
    }

    void _ExtendFileSize(size_t size) {
        assert(file_fd_ != kInvaild_);
        if(size > size_) {
            Truncate(size);
        }
    }
};