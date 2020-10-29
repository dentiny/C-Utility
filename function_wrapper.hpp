#ifndef FUNCTION_WRAPPER_HPP__
#define FUNCTION_WRAPPER_HPP__

#include <memory>

class FunctionWrapper
{
private:
	struct ImplBase
	{
		virtual void call() = 0;
		virtual ~ImplBase() noexcept {}
	};

	template<typename Func>
	struct ImplType : ImplBase
	{
		Func _func;
		ImplType(Func && func) : 
			_func { std::move(func) }
			{}

		virtual void call() override { _func(); }
	};

public:
	template<typename Func>
	FunctionWrapper(Func && func) :
		impl { new ImplType<Func>(std::move(func)) }
		{}

	FunctionWrapper(FunctionWrapper && rhs) :
		impl { std::move(rhs.impl) }
		{}

	FunctionWrapper(const FunctionWrapper &) = delete;

	FunctionWrapper & operator=(const FunctionWrapper &) = delete;

	void operator()() { impl->call(); }

private:
	std::unique_ptr<ImplBase> impl;
};

#endif // FUNCTION_WRAPPER_HPP__