#pragma once

template <typename T>
class RawPtr
{
public:
  RawPtr() throw() : ptr_(nullptr) {}

  template <typename Y>
  explicit RawPtr(Y* p) throw()
    : ptr_(p)
  {
  }

  ~RawPtr() throw() {}

  RawPtr(const RawPtr& r) throw() : ptr_(r.ptr_) {}
  RawPtr& operator=(const RawPtr& r) throw()
  {
    ptr_ = r.ptr_;
    return *this;
  }
  T& operator*() const throw() { return *ptr_; }
  T* operator->() const throw() { return ptr_; }
  T* get() const throw() { return ptr_; }

  operator bool() const throw() { return ptr_ != nullptr; }

private:
  T* ptr_;
};

template <typename T>
bool operator==(const RawPtr<T>& a, const RawPtr<T>& b) throw()
{
  return a.get() == b.get();
}

template <typename T>
bool operator!=(const RawPtr<T>& a, const RawPtr<T>& b) throw()
{
  return a.get() != b.get();
}

template <typename T>
bool operator<(const RawPtr<T>& a, const RawPtr<T>& b) throw()
{
  return a.get() < b.get();
}

