//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

template<typename T>
class RawPtr {
public:
        RawPtr() throw () : ptr_(0)
        {}

        template<typename Y> explicit RawPtr(Y* p) throw () : ptr_(p)
        {}

        ~RawPtr() throw () {}

        RawPtr(const RawPtr& r) throw () : ptr_(r.ptr_)
        {}
        RawPtr& operator=(const RawPtr& r) throw () {
                ptr_ = r.ptr_;
                return *this;
        }
        T& operator*() const throw () { return *ptr_; }
        T* operator->() const throw () { return ptr_; }
        T* get() const throw () { return ptr_; }

        operator bool() const throw () { return ptr_ != 0; }
private:
        T* ptr_;
};

template<typename T>
bool operator==(const RawPtr<T>& a, const RawPtr<T>& b) throw () {
        return a.get() == b.get();
}

template<typename T>
bool operator!=(const RawPtr<T>& a, const RawPtr<T>& b) throw () {
        return a.get() != b.get();
}

template<typename T>
bool operator<(const RawPtr<T>& a, const RawPtr<T>& b) throw () {
        return a.get() < b.get();
}
