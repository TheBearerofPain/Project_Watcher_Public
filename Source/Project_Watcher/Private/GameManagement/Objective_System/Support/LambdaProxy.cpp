// Project Watcher 2024 & Beyond.


#include "LambdaProxy.h"

/*template <typename ... Args>
TLambdaProxy<Args...> * MakeLambdaProxy(UObject* Outer, TFunction<void(Args...)> CallbackIn)
{
	TLambdaProxy<Args...> * Proxy = NewObject<TLambdaProxy<Args...>>(Outer);

	if (!IsValid(Proxy))
	{
		ensureMsgf(false, TEXT("Failed to Create ProxyLambda"));
		return nullptr;
	}
	
	Proxy->Init(MoveTemp(CallbackIn));
	return Proxy;
}*/
