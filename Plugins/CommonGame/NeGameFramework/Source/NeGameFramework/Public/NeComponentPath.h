// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeComponentPath.generated.h"

/**
 * FNeComponentPath
 * 用来做Component的软引用
 * 类似引擎默认的 `FComponentReference`
 */
USTRUCT(BlueprintType)
struct NEGAMEFRAMEWORK_API FNeComponentPath
{
	GENERATED_BODY()
public:
	FNeComponentPath() {}
	FNeComponentPath(const class UActorComponent* InComponent) { *this = InComponent; }
	FORCEINLINE FNeComponentPath& operator=(const UActorComponent* Other) { SetComponent(Other); return *this; }

	FORCEINLINE bool IsNone()const { return Path == NAME_None; }
	void Reset() { Path = NAME_None; }

	void SetComponent(const UActorComponent* InComponent);
	UActorComponent* ResolveComponent(AActor* OwnerActor) const;
	void SetPath(const FName& InPath) { Path = InPath; }
	FNeComponentPath& operator = (const FName& InPath) { Path = InPath; return *this; }
	bool operator == (const FNeComponentPath& Other) const { return Path == Other.Path; }
	FName GetPath() const { return Path; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Path;

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(EditDefaultsOnly)
	AActor* HostActor = nullptr;
#endif
};

//=============================================================================
/**
 * FNeComponentSocketPath
 *
 * 挂接组件上的Socket
 */
USTRUCT(BlueprintType)
struct NEGAMEFRAMEWORK_API FNeComponentSocketName
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FName SocketName;

#if WITH_EDITORONLY_DATA
public:
	DECLARE_DELEGATE(FOnHostComponentChanged);
	FOnHostComponentChanged OnHostComponentChange;

	UPROPERTY()
	FNeComponentPath HostComponentPath;

	USceneComponent* HostComponent = nullptr;
#endif

#if WITH_EDITOR
public:
	void SetHostComponent(AActor* HostActor, const FNeComponentPath& Path);
#endif
};