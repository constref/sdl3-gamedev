#pragma once

#include <concepts>
#include <memory>
#include <nodehandle.h>

struct SDLState;
class Services;

class Application
{
	NodeHandle root;
protected:
	void setRoot(NodeHandle root) { this->root = root; }

public:
	NodeHandle getRoot() const { return root; }
	virtual bool initialize(Services &services, SDLState &state) = 0;
	virtual void start(Services &services, SDLState &state) = 0;
	virtual void cleanup() {}

};
