#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "DynamicFieldComputeShader.generated.h"

struct DYNAMICFIELD_API FDynamicFieldComputeShaderDispatchParams
{
	int X;
	int Y;
	int Z;


	int Input[2];
	int Output;


	FDynamicFieldComputeShaderDispatchParams(int x, int y, int z)
		: X(x), Y(y), Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class DYNAMICFIELD_API FDynamicFieldComputeShaderInterface
{
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FDynamicFieldComputeShaderDispatchParams Params,
		TFunction<void(int OutputVal)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FDynamicFieldComputeShaderDispatchParams Params,
		TFunction<void(int OutputVal)> AsyncCallback
	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
			[Params, AsyncCallback](FRHICommandListImmediate& RHICmdList)
			{
				DispatchRenderThread(RHICmdList, Params, AsyncCallback);
			});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FDynamicFieldComputeShaderDispatchParams Params,
		TFunction<void(int OutputVal)> AsyncCallback
	)
	{
		if (IsInRenderingThread())
		{
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, AsyncCallback);
		}
		else
		{
			DispatchGameThread(Params, AsyncCallback);
		}
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicFieldComputeShaderLibrary_AsyncExecutionCompleted, const int,
                                            Value);


UCLASS() // Change the _API to match your project
class DYNAMICFIELD_API UDynamicFieldComputeShaderLibrary_AsyncExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Execute the actual load
	virtual void Activate() override
	{
		// Create a dispatch parameters struct and fill it the input array with our args
		FDynamicFieldComputeShaderDispatchParams Params(1, 1, 1);
		Params.Input[0] = Arg1;
		Params.Input[1] = Arg2;

		// Dispatch the compute shader and wait until it completes
		FDynamicFieldComputeShaderInterface::Dispatch(Params, [this](int OutputVal)
		{
			this->Completed.Broadcast(OutputVal);
		});
	}


	UFUNCTION(BlueprintCallable,
		meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
	static UDynamicFieldComputeShaderLibrary_AsyncExecution* ExecuteFieldComputeShader(
		UObject* WorldContextObject, int Arg1, int Arg2)
	{
		UDynamicFieldComputeShaderLibrary_AsyncExecution* Action = NewObject<
			UDynamicFieldComputeShaderLibrary_AsyncExecution>();
		Action->Arg1 = Arg1;
		Action->Arg2 = Arg2;
		Action->RegisterWithGameInstance(WorldContextObject);

		return Action;
	}

	UPROPERTY(BlueprintAssignable)
	FOnDynamicFieldComputeShaderLibrary_AsyncExecutionCompleted Completed;


	int Arg1;
	int Arg2;
};
