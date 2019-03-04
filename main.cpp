#include <dlfcn.h>
#include <string>
#include <iostream>
#include "disp_modules.hpp"

class DynamicLibrary {
private:
	void	*handle;

public:
	DynamicLibrary(const std::string &path, int flags = RTLD_LAZY)
        {
		this->handle = dlopen(path.c_str(), flags);
		if (!this->handle)
			throw std::invalid_argument(dlerror());
	};
	~DynamicLibrary() {
		dlclose(handle);
	};

	template<typename return_type, typename ... args_types>
	return_type	callFunction(const std::string &name, args_types... args)
	{
		return_type (*function)(args_types...);

		function = reinterpret_cast<return_type (*)(args_types...)>(dlsym(this->handle, name.c_str()));
		if (!function)
			throw std::invalid_argument(dlerror());
		return function(args...);
	}
};

template<typename T>
class DLLoader : DynamicLibrary {
private:
	T *instance;

public:
	DLLoader(const std::string &path, int flags = RTLD_LAZY) : DynamicLibrary(path, flags) {
		this->instance = this->callFunction<T>("create");
	};
	~DLLoader() {
		delete this->instance;
	}

	T *getInstance() { return this->instance; };
};

int main(int argc, char **argv)
{
	IDisplayModule	*mod;
	DynamicLibrary	*test;

	if (argc == 1) {
		std::cout << "Usage: " << argv[0] << " [lib.so ...]" << std::endl;
		return EXIT_FAILURE;
	}
	for (int i = 1; i < argc; i++) {
		test = new DynamicLibrary(argv[i]);
		mod = test->callFunction<IDisplayModule *, const std::string &>("create", "test");
		mod->init();
		mod->stop();
		delete mod;
		delete test;
	}
	return EXIT_SUCCESS;
}
