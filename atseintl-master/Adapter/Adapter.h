#pragma once

class Adapter
{
public:
  virtual ~Adapter() = default;

  virtual bool initialize(int argc, char* argv[]) final { return initialize(); }

  // Abstract interface
  //
  virtual void postInitialize() {}
  virtual void preShutdown() {}
  virtual void shutdown() {}

private:
  virtual bool initialize() = 0;
};

