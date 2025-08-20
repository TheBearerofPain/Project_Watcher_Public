// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "UObject/Object.h"

// Base Specialization
template<typename T>
struct TFunctionTraits : TFunctionTraits<decltype(&T::operator())> {};

// Specialization for lambdas and functors
template<typename ClassType, typename Ret, typename... Args>
struct TFunctionTraits<Ret(ClassType::*)(Args...) const>
{
	using FReturnType = Ret;
	using FArgsTuple = std::tuple<Args...>;
};

// Specialization for TFunction
template<typename Ret, typename... Args>
struct TFunctionTraits<TFunction<Ret(Args...)>>
{
	using FReturnType = Ret;
	using FArgsTuple = std::tuple<Args...>;
};

/**
 * Specialized Wrapper UObject for allowing easier binding of Lambdas onto UE Delegates
 */
template<typename ... Args>
class PROJECT_WATCHER_API TLambdaProxy final : public UObject
{
	
public:
	
	/* Alias for TFunctions with arbitrary parameters */
	using FCallback = TFunction<void(Args...)>;
	
private:
	
	/* Locally stored TFunction */
	FCallback Callback;

	/**
	 * Forwards Args & Signature into a TLambdaProxy UObject
	 * @tparam TupleType Args
	 * @tparam LambdaType Lambda Signature 
	 * @tparam Indices Arg Indices
	 * @param Outer Parent UObject
	 * @param LambdaIn LambdaIn
	 * @return TLambdaProxy UObject
	 */
	template<typename TupleType, typename LambdaType, std::size_t... Indices>
	static auto* MakeImpl(UObject * Outer, LambdaType&& LambdaIn, std::index_sequence<Indices...>)
	{
		using FProxyType = TLambdaProxy<std::tuple_element_t<Indices, TupleType>...>;
		auto * Proxy = NewObject<FProxyType>(Outer);
		Proxy->Init(TFunction<void(std::tuple_element_t<Indices, TupleType>...)>(std::forward<LambdaType>(LambdaIn)));
		return Proxy;
	}

	/**
	 * Forwards Args & Signature into a TLambdaProxy UObject
	 * @tparam TupleType Lambda Args
	 * @tparam LambdaType Lambda Signature
	 * @param Outer Parent UObject
	 * @param LambdaIn LambdaIn
	 * @return TLambdaProxy UObject with nested Lambda inside
	 */
	template<typename TupleType, typename LambdaType>
	static auto* MakeImpl(UObject* Outer, LambdaType&& LambdaIn)
	{
		return TLambdaProxy::MakeImpl<TupleType>(Outer, std::forward<LambdaType>(LambdaIn), std::make_index_sequence<std::tuple_size_v<TupleType>>{});
	}

public:
	
	/**
	 * Sets Callback TFunction Internally
	 * @param CallbackIn TFunction we want to store
	 */
	void Init(TFunction<void(Args...)> CallbackIn)
	{
		this->Callback = CallbackIn; 
	}
	
	/**
	 * Executes the Stored TFunction
	 * @param ArgsIn Arguments we are passing to the Stored TFunction
	 */
	UFUNCTION()
	void Execute(Args... ArgsIn)
	{
		if (!this->Callback)
		{
			UE_LOG(LogTemp, Warning, TEXT("TLambdaProxy::Execute Callback Invalid"));
			return;
		}
		this->Callback(ArgsIn...);
	}

	/**
	 * Static Factory for Wrapping a Lambda into a UObject for easy Delegate programming
	 * @tparam LambdaType Lambda Signature
	 * @param Outer UObject we use to help manage the newly created ones memory as this isn't a UCLASS
	 * @param LambdaIn Lambda we are wrapping in a newly created UObject
	 * @return Constructed Templated UObject with the nested Lambda within
	 */
	template<typename LambdaType>
	static auto * Make(UObject* Outer, LambdaType&& LambdaIn)
	{
		using FTraits = TFunctionTraits<decltype(&LambdaType::operator())>;
		using ArgsTuple = typename FTraits::FArgsTuple;
		return TLambdaProxy::MakeImpl<ArgsTuple>(Outer, std::forward<LambdaType>(LambdaIn));
	}
};
